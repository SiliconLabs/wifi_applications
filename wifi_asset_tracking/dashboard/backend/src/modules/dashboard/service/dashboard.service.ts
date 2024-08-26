import * as path from 'path';
import * as fs from 'fs/promises';
import * as ejs from 'ejs';
import { BadRequestException, Injectable } from '@nestjs/common';
import { InjectModel } from '@nestjs/mongoose';
import { Model } from 'mongoose';
import { Heat } from '../../../models/schema/heat/heat.schema';
import { Telemetry, TelemetrySchema } from '../../../models/telemetry.schema';
import DashboardServiceInterface from '../interfaces/dashboard-service.interface';
import { SENSOR_TYPE } from '../dtos/sensor-type.dto';
import { generateXLS } from '../../../utilities/common/helper';
import { Messages } from '../../../utilities';

@Injectable()
export class DashboardService implements DashboardServiceInterface {
  constructor(@InjectModel(Telemetry.name) private telemetrySchema: Model<TelemetrySchema>) {}
  async getTelemetryData(type: string): Promise<Telemetry[] | any> {
    switch (type) {
      case SENSOR_TYPE.temperature:
        let temperatureData = await this.telemetrySchema
          .find(
            {
              type: SENSOR_TYPE.heat,
              'heat.temperature': { $exists: true },
            },
            {
              'heat.timestamp': 1,
              'heat.temperature': 1,
            },
          )
          .sort({ _id: -1 })
          .limit(10)
          .exec();
        temperatureData = temperatureData.reverse();
        return temperatureData.map((result) => ({
          temperature: result?.heat?.temperature?.value,
          unit: result?.heat?.temperature?.unit,
          timestamp: result?.heat?.timestamp,
        }));
      case SENSOR_TYPE.humidity:
        let humidityData = await this.telemetrySchema
          .find(
            {
              type: SENSOR_TYPE.heat,
              'heat.humidity': { $exists: true },
            },
            {
              'heat.timestamp': 1,
              'heat.humidity': 1,
            },
          )
          .sort({ _id: -1 })
          .limit(10)
          .exec();
        humidityData = humidityData.reverse();
        return humidityData.map((result) => ({
          humidity: result?.heat?.humidity,
          timestamp: result?.heat?.timestamp,
        }));

      case SENSOR_TYPE.wifi:
        const wifiData = await this.telemetrySchema
          .find(
            { type: SENSOR_TYPE.wifi },
            {
              'wifi.timestamp': 1,
              'wifi.macid': 1,
              'wifi.ssid': 1,
              'wifi.rssi': 1,
            },
          )
          .exec();
        return wifiData.map((result) => ({
          timestamp: result?.wifi?.timestamp,
          macid: result?.wifi?.macid,
          ssid: result?.wifi?.ssid,
          rssi: result?.wifi?.rssi,
        }));

      case SENSOR_TYPE.gps:
        let gpsData = await this.telemetrySchema
          .find(
            { type: SENSOR_TYPE.gps },
            {
              'gps.timestamp': 1,
              'gps.latitude': 1,
              'gps.longitude': 1,
              'gps.altitude': 1,
              'gps.satellites': 1,
            },
          )
          .sort({ _id: -1 })
          .limit(10)
          .exec();
        gpsData = gpsData.reverse();

        return gpsData.map((result) => ({
          timestamp: result?.gps?.timestamp,
          latitude: result?.gps?.latitude,
          longitude: result?.gps?.longitude,
          altitude: result?.gps?.altitude,
          satellites: result?.gps?.satellites,
        }));

      case SENSOR_TYPE.accelero:
        let acceleroData = await this.telemetrySchema
          .find(
            {
              type: SENSOR_TYPE.imu,
              'accelGyroData.accelero': { $exists: true },
            },
            {
              'accelGyroData.timestamp': 1,
              'accelGyroData.accelero': 1,
            },
          )
          .sort({ _id: -1 })
          .limit(10)
          .exec();
        acceleroData = acceleroData.reverse();

        const updatedAcceleroData = acceleroData.map((result) => ({
          timestamp: result?.accelGyroData?.timestamp,
          accelero: result?.accelGyroData?.accelero,
        }));
        return {
          type: SENSOR_TYPE.accelero,
          acceleroX: updatedAcceleroData.map((item) => ({
            timestamp: item.timestamp,
            x: item.accelero[0],
          })),
          acceleroY: updatedAcceleroData.map((item) => ({
            timestamp: item.timestamp,
            y: item.accelero[1],
          })),
          acceleroZ: updatedAcceleroData.map((item) => ({
            timestamp: item.timestamp,
            z: item.accelero[2],
          })),
        };

      case SENSOR_TYPE.gyro:
        let gyroData = await this.telemetrySchema
          .find(
            {
              type: SENSOR_TYPE.imu,
              'accelGyroData.gyro': { $exists: true },
            },
            {
              'accelGyroData.timestamp': 1,
              'accelGyroData.gyro': 1,
            },
          )
          .sort({ _id: -1 })
          .limit(10)
          .exec();
        gyroData = gyroData.reverse();
        const updatedGyroData = gyroData.map((result) => ({
          timestamp: result?.accelGyroData?.timestamp,
          gyro: result?.accelGyroData?.gyro,
        }));
        return {
          type: SENSOR_TYPE.gyro,
          gyroX: updatedGyroData.map((item) => ({
            timestamp: item.timestamp,
            x: item?.gyro[0],
          })),
          gyroY: updatedGyroData.map((item) => ({
            timestamp: item.timestamp,
            y: item?.gyro[1],
          })),
          gyroZ: updatedGyroData.map((item) => ({
            timestamp: item.timestamp,
            z: item?.gyro[2],
          })),
        };

      default:
        throw new BadRequestException(Messages.InvalidType);
    }
  }

  async generateGPX(): Promise<string> {
    const gpsData = await this.telemetrySchema
      .find(
        { type: 'gps' },
        {
          'gps.latitude': 1,
          'gps.longitude': 1,
          'gps.altitude': 1,
        },
      )
      .exec();
    if (!gpsData.length) {
      throw new BadRequestException(Messages.NoGpsDataFound);
    }

    const updatedGpsData = gpsData.map((data) => {
      return {
        lat: data.gps.latitude,
        lon: data.gps.longitude,
        ele: data.gps.altitude,
      };
    });

    // Read the EJS template file
    const headerTemplate = await fs.readFile('src/templates/gpx-header.ejs', 'utf8');
    const gpxHeader = ejs.render(headerTemplate, { currentTime: new Date() });

    const footerTemplate = await fs.readFile('src/templates/gpx-footer.ejs', 'utf8');
    const gpxFooter = ejs.render(footerTemplate);

    const bodyTemplate = await fs.readFile('src/templates/gpx-body.ejs', 'utf8');

    const gpxBody = updatedGpsData
      .map((point) =>
        ejs.render(bodyTemplate, {
          lat: point.lat,
          lon: point.lon,
          ele: point.ele,
        }),
      )
      .join('');

    const gpxContent = gpxHeader + gpxBody + gpxFooter;

    const filePath = path.join(__dirname, 'GPX.gpx');

    await fs.writeFile(filePath, gpxContent, 'utf-8');

    return filePath;
  }

  async generateCSV(): Promise<ArrayBuffer> {
    const telemetryObj = { gps: [], temperature: [], humidity: [], wifi: [], accelGyroData: [] };
    const telemetryData = await this.telemetrySchema.find().exec();
    if (!telemetryData.length) {
      throw new BadRequestException(Messages.NoTelemetryDataFound);
    }
    telemetryObj.gps = telemetryData
      .filter((sensorData) => sensorData.type == 'gps')
      .map((telemetry) => {
        return {
          latitude: telemetry?.gps?.latitude,
          longitude: telemetry?.gps?.longitude,
          altitude: telemetry?.gps?.altitude,
          satellites: telemetry?.gps?.satellites,
          timestamp: telemetry?.gps?.timestamp.toISOString(),
        };
      });
    telemetryObj.temperature = telemetryData
      .filter((sensorData) => sensorData.type == SENSOR_TYPE.heat)
      .map((telemetry) => {
        return {
          temperature: telemetry.heat?.temperature?.value,
          unit: telemetry.heat?.temperature?.unit,
          timestamp: telemetry.heat?.timestamp.toISOString(),
        };
      });
    telemetryObj.humidity = telemetryData
      .filter((sensorData) => sensorData.type == SENSOR_TYPE.heat)
      .map((telemetry) => {
        return {
          humidity: telemetry.heat?.humidity,
          timestamp: telemetry.heat?.timestamp.toISOString(),
        };
      });
    telemetryObj.wifi = telemetryData
      .filter((sensorData) => sensorData.type == SENSOR_TYPE.wifi)
      .map((telemetry) => {
        return {
          macid: telemetry.wifi?.macid,
          ssid: telemetry.wifi?.ssid,
          rssi: telemetry.wifi?.rssi,
          timestamp: telemetry.wifi?.timestamp.toISOString(),
        };
      });
    telemetryObj.accelGyroData = telemetryData
      .filter((sensorData) => sensorData.type == SENSOR_TYPE.imu)
      .map((telemetry) => {
        return {
          accelero: (telemetry.accelGyroData?.accelero).join(),
          gyro: (telemetry.accelGyroData?.gyro).join(),
          timeStamp: telemetry.accelGyroData?.timestamp.toISOString(),
        };
      });
    return generateXLS(telemetryObj);
  }

  async addTelemetryData(heatData: Heat): Promise<Telemetry> {
    const newHeatData = new this.telemetrySchema(heatData);
    return newHeatData.save();
  }
}

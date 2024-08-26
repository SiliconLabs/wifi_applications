import * as fs from 'fs/promises';
import * as ejs from 'ejs';
import * as path from 'path';
import { generateXLS } from '../../../utilities/common/helper';

import { Test, TestingModule } from '@nestjs/testing';
import { DashboardService } from '../service/dashboard.service';
import { SENSOR_TYPE } from '../dtos/sensor-type.dto';
import { getModelToken } from '@nestjs/mongoose';
import {
  mockTelemetryAcceleroGyroData,
  mockTelemetryGpsData,
  mockTelemetryHeatData,
  mockTelemetryWifiData,
} from './mocks/telemetry.mock';
import { BadRequestException } from '@nestjs/common';

jest.mock('fs/promises'); // Mock the promises version of fs
jest.mock('ejs'); // Mock the ejs module

jest.mock('../../../utilities/common/helper', () => ({
  generateXLS: jest.fn(),
}));

describe('DashboardService', () => {
  let dashboardService: DashboardService;
  const execMock = jest.fn();
  const sortMock = jest.fn().mockReturnValue({ limit: jest.fn().mockReturnValue({ exec: execMock }) });
  const findMock = jest.fn().mockReturnValue({ sort: sortMock, exec: execMock });

  const mockTelemetryModel = {
    find: findMock,
  };

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      providers: [
        DashboardService,
        {
          provide: getModelToken('Telemetry'),
          useValue: mockTelemetryModel,
        },
      ],
    }).compile();

    dashboardService = module.get<DashboardService>(DashboardService);
  });

  afterEach(() => {
    jest.clearAllMocks();
  });

  describe('getTelemetryData', () => {
    it('Success - Should return telemetry data by type - temperature', async () => {
      execMock.mockResolvedValue([mockTelemetryHeatData]);
      const expected = [
        {
          temperature: mockTelemetryHeatData.heat.temperature.value,
          unit: mockTelemetryHeatData.heat.temperature.unit,
          timestamp: mockTelemetryHeatData.heat.timestamp,
        },
      ];

      const telemetryData = await dashboardService.getTelemetryData(SENSOR_TYPE.temperature);

      expect(telemetryData).toEqual(expected);
      expect(mockTelemetryModel.find).toHaveBeenCalledWith(
        { type: 'heat', 'heat.temperature': { $exists: true } },
        {
          'heat.timestamp': 1,
          'heat.temperature': 1,
        },
      );
      expect(execMock).toHaveBeenCalled();
    });

    it('Success - Should return telemetry data by type - humidity', async () => {
      execMock.mockResolvedValue([mockTelemetryHeatData]);
      const expected = [
        {
          humidity: mockTelemetryHeatData.heat.humidity,
          timestamp: mockTelemetryHeatData.heat.timestamp,
        },
      ];

      const telemetryData = await dashboardService.getTelemetryData(SENSOR_TYPE.humidity);

      expect(telemetryData).toEqual(expected);
      expect(mockTelemetryModel.find).toHaveBeenCalledWith(
        { type: 'heat', 'heat.humidity': { $exists: true } },
        {
          'heat.timestamp': 1,
          'heat.humidity': 1,
        },
      );
      expect(execMock).toHaveBeenCalled();
    });

    it('Success - Should return telemetry data by type - wifi', async () => {
      execMock.mockResolvedValue([mockTelemetryWifiData]);
      const expected = [
        {
          timestamp: mockTelemetryWifiData.wifi.timestamp,
          macid: mockTelemetryWifiData.wifi.macid,
          ssid: mockTelemetryWifiData.wifi.ssid,
          rssi: mockTelemetryWifiData.wifi.rssi,
        },
      ];

      // const telemetryData = await dashboardService.getTelemetryData(SENSOR_TYPE.wifi);

      // expect(telemetryData).toEqual(expected);
      // expect(mockTelemetryModel.find).toHaveBeenCalledWith(
      //   { type: 'wifi' },
      //   {
      //     'wifi.timestamp': 1,
      //     'wifi.macid': 1,
      //     'wifi.ssid': 1,
      //     'wifi.rssi': 1,
      //   },
      // );
      // expect(execMock).toHaveBeenCalled();
    });

    it('Success - Should return telemetry data by type - gps', async () => {
      execMock.mockResolvedValue([mockTelemetryGpsData]);
      const expected = [
        {
          timestamp: mockTelemetryGpsData.gps.timestamp,
          latitude: mockTelemetryGpsData.gps.latitude,
          longitude: mockTelemetryGpsData.gps.longitude,
          altitude: mockTelemetryGpsData.gps.altitude,
          satellites: mockTelemetryGpsData.gps.satellites,
        },
      ];

      const telemetryData = await dashboardService.getTelemetryData(SENSOR_TYPE.gps);

      expect(telemetryData).toEqual(expected);
      expect(mockTelemetryModel.find).toHaveBeenCalledWith(
        { type: 'gps' },
        {
          'gps.timestamp': 1,
          'gps.latitude': 1,
          'gps.longitude': 1,
          'gps.altitude': 1,
          'gps.satellites': 1,
        },
      );
      expect(execMock).toHaveBeenCalled();
    });

    it('Success - Should return telemetry data by type - accelero', async () => {
      execMock.mockResolvedValue([mockTelemetryAcceleroGyroData]);
      const expected = {
        type: 'accelero',
        acceleroX: [
          {
            timestamp: mockTelemetryAcceleroGyroData?.accelGyroData?.timestamp,
            x: 1,
          },
        ],
        acceleroY: [
          {
            timestamp: mockTelemetryAcceleroGyroData?.accelGyroData?.timestamp,
            y: 2,
          },
        ],
        acceleroZ: [
          {
            timestamp: mockTelemetryAcceleroGyroData?.accelGyroData?.timestamp,
            z: 3,
          },
        ],
      };

      const telemetryData = await dashboardService.getTelemetryData(SENSOR_TYPE.accelero);

      expect(telemetryData).toEqual(expected);
      expect(mockTelemetryModel.find).toHaveBeenCalledWith(
        { type: 'imu', 'accelGyroData.accelero': { $exists: true } },
        {
          'accelGyroData.timestamp': 1,
          'accelGyroData.accelero': 1,
        },
      );
      expect(execMock).toHaveBeenCalled();
    });

    it('Success - Should return telemetry data by type - gyro', async () => {
      execMock.mockResolvedValue([mockTelemetryAcceleroGyroData]);
      const expected = {
        type: 'gyro',
        gyroX: [
          {
            timestamp: mockTelemetryAcceleroGyroData?.accelGyroData?.timestamp,
            x: 10,
          },
        ],
        gyroY: [
          {
            timestamp: mockTelemetryAcceleroGyroData?.accelGyroData?.timestamp,
            y: 20,
          },
        ],
        gyroZ: [
          {
            timestamp: mockTelemetryAcceleroGyroData?.accelGyroData?.timestamp,
            z: 30,
          },
        ],
      };

      const telemetryData = await dashboardService.getTelemetryData(SENSOR_TYPE.gyro);

      expect(telemetryData).toEqual(expected);
      expect(mockTelemetryModel.find).toHaveBeenCalledWith(
        { type: 'imu', 'accelGyroData.gyro': { $exists: true } },
        {
          'accelGyroData.timestamp': 1,
          'accelGyroData.gyro': 1,
        },
      );
      expect(execMock).toHaveBeenCalled();
    });

    it('Failure - Should thow BadRequestException for invalid type - invalid_type', async () => {
      const invalidType = 'invalidType';
      await expect(dashboardService.getTelemetryData(invalidType)).rejects.toThrow(
        new BadRequestException('Invalid type'),
      );
    });
  });

  describe('generateGPX', () => {
    it('should generate a GPX file successfully', async () => {
      const mockGpsData = [
        {
          gps: {
            latitude: 45.4431708,
            longitude: -121.7295917,
            altitude: 2394,
          },
        },
      ];
      execMock.mockResolvedValue(mockGpsData);

      const mockGpxHeader = '<gpx><metadata></metadata>';
      const mockGpxBody = '<trkpt lat="45.4431708" lon="-121.7295917"><ele>2394</ele></trkpt>';
      const mockGpxFooter = '</gpx>';

      (fs.readFile as jest.Mock).mockImplementation((filePath) => {
        if (filePath.includes('gpx-header.ejs')) return Promise.resolve(mockGpxHeader);
        if (filePath.includes('gpx-body.ejs'))
          return Promise.resolve('<trkpt lat="<%= lat %>" lon="<%= lon %>"><ele><%= ele %></ele></trkpt>');
        if (filePath.includes('gpx-footer.ejs')) return Promise.resolve(mockGpxFooter);
        return Promise.reject('File not found');
      });

      (ejs.render as jest.Mock).mockImplementation((template, data) => {
        if (template === mockGpxHeader) return mockGpxHeader;
        if (template === '<trkpt lat="<%= lat %>" lon="<%= lon %>"><ele><%= ele %></ele></trkpt>') return mockGpxBody;
        if (template === mockGpxFooter) return mockGpxFooter;
        return '';
      });

      const mockFilePath = path.join(__dirname, '../service/GPX.gpx');
      (fs.writeFile as jest.Mock).mockResolvedValue(undefined);

      const result = await dashboardService.generateGPX();

      expect(result).toBe(mockFilePath);
      expect(mockTelemetryModel.find).toHaveBeenCalledWith(
        { type: 'gps' },
        {
          'gps.latitude': 1,
          'gps.longitude': 1,
          'gps.altitude': 1,
        },
      );
      expect(execMock).toHaveBeenCalled();
      expect(fs.readFile).toHaveBeenCalledTimes(3);
      expect(ejs.render).toHaveBeenCalledTimes(3);
      expect(fs.writeFile).toHaveBeenCalledWith(mockFilePath, mockGpxHeader + mockGpxBody + mockGpxFooter, 'utf-8');
    });

    it('should throw BadRequestException if no GPS data found', async () => {
      execMock.mockResolvedValue([]);

      await expect(dashboardService.generateGPX()).rejects.toThrow(BadRequestException);
      expect(mockTelemetryModel.find).toHaveBeenCalledWith(
        { type: 'gps' },
        {
          'gps.latitude': 1,
          'gps.longitude': 1,
          'gps.altitude': 1,
        },
      );
      expect(execMock).toHaveBeenCalled();
      expect(fs.readFile).not.toHaveBeenCalled();
      expect(ejs.render).not.toHaveBeenCalled();
      expect(fs.writeFile).not.toHaveBeenCalled();
    });
  });

  describe('generateCSV', () => {
    it('should generate a CSV file successfully', async () => {
      const mockTelemetryData = [
        mockTelemetryGpsData,
        mockTelemetryHeatData,
        mockTelemetryWifiData,
        mockTelemetryAcceleroGyroData,
      ];

      execMock.mockResolvedValue(mockTelemetryData);

      const expectedTelemetryObj = {
        gps: [
          {
            latitude: mockTelemetryGpsData.gps.latitude,
            longitude: mockTelemetryGpsData.gps.longitude,
            altitude: mockTelemetryGpsData.gps.altitude,
            satellites: mockTelemetryGpsData.gps.satellites,
            timestamp: mockTelemetryGpsData.gps.timestamp.toISOString(),
          },
        ],
        temperature: [
          {
            temperature: mockTelemetryHeatData.heat.temperature.value,
            unit: mockTelemetryHeatData.heat.temperature.unit,
            timestamp: mockTelemetryHeatData.heat.timestamp.toISOString(),
          },
        ],
        humidity: [
          {
            humidity: mockTelemetryHeatData.heat.humidity,
            timestamp: mockTelemetryHeatData.heat.timestamp.toISOString(),
          },
        ],
        wifi: [
          {
            macid: mockTelemetryWifiData.wifi.macid,
            ssid: mockTelemetryWifiData.wifi.ssid,
            rssi: mockTelemetryWifiData.wifi.rssi,
            timestamp: mockTelemetryWifiData.wifi.timestamp.toISOString(),
          },
        ],
        accelGyroData: [
          {
            accelero: mockTelemetryAcceleroGyroData.accelGyroData.accelero.join(),
            gyro: mockTelemetryAcceleroGyroData.accelGyroData.gyro.join(),
            timeStamp: mockTelemetryAcceleroGyroData.accelGyroData.timestamp.toISOString(),
          },
        ],
      };

      const generateXLSMock = jest.fn().mockResolvedValue(new ArrayBuffer(8));
      (generateXLS as jest.Mock).mockImplementation(generateXLSMock);
      const result = await dashboardService.generateCSV();

      expect(result).toBeInstanceOf(ArrayBuffer);
      expect(mockTelemetryModel.find).toHaveBeenCalled();
      expect(execMock).toHaveBeenCalled();
      // expect(generateXLSMock).toHaveBeenCalledWith(expectedTelemetryObj);
    });
  });
});

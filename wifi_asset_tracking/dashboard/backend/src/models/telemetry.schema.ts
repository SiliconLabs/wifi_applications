import { Prop, Schema, SchemaFactory } from '@nestjs/mongoose';
import { Document } from 'mongoose';
import { Heat } from './schema/heat/heat.schema';
import { Wifi } from './schema/wifi/wifi.schema';
import { Gps } from './schema/gps/gps.schema';
import { AccelGyroData } from './schema/AccelGyroData/accel-gyro-data.schema';
import { HydratedDocument } from 'mongoose';

export enum SENSOR_TYPE {
  heat = 'heat',
  gps = 'gps',
  wifi = 'wifi',
  imu = 'imu',
  temperature = 'temperature',
  humidity = 'humidity',
  accelGyroData = 'accelGyroData',
}

export type TelemetrySchema = Telemetry & Document;
// export type TelemetryDocument = HydratedDocument<Telemetry>;

@Schema()
export class Telemetry {
  @Prop({ type: Wifi })
  wifi: Wifi;

  @Prop({ type: Gps })
  gps: Gps;

  @Prop({ type: Heat })
  heat: Heat;

  @Prop({ type: AccelGyroData })
  accelGyroData: AccelGyroData;

  @Prop({ type: Date, default: Date })
  createdAt: Date;

  @Prop({ type: Date, default: Date })
  updatedAt: Date;

  @Prop({ type: String, enum: SENSOR_TYPE })
  type: string;
}

export const TelemetrySchema = SchemaFactory.createForClass(Telemetry);

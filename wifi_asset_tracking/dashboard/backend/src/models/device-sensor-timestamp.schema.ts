import { Prop, Schema, SchemaFactory } from '@nestjs/mongoose';
import { Document } from 'mongoose';
import { Status } from './schema/sensorStatus/sensor-status.schema';
import { Timestamp } from './schema/sensorTimestamp/sensor-timestamp.schema';

export type SensorTimestampSchema = SensorTimestamp & Document;

@Schema()
export class SensorTimestamp {
  @Prop({ type: String })
  deviceId: string;

  @Prop({ type: Timestamp })
  timestamp: Timestamp;

  @Prop({ type: Status })
  status: Status;

  @Prop({ type: Date, default: Date.now() })
  createdAt: Date;

  @Prop({ type: Date })
  updatedAt: Date;
}

export const SensorTimestampSchema = SchemaFactory.createForClass(SensorTimestamp);

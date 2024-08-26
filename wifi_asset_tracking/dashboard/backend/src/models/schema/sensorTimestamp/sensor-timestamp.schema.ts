import { Prop, Schema, SchemaFactory } from '@nestjs/mongoose';

export type TimestampSchema = Timestamp & Document;

@Schema()
export class Timestamp {
  @Prop({ type: Date })
  heat: Date;

  @Prop({ type: Date })
  wifi: Date;

  @Prop({ type: Date })
  gps: Date;

  @Prop({ type: Date })
  imu: Date;
}

export const TimestampSchema = SchemaFactory.createForClass(Timestamp);

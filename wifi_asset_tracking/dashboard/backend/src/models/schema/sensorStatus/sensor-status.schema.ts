import { Prop, Schema, SchemaFactory } from '@nestjs/mongoose';

export type StatusSchema = Status & Document;

@Schema()
export class Status {
  @Prop({ required: true })
  heat: Boolean;

  @Prop({ required: true })
  wifi: Boolean;

  @Prop({ required: true })
  imu: Boolean;

  @Prop({ required: true })
  gps: Boolean;


  @Prop({ required: true })
  device: Boolean;
}

export const StatusSchema = SchemaFactory.createForClass(Status);

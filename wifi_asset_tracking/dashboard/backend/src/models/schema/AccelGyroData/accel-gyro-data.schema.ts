import { Prop, Schema, SchemaFactory } from '@nestjs/mongoose';

export type AccelGyroDataSchema = AccelGyroData & Document;

@Schema()
export class AccelGyroData {
  @Prop({ required: true })
  timestamp: Date;

  @Prop({ required: true })
  accelero: number[];

  @Prop({ required: true })
  gyro: number[];
}

export const AccelGyroDataSchema = SchemaFactory.createForClass(AccelGyroData);

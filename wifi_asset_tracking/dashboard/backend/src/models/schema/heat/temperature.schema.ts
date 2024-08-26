import { Prop, Schema, SchemaFactory } from '@nestjs/mongoose';

export type TemperatureSchema = Temperature & Document;

@Schema()
export class Temperature {
  @Prop({ required: true })
  value: number;

  @Prop({ required: true })
  unit: string;
}

export const TemperatureSchema = SchemaFactory.createForClass(Temperature);

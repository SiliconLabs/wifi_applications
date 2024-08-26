import { Prop, Schema, SchemaFactory } from '@nestjs/mongoose';
import { Temperature } from './temperature.schema';

export type HeatSchema = Heat & Document;

@Schema()
export class Heat {
  @Prop({ required: true })
  timestamp: Date;

  @Prop({ type: Temperature, required: true })
  temperature: Temperature;

  @Prop({ required: true })
  humidity: number;
}

export const HeatSchema = SchemaFactory.createForClass(Heat);

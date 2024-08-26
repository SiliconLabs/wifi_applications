import { Prop, Schema, SchemaFactory } from '@nestjs/mongoose';

export type GpsSchema = Gps & Document;

@Schema()
export class Gps {
  @Prop({ required: true })
  timestamp: Date;

  @Prop({ required: true })
  latitude: string;

  @Prop({ required: true })
  longitude: string;

  @Prop({ required: true })
  altitude: string;

  @Prop({ required: true })
  satellites: number;
}

export const GpsSchema = SchemaFactory.createForClass(Gps);

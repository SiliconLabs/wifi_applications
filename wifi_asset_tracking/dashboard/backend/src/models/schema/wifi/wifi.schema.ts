import { Prop, Schema, SchemaFactory } from '@nestjs/mongoose';

export type WifiSchema = Wifi & Document;

@Schema()
export class Wifi {
  @Prop({ required: true })
  timestamp: Date;

  @Prop({ required: true })
  macid: string;

  @Prop({ required: true })
  ssid: string;

  @Prop({ required: true })
  rssi: number;
}

export const WifiSchema = SchemaFactory.createForClass(Wifi);

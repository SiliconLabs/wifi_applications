import { Prop, Schema, SchemaFactory } from '@nestjs/mongoose';
import { Document } from 'mongoose';

export type KeepAliveSchema = KeepAlive & Document;

@Schema()
export class KeepAlive {
  @Prop({ type: String })
  deviceId: string;

  @Prop()
  interval: number[];

  @Prop({ type: Date, default: Date.now() })
  createdAt: Date;

  @Prop({ type: Date })
  updatedAt: Date;
}

export const KeepAliveSchema = SchemaFactory.createForClass(KeepAlive);

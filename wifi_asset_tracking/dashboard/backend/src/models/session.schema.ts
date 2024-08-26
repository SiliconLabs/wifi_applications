import { Prop, Schema, SchemaFactory } from '@nestjs/mongoose';
import { Document } from 'mongoose';

export type SessionSchema = Session & Document;

@Schema()
export class Session {
  @Prop({ type: String, required: true, unique: true })
  token: string;

  @Prop({ type: String, required: true })
  userEmail: string;

  @Prop({ type: Date, default: Date.now() })
  createdAt: Date;

  @Prop({ type: Date })
  expiresAt: Date;
}

export const SessionSchema = SchemaFactory.createForClass(Session);

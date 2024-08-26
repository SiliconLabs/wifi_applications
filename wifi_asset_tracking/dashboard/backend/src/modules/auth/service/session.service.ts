import { Injectable } from '@nestjs/common';
import { InjectModel } from '@nestjs/mongoose';
import { Model } from 'mongoose';
import { v4 as uuidv4 } from 'uuid';

import { Session, SessionSchema } from '../../../models/session.schema';
import SessionServiceInterface from '../Interfaces/session-service.interface';

@Injectable()
export class SessionService implements SessionServiceInterface {
  constructor(@InjectModel(Session.name) private sessionSchema: Model<SessionSchema>) {}

  /**
   * Save user session to database
   * @param {string} userEmail - user email
   * @returns {string} - token
   */
  async createSession(userEmail: string): Promise<Date> {
    const token = uuidv4();
    const createdAt = new Date();

    const expiresAt = new Date();
    expiresAt.setHours(expiresAt.getHours() + 1); // 1 hour expiry

    const session = new this.sessionSchema({ token, userEmail, expiresAt, createdAt });
    await session.save();

    return expiresAt;
  }

  /**
   * Check if users session is active
   * @param {string} userEmail - user email
   * @returns {boolean} - isActive or not
   */
  async validateSession(userEmail: string): Promise<boolean> {
    const session = await this.sessionSchema.findOne({ userEmail, expiresAt: { $gt: new Date() } });
    return !!session;
  }

  /**
   * Check if there are any active sessions
   * @returns {boolean} - is active or not
   */
  async checkActiveSession(): Promise<boolean> {
    const session = await this.sessionSchema.findOne({ expiresAt: { $gt: new Date() } });
    return !!session;
  }

  /**
   * Deletes user session from database
   * @param {string} userEmail - user email
   * @returns {void}
   */
  async deleteSession(): Promise<void> {
    await this.sessionSchema.deleteMany({});
  }
}

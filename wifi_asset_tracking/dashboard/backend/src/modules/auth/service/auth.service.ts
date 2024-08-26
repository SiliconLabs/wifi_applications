import { BadRequestException, Injectable, Logger, UnauthorizedException } from '@nestjs/common';
import { JwtService } from '@nestjs/jwt';
import { AzureMsalService } from '../../../utilities/azure/msal-auth.service';
import { ITokenInfo } from '../Interfaces/token-info.interface';
import AuthServiceInterface from '../Interfaces/auth-service.interface';
import { InjectModel } from '@nestjs/mongoose';
import { Telemetry, TelemetrySchema } from '../../../models/telemetry.schema';
import { Model } from 'mongoose';

import * as jwkToPem from 'jwk-to-pem';
import { UserData } from '../Interfaces/custom-request.interface';
import { SessionService } from './session.service';
import { KeepAlive, KeepAliveSchema } from '../../../models/device-keep-alive.schema';
import { Messages } from '../../../utilities';

@Injectable()
export class AuthService implements AuthServiceInterface {
  private readonly logger = new Logger();

  constructor(
    @InjectModel(Telemetry.name) private telemetrySchema: Model<TelemetrySchema>,
    @InjectModel(KeepAlive.name) private keepAliveSchema: Model<KeepAliveSchema>,
    private readonly azureMsalService: AzureMsalService,
    private readonly jwtService: JwtService,
    private readonly sessionService: SessionService,
  ) {}

  /**
   * Validate jwt token
   * @param {string} token - jwt token
   * @returns {ITokenInfo} - token info once user is validated
   */
  validateToken(token: string): ITokenInfo {
    try {
      return this.jwtService.verify(token);
    } catch (error) {
      throw new UnauthorizedException(error.message);
    }
  }

  /**
   * Validate id token
   * @param {string} token - jwt token
   * @returns {UserData} - user info once user is validated
   */
  async validateIdToken(token: string): Promise<UserData> {
    try {
      const jwks = await this.azureMsalService.getMicrosoftPublicKeys();
      const decodedHeader = this.jwtService.decode(token, { complete: true })?.header;

      const jwk = jwks.find((key) => key.kid === decodedHeader.kid);
      const publicKey = jwkToPem(jwk);

      const verifiedToken = this.jwtService.verify(token, { algorithms: [decodedHeader.alg], secret: publicKey });
      if (verifiedToken) {
        return { username: verifiedToken.preferred_username, exp: verifiedToken.exp, name: verifiedToken.name };
      }

      if (!jwk) {
        throw new UnauthorizedException(Messages.PublicKeyNotFound);
      }
    } catch (error) {
      throw new UnauthorizedException(error.message);
    }
  }

  /**
   * Generate Jwt and store session in DB
   * @param {ITokenInfo} userData - user details
   * @returns {string} - jwt string
   */
  async generateSession(userData: ITokenInfo): Promise<string> {
    const isActiveSession = await this.sessionService.checkActiveSession();
    this.logger.log('isActiveSession', isActiveSession);
    if (isActiveSession) {
      throw new BadRequestException(Messages.OneSessionActive);
    }

    //deleting old session
    await this.sessionService.deleteSession();

    //creating new session
    const sessionExpiry = await this.sessionService.createSession(userData.username);
    const expiresAtTimestampInSeconds = Math.floor(sessionExpiry.getTime() / 1000);

    //deleting previous session data
    await this.telemetrySchema.deleteMany({});

    return this.jwtService.signAsync({
      username: userData.username,
      name: userData.name,
      sessionTimer: expiresAtTimestampInSeconds,
    });
  }

  /**
   * Logout user
   *
   */
  async logout(): Promise<void> {
    try {
      await this.sessionService.deleteSession();
      await this.keepAliveSchema.deleteMany({});
      await this.telemetrySchema.deleteMany({});
    } catch (error) {
      throw new UnauthorizedException(error.message);
    }
  }
}

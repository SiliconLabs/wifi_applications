import { BadRequestException, Injectable } from '@nestjs/common';
import { ConfigService } from '@nestjs/config';

import AzureMsalServiceInterface from './interfaces/msal-auth-service.interface';
import { IMsalKeys } from './interfaces/msal-keys.interface';
import { Messages } from '../constants';

@Injectable()
export class AzureMsalService implements AzureMsalServiceInterface {
  constructor(private readonly config: ConfigService) {}

  /**
   * Get Microsoft pubic keys
   *
   * @returns {IMsalKeys[]} - keys array
   */
  async getMicrosoftPublicKeys(): Promise<IMsalKeys[]> {
    try {
      const jwksUrl = this.config.get('jwt.jwksUrl');

      const response = await fetch(jwksUrl);
      if (!response.ok) {
        throw new BadRequestException(`HTTP error! status: ${response.status}`);
      }
      const responeData = await response.json();
      return responeData?.keys;
    } catch (error) {
      throw new BadRequestException(Messages.FailedtoFetchKeys);
    }
  }
}

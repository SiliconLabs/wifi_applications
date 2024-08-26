import { IMsalKeys } from './msal-keys.interface';

export interface AzureMsalServiceInterface {
  getMicrosoftPublicKeys(): Promise<IMsalKeys[]>;
}
export default AzureMsalServiceInterface;

import { UserData } from './custom-request.interface';
import { ITokenInfo } from './token-info.interface';

export interface AuthServiceInterface {
  generateSession(userData: ITokenInfo): Promise<string>;

  validateToken(token: string): ITokenInfo;

  validateIdToken(token: string): Promise<UserData>;

  logout(token: string): Promise<void>;
}
export default AuthServiceInterface;

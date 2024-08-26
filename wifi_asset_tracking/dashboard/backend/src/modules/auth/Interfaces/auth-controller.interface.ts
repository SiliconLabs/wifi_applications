import { Request, Response } from 'express';
import { CustomRequest } from './custom-request.interface';
import { IToken } from './token-info.interface';

export interface AuthControllerInterface {
  authCallback(req: CustomRequest, res: Response): Promise<IToken>;
  logout(req: Request, res: Response): Promise<Response>;
}

export default AuthControllerInterface;

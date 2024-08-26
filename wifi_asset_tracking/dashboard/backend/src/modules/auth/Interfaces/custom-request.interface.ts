import { Request } from 'express';

interface UserData {
  username: string;
  exp: number;
  name: string;
}

interface CustomRequest extends Request {
  userData?: UserData;
}

export { CustomRequest, UserData };

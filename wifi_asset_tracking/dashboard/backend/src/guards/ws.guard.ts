import { CanActivate, ExecutionContext, Injectable } from '@nestjs/common';
import { WsException } from '@nestjs/websockets';
import { JwtService } from '@nestjs/jwt';
import { ConfigService } from '@nestjs/config';

@Injectable()
export class WsGuard implements CanActivate {
  constructor(
    private jwtService: JwtService,
    private configService: ConfigService,
  ) {}

  async canActivate(context: ExecutionContext | any): Promise<any> {
    const headers = context.args[0].handshake.headers;
    const request = context.switchToHttp().getRequest();

    let token = headers.authentication;

    const payload = await this.jwtService.verify(token.split(' ')[1], {
      secret: this.configService.get('jwt.secret'),
    });

    if (!payload) {
      throw new WsException('Invalid token');
    }

    request['iat'] = payload.lat;

    return true;
  }
  catch(err) {
    throw new WsException(err.message);
  }
}

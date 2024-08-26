import { CanActivate, ExecutionContext, Injectable, UnauthorizedException, ForbiddenException } from '@nestjs/common';
import { AuthService } from '../modules/auth/service/auth.service';
import { Messages } from '../utilities';
import { SessionService } from '../modules/auth/service/session.service';

@Injectable()
export class DownloadAPIGuard implements CanActivate {
  constructor(private readonly authService: AuthService) {}

  async canActivate(context: ExecutionContext): Promise<boolean> {
    const request = context.switchToHttp().getRequest();

    if (request.cookies && request.cookies.access_token) {
      const userData = this.authService.validateToken(request.cookies.access_token);
      if (userData && userData.username) {
        return true;
      }
      throw new UnauthorizedException(Messages.JWTTokenExpired);
    }
    throw new UnauthorizedException(Messages.NoTokenProvided);
  }
}

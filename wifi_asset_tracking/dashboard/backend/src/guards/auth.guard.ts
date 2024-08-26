import { CanActivate, ExecutionContext, Injectable, UnauthorizedException, ForbiddenException } from '@nestjs/common';
import { AuthService } from '../modules/auth/service/auth.service';
import { Messages } from '../utilities';
import { SessionService } from '../modules/auth/service/session.service';

@Injectable()
export class AuthGuard implements CanActivate {
  constructor(
    private readonly authService: AuthService,
    private readonly sessionService: SessionService,
  ) {}

  async canActivate(context: ExecutionContext): Promise<boolean> {
    const request = context.switchToHttp().getRequest();

    if (request.cookies && request.cookies.access_token) {
      const userData = this.authService.validateToken(request.cookies.access_token);
      if (userData && userData.username) {
        const isValid = await this.sessionService.validateSession(userData.username);
        if (isValid) {
          return true;
        }
      }
      throw new UnauthorizedException(Messages.JWTTokenExpired);
    }
    throw new UnauthorizedException(Messages.NoTokenProvided);
  }
}

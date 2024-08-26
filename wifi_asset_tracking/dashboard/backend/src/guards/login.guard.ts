import { CanActivate, ExecutionContext, Injectable, UnauthorizedException, ForbiddenException } from '@nestjs/common';
import { AuthService } from '../modules/auth/service/auth.service';
import { Messages } from '../utilities';

@Injectable()
export class LoginGuard implements CanActivate {
  constructor(private readonly authService: AuthService) {}

  async canActivate(context: ExecutionContext): Promise<boolean> {
    const request = context.switchToHttp().getRequest();

    if (request.headers?.authorization) {
      const authToken = request.headers?.authorization;
      const userData = this.authService.validateIdToken(authToken);
      if (userData) {
        request.userData = userData;

        return true;
      }
      throw new UnauthorizedException(Messages.JWTTokenExpired);
    }
    throw new UnauthorizedException(Messages.NoTokenProvided);
  }
}

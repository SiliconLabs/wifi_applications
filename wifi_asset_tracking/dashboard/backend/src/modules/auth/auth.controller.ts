import { Controller, Get, Req, Res, UseGuards } from '@nestjs/common';
import { Request, Response } from 'express';
import { ApiOperation, ApiTags } from '@nestjs/swagger';
import { AuthService } from './service/auth.service';
import { Time } from '../../utilities/constants';
import AuthControllerInterface from './Interfaces/auth-controller.interface';
import { LoginGuard } from '../../guards/login.guard';
import { CustomRequest } from './Interfaces/custom-request.interface';
import { ConfigService } from '@nestjs/config';
import { IToken } from './Interfaces/token-info.interface';

@Controller('auth')
@ApiTags('Auth')
export class AuthController implements AuthControllerInterface {
  constructor(
    private readonly authService: AuthService,
    private readonly config: ConfigService,
  ) {}

  /**
   * Validate idToken and set cookie
   * @param {CustomRequest} res - userData
   * @returns {Promise<IToken>} - token
   */
  @UseGuards(LoginGuard)
  @Get('/callback')
  @ApiOperation({ summary: 'Authenticate user' })
  async authCallback(@Req() req: CustomRequest, @Res({ passthrough: true }) res: Response): Promise<IToken> {
    const userData = await req.userData;
    const accessToken = await this.authService.generateSession(userData);
    res.cookie('access_token', accessToken, {
      httpOnly: false,
      maxAge: Time.OneYearInMilliseconds,
    });
    return { accessToken: accessToken };
  }

  @Get('/logout')
  @ApiOperation({ summary: 'Logout user' })

  /**
   * Logout user
   * @returns {Promise<Response>}
   */
  async logout(@Req() req: Request, @Res({ passthrough: true }) res: Response): Promise<Response> {
    await this.authService.logout();
    res.clearCookie('access_token', {
      httpOnly: true,
    });
    return;
  }
}

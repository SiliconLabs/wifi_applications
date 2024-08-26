import { Test, TestingModule } from '@nestjs/testing';
import { AuthController } from '../auth.controller';
import { AuthService } from '../service/auth.service';
import { Response } from 'express';
import { IToken } from '../Interfaces/token-info.interface';
import { CustomRequest } from '../Interfaces/custom-request.interface';
import { Time } from '../../../utilities/constants';
import { SessionService } from '../service/session.service';
import { ConfigService } from '@nestjs/config';

describe('AuthController', () => {
  let authController: AuthController;
  let authService: AuthService;

  const mockedAuthService = {
    generateSession: jest.fn(),
    logout: jest.fn(),
  };

  const mockedSessionService = {
    createSession: jest.fn(),
  };

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      controllers: [AuthController],
      providers: [
        {
          provide: AuthService,
          useValue: mockedAuthService,
        },
        {
          provide: SessionService,
          useValue: mockedSessionService,
        },
        ConfigService,
      ],
    }).compile();

    authController = module.get<AuthController>(AuthController);
    authService = module.get<AuthService>(AuthService);
  });

  describe('authCallback', () => {
    it('should generate an access token and set a cookie', async () => {
      const userData = { username: 'user1', exp: 'Test User' }; // Mocked user data
      const accessToken = 'mockedAccessToken'; // Mocked access token
      const req = { userData } as unknown as CustomRequest;
      const res = {
        cookie: jest.fn(),
      } as unknown as Response;

      mockedAuthService.generateSession.mockResolvedValue(accessToken);

      const result: IToken = await authController.authCallback(req, res);

      expect(mockedAuthService.generateSession).toHaveBeenCalledWith(userData);
      expect(res.cookie).toHaveBeenCalledWith('access_token', accessToken, {
        httpOnly: false,
        maxAge: Time.OneYearInMilliseconds,
      });
      expect(result).toEqual({ accessToken });
    });
  });

  describe('logout', () => {
    it('should clear the access token cookie and call logout', async () => {
      const accessToken = 'mockedAccessToken';
      const req = {
        cookies: {
          access_token: accessToken,
        },
      } as unknown as CustomRequest;
      const res = {
        clearCookie: jest.fn(),
      } as unknown as Response;

      mockedAuthService.logout.mockResolvedValue(undefined);

      await authController.logout(req, res);

      expect(mockedAuthService.logout).toHaveBeenCalled();
      expect(res.clearCookie).toHaveBeenCalledWith('access_token', {
        httpOnly: true,
      });
    });
  });
});

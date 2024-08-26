import { Test, TestingModule } from '@nestjs/testing';
import { AuthService } from '../service/auth.service';
import { AzureMsalService } from '../../../utilities/azure/msal-auth.service';
import { JwtService } from '@nestjs/jwt';
import { ITokenInfo } from '../Interfaces/token-info.interface';
import { BadRequestException } from '@nestjs/common';
import { getModelToken } from '@nestjs/mongoose';
import { SessionService } from '../service/session.service';
import { UserData } from '../Interfaces/custom-request.interface';

describe('AuthService', () => {
  let authService: AuthService;
  let azureMsalService: AzureMsalService;
  let jwtService: JwtService;
  let sessionService: SessionService;

  const mockedAzureMsalService = {
    authenticateUser: jest.fn(),
    getMicrosoftPublicKeys: jest.fn(),
  };

  const mockedSessionService = {
    createSession: jest.fn(),
    checkActiveSession: jest.fn(),
    deleteSession: jest.fn(),
  };

  const mockTelemetryModel = {
    find: jest.fn(),
    deleteMany: jest.fn(),
  };

  const mockKeepAliveModel = {
    deleteMany: jest.fn(),
  };

  const mockedJwtService = {
    signAsync: jest.fn().mockResolvedValue('mockedJwtToken'),
    verify: jest.fn().mockReturnValue({
      username: 'test@test.com',
      iat: 123,
      exp: 123123,
    }),
    decode: jest.fn(),
  };

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      providers: [
        AuthService,
        {
          provide: AzureMsalService,
          useValue: mockedAzureMsalService,
        },
        {
          provide: JwtService,
          useValue: mockedJwtService,
        },
        {
          provide: getModelToken('Telemetry'),
          useValue: mockTelemetryModel,
        },
        {
          provide: SessionService,
          useValue: mockedSessionService,
        },
        {
          provide: getModelToken('KeepAlive'),
          useValue: mockKeepAliveModel,
        },
      ],
    }).compile();

    authService = module.get<AuthService>(AuthService);
    azureMsalService = module.get<AzureMsalService>(AzureMsalService);
    jwtService = module.get<JwtService>(JwtService);
    sessionService = module.get<SessionService>(SessionService);
  });

  describe('validateToken', () => {
    it('Success - should validate jwt token', async () => {
      const inputStubToken = 'mockedJwtToken';

      const actual: ITokenInfo = authService.validateToken(inputStubToken);

      expect(jwtService.verify).toHaveBeenCalledWith(inputStubToken);
      expect(actual).toEqual({
        username: 'test@test.com',
        iat: 123,
        exp: 123123,
      });
    });
  });

  describe('validateIdToken', () => {
    it('Success - should validate id token', async () => {
      const inputStubToken = 'mockedIdToken';
      const mockJwks = [{ kid: 'mockKid', alg: 'RS256', kty: 'RSA', use: 'sig', n: 'mockN', e: 'AQAB' }];
      const decodedHeader = { kid: 'mockKid', alg: 'RS256' };
      const decodedToken = {
        preferred_username: 'test@test.com',
        exp: 123123,
        name: 'Test User',
      };

      mockedAzureMsalService.getMicrosoftPublicKeys.mockResolvedValue(mockJwks);
      mockedJwtService.decode.mockReturnValue({ header: decodedHeader });
      mockedJwtService.verify.mockReturnValue(decodedToken);

      const actual: UserData = await authService.validateIdToken(inputStubToken);

      expect(azureMsalService.getMicrosoftPublicKeys).toHaveBeenCalled();
      expect(jwtService.decode).toHaveBeenCalledWith(inputStubToken, { complete: true });
      // expect(jwtService.verify).toHaveBeenCalledWith(inputStubToken, {
      //   algorithms: ['RS256'],
      //   secret: 'mockPublicKey',
      // });
      expect(actual).toEqual({
        username: decodedToken.preferred_username,
        exp: decodedToken.exp,
        name: decodedToken.name,
      });
    });
  });

  describe('generateSession', () => {
    const mockUserData = {
      username: 'test@test.com',
      iat: 123,
      exp: 123123,
      name: 'Test',
    };
    it('Success - should validate and return new token', async () => {
      const timeNow = new Date();
      mockedSessionService.checkActiveSession.mockResolvedValue(false);
      mockedSessionService.createSession.mockResolvedValue(timeNow);

      const token = await authService.generateSession(mockUserData);

      expect(sessionService.checkActiveSession).toHaveBeenCalled();
      expect(sessionService.createSession).toHaveBeenCalledWith(mockUserData.username);
      expect(mockTelemetryModel.deleteMany).toHaveBeenCalledWith({});
      expect(jwtService.signAsync).toHaveBeenCalledWith({
        username: mockUserData.username,
        name: mockUserData.name,
        sessionTimer: Math.floor(timeNow.getTime() / 1000),
      });
      expect(token).toBe('mockedJwtToken');
    });

    it('Failure - should throw BadRequestException when user session is active', async () => {
      mockedSessionService.checkActiveSession.mockResolvedValue(true);

      await expect(authService.generateSession(mockUserData)).rejects.toThrow(
        new BadRequestException('One session is already active currently'),
      );
    });

    describe('logout', () => {
      it('Success - should logout the user', async () => {
        await authService.logout();

        expect(sessionService.deleteSession).toHaveBeenCalled();
        expect(mockKeepAliveModel.deleteMany).toHaveBeenCalled();
        expect(mockTelemetryModel.deleteMany).toHaveBeenCalled();
      });
    });
  });
});

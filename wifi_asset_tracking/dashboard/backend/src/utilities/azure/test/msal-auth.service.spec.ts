import { Test } from '@nestjs/testing';
import { AzureMsalService } from '../msal-auth.service';
import { ConfigService } from '@nestjs/config';

global.fetch = jest.fn();

describe('CatsController', () => {
  let azureMsalService: AzureMsalService;
  let configService: ConfigService;

  beforeEach(async () => {
    const moduleRef = await Test.createTestingModule({
      providers: [
        AzureMsalService,
        {
          provide: ConfigService,
          useValue: {
            get: jest.fn(),
          },
        },
      ],
    }).compile();

    azureMsalService = moduleRef.get<AzureMsalService>(AzureMsalService);
    configService = moduleRef.get<ConfigService>(ConfigService);
  });

  afterEach(() => {
    jest.clearAllMocks();
  });

  describe('getMicrosoftPublicKeys', () => {
    it('Success - should return keys when the fetch is successful', async () => {
      const mockJwksUrl = 'https://example.com/.well-known/jwks.json';
      const mockKeys = [{ kid: 'key1', kty: 'RSA', n: 'abc', e: 'AQAB' }];
      const mockResponse = {
        ok: true,
        json: jest.fn().mockResolvedValue({ keys: mockKeys }),
      };

      (configService.get as jest.Mock).mockReturnValue(mockJwksUrl);
      (global.fetch as jest.Mock).mockResolvedValue(mockResponse);

      const result = await azureMsalService.getMicrosoftPublicKeys();
      expect(result).toEqual(mockKeys);
      expect(configService.get).toHaveBeenCalledWith('jwt.jwksUrl');
      expect(global.fetch).toHaveBeenCalledWith(mockJwksUrl);
    });
  });
});

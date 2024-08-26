import { Test, TestingModule } from '@nestjs/testing';
import { EventsGateway } from './events.gateway';
import { JwtService } from '@nestjs/jwt';
import { ConfigService } from '@nestjs/config';
import { IoTService } from '../iot-connectivity/service/iot-connection.service';
import { InternalServerErrorException, UnauthorizedException } from '@nestjs/common';
import { Socket } from 'socket.io';
import { IDeviceData } from '../iot-connectivity/interface/iot-data.interface';

describe('EventsGateway', () => {
  let gateway: EventsGateway;
  let jwtService: JwtService;
  let configService: ConfigService;
  let ioTService: IoTService;
  let client: Socket;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      providers: [
        EventsGateway,
        {
          provide: JwtService,
          useValue: {
            verify: jest.fn(),
          },
        },
        {
          provide: ConfigService,
          useValue: {
            get: jest.fn(),
          },
        },
        {
          provide: IoTService,
          useValue: {
            destroyUserSession: jest.fn(),
          },
        },
      ],
    }).compile();

    gateway = module.get<EventsGateway>(EventsGateway);
    jwtService = module.get<JwtService>(JwtService);
    configService = module.get<ConfigService>(ConfigService);
    ioTService = module.get<IoTService>(IoTService);
    client = {
      join: jest.fn(),
      id: 'client-id',
      handshake: { headers: { authentication: 'Bearer valid.token' } },
    } as unknown as Socket;
  });

  describe('handleConnection', () => {
    it('should connect successfully with valid token', async () => {
      const payload = { userId: 1 };
      (jwtService.verify as jest.Mock).mockResolvedValue(payload);
      (configService.get as jest.Mock).mockReturnValue('test-secret');

      await expect(gateway.handleConnection(client as any)).resolves.not.toThrow();
      expect(client.join).toHaveBeenCalledWith(gateway.roomId);
    });

    it('should throw UnauthorizedException with invalid token', async () => {
      (jwtService.verify as jest.Mock).mockImplementation(() => {
        throw new Error('Invalid token');
      });
      (configService.get as jest.Mock).mockReturnValue('test-secret');

      expect(gateway.handleConnection(client as any)).rejects.toThrow(InternalServerErrorException);
    });
  });

  describe('handleDisconnect', () => {
    it('should log disconnection and destroy user session', () => {
      const loggerSpy = jest.spyOn(gateway.logger, 'log');

      gateway.handleDisconnect(client);

      expect(loggerSpy).toHaveBeenCalledWith(`Disconnected: ${client.id}`);
    });
  });

  describe('sendMessage', () => {
    it('should emit message to room', async () => {
      const data = { type: 'imu' } as IDeviceData;
      const emitMessageSpy = jest.spyOn(gateway, 'emitMessage').mockImplementation(jest.fn());

      await gateway.sendMessage(data);

      expect(emitMessageSpy).toHaveBeenCalledWith(gateway.roomId, data);
    });
  });

  describe('handleReceiveIoTDataEvent', () => {
    it('should handle IoT data event', async () => {
      const payload = { type: 'test' };
      const emitMessageSpy = jest.spyOn(gateway, 'emitMessage').mockImplementation(jest.fn());

      await gateway.handleReceiveIoTDataEvent(payload);

      expect(emitMessageSpy).toHaveBeenCalledWith(gateway.roomId, payload);
    });
  });

  describe('handleOnConnectDisconnectEvent', () => {
    it('should handle connect/disconnect event', async () => {
      const payload = { type: 'test' };
      const emitConnectDisconnectEventSpy = jest
        .spyOn(gateway, 'emitConnectDisconnectEvent')
        .mockImplementation(jest.fn());

      await gateway.handleOnConnectDisconnectEvent(payload);

      expect(emitConnectDisconnectEventSpy).toHaveBeenCalledWith(gateway.roomId, payload);
    });
  });

  describe('verifyToken', () => {
    it('should verify token successfully', async () => {
      const token = 'Bearer valid.token';
      const payload = { userId: 1 };
      (jwtService.verify as jest.Mock).mockResolvedValue(payload);
      (configService.get as jest.Mock).mockReturnValue('test-secret');

      const result = await gateway.verifyToken(token);

      expect(result).toEqual(payload);
    });

    it('should throw error for invalid token', async () => {
      const token = 'Bearer invalid.token';
      (jwtService.verify as jest.Mock).mockImplementation(() => {
        throw new Error('Invalid token');
      });
      (configService.get as jest.Mock).mockReturnValue('test-secret');

      await expect(gateway.verifyToken(token)).rejects.toThrow('Invalid token');
    });
  });
});

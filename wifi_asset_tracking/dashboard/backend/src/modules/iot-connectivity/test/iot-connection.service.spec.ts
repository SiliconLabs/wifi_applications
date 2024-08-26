import { Test, TestingModule } from '@nestjs/testing';
import { IoTService } from '../service/iot-connection.service';
import { ConfigService } from '@nestjs/config';
import { EventEmitter2 } from '@nestjs/event-emitter';
import { getModelToken } from '@nestjs/mongoose';
import { EventHubConsumerClient } from '@azure/event-hubs';
import { Model } from 'mongoose';
import { Telemetry } from '../../../models/telemetry.schema';
import { KeepAlive } from '../../../models/device-keep-alive.schema';
import { Session } from '../../../models/session.schema';
import { SensorTimestamp } from '../../../models/device-sensor-timestamp.schema';
import { Time } from '../../../utilities/constants';
import { timestamp } from 'rxjs';
import internal from 'stream';

// Mocking dependencies
jest.mock('@nestjs/config');
jest.mock('@nestjs/event-emitter');
jest.mock('@azure/event-hubs');
jest.mock('@azure/storage-blob', () => ({
  ContainerClient: jest.fn(),
}));

jest.mock('@azure/eventhubs-checkpointstore-blob', () => ({
  BlobCheckpointStore: jest.fn(),
}));

describe('IoTService', () => {
  let service: IoTService;
  let telemetryModel: Model<Telemetry>;
  let keepAliveModel: Model<KeepAlive>;
  let sessionModel: Model<Session>;
  let sensorTimestampModel: Model<SensorTimestamp>;

  const mockTelemetryModel = {
    save: jest.fn(),
    deleteMany: jest.fn(),
  };

  const mockSensorTimestampModel = {
    deleteOne: jest.fn(),
    findOneAndUpdate: jest.fn(),
    findOne: jest.fn(),
  };

  const mockKeepAliveModel = {
    findOne: jest.fn(),
    findOneAndUpdate: jest.fn(),
    deleteOne: jest.fn(),
  };

  const mockSessionModel = {
    findOne: jest.fn(),
    updateOne: jest.fn(),
  };

  const mockConfigService = {
    getOrThrow: jest.fn((key: string) => {
      const configMap = {
        'iot.iotEventHubEndPoint': 'mockEndPoint',
        'iot.iotConsumerGroup': 'mockConsumerGroup',
        'iot.storageConnectionString': 'mockStorageConnectionString',
        'iot.containerName': 'mockContainerName',
      };
      return configMap[key];
    }),
  };

  const mockEventEmitter = {
    emit: jest.fn(),
  };

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      providers: [
        IoTService,
        { provide: ConfigService, useValue: mockConfigService },
        { provide: EventEmitter2, useValue: mockEventEmitter },
        { provide: getModelToken(Telemetry.name), useValue: mockTelemetryModel },
        { provide: getModelToken(KeepAlive.name), useValue: mockKeepAliveModel },
        { provide: getModelToken(Session.name), useValue: mockSessionModel },
        { provide: getModelToken(SensorTimestamp.name), useValue: mockSensorTimestampModel },
      ],
    }).compile();

    service = module.get<IoTService>(IoTService);
    telemetryModel = module.get<Model<Telemetry>>(getModelToken(Telemetry.name));
    keepAliveModel = module.get<Model<KeepAlive>>(getModelToken(KeepAlive.name));
    sessionModel = module.get<Model<Session>>(getModelToken(Session.name));
  });

  afterEach(() => {
    jest.clearAllMocks();
  });

  it('should be defined', () => {
    expect(service).toBeDefined();
  });

  it('should initialize with correct configuration', () => {
    expect(service.eventHubEndPoint).toBe('mockEndPoint');
    expect(service.consumerGroupName).toBe('mockConsumerGroup');
    expect(service.storageConnectionString).toBe('mockStorageConnectionString');
    expect(service.containerName).toBe('mockContainerName');
    expect(service.client).toBeInstanceOf(EventHubConsumerClient);
  });

  it('should subscribe to events on initialization', () => {
    const subscribeSpy = jest.spyOn(service, 'subscribeEvent');
    service.subscribeEvent();
    expect(subscribeSpy).toHaveBeenCalled();
  });

  describe('parseIoTData', () => {
    it('should parse "heat" data correctly', () => {
      const data = {
        msgtype: 'heat',
        timestamp: new Date().toISOString(),
        heat: {
          temperature: 22,
          timestamp: new Date(),
        },
      };
      const result = service.parseIoTData(data);
      expect(result.type).toBe('heat');
      expect(result.heat['temperature']).toBe(22);
      expect(result.heat['timestamp']).toBeInstanceOf(Date);
    });

    it('should parse "gps" data correctly', () => {
      const data = {
        msgtype: 'gps',
        timestamp: new Date().toISOString(),
        gps: { latitude: 10, longitude: 20 },
      };
      const result = service.parseIoTData(data);
      expect(result.type).toBe('gps');
      expect(result.gps['latitude']).toBe(10);
      expect(result.gps['longitude']).toBe(20);
      expect(result.gps['timestamp']).toBeInstanceOf(Date);
    });

    it('should parse "wifi" data correctly', () => {
      const data = {
        msgtype: 'wifi',
        timestamp: new Date().toISOString(),
        wifi: { macid: 'testmacid', ssid: 'testssid', rssi: 11 },
      };
      const result = service.parseIoTData(data);
      expect(result.type).toBe('wifi');
      expect(result.wifi['macid']).toBe('testmacid');
      expect(result.wifi['ssid']).toBe('testssid');
      expect(result.wifi['rssi']).toBe(11);
      expect(result.wifi['timestamp']).toBeInstanceOf(Date);
    });

    it('should parse "imu" data correctly', () => {
      const data = {
        msgtype: 'imu',
        timestamp: new Date().toISOString(),
        accelero: [1, 2, 3],
        gyro: [4, 5, 6],
      };
      const result = service.parseIoTData(data);

      expect(result.type).toBe('imu');
      expect(result?.['accelGyroData'].accelero).toEqual([1, 2, 3]);
      expect(result?.['accelGyroData'].gyro).toEqual([4, 5, 6]);
      expect(result?.['accelGyroData'].timestamp).toBeInstanceOf(Date);
    });

    it('should parse "keep-alive" data correctly', () => {
      const data = {
        msgtype: 'keep-alive',
        timestamp: new Date().toISOString(),
        interval: [1, 2, 3, 4],
      };
      const result = service.parseIoTData(data);

      expect(result.type).toBe('keep-alive');
      expect(result?.intervalData?.wifi).toEqual(data.interval[0] * Time.wifiSamplingInterval);
      expect(result?.intervalData?.heat).toEqual(data.interval[1] * Time.heatSamplingInterval);
      expect(result?.intervalData?.imu).toEqual(data.interval[2] * Time.imuSamplingInterval);
      expect(result?.intervalData?.gps).toEqual(data.interval[3] * Time.gpsSamplingInterval);
    });

    it('should handle unknown message types gracefully', () => {
      const data = {
        msgtype: 'unknown',
        timestamp: new Date().toISOString(),
      };
      const result = service.parseIoTData(data);
      expect(result.type).toBeUndefined();
    });
  });

  describe('checkDeviceKeepAlive', () => {
    it('should not destroy session if last message is within 30 seconds', async () => {
      const now = new Date();
      mockKeepAliveModel.findOne.mockResolvedValueOnce({ updatedAt: new Date(now.getTime() - 29000) });

      const destroyUserSessionSpy = jest.spyOn(service, 'destroyUserSession');
      await service.checkDeviceKeepAlive();
      expect(destroyUserSessionSpy).not.toHaveBeenCalled();
    });

    it('should destroy session if last message is older than 30 seconds', async () => {
      const now = new Date();
      mockKeepAliveModel.findOne.mockResolvedValueOnce({ updatedAt: new Date(now.getTime() - 31000) });
      mockSensorTimestampModel.findOne.mockResolvedValueOnce({ status: { device: true } });

      const destroyUserSessionSpy = jest.spyOn(service, 'destroyUserSession');
      await service.checkDeviceKeepAlive();
      expect(mockSensorTimestampModel.findOneAndUpdate).toHaveBeenCalled();
    });

    it('should not attempt to destroy session if no keep-alive message is found', async () => {
      mockKeepAliveModel.findOne.mockResolvedValueOnce(null);

      const destroyUserSessionSpy = jest.spyOn(service, 'destroyUserSession');
      await service.checkDeviceKeepAlive();
      expect(destroyUserSessionSpy).not.toHaveBeenCalled();
    });
  });

  describe('deletePrevSessionData', () => {
    it('should delete previous session data if no current session exists', async () => {
      mockSessionModel.findOne.mockResolvedValueOnce(null);

      const destroyUserSessionDataSpy = jest.spyOn(service, 'destroyUserSessionData');
      await service.deletePrevSessionData();
      expect(destroyUserSessionDataSpy).toHaveBeenCalled();
    });

    it('should delete previous session data if expired more than 3.5 minutes ago', async () => {
      const expiredSession = {
        expiresAt: new Date(new Date().getTime() - 4 * 60000), // expired 4 minutes ago
      };
      mockSessionModel.findOne.mockResolvedValueOnce(false);

      const destroyUserSessionDataSpy = jest.spyOn(service, 'destroyUserSessionData');
      await service.deletePrevSessionData();
      expect(destroyUserSessionDataSpy).toHaveBeenCalled();
    });

    it('should not delete current session data if within 3.5 minutes after expiry', async () => {
      const activeSession = {
        expiresAt: new Date(new Date().getTime() - 2 * 60000), // expired 2 minutes ago
      };
      mockSessionModel.findOne.mockResolvedValueOnce(activeSession);

      const destroyUserSessionDataSpy = jest.spyOn(service, 'destroyUserSessionData');
      await service.deletePrevSessionData();
      expect(destroyUserSessionDataSpy).not.toHaveBeenCalled();
    });
  });

  describe('handleCron', () => {
    it('should call checkDeviceKeepAlive and deletePrevSessionData', async () => {
      const checkDeviceKeepAliveSpy = jest.spyOn(service, 'checkDeviceKeepAlive');
      const deletePrevSessionDataSpy = jest.spyOn(service, 'deletePrevSessionData');

      await service.handleCron();
      expect(checkDeviceKeepAliveSpy).toHaveBeenCalled();
      expect(deletePrevSessionDataSpy).toHaveBeenCalled();
    });
  });

  describe('setupInitialListenerTime', () => {
    it('should update sensortimestamp on receiving the telemetry data first time', async () => {
      const currentTimestamp = new Date();

      mockSensorTimestampModel.findOne.mockResolvedValueOnce(null);
      await service.setupInitialListenerTime();
      expect(mockSensorTimestampModel.findOneAndUpdate).toHaveBeenCalled();
    });
  });

  describe('updateLatestSensorData', () => {
    it('should update timestamp details for imu in sensorTimestamp if keepAlive is received', async () => {
      const dataReceivedTimestamp = new Date(new Date().toUTCString());
      mockKeepAliveModel.findOne.mockResolvedValueOnce({ interval: [[1, 2, 3, 4]] });
      await service.updateLatestSensorData({ type: 'imu' }, 'test-deviceId');
      expect(mockSensorTimestampModel.findOneAndUpdate).toHaveBeenCalledWith(
        { deviceId: 'test-deviceId' },
        {
          $set: {
            'timestamp.imu': dataReceivedTimestamp,
            'status.imu': true,
            'status.device': true,
          },
        },
        {
          upsert: true,
        },
      );
    });

    it('should update timestamp details for heat in sensorTimestamp if keepAlive is received', async () => {
      const dataReceivedTimestamp = new Date(new Date().toUTCString());
      mockKeepAliveModel.findOne.mockResolvedValueOnce({ interval: [[1, 2, 3, 4]] });
      await service.updateLatestSensorData({ type: 'heat' }, 'test-deviceId');
      expect(mockSensorTimestampModel.findOneAndUpdate).toHaveBeenCalledWith(
        { deviceId: 'test-deviceId' },
        {
          $set: {
            'timestamp.heat': dataReceivedTimestamp,
            'status.heat': true,
            'status.device': true,
          },
        },
        {
          upsert: true,
        },
      );
    });

    it('should update timestamp details for gps in sensorTimestamp if keepAlive is received', async () => {
      const dataReceivedTimestamp = new Date(new Date().toUTCString());
      mockKeepAliveModel.findOne.mockResolvedValueOnce({ interval: [[1, 2, 3, 4]] });
      await service.updateLatestSensorData({ type: 'gps' }, 'test-deviceId');
      expect(mockSensorTimestampModel.findOneAndUpdate).toHaveBeenCalledWith(
        { deviceId: 'test-deviceId' },
        {
          $set: {
            'timestamp.gps': dataReceivedTimestamp,
            'status.gps': true,
            'status.device': true,
          },
        },
        {
          upsert: true,
        },
      );
    });

    it('should update timestamp details for wifi in sensorTimestamp if keepAlive is received', async () => {
      const dataReceivedTimestamp = new Date(new Date().toUTCString());
      mockKeepAliveModel.findOne.mockResolvedValueOnce({ interval: [[1, 2, 3, 4]] });
      await service.updateLatestSensorData({ type: 'wifi' }, 'test-deviceId');
      expect(mockSensorTimestampModel.findOneAndUpdate).toHaveBeenCalledWith(
        { deviceId: 'test-deviceId' },
        {
          $set: {
            'timestamp.wifi': dataReceivedTimestamp,
            'status.wifi': true,
            'status.device': true,
          },
        },
        {
          upsert: true,
        },
      );
    });
  });

  describe('resetInterval', () => {
    it('should delete sensorTimstamp data and reset interval in keepAlive', async () => {
      await service.resetInterval();

      expect(mockSensorTimestampModel.deleteOne).toHaveBeenCalled();
      expect(mockKeepAliveModel.findOneAndUpdate).toHaveBeenCalledWith({ $set: { interval: [] } });
    });
  });

  describe('checkInterval', () => {
    it('should update timestamp and status details of heat in sensorTimestamp ', async () => {
      let latestReceivedTimestamp = new Date();

      latestReceivedTimestamp.setSeconds(latestReceivedTimestamp.getSeconds() - 20);

      mockSensorTimestampModel.findOne.mockResolvedValueOnce({
        timestamp: { heat: latestReceivedTimestamp },
        status: { heat: true },
      });
      mockKeepAliveModel.findOne.mockResolvedValueOnce({ interval: [{ heat: 15 }] });
      await service.checkInterval();
      expect(mockEventEmitter.emit).toHaveBeenCalledWith('onSensorDisconnect', {
        type: 'heat',
      });
      expect(mockSensorTimestampModel.findOneAndUpdate).toHaveBeenCalledWith(
        {},
        {
          $set: { 'status.heat': false },
        },
        {
          upsert: true,
        },
      );
    });

    it('should update timestamp and status details of gps in sensorTimestamp ', async () => {
      let latestReceivedTimestamp = new Date();

      latestReceivedTimestamp.setSeconds(latestReceivedTimestamp.getSeconds() - 20);

      mockSensorTimestampModel.findOne.mockResolvedValueOnce({
        timestamp: { gps: latestReceivedTimestamp },
        status: { gps: true },
      });
      mockKeepAliveModel.findOne.mockResolvedValueOnce({ interval: [{ gps: 15 }] });
      await service.checkInterval();
      expect(mockEventEmitter.emit).toHaveBeenCalledWith('onSensorDisconnect', {
        type: 'gps',
      });
      expect(mockSensorTimestampModel.findOneAndUpdate).toHaveBeenCalledWith(
        {},
        {
          $set: { 'status.gps': false },
        },
        {
          upsert: true,
        },
      );
    });

    it('should update timestamp and status details of wifi in sensorTimestamp ', async () => {
      let latestReceivedTimestamp = new Date();

      latestReceivedTimestamp.setSeconds(latestReceivedTimestamp.getSeconds() - 20);

      mockSensorTimestampModel.findOne.mockResolvedValueOnce({
        timestamp: { wifi: latestReceivedTimestamp },
        status: { wifi: true },
      });
      mockKeepAliveModel.findOne.mockResolvedValueOnce({ interval: [{ wifi: 15 }] });
      await service.checkInterval();
      expect(mockEventEmitter.emit).toHaveBeenCalledWith('onSensorDisconnect', {
        type: 'wifi',
      });
      expect(mockSensorTimestampModel.findOneAndUpdate).toHaveBeenCalledWith(
        {},
        {
          $set: { 'status.wifi': false },
        },
        {
          upsert: true,
        },
      );
    });

    it('should update timestamp and status details of imu in sensorTimestamp ', async () => {
      let latestReceivedTimestamp = new Date();

      latestReceivedTimestamp.setSeconds(latestReceivedTimestamp.getSeconds() - 20);

      mockSensorTimestampModel.findOne.mockResolvedValueOnce({
        timestamp: { imu: latestReceivedTimestamp },
        status: { imu: true },
      });
      mockKeepAliveModel.findOne.mockResolvedValueOnce({ interval: [{ imu: 15 }] });
      await service.checkInterval();
      expect(mockEventEmitter.emit).toHaveBeenCalledWith('onSensorDisconnect', {
        type: 'imu',
      });
      expect(mockSensorTimestampModel.findOneAndUpdate).toHaveBeenCalledWith(
        {},
        {
          $set: { 'status.imu': false },
        },
        {
          upsert: true,
        },
      );
    });
  });
});

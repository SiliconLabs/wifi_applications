import { Test, TestingModule } from '@nestjs/testing';
import { DashboardController } from '../dashboard.controller';
import { DashboardService } from '../service/dashboard.service';
import { AuthService } from '../../auth/service/auth.service';
import { SENSOR_TYPE } from '../dtos/sensor-type.dto';
import { SessionService } from '../../../modules/auth/service/session.service';
import { BadRequestException } from '@nestjs/common';
import { Messages } from '../../../utilities';
import { Response } from 'express';
import * as path from 'path';
import * as fs from 'fs';

jest.mock('fs'); // Mocking the 'fs' module

describe('DashboardController', () => {
  let dashboardController: DashboardController;
  let dashboardService: DashboardService;

  const mockTemperatureData = {
    temperature: 50,
    unit: 'cel',
    timestamp: '2024-05-31T12:00:00.000Z',
  };

  const mockHumidityData = {
    humidity: 35,
    timestamp: '2024-05-31T12:00:00.000Z',
  };

  const mockGpsData = {
    timestamp: '2024-05-31T12:00:00.000Z',
    latitude: '45.4431708',
    longitude: '-121.7295917',
    altitude: '2394',
    satellites: 12,
  };

  const mockWifiData = {
    timestamp: '2024-05-31T12:00:00.000Z',
    macId: '454431708',
    ssId: '12412',
    rssi: 12,
  };

  const mockedDashboardService = {
    getTelemetryData: jest.fn().mockImplementation((type) => {
      switch (type) {
        case 'temperature':
          return Promise.resolve([mockTemperatureData]);
        case 'humidity':
          return Promise.resolve([mockHumidityData]);
        case 'wifi':
          return Promise.resolve([mockWifiData]);
        case 'gps':
          return Promise.resolve([mockGpsData]);
        default:
          throw new BadRequestException(Messages.InvalidType);
      }
    }),
    generateCSV: jest.fn().mockResolvedValue(Buffer.from('csv data')),
    generateGPX: jest.fn().mockResolvedValue(path.join(__dirname, 'test.gpx')),
  };

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      controllers: [DashboardController],
      providers: [
        {
          provide: DashboardService,
          useValue: mockedDashboardService,
        },
        {
          provide: AuthService,
          useValue: {},
        },
        {
          provide: SessionService,
          useValue: {},
        },
      ],
    }).compile();

    dashboardController = module.get<DashboardController>(DashboardController);
    dashboardService = module.get<DashboardService>(DashboardService);
  });

  describe('getTelemetryData', () => {
    it('should return temeletry data with type temperature', async () => {
      const expected = [mockTemperatureData];

      const param = { type: SENSOR_TYPE.temperature };
      const result = await dashboardController.getTelemetryData(param);

      expect(dashboardService.getTelemetryData).toHaveBeenCalledWith(param.type);
      expect(result).toEqual(expected);
    });

    it('should return temeletry data with type humdity', async () => {
      const expected = [mockHumidityData];

      const param = { type: SENSOR_TYPE.humidity };
      const result = await dashboardController.getTelemetryData(param);

      expect(dashboardService.getTelemetryData).toHaveBeenCalledWith(param.type);
      expect(result).toEqual(expected);
    });

    it('should return temeletry data with type gps', async () => {
      const expected = [mockGpsData];

      const param = { type: SENSOR_TYPE.gps };
      const result = await dashboardController.getTelemetryData(param);

      expect(dashboardService.getTelemetryData).toHaveBeenCalledWith(param.type);
      expect(result).toEqual(expected);
    });

    it('should return temeletry data with type wifi', async () => {
      const expected = [mockWifiData];
      const param = { type: SENSOR_TYPE.wifi };

      const result = await dashboardController.getTelemetryData(param);

      expect(dashboardService.getTelemetryData).toHaveBeenCalledWith(param.type);
      expect(result).toEqual(expected);
    });
  });

  describe('getCSVFile', () => {
    it('should return a CSV file', async () => {
      const mockResponse = {
        set: jest.fn().mockReturnThis(),
        type: jest.fn().mockReturnThis(),
        send: jest.fn(),
      } as unknown as Response;

      await dashboardController.getCSVFile(mockResponse);

      expect(dashboardService.generateCSV).toHaveBeenCalled();
      expect(mockResponse.set).toHaveBeenCalledWith('Content-Disposition', 'attachment; filename=telemetry-data.xlsx');
      expect(mockResponse.type).toHaveBeenCalledWith('application/vnd.ms-excel');
      expect(mockResponse.send).toHaveBeenCalledWith(Buffer.from('csv data'));
    });
  });

  describe('getGpxFile', () => {
    describe('getGpxFile', () => {
      it('should return a GPX file', async () => {
        const filePath = path.join(__dirname, 'test.gpx');
        const mockResponse = {
          download: jest.fn((path, filename, callback) => callback()),
        } as unknown as Response;

        const fsUnlinkSpy = jest.spyOn(fs, 'unlink').mockImplementation((path, callback) => callback(null));

        await dashboardController.getGpxFile(mockResponse);

        expect(dashboardService.generateGPX).toHaveBeenCalled();
        expect(mockResponse.download).toHaveBeenCalledWith(filePath, 'gpx_file.gpx', expect.any(Function));
        expect(fsUnlinkSpy).toHaveBeenCalledWith(filePath, expect.any(Function));
      });
    });
  });
});

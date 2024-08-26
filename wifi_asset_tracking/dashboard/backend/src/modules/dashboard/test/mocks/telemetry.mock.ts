import { SENSOR_TYPE } from '../../dtos/sensor-type.dto';

export const mockTelemetryHeatData = {
  _id: '664f29062a2f8f44bc704c71',
  heat: {
    timestamp: new Date(),
    temperature: {
      value: 100,
      unit: 'cel',
      _id: '664f29062a2f8f44bc704c73',
    },
    humidity: 30,
    _id: '664f29062a2f8f44bc704c72',
  },
  type: SENSOR_TYPE.temperature,
  createdAt: '2024-05-23T11:31:18.000Z',
  updatedAt: '2024-05-23T11:31:18.000Z',
  __v: 0,
};

export const mockTelemetryWifiData = {
  _id: '664f29062a2f8f44bc704c71',
  wifi: {
    timestamp: new Date(),
    macid: '1234',
    ssid: '5678',
    rssi: 10,
    _id: '664f29062a2f8f44bc704c72',
  },
  type: SENSOR_TYPE.wifi,
  createdAt: '2024-05-23T11:31:18.000Z',
  updatedAt: '2024-05-23T11:31:18.000Z',
  __v: 0,
};

export const mockTelemetryGpsData = {
  _id: '664f29062a2f8f44bc704c71',
  gps: {
    timestamp: new Date(),
    latitude: '1234',
    longitude: '5678',
    altitude: '123453',
    satellites: 3,
    _id: '664f29062a2f8f44bc704c72',
  },
  type: SENSOR_TYPE.gps,
  createdAt: '2024-05-23T11:31:18.000Z',
  updatedAt: '2024-05-23T11:31:18.000Z',
  __v: 0,
};

export const mockTelemetryAcceleroGyroData = {
  _id: '664f29062a2f8f44bc704c71',
  accelGyroData: {
    timestamp: new Date(),

    accelero: [1, 2, 3],
    gyro: [10, 20, 30],
    _id: '664f29062a2f8f44bc704c72',
  },
  type: SENSOR_TYPE.gps,
  createdAt: '2024-05-23T11:31:18.000Z',
  updatedAt: '2024-05-23T11:31:18.000Z',
  __v: 0,
};

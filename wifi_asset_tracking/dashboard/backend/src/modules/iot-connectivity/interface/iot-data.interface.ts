interface IBody {
  temperature: number;
}

interface ISystemProperties {
  'iothub-connection-device-id': string;
  'iothub-connection-auth-generation-id': string;
  'iothub-enqueuedtime': number;
  'iothub-message-source': string; //'Telemetry';
  'iothub-connection-auth-method': {
    scope: string; // 'device';
    type: string; //'sas';
    issuer: string; //'iothub';
  };
  'dt-dataschema': string;
  'dt-subject': string;
  'x-opt-sequence-number-epoch': number;
  contentType: string; //'application/json';
  contentEncoding: string; //'utf-8';
}

export interface IIoTData {
  body: IBody;
  properties?: undefined;
  offset: string;
  sequenceNumber: number;
  enqueuedTimeUtc: Date;
  partitionKey?: undefined;
  systemProperties: ISystemProperties;
  getRawAmqpMessage?: [];
  contentType: string; //'application/json';
}

export interface IDeviceData {
  type?: string; //string
  msgtype?: string; //string
  timestamp: Date;
  wifi?: {
    timestamp: Date;
    macid: string;
    ssid: string;
    'signal-strength': number;
  };
  heat?: {
    temperature: {
      value: number;
      unit: string;
    };
    humidity: number;
  };
  gps?: {
    latitude: number;
    longitude: number;
    altitude: number;
    satellites: number;
  };
  accelero?: {
    value: [number];
  };
  gyro?: {
    value: [number];
  };
  intervalData?: {
    wifi: number;
    heat: number;
    imu: number;
    gps: number;
  };
}

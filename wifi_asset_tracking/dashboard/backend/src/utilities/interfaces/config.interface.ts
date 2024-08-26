export interface IConfiguration {
  server: {
    port: number;
    env: string;
    routePrefix: string;
  };
  mongo: {
    uri: string;
  };
  jwt: {
    secret: string;
    jwksUrl: string;
  };
  iot: {
    iotEventHubEndPoint: string;
    iotConsumerGroup: string;
    storageConnectionString: string;
    containerName: string;
  };
}

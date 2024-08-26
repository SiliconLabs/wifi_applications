import { IConfiguration } from 'src/utilities';
import { Azure } from 'src/utilities/constants';

const appEnv = process.env.NODE_ENV || 'development';

export function environment(): IConfiguration {
  try {
    return {
      server: {
        env: appEnv,
        port: process.env.PORT ? +process.env.PORT : 3005,
        routePrefix: 'api/v1',
      },
      mongo: {
        uri: process.env.DB_URL,
      },
      jwt: {
        secret: process.env.JWT_SECRET,
        jwksUrl: Azure.jwk_uri,
      },
      iot: {
        iotEventHubEndPoint: process.env.IOT_EVENT_HUB_ENDPOINT,
        iotConsumerGroup: process.env.IOT_CONSUMER_GROUP,
        storageConnectionString: process.env.STORAGE_CONNECTION_STRING,
        containerName: process.env.CONTAINER_NAME,
      },
    };
  } catch (error) {
    throw new Error(error);
  }
}

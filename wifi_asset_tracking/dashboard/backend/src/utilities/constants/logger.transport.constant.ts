import * as winston from 'winston';
import * as TransportStream from 'winston-transport';
import { utilities } from 'nest-winston';
// import CloudWatchTransport from 'winston-cloudwatch';

export const LoggerTransports: TransportStream[] = [
  new winston.transports.Console({
    format: winston.format.combine(
      winston.format.timestamp(),
      winston.format.ms(),
      utilities.format.nestLike('App', { prettyPrint: true }),
    ),
  }),
];

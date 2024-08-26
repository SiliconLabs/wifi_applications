import 'dotenv/config';
import { ValidationPipe } from '@nestjs/common';
import { LazyModuleLoader, NestFactory } from '@nestjs/core';
import { ConfigService } from '@nestjs/config';
import { DocumentBuilder, SwaggerModule } from '@nestjs/swagger';
import { AppModule } from './app.module';
import { WinstonModule } from 'nest-winston';
import { LoggerTransports, TransformInterceptor, AppExceptionFilter } from './utilities';
import * as cookieParser from 'cookie-parser';
import * as express from 'express';
import * as winston from 'winston';
import helmet from 'helmet';

async function bootstrap() {
  const appLogger = WinstonModule.createLogger({
    format: winston.format.uncolorize(),
    transports: LoggerTransports,
  });

  const app = await NestFactory.create(AppModule, {
    // logger: process.env.IS_LOG_ENABLED == 'true' ? appLogger : ['error', 'fatal'], //if log is enabled then print all logs else only errors
    cors: {
      origin: 'http://localhost:5173',
      credentials: true,
    },
    bodyParser: true,
  });
  const configService = app.get(ConfigService);

  const routePrefix = 'api/v1';

  app.use(helmet());
  app.use(express.urlencoded({ extended: true }));
  app.use(cookieParser());

  // Lazy Loading Module
  const lazyModuleLoader = app.get(LazyModuleLoader);
  await lazyModuleLoader.load(() => AppModule);

  let isLogEnabled = true;
  process.argv.every(function (value) {
    if (value == '--no-debug') {
      isLogEnabled = false;
      return false;
    } else if (value == '--debug') {
      isLogEnabled = true;
      return false;
    }

    return true;
  });

  app.useLogger(isLogEnabled == true ? appLogger : ['error', 'fatal']);

  app.setGlobalPrefix(routePrefix);
  app.useGlobalPipes(
    new ValidationPipe({
      transform: true,
      whitelist: true,
      forbidNonWhitelisted: true,
    }),
  );
  app.useGlobalFilters(new AppExceptionFilter(appLogger));
  app.useGlobalInterceptors(new TransformInterceptor(appLogger));

  //Setting Up Swagger Documentation
  const config = new DocumentBuilder()
    .setTitle('Si Labs Application')
    .setDescription('API Documentation')
    .setVersion('1.0.0')
    .addServer('http://localhost:3005/', 'Local environment')
    .addBasicAuth()
    .addBearerAuth()
    .build();

  const document = SwaggerModule.createDocument(app, config);

  SwaggerModule.setup(routePrefix + '/swagger', app, document);
  app.getHttpAdapter().getInstance().set('trust proxy', 1);

  //Initiating Server
  const port = configService.get('server.port');
  await app.listen(port, '0.0.0.0');
  appLogger.log('App running at port ' + port);
}

bootstrap();

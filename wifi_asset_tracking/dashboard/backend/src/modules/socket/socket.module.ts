import { Module } from '@nestjs/common';
import { EventsGateway } from './events.gateway';
import { MongooseModule } from '@nestjs/mongoose';
import { JwtService } from '@nestjs/jwt';
import { Telemetry, TelemetrySchema } from '../../models/telemetry.schema';
import { IoTModule } from '../iot-connectivity/iot.module';

@Module({
  imports: [IoTModule, MongooseModule.forFeature([{ name: Telemetry.name, schema: TelemetrySchema }])],
  controllers: [],
  providers: [EventsGateway, JwtService],
  exports: [EventsGateway],
})
export class SocketModule {}

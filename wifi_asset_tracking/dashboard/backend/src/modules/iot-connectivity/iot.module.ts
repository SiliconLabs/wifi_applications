import { Module } from '@nestjs/common';
import { IoTService } from './service/iot-connection.service';
import { MongooseModule } from '@nestjs/mongoose';
import { Telemetry, TelemetrySchema } from '../../models/telemetry.schema';
import { KeepAlive, KeepAliveSchema } from '../../models/device-keep-alive.schema';
import { Session, SessionSchema } from 'src/models/session.schema';
import { SensorTimestamp, SensorTimestampSchema } from 'src/models/device-sensor-timestamp.schema';

@Module({
  imports: [
    MongooseModule.forFeature([
      { name: Telemetry.name, schema: TelemetrySchema },
      { name: KeepAlive.name, schema: KeepAliveSchema },
      { name: Session.name, schema: SessionSchema },
      { name: SensorTimestamp.name, schema: SensorTimestampSchema },
    ]),
  ],
  controllers: [],
  providers: [IoTService],
  exports: [IoTService],
})
export class IoTModule {}

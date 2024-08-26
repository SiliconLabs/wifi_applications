import { Module } from '@nestjs/common';
import { AuthService } from './service/auth.service';
import { MongooseModule } from '@nestjs/mongoose';
import { AuthController } from './auth.controller';
import { AzureModule } from 'src/utilities/azure/azure.module';
import { JwtModule } from '@nestjs/jwt';
import { Telemetry, TelemetrySchema } from '../../models/telemetry.schema';
import { SessionService } from './service/session.service';
import { KeepAlive, KeepAliveSchema } from '../../models/device-keep-alive.schema';
import { Session, SessionSchema } from '../../models/session.schema';

@Module({
  imports: [
    AzureModule,
    JwtModule.registerAsync({
      useFactory: async () => ({
        secret: process.env.JWT_SECRET,
        signOptions: {
          expiresIn: '365d',
        },
      }),
    }),
    MongooseModule.forFeature([
      { name: Telemetry.name, schema: TelemetrySchema },
      { name: Session.name, schema: SessionSchema },
      { name: KeepAlive.name, schema: KeepAliveSchema },
    ]),
  ],
  controllers: [AuthController],
  providers: [AuthService, SessionService],
  exports: [AuthService],
})
export class AuthModule {}

import { Module } from '@nestjs/common';
import { DashboardController } from './dashboard.controller';
import { DashboardService } from './service/dashboard.service';
import { MongooseModule } from '@nestjs/mongoose';
import { Telemetry, TelemetrySchema } from '../../models/telemetry.schema';
import { AuthModule } from '../auth/auth.module';
import { SessionService } from '../auth/service/session.service';
import { Session, SessionSchema } from 'src/models/session.schema';

@Module({
  controllers: [DashboardController],
  providers: [DashboardService, SessionService],
  imports: [
    MongooseModule.forFeature([
      { name: Telemetry.name, schema: TelemetrySchema },
      { name: Session.name, schema: SessionSchema },
    ]),
    AuthModule,
  ],
})
export class DashboardModule {}

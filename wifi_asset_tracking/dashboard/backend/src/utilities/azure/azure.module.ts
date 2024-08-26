import { Module } from '@nestjs/common';
import { AzureMsalService } from './msal-auth.service';

@Module({
  providers: [AzureMsalService],
  exports: [AzureMsalService],
})
export class AzureModule {}

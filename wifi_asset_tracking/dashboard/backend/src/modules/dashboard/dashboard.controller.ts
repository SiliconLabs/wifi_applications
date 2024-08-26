import * as fs from 'fs';
import { Response } from 'express';
import { ApiOperation, ApiTags } from '@nestjs/swagger';
import { BadRequestException, Body, Controller, Get, Param, Post, Res, UseGuards } from '@nestjs/common';
import { DashboardService } from './service/dashboard.service';
import { AuthGuard } from '../../guards/auth.guard';
import DashboardControllerInterface from './interfaces/dashboard-controller.interface';
import { Telemetry } from '../../models/telemetry.schema';
import { DownloadAPIGuard } from '../../guards/download-api.guard';
import { SensorTypeDTO } from './dtos/sensor-type.dto';
import { Messages } from '../../utilities';

@Controller('dashboard')
@ApiTags('Dashboard')
export class DashboardController implements DashboardControllerInterface {
  constructor(private readonly dashboardService: DashboardService) {}

  @UseGuards(DownloadAPIGuard)
  @Get('download-gpx')
  @ApiOperation({ summary: 'Download Gpx File' })
  async getGpxFile(@Res() res: Response): Promise<void> {
    const filePath = await this.dashboardService.generateGPX();
    res.download(filePath, 'gpx_file.gpx', (err) => {
      if (err) {
        throw new BadRequestException(Messages.UnabletoDownload);
      }
      // delete the file after sending
      fs.unlink(filePath, (err) => {
        if (err) {
          throw new BadRequestException(Messages.UnabletoDownload);
        }
      });
    });
  }

  @UseGuards(DownloadAPIGuard)
  @Get('generate-csv')
  @ApiOperation({ summary: 'Download Excel File' })
  async getCSVFile(@Res() res: Response): Promise<void> {
    const xlsBuffer = await this.dashboardService.generateCSV();
    res.set('Content-Disposition', 'attachment; filename=telemetry-data.xlsx');
    res.type('application/vnd.ms-excel');
    res.send(xlsBuffer);
  }

  @UseGuards(AuthGuard)
  @Get('/:type')
  @ApiOperation({ summary: 'Retrieve Telemetry Data' })
  getTelemetryData(@Param() param: SensorTypeDTO): Promise<Telemetry[]> {
    let { type } = param;
    return this.dashboardService.getTelemetryData(type);
  }
}

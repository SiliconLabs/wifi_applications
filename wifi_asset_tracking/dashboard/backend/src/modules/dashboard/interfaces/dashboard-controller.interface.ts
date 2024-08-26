import { Response } from 'express';
import { Telemetry } from '../../../models/telemetry.schema';
import { SensorTypeDTO } from '../dtos/sensor-type.dto';

export interface DashboardControllerInterface {
  getTelemetryData(param: SensorTypeDTO): Promise<Telemetry[]>;

  getGpxFile(res: Response): Promise<void>;

  getCSVFile(res: Response): Promise<void>;
}

export default DashboardControllerInterface;

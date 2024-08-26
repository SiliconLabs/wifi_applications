import { Telemetry } from '../../../models/telemetry.schema';

export interface DashboardServiceInterface {
  getTelemetryData(type: string): Promise<Telemetry[]>;

  generateGPX(): Promise<string>;

  generateCSV(): Promise<ArrayBuffer>;
}
export default DashboardServiceInterface;

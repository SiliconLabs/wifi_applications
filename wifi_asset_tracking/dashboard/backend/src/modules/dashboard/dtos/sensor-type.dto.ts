import { ApiProperty } from '@nestjs/swagger';
import { IsEnum, IsNotEmpty } from 'class-validator';

export enum SENSOR_TYPE {
  temperature = 'temperature',
  gps = 'gps',
  wifi = 'wifi',
  humidity = 'humidity',
  heat = 'heat',
  accelero = 'accelero',
  gyro = 'gyro',
  accelGyroData = 'accelGyroData',
  imu = 'imu',
}

export class SensorTypeDTO {
  @ApiProperty({ enum: SENSOR_TYPE })
  @IsEnum(SENSOR_TYPE)
  @IsNotEmpty()
  type: SENSOR_TYPE;
}

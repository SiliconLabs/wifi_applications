import { ApiProperty, ApiPropertyOptional } from '@nestjs/swagger';
import { IsNotEmpty, IsString, IsOptional } from 'class-validator';

export class GetChatHistory {
  @ApiProperty()
  @IsString()
  @IsNotEmpty()
  page: number;

  @ApiProperty()
  @IsNotEmpty()
  @IsString()
  limit: number;

  // @ApiPropertyOptional()
  // @IsOptional()
  // @IsString()
  // roomId: string;

  @ApiProperty()
  @IsNotEmpty()
  @IsString()
  categoryId: string;

  @ApiProperty()
  @IsNotEmpty()
  @IsString()
  userId: string;
}

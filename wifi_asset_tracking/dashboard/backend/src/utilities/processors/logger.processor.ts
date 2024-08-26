import { Injectable, Logger, NestMiddleware } from '@nestjs/common';
import { Request, Response, NextFunction } from 'express';

@Injectable()
export class LoggerMiddleware implements NestMiddleware {
  private readonly logger = new Logger();

  use(req: Request, res: Response, next: NextFunction) {
    this.logger.log('Request Method:', req.method);
    this.logger.log('Request URL:', req.originalUrl);
    this.logger.log('Request Payload:', JSON.stringify(req.body));
    next();
  }
}

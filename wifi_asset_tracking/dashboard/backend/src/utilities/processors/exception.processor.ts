import {
  ExceptionFilter,
  Catch,
  ArgumentsHost,
  HttpException,
  HttpStatus,
  Inject,
  Logger,
  LoggerService,
} from '@nestjs/common';
import { Messages } from '../constants';
import { IBaseResponse } from '../interfaces';
import { Response } from 'express';
import { TypeORMError } from 'typeorm';
import { MongoError } from 'mongodb';

@Catch()
export class AppExceptionFilter implements ExceptionFilter {
  constructor(@Inject(Logger) private readonly logger: LoggerService) {}

  catch(exception: unknown, host: ArgumentsHost) {
    const ctx = host.switchToHttp();
    const response = ctx.getResponse<Response>();

    const responseModel: IBaseResponse = {
      IsSuccess: false,
      Message: Messages.Error,
      Data: {},
      Errors: [],
    };

    let status = HttpStatus.INTERNAL_SERVER_ERROR;

    if (exception instanceof HttpException) {
      status = exception.getStatus();
      responseModel.Message = exception.message ? exception.message : Messages.Error;
      responseModel.Errors = [
        {
          Exception: exception.getResponse(),
        },
      ];
    } else if (exception instanceof TypeORMError) {
      status = HttpStatus.UNPROCESSABLE_ENTITY;
      responseModel.Message = Messages.DatabaseError;
    } else if (exception instanceof MongoError) {
      status = HttpStatus.UNPROCESSABLE_ENTITY;
      responseModel.Message = Messages.DatabaseError;
    } else {
      status = HttpStatus.INTERNAL_SERVER_ERROR;
      responseModel.Message = Messages.Error;
      responseModel.Errors = [
        {
          Exception: exception,
        },
      ];
    }

    if (responseModel.Message !== Messages.RouteNotFound) {
      this.logger.error(exception);
    }
    response.status(status).json(responseModel);
  }
}

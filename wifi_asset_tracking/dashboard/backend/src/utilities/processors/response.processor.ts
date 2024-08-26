import {
  Injectable,
  NestInterceptor,
  ExecutionContext,
  CallHandler,
  Inject,
  Logger,
  LoggerService,
} from '@nestjs/common';
import { Observable } from 'rxjs';
import { map } from 'rxjs/operators';
import { Messages } from '../constants';
import { IBaseResponse } from '../interfaces';

@Injectable()
export class TransformInterceptor<T> implements NestInterceptor<T, IBaseResponse> {
  constructor(@Inject(Logger) private readonly logger: LoggerService) {}

  intercept(context: ExecutionContext, next: CallHandler): Observable<IBaseResponse> {
    if (context.getType() === 'http') {
      return next.handle().pipe(
        map((data) => {
          return {
            IsSuccess: true,
            Message: Messages.Success,
            Data: data ? data : {},
            Errors: [],
          };
        }),
      );
    } else {
      return next.handle().pipe(
        map((data) => {
          return data;
        }),
      );
    }
  }
}

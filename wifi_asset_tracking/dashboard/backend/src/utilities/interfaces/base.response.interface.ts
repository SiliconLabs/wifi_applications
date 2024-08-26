export interface IBaseResponse {
  IsSuccess: boolean;
  Message: string;
  Data: unknown;
  Errors: unknown[];
}

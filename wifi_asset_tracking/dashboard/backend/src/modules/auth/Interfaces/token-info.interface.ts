export interface ITokenInfo {
  username: string;
  iat?: number;
  exp?: number;
  name?: string;
}

export interface IToken {
  accessToken: string;
}

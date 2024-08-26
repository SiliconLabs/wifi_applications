import { Socket } from 'socket.io';

export interface IRequestSocket extends Socket {
  _id: string;
}

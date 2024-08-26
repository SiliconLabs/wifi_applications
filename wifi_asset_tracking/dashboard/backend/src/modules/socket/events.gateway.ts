import {
  MessageBody,
  OnGatewayConnection,
  OnGatewayDisconnect,
  SubscribeMessage,
  WebSocketGateway,
  WebSocketServer,
} from '@nestjs/websockets';
import { Socket, Server } from 'socket.io';
import { UseGuards, UnauthorizedException, Logger, InternalServerErrorException } from '@nestjs/common';
import { WsGuard } from '../../guards/ws.guard';
import { JwtService } from '@nestjs/jwt';
import { ConfigService } from '@nestjs/config';
import { OnEvent } from '@nestjs/event-emitter';
import { IRequestSocket } from './interface/socket.interface';
import { IDeviceData } from '../iot-connectivity/interface/iot-data.interface';
import { IoTService } from '../iot-connectivity/service/iot-connection.service';

@WebSocketGateway({
  //transports: ['websocket'],
  cors: {
    origin: '*',
  },
})
export class EventsGateway implements OnGatewayConnection, OnGatewayDisconnect {
  @WebSocketServer()
  server: Server;
  roomId: string = 'si-labs-roomId';

  logger = new Logger(EventsGateway.name);

  constructor(
    private jwtService: JwtService,
    private configService: ConfigService,
    private ioTService: IoTService,
  ) {}

  async joinRoom(roomId, client): Promise<void> {
    client.join(roomId);
  }

  @OnEvent('receiveIotData')
  async handleReceiveIoTDataEvent(payload: { type: string }): Promise<void> {
    this.logger.log('event payload', payload);
    await this.emitMessage(this.roomId, payload);
  }

  @OnEvent('onConnectDisconnect')
  async handleOnConnectDisconnectEvent(payload: { type: string }): Promise<void> {
    await this.emitConnectDisconnectEvent(this.roomId, payload);
  }

  @OnEvent('onDisconnectSocketClient')
  async handleDisconnectSocket(payload: { type: string }): Promise<void> {
    await this.emitDisconnectSocketClient(this.roomId, payload);
  }

  @OnEvent('onSensorDisconnect')
  async handleSensorDisconnect(payload: { type: string }): Promise<void> {
    await this.emitSensorDisconnect(this.roomId, payload);
  }

  async handleDisconnect(client: Socket): Promise<void> {
    this.logger.log(`Disconnected: ${client.id}`);
  }

  async verifyToken(token): Promise<{
    username: string;
    name: string;
  }> {
    try {
      let jwttoken = token.split(' ')[1];

      const payload: {
        username: string;
        name: string;
      } = await this.jwtService.verify(jwttoken, {
        secret: this.configService.get('jwt.secret'),
      });

      if (!payload) {
        throw new UnauthorizedException('Invalid token');
      }

      return payload;
    } catch (error) {
      this.logger.error('error ' + error.message);
      throw new Error(error.message);
    }
  }

  async handleConnection(client: IRequestSocket): Promise<void> {
    try {
      let token: string = client?.handshake?.headers?.authentication as string;
      const verifyToken = await this.verifyToken(token);

      if (!verifyToken) {
        throw new UnauthorizedException('Invalid token');
      }

      await this.joinRoom(this.roomId, client);
    } catch (error) {
      this.logger.error('error ' + error.message);
      throw new InternalServerErrorException(error.message);
    }
  }

  @SubscribeMessage('message')
  @UseGuards(WsGuard)
  async sendMessage(@MessageBody() data: Partial<IDeviceData>): Promise<void> {
    this.logger.log('data', data);

    this.emitMessage(this.roomId, data);
  }

  async emitMessage(roomId, data): Promise<void> {
    this.server.to(roomId).emit('receiveMessage', data);
  }

  async emitConnectDisconnectEvent(roomId, data): Promise<void> {
    this.server.to(roomId).emit('onConnectDisconnect', data);
  }

  async emitSensorDisconnect(roomId, data): Promise<void> {
    this.server.to(roomId).emit('onSensorDisconnect', data);
    this.logger.log('sensor Disconnected', data);
  }

  async emitDisconnectSocketClient(roomId, data): Promise<void> {
    const room = this.server.of('/').adapter.rooms.get(this.roomId);
    if (room) {
      // Iterate through each socket ID in the room and disconnect them
      room.forEach((socketId: string) => {
        const socket = this.server.sockets.sockets.get(socketId);
        if (socket) {
          socket.disconnect(true); // true parameter means force disconnect
        }
      });
    }
  }
}

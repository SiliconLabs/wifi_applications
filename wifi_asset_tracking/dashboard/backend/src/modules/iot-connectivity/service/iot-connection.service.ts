import { Logger, Injectable, InternalServerErrorException } from '@nestjs/common';
import {
  EventHubConsumerClient,
  MessagingError,
  PartitionContext,
  ReceivedEventData,
  Subscription,
  earliestEventPosition,
} from '@azure/event-hubs';
import { ConfigService } from '@nestjs/config';
import { deviceConfig } from '../../../utilities';
import { InjectModel } from '@nestjs/mongoose';
import { Telemetry, TelemetrySchema } from '../../../models/telemetry.schema';
import { KeepAlive, KeepAliveSchema } from '../../../models/device-keep-alive.schema';
import { SessionSchema, Session } from '../../../models/session.schema';
import { Model } from 'mongoose';
import { IDeviceData } from '../interface/iot-data.interface';
import { EventEmitter2 } from '@nestjs/event-emitter';
import { Cron } from '@nestjs/schedule';
import { getTimeDifference, millisToSeconds } from '../../../utilities/common/helper';
import { SensorTimestamp, SensorTimestampSchema } from '../../../models/device-sensor-timestamp.schema';
import { Messages, Time } from '../../../utilities/constants';
const { ContainerClient } = require('@azure/storage-blob');
const { BlobCheckpointStore } = require('@azure/eventhubs-checkpointstore-blob');

@Injectable()
export class IoTService {
  private readonly logger = new Logger();

  eventHubEndPoint: string;
  consumerGroupName: string; // sa1_test_eic_AjeetThakur
  storageConnectionString: string;
  containerName: string;

  containerClient;
  checkpointStore;

  appStartTime;

  client: EventHubConsumerClient;

  constructor(
    @InjectModel(Telemetry.name) private telemetryModel: Model<TelemetrySchema>,
    @InjectModel(KeepAlive.name) private KeepAliveModel: Model<KeepAliveSchema>,
    @InjectModel(Session.name) private sessionModel: Model<SessionSchema>,
    @InjectModel(SensorTimestamp.name) private sensorTimestampModel: Model<SensorTimestampSchema>,
    private config: ConfigService,
    private eventEmitter: EventEmitter2,
  ) {
    this.eventHubEndPoint = this.config.getOrThrow('iot.iotEventHubEndPoint');
    this.consumerGroupName = this.config.getOrThrow('iot.iotConsumerGroup');
    this.storageConnectionString = this.config.getOrThrow('iot.storageConnectionString');
    this.containerName = this.config.getOrThrow('iot.containerName');

    const connectionOptions = {};

    this.containerClient = new ContainerClient(this.storageConnectionString, this.containerName);
    this.checkpointStore = new BlobCheckpointStore(this.containerClient);

    this.client = new EventHubConsumerClient(
      this.consumerGroupName,
      this.eventHubEndPoint,
      this.checkpointStore,
      connectionOptions,
    );

    this.subscribeEvent();

    this.appStartTime = new Date().getTime();
  }

  getCurrentSession() {
    return this.sessionModel.findOne({ expiresAt: { $gt: new Date() } });
  }

  @Cron('*/01 * * * * *') //running every 01 sec
  async handleCron() {
    //disconnect when no message received from deive from past 30 sec
    await this.checkDeviceKeepAlive();

    //Destroy all the prev session after the 3 minutes of session ended
    await this.deletePrevSessionData();

    const isSessionActive = await this.getCurrentSession();

    if (!isSessionActive) {
      this.resetInterval();
    }

    await this.checkInterval();
  }

  async checkInterval() {
    //temp
    const currentTimestamp = new Date();
    const sensorTimestampData = await this.sensorTimestampModel.findOne();
    const sensorTimestamp = sensorTimestampData?.timestamp;
    const sensorStatus = sensorTimestampData?.status;
    const keepAlive = await this.KeepAliveModel.findOne();
    const sensorInterval = keepAlive?.interval[0] || null;

    if (sensorInterval && sensorTimestamp) {
      const heatTimestamp = sensorTimestamp?.heat ? new Date(sensorTimestamp.heat) : null;
      if (
        heatTimestamp instanceof Date &&
        !isNaN(heatTimestamp.getTime()) &&
        currentTimestamp instanceof Date &&
        !isNaN(currentTimestamp.getTime())
      ) {
        const diffInMillisecondsHeat = currentTimestamp.getTime() - heatTimestamp.getTime();
        const diffInSecondsHeat = Math.floor(diffInMillisecondsHeat / 1000);

        if (sensorInterval && diffInSecondsHeat > sensorInterval['heat'] && sensorStatus.heat) {
          const payloadHeat = {
            type: 'heat',
          };
          this.eventEmitter.emit('onSensorDisconnect', payloadHeat);
          await this.sensorTimestampModel.findOneAndUpdate(
            {},
            {
              $set: { 'status.heat': false },
            },
            {
              upsert: true,
            },
          );
          // to avoid event to get emitted contineously
        }
      }

      //wifi

      const wifiTimestamp = sensorTimestamp?.wifi ? new Date(sensorTimestamp?.wifi) : null;
      if (
        wifiTimestamp instanceof Date &&
        !isNaN(wifiTimestamp.getTime()) &&
        currentTimestamp instanceof Date &&
        !isNaN(currentTimestamp.getTime())
      ) {
        const diffInMillisecondsWifi = currentTimestamp.getTime() - wifiTimestamp.getTime();
        const diffInSecondsWifi = Math.floor(diffInMillisecondsWifi / 1000);
        if (sensorInterval && diffInSecondsWifi > sensorInterval['wifi'] && sensorStatus.wifi) {
          const payloadWifi = {
            type: 'wifi',
          };
          this.eventEmitter.emit('onSensorDisconnect', payloadWifi);
          await this.sensorTimestampModel.findOneAndUpdate(
            {},
            {
              $set: { 'status.wifi': false },
            },
            {
              upsert: true,
            },
          );

          // to avoid event to get emitted contineously
        }
      }

      //gps

      const gpsTimestamp = sensorTimestamp?.gps ? new Date(sensorTimestamp?.gps) : null;
      if (
        gpsTimestamp instanceof Date &&
        !isNaN(gpsTimestamp.getTime()) &&
        currentTimestamp instanceof Date &&
        !isNaN(currentTimestamp.getTime())
      ) {
        const diffInMillisecondsGps = currentTimestamp.getTime() - gpsTimestamp.getTime();
        const diffInSecondsGps = Math.floor(diffInMillisecondsGps / 1000);
        if (sensorInterval && diffInSecondsGps > sensorInterval['gps'] && sensorStatus.gps) {
          const payloadGps = {
            type: 'gps',
          };
          this.eventEmitter.emit('onSensorDisconnect', payloadGps);
          await this.sensorTimestampModel.findOneAndUpdate(
            {},
            {
              $set: { 'status.gps': false },
            },
            {
              upsert: true,
            },
          );

          // to avoid event to get emitted contineously
        }
      }

      //imu

      const imuTimestamp = sensorTimestamp?.imu ? new Date(sensorTimestamp?.imu) : null;
      if (
        imuTimestamp instanceof Date &&
        !isNaN(imuTimestamp.getTime()) &&
        currentTimestamp instanceof Date &&
        !isNaN(currentTimestamp.getTime())
      ) {
        const diffInMillisecondsImu = currentTimestamp.getTime() - imuTimestamp.getTime();
        const diffInSecondsImu = Math.floor(diffInMillisecondsImu / 1000);

        if (sensorInterval && diffInSecondsImu > sensorInterval['imu'] && sensorStatus.imu) {
          const payloadImu = {
            type: 'imu',
          };
          this.eventEmitter.emit('onSensorDisconnect', payloadImu);
          await this.sensorTimestampModel.findOneAndUpdate(
            {},
            {
              $set: { 'status.imu': false },
            },
            {
              upsert: true,
            },
          );

          // to avoid event to get emitted contineously
        }
      }
    }
  }

  async resetInterval() {
    await this.sensorTimestampModel.deleteOne();
    await this.KeepAliveModel.findOneAndUpdate({ $set: { interval: [] } });
  }

  subscribeEvent() {
    const subscriptionOptions = {
      startPosition: earliestEventPosition,
    };

    const subscription = this.client.subscribe(
      {
        processEvents: async (events: ReceivedEventData[], context: PartitionContext) => {
          try {
            const systemProperties = events[0]?.systemProperties;
            // console.log('systemProperties==========', systemProperties);

            //Destroy all the message which is published before app start
            if (systemProperties && systemProperties?.['iothub-enqueuedtime'] < this.appStartTime) {
              this.logger.log('message published before app start');
              if (events && events.length > 0) {
                await context.updateCheckpoint(events[events.length - 1]);
              }
              return;
            }

            let isSessionActive = await this.getCurrentSession();
            this.logger.log('isSessionActive', isSessionActive);
            if (!isSessionActive) return;

            this.logger.log(
              'new Date(isSessionActive.createdAt).getTime()',
              new Date(isSessionActive.createdAt).getTime(),
            );
            this.logger.log("systemProperties?.['iothub-enqueuedtime']", systemProperties?.['iothub-enqueuedtime']);

            //Destroy all the message which is published before login
            if (
              systemProperties &&
              systemProperties?.['iothub-enqueuedtime'] < new Date(isSessionActive.createdAt).getTime()
            ) {
              this.logger.log('message published before user logged in');
              if (events && events.length > 0) {
                await context.updateCheckpoint(events[events.length - 1]);
              }
              return;
            }

            const body: Partial<IDeviceData> = events[0]?.body;

            // console.log('body==========', body);
            //Check if session is active
            if (isSessionActive) {
              if (systemProperties?.['iothub-connection-device-id'] === deviceConfig.DEFAULT_DEVICE_ID) {
                // console.log('body', body);
                //updation last event time to Check device keep alive status
                // if (body.msgtype === 'keep-alive') {}
                let deviceData = {
                  deviceId: systemProperties?.properties?.deviceId,
                  updatedAt: new Date(),
                };
                await this.KeepAliveModel.findOneAndUpdate({ deviceId: deviceData.deviceId }, deviceData, {
                  new: true,
                  upsert: true,
                });
                await this.sensorTimestampModel.findOneAndUpdate(
                  {},
                  {
                    $set: { 'status.device': true },
                  },
                  {
                    upsert: true,
                  },
                );

                //All the telemory data published to client and saving in DB
                if (body?.[body.msgtype] || body['accelero'] || body['gyro'] || body['interval']) {
                  let payload = this.parseIoTData(body);
                  if (payload.type == 'keep-alive') {
                    await this.setupInitialListenerTime();
                    await this.KeepAliveModel.findOneAndUpdate(
                      { deviceId: deviceData.deviceId },
                      { $set: { interval: payload?.intervalData } },
                    );
                  } else {
                    await this.updateLatestSensorData(payload, deviceData.deviceId);
                    await this.saveDeviceData(payload);

                    const enqueuedTime = new Date(events[0]?.enqueuedTimeUtc).getTime();
                    const currentTime = new Date().getTime();
                    const timeDifferenceInMilliseconds = currentTime - enqueuedTime;
                    const timeDifferenceInSeconds = timeDifferenceInMilliseconds / 1000;

                    if (timeDifferenceInSeconds <= 65) {
                      this.eventEmitter.emit('receiveIotData', payload);
                    }
                  }

                  // this.logger.log('payload  save data', payload);
                }
              }
            }

            //Acknowedge Eventhub after receiving events
            if (events && events.length > 0) {
              let resp = await context.updateCheckpoint(events[events.length - 1]);
            }
          } catch (error) {
            this.logger.error('error during fetch message', error.message);
          }
        },
        processError: async (error: Error | MessagingError, context: PartitionContext) => {
          let payload;
          if (error instanceof MessagingError && error.code === 'ENOTFOUND') {
            payload = {
              type: Messages.NetworkError,
            };
          } else if (error instanceof MessagingError && error.code === 'ETIMEDOUT') {
            payload = {
              type: Messages.IotConnectionTimeout,
            };
          } else {
            payload = {
              type: Messages.IotConnectionTimeout,
            };
          }
          this.eventEmitter.emit('onConnectDisconnect', payload);
          this.logger.error(error);
        },
      },
      subscriptionOptions,
    );

    return subscription;
  }

  async setupInitialListenerTime() {
    const timestamp = await this.sensorTimestampModel.findOne();
    if (!timestamp) {
      const currentTimestamp = new Date();
      await this.sensorTimestampModel.findOneAndUpdate(
        {},
        {
          timestamp: { heat: currentTimestamp, wifi: currentTimestamp, gps: currentTimestamp, imu: currentTimestamp },
          status: { heat: true, wifi: true, imu: true, gps: true, device: true },
        },
        {
          upsert: true,
        },
      );
    }
  }

  async destroySubscribedEvent(subscription: Subscription): Promise<void> {
    return await subscription.close();
  }

  async destroyUserSession(expiredAt): Promise<void> {
    await this.KeepAliveModel.deleteOne();
    await this.sensorTimestampModel.deleteOne();
    let currentSession = await this.sessionModel.findOne();
    this.logger.log('currentSession------------', currentSession);
    if (currentSession) {
      await this.sessionModel.updateOne({ _id: currentSession._id }, { expiresAt: new Date(expiredAt) });
    }
  }

  async destroyUserSessionData(): Promise<void> {
    await this.telemetryModel.deleteMany({});
  }

  async deletePrevSessionData(): Promise<boolean> {
    let getCurrentSesstion = await this.sessionModel.findOne();
    this.logger.log('getCurrentSesstion', getCurrentSesstion);
    if (!getCurrentSesstion) {
      this.logger.log('data delete for prev session 1');
      await this.destroyUserSessionData();
      return true;
    }

    let deletionTime = new Date(getCurrentSesstion.expiresAt);
    deletionTime.setMinutes(deletionTime.getMinutes() + 3.5);
    // this.logger.log('deletionTime', deletionTime);
    // if (new Date() > new Date(deletionTime)) {
    //   //Destroy session on device disconnect
    //   await this.destroyUserSessionData();
    // }
    return true;
  }

  async checkDeviceKeepAlive(): Promise<boolean> {
    try {
      let lastmessage = await this.KeepAliveModel.findOne();
      this.logger.log('lastmessage', lastmessage);
      if (!lastmessage) {
        return false;
      }

      let currentTime = new Date();
      let lastMessageTime = lastmessage.updatedAt || lastmessage.createdAt;
      // this.logger.log('lastMessageTime', lastMessageTime, currentTime);

      const timeDiff = getTimeDifference(currentTime, lastMessageTime);
      // this.logger.log('timeDiff', timeDiff);

      let diffInSec = millisToSeconds(timeDiff);
      // this.logger.log('diffInSec', diffInSec);
      //Destroy session when no message received from last 30 sec
      const sensorTimestampData = await this.sensorTimestampModel.findOne();
      const sensorStatus = sensorTimestampData?.status;
      if (Number(diffInSec) > 30 && sensorStatus?.device) {
        let payload = {
          type: 'deviceDisconnected',
        };

        this.eventEmitter.emit('onConnectDisconnect', payload);
        await this.sensorTimestampModel.findOneAndUpdate(
          {},
          {
            $set: { 'status.device': false },
          },
          {
            upsert: true,
          },
        );
        return true;
      }
    } catch (error) {
      this.logger.error('error', error.message);
      throw new InternalServerErrorException(error.message);
    }
  }

  async destroyClient(client: EventHubConsumerClient): Promise<void> {
    return await client.close();
  }

  async saveDeviceData(data): Promise<TelemetrySchema> {
    const telemory = new this.telemetryModel(data);
    let resp = await telemory.save();
    return resp;
  }

  async updateLatestSensorData(data, deviceId) {
    //update details to mongodb sensorTimestamp if keepAlive is received
    const keepAlive = await this.KeepAliveModel.findOne();
    const dataReceivedTimestamp = new Date(new Date().toUTCString());
    if (keepAlive && keepAlive.interval[0]) {
      let updateData = {};
      switch (data.type) {
        case 'heat':
          updateData = {
            $set: {
              'timestamp.heat': dataReceivedTimestamp,
              'status.heat': true,
              'status.device': true,
            },
          };
          break;

        case 'wifi':
          updateData = {
            $set: {
              'timestamp.wifi': dataReceivedTimestamp,
              'status.wifi': true,
              'status.device': true,
            },
          };

          break;

        case 'gps':
          updateData = {
            $set: {
              'timestamp.gps': dataReceivedTimestamp,
              'status.gps': true,
              'status.device': true,
            },
          };
          break;

        case 'imu':
          updateData = {
            $set: {
              'timestamp.imu': dataReceivedTimestamp,
              'status.imu': true,
              'status.device': true,
            },
          };
          break;
      }

      await this.sensorTimestampModel.findOneAndUpdate({ deviceId }, updateData, {
        upsert: true,
      });
    }
  }

  parseIoTData(data): IDeviceData {
    let payload: IDeviceData = {
      timestamp: new Date(),
    };

    this.logger.log('data=============', data);

    switch (data.msgtype) {
      case 'heat':
        payload[data.msgtype] = {
          ...data[data.msgtype],
          timestamp: new Date(data.timestamp),
        };
        payload.type = data.msgtype;
        break;
      case 'gps':
        payload[data.msgtype] = {
          ...data[data.msgtype],
          timestamp: new Date(data.timestamp),
        };
        payload.type = data.msgtype;
        break;
      case 'imu':
        payload.type = data.msgtype;
        payload['accelGyroData'] = {
          accelero: data.accelero || [],
          gyro: data.gyro || [],
          timestamp: new Date(data.timestamp),
        };
        break;
      case 'wifi':
        payload[data.msgtype] = {
          ...data[data.msgtype],
          timestamp: new Date(data.timestamp),
        };
        payload.type = data.msgtype;
        break;

      case 'keep-alive':
        const { wifiSamplingInterval, heatSamplingInterval, imuSamplingInterval, gpsSamplingInterval } = Time;
        payload.intervalData = {
          wifi: data.interval[0] * wifiSamplingInterval,
          heat: data.interval[1] * heatSamplingInterval,
          imu: data.interval[2] * imuSamplingInterval,
          gps: data.interval[3] * gpsSamplingInterval,
        };
        payload.type = data.msgtype;
        break;
    }
    return payload;
  }
}

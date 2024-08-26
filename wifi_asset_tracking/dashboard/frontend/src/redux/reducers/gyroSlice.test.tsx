import { fetchGyroData } from '../apiservice';
import gyroSlice, { resetGyro } from './gyroSlice';

interface Gyroscope {
  gyroX: Array<{ timestamp: string; x?: number }>;
  gyroY: Array<{ timestamp: string; y?: number }>;
  gyroZ: Array<{ timestamp: string; z?: number }>;
  timestamp: string;
}

const initialState: Gyroscope = {
  gyroX: [],
  gyroY: [],
  gyroZ: [],
  timestamp: '',
};

describe('gyroSlice', () => {
  it('should return the initial state', () => {
    expect(gyroSlice.reducer(undefined, { type: undefined })).toEqual(initialState);
  });

  it('should handle addDataPointGyro action', () => {
    const previousState = { ...initialState };
    const actionPayload = {
      timestamp: '2024-07-04T10:00:00Z',
      gyro: { gyro: [1, 2, 3] },
    };
    const action = gyroSlice.actions.addDataPointGyro(actionPayload);
    const newState = gyroSlice.reducer(previousState, action);

    expect(newState.gyroX).toHaveLength(1);
    expect(newState.gyroX[0]).toEqual({ timestamp: actionPayload.timestamp, x: 1 });

    expect(newState.gyroY).toHaveLength(1);
    expect(newState.gyroY[0]).toEqual({ timestamp: actionPayload.timestamp, y: 2 });

    expect(newState.gyroZ).toHaveLength(1);
    expect(newState.gyroZ[0]).toEqual({ timestamp: actionPayload.timestamp, z: 3 });

    expect(newState.timestamp).toEqual(actionPayload.timestamp);
  });

  it('should handle resetGyro action', () => {
    const modifiedState: Gyroscope = {
      gyroX: [{ timestamp: '2024-07-04T10:00:00Z', x: 1 }],
      gyroY: [{ timestamp: '2024-07-04T10:00:00Z', y: 2 }],
      gyroZ: [{ timestamp: '2024-07-04T10:00:00Z', z: 3 }],
      timestamp: '2024-07-04T10:00:00Z',
    };
    expect(gyroSlice.reducer(modifiedState, resetGyro())).toEqual(initialState);
  });

  it('should handle fetchGyroData.fulfilled', () => {
    const previousState = { ...initialState };
    const gyroData = {
      gyroX: [{ timestamp: '2024-07-04T10:00:00Z', x: 1 }],
      gyroY: [{ timestamp: '2024-07-04T10:00:00Z', y: 2 }],
      gyroZ: [{ timestamp: '2024-07-04T10:00:00Z', z: 3 }],
    };
    const action = { type: fetchGyroData.fulfilled.type, payload: gyroData };
    const newState = gyroSlice.reducer(previousState, action);

    expect(newState.gyroX).toEqual(gyroData.gyroX);
    expect(newState.gyroY).toEqual(gyroData.gyroY);
    expect(newState.gyroZ).toEqual(gyroData.gyroZ);
    expect(newState.timestamp).toEqual('2024-07-04T10:00:00Z');
  });
});

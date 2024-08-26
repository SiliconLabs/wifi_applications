import { fetchAcceleroData } from '../apiservice';
import acceleroSlice, { resetAccelero } from './acceleroSlice';

interface Accelerometer {
  acceleroX: Array<{ timestamp: string; x?: number }>;
  acceleroY: Array<{ timestamp: string; y?: number }>;
  acceleroZ: Array<{ timestamp: string; z?: number }>;
  timestamp: string;
}

const initialState: Accelerometer = {
  acceleroX: [],
  acceleroY: [],
  acceleroZ: [],
  timestamp: '',
};

describe('acceleroSlice', () => {
  it('should return the initial state', () => {
    expect(acceleroSlice.reducer(undefined, { type: undefined })).toEqual(initialState);
  });

  it('should handle addDataPointAccelero action', () => {
    const previousState = { ...initialState };
    const actionPayload = {
      timestamp: '2024-07-04T10:00:00Z',
      accelero: { accelero: [1, 2, 3] },
    };
    const action = acceleroSlice.actions.addDataPointAccelero(actionPayload);
    const newState = acceleroSlice.reducer(previousState, action);

    expect(newState.acceleroX).toHaveLength(1);
    expect(newState.acceleroX[0]).toEqual({ timestamp: actionPayload.timestamp, x: 1 });

    expect(newState.acceleroY).toHaveLength(1);
    expect(newState.acceleroY[0]).toEqual({ timestamp: actionPayload.timestamp, y: 2 });

    expect(newState.acceleroZ).toHaveLength(1);
    expect(newState.acceleroZ[0]).toEqual({ timestamp: actionPayload.timestamp, z: 3 });

    expect(newState.timestamp).toEqual(actionPayload.timestamp);
  });

  it('should handle resetAccelero action', () => {
    const modifiedState: Accelerometer = {
      acceleroX: [{ timestamp: '2024-07-04T10:00:00Z', x: 1 }],
      acceleroY: [{ timestamp: '2024-07-04T10:00:00Z', y: 2 }],
      acceleroZ: [{ timestamp: '2024-07-04T10:00:00Z', z: 3 }],
      timestamp: '2024-07-04T10:00:00Z',
    };
    expect(acceleroSlice.reducer(modifiedState, resetAccelero())).toEqual(initialState);
  });

  it('should handle fetchAcceleroData.fulfilled', () => {
    const previousState = { ...initialState };
    const acceleroData = {
      acceleroX: [{ timestamp: '2024-07-04T10:00:00Z', x: 1 }],
      acceleroY: [{ timestamp: '2024-07-04T10:00:00Z', y: 2 }],
      acceleroZ: [{ timestamp: '2024-07-04T10:00:00Z', z: 3 }],
    };
    const action = { type: fetchAcceleroData.fulfilled.type, payload: acceleroData };
    const newState = acceleroSlice.reducer(previousState, action);

    expect(newState.acceleroX).toEqual(acceleroData.acceleroX);
    expect(newState.acceleroY).toEqual(acceleroData.acceleroY);
    expect(newState.acceleroZ).toEqual(acceleroData.acceleroZ);
    expect(newState.timestamp).toEqual('2024-07-04T10:00:00Z');
  });
});

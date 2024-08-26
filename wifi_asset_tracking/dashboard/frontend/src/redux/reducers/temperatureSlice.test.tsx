import temperatureSlice, { resetTemperature } from './temperatureSlice';
import { fetchTemperatureData } from '../apiservice';

interface TemperatureWidget {
  timestamp: string;
  temperature: string;
  unit: string;
}

interface Temperature {
  temperatureWidget: TemperatureWidget;
  temperatureGraph: Array<{ timestamp: string; temperature?: number; unit?: string }>;
}

const initialState: Temperature = {
  temperatureWidget: {
    timestamp: '',
    temperature: '',
    unit: '',
  },
  temperatureGraph: [],
};

describe('temperatureSlice', () => {
  it('should return the initial state', () => {
    expect(temperatureSlice.reducer(undefined, { type: undefined })).toEqual(initialState);
  });

  it('should handle updateWidget action', () => {
    const previousState = { ...initialState };
    const actionPayload = {
      timestamp: '2024-07-04T10:00:00Z',
      temperature: '25',
      unit: 'C',
    };
    const action = temperatureSlice.actions.updateWidget(actionPayload);
    const newState = temperatureSlice.reducer(previousState, action);

    expect(newState.temperatureWidget).toEqual(actionPayload);
  });

  it('should handle addDataPoint action', () => {
    const previousState = { ...initialState };
    const actionPayload = {
      timestamp: '2024-07-04T10:00:00Z',
      temperature: 25,
    };
    const action = temperatureSlice.actions.addDataPoint(actionPayload);
    const newState = temperatureSlice.reducer(previousState, action);

    expect(newState.temperatureGraph).toHaveLength(1);
    expect(newState.temperatureGraph[0]).toEqual(actionPayload);
  });

  it('should handle resetTemperature action', () => {
    const modifiedState: Temperature = {
      temperatureWidget: {
        timestamp: '2024-07-04T10:00:00Z',
        temperature: '25',
        unit: 'C',
      },
      temperatureGraph: [{ timestamp: '2024-07-04T10:00:00Z', temperature: 25 }],
    };
    expect(temperatureSlice.reducer(modifiedState, resetTemperature())).toEqual(initialState);
  });

  it('should handle fetchTemperatureData.fulfilled', () => {
    const previousState = { ...initialState };
    const temperatureData = [
      { timestamp: '2024-07-04T09:00:00Z', temperature: 24, unit: 'C' },
      { timestamp: '2024-07-04T10:00:00Z', temperature: 25, unit: 'C' },
    ];
    const action = { type: fetchTemperatureData.fulfilled.type, payload: temperatureData };
    const newState = temperatureSlice.reducer(previousState, action);

    expect(newState.temperatureWidget.timestamp).toEqual('2024-07-04T10:00:00Z');
    expect(newState.temperatureWidget.temperature).toEqual(25);
    expect(newState.temperatureGraph).toEqual(temperatureData);
  });
});

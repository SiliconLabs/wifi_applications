import { fetchHumidityData } from '../apiservice';
import humiditySlice, { resetHumidity } from './humiditySlice';
interface HumidityWidget {
  timestamp: string;
  humidity: string;
}

interface Humidity {
  humidityWidget: HumidityWidget;
  humidityGraph: Array<{ timestamp: string; humidity?: number }>;
}

const initialState: Humidity = {
  humidityWidget: {
    timestamp: '',
    humidity: '',
  },
  humidityGraph: [],
};

describe('humiditySlice', () => {
  it('should return the initial state', () => {
    expect(humiditySlice.reducer(undefined, { type: undefined })).toEqual(initialState);
  });

  it('should handle updateWidget action', () => {
    const previousState = { ...initialState };
    const actionPayload = {
      timestamp: '2024-07-04T10:00:00Z',
      humidity: '75',
    };
    const action = humiditySlice.actions.updateWidget(actionPayload);
    const newState = humiditySlice.reducer(previousState, action);

    expect(newState.humidityWidget).toEqual(actionPayload);
  });

  it('should handle addDataPoint action', () => {
    const previousState = { ...initialState };
    const actionPayload = {
      timestamp: '2024-07-04T10:00:00Z',
      humidity: 75,
    };
    const action = humiditySlice.actions.addDataPoint(actionPayload);
    const newState = humiditySlice.reducer(previousState, action);

    expect(newState.humidityGraph).toHaveLength(1);
    expect(newState.humidityGraph[0]).toEqual(actionPayload);
  });

  it('should handle resetHumidity action', () => {
    const modifiedState: Humidity = {
      humidityWidget: {
        timestamp: '2024-07-04T10:00:00Z',
        humidity: '75',
      },
      humidityGraph: [{ timestamp: '2024-07-04T10:00:00Z', humidity: 75 }],
    };
    expect(humiditySlice.reducer(modifiedState, resetHumidity())).toEqual(initialState);
  });

  it('should handle fetchHumidityData.fulfilled', () => {
    const previousState = { ...initialState };
    const humidityData = [
      { timestamp: '2024-07-04T09:00:00Z', humidity: 70 },
      { timestamp: '2024-07-04T10:00:00Z', humidity: 75 },
    ];
    const action = { type: fetchHumidityData.fulfilled.type, payload: humidityData };
    const newState = humiditySlice.reducer(previousState, action);

    expect(newState.humidityWidget.timestamp).toEqual('2024-07-04T10:00:00Z');
    expect(newState.humidityGraph).toEqual(humidityData);
  });
});

import { fetchWifiData } from '../apiservice';
import wifiSlice, { resetWifi } from './wifiSlice';

interface Wifi {
  timestamp: string;
  device: string;
  strength: string;
}

const initialState: Wifi = {
  timestamp: '',
  device: '',
  strength: '',
};

describe('wifiSlice', () => {
  it('should return the initial state', () => {
    expect(wifiSlice.reducer(undefined, { type: undefined })).toEqual(initialState);
  });

  it('should handle updateWifiWidget action', () => {
    const previousState = { ...initialState };
    const actionPayload = {
      timestamp: '2024-07-04T10:00:00Z',
      ssid: 'MyWifiNetwork',
      rssi: '-70dBm',
    };
    const action = wifiSlice.actions.updateWifiWidget(actionPayload);
    const newState = wifiSlice.reducer(previousState, action);

    expect(newState).toEqual({
      timestamp: '2024-07-04T10:00:00Z',
      device: 'MyWifiNetwork',
      strength: '-70dBm',
    });
  });

  it('should handle resetWifi action', () => {
    const modifiedState: Wifi = {
      timestamp: '2024-07-04T10:00:00Z',
      device: 'MyWifiNetwork',
      strength: '-70dBm',
    };
    expect(wifiSlice.reducer(modifiedState, resetWifi())).toEqual(initialState);
  });
  it('should handle fetchWifiData.fulfilled', () => {
    const previousState = { ...initialState };
    const wifiData = [
      { timestamp: '2024-07-04T09:00:00Z', ssid: 'Network1', rssi: '-60dBm' },
      { timestamp: '2024-07-04T10:00:00Z', ssid: 'MyWifiNetwork', rssi: '-70dBm' },
    ];
    const action = { type: fetchWifiData.fulfilled.type, payload: wifiData };
    const newState = wifiSlice.reducer(previousState, action);

    expect(newState).toEqual({
      timestamp: '2024-07-04T10:00:00Z',
      device: 'MyWifiNetwork',
      strength: '-70dBm',
    });
  });
});

import { createSlice } from '@reduxjs/toolkit';
import { fetchWifiData } from '../apiservice';

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

const wifiSlice = createSlice({
  name: 'wifi',
  initialState,
  reducers: {
    updateWifiWidget: (state, action) => {
      const { timestamp, ssid, rssi } = action.payload;
      state.timestamp = timestamp;
      state.device = ssid;
      state.strength = rssi;
    },
    resetWifi: () => {
      return initialState;
    },
  },
  extraReducers: (builder) => {
    builder.addCase(fetchWifiData.fulfilled, (state, action) => {
      const wifiData = action.payload;
      const currentData = wifiData.slice(-1)[0];
      if (currentData) {
        state.timestamp = currentData.timestamp;
        state.device = currentData.ssid;
        state.strength = currentData.rssi;
      }
    });
  },
});

export default wifiSlice;
export const { resetWifi } = wifiSlice.actions;

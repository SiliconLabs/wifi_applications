// store/index.ts
import { createSlice } from '@reduxjs/toolkit';
import { fetchHumidityData } from '../apiservice';

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

const humiditySlice = createSlice({
  name: 'humidity',
  initialState,
  reducers: {
    updateWidget: (state, action) => {
      const { timestamp, humidity } = action.payload;
      state.humidityWidget.timestamp = timestamp;
      state.humidityWidget.humidity = humidity;
    },
    addDataPoint: (state, action) => {
      const { timestamp, humidity } = action.payload;
      state.humidityGraph.push({ timestamp, humidity });
      while (state.humidityGraph.length > 10) {
        state.humidityGraph.shift();
      }
    },
    resetHumidity: () => {
      return initialState;
    },
  },
  extraReducers: (builder) => {
    builder.addCase(fetchHumidityData.fulfilled, (state, action) => {
      const humidityData = action.payload;
      const currentData = humidityData.slice(-1)[0];
      if (currentData) {
        state.humidityWidget.timestamp = currentData.timestamp;
        state.humidityWidget.humidity = currentData.humidity;
        state.humidityGraph = humidityData;
      }
    });
  },
});

export default humiditySlice;
export const { resetHumidity } = humiditySlice.actions;

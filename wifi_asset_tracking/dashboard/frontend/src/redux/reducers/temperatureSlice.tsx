// store/index.ts
import { createSlice } from '@reduxjs/toolkit';
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

const temperatureSlice = createSlice({
  name: 'temperature',
  initialState,
  reducers: {
    updateWidget: (state, action) => {
      const { timestamp, temperature, unit } = action.payload;
      state.temperatureWidget.timestamp = timestamp;
      state.temperatureWidget.temperature = temperature;
      state.temperatureWidget.unit = unit;
    },
    addDataPoint: (state, action) => {
      const { timestamp, temperature } = action.payload;
      state.temperatureGraph.push({ timestamp, temperature });
      while (state.temperatureGraph.length > 10) {
        state.temperatureGraph.shift();
      }
    },
    resetTemperature: () => {
      return initialState;
    },
  },
  extraReducers: (builder) => {
    builder.addCase(fetchTemperatureData.fulfilled, (state, action) => {
      const temperatureData = action.payload;
      const currentData = temperatureData.slice(-1)[0];
      if (currentData) {
        state.temperatureWidget.timestamp = currentData.timestamp;
        state.temperatureWidget.temperature = currentData.temperature;
        state.temperatureWidget.unit = currentData.temperature.unit;
        state.temperatureGraph = temperatureData;
      }
    });
  },
});

export default temperatureSlice;
export const { resetTemperature } = temperatureSlice.actions;

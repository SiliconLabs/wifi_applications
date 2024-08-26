// store/index.ts
import { createSlice } from '@reduxjs/toolkit';
import { fetchAcceleroData } from '../apiservice';

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

const acceleroSlice = createSlice({
  name: 'accelero',
  initialState,
  reducers: {
    addDataPointAccelero: (state, action) => {
      const { timestamp, accelero } = action.payload;
      state.acceleroX.push({ timestamp, x: accelero.accelero[0] });
      state.acceleroY.push({ timestamp, y: accelero.accelero[1] });
      state.acceleroZ.push({ timestamp, z: accelero.accelero[2] });
      while (state.acceleroX.length > 10) {
        state.acceleroX.shift();
      }
      while (state.acceleroY.length > 10) {
        state.acceleroY.shift();
      }
      while (state.acceleroZ.length > 10) {
        state.acceleroZ.shift();
      }
      state.timestamp = timestamp;
    },
    resetAccelero: () => {
      return initialState;
    },
  },
  extraReducers: (builder) => {
    builder.addCase(fetchAcceleroData.fulfilled, (state, action) => {
      const acceleroData = action.payload;
      state.acceleroX = acceleroData.acceleroX;
      state.acceleroY = acceleroData.acceleroY;
      state.acceleroZ = acceleroData.acceleroZ;
      const currentData = acceleroData.acceleroX.slice(-1)[0];
      if (currentData) {
        state.timestamp = currentData.timestamp;
      }
    });
  },
});

export default acceleroSlice;
export const { resetAccelero } = acceleroSlice.actions;

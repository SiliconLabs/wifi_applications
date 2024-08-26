// store/index.ts
import { createSlice } from '@reduxjs/toolkit';
import { fetchGyroData } from '../apiservice';

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

const gyroSlice = createSlice({
  name: 'gyro',
  initialState,
  reducers: {
    addDataPointGyro: (state, action) => {
      const { timestamp, gyro } = action.payload;
      state.gyroX.push({ timestamp, x: gyro.gyro[0] });
      state.gyroY.push({ timestamp, y: gyro.gyro[1] });
      state.gyroZ.push({ timestamp, z: gyro.gyro[2] });
      while (state.gyroX.length > 10) {
        state.gyroX.shift();
      }
      while (state.gyroY.length > 10) {
        state.gyroY.shift();
      }
      while (state.gyroZ.length > 10) {
        state.gyroZ.shift();
      }
      state.timestamp = timestamp;
    },
    resetGyro: () => {
      return initialState;
    },
  },
  extraReducers: (builder) => {
    builder.addCase(fetchGyroData.fulfilled, (state, action) => {
      const gyroData = action.payload;
      state.gyroX = gyroData.gyroX;
      state.gyroY = gyroData.gyroY;
      state.gyroZ = gyroData.gyroZ;
      const currentData = gyroData.gyroX.slice(-1)[0];
      if (currentData) {
        state.timestamp = currentData.timestamp;
      }
    });
  },
});

export default gyroSlice;
export const { resetGyro } = gyroSlice.actions;

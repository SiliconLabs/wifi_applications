import { createSlice } from '@reduxjs/toolkit';
import { fetchGpsData } from '../apiservice';

interface SatelliteWidget {
  timestamp: string;
  satellites: string;
}

interface LatLongWidget {
  lat: number;
  long: number;
}

interface Gps {
  satelliteWidget: SatelliteWidget;
  latLongWidget: LatLongWidget;
  elevationGraph: Array<{ timestamp: string; altitude?: number }>;
}

const initialState: Gps = {
  satelliteWidget: {
    timestamp: '',
    satellites: '',
  },
  latLongWidget: {
    lat: 0.0,
    long: 0.0,
  },
  elevationGraph: [],
};

const gpsSlice = createSlice({
  name: 'gps',
  initialState,
  reducers: {
    updateSatellite: (state, action) => {
      const { timestamp, gps } = action.payload;
      state.satelliteWidget.timestamp = timestamp;
      state.satelliteWidget.satellites = gps.satellites;
    },
    updateLatLong: (state, action) => {
      const { timestamp, gps } = action.payload;
      state.satelliteWidget.timestamp = timestamp;
      state.latLongWidget.lat = gps.latitude;
      state.latLongWidget.long = gps.longitude;
    },
    updateElevation: (state, action) => {
      const { timestamp, altitude } = action.payload;
      state.elevationGraph.push({ timestamp, altitude });
      while (state.elevationGraph.length > 10) {
        state.elevationGraph.shift();
      }
    },
    resetGps: () => {
      return initialState;
    },
  },
  extraReducers: (builder) => {
    builder.addCase(fetchGpsData.fulfilled, (state, action) => {
      const gpsData = action.payload;
      const currentData = gpsData.slice(-1)[0];
      const filteredData = gpsData.map((obj: NonNullable<unknown>) => {
        const { ...rest } = obj;
        return rest;
      });
      if (currentData) {
        state.satelliteWidget.timestamp = currentData.timestamp;
        state.satelliteWidget.satellites = currentData.satellites;
        state.latLongWidget.lat = currentData.latitude;
        state.latLongWidget.long = currentData.longitude;
        state.elevationGraph = filteredData;
      }
    });
  },
});

export default gpsSlice;
export const { resetGps } = gpsSlice.actions;

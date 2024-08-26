import gpsSlice, { resetGps } from './gpsSlice';
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

describe('gpsSlice', () => {
  it('should return the initial state', () => {
    expect(gpsSlice.reducer(undefined, { type: undefined })).toEqual(initialState);
  });

  it('should handle updateSatellite action', () => {
    const previousState = { ...initialState };
    const actionPayload = {
      timestamp: '2024-07-04T10:00:00Z',
      gps: { satellites: '10' },
    };
    const action = gpsSlice.actions.updateSatellite(actionPayload);
    const newState = gpsSlice.reducer(previousState, action);
    expect(newState.satelliteWidget.timestamp).toEqual(actionPayload.timestamp);
    expect(newState.satelliteWidget.satellites).toEqual(actionPayload.gps.satellites);
  });

  it('should handle updateLatLong action', () => {
    const previousState = { ...initialState };
    const actionPayload = {
      timestamp: '2024-07-04T10:00:00Z',
      gps: { latitude: 12.34, longitude: 56.78 },
    };
    const action = gpsSlice.actions.updateLatLong(actionPayload);
    const newState = gpsSlice.reducer(previousState, action);
    expect(newState.satelliteWidget.timestamp).toEqual(actionPayload.timestamp);
    expect(newState.latLongWidget.lat).toEqual(actionPayload.gps.latitude);
    expect(newState.latLongWidget.long).toEqual(actionPayload.gps.longitude);
  });

  it('should handle updateElevation action', () => {
    const previousState = { ...initialState };
    const actionPayload = {
      timestamp: '2024-07-04T10:00:00Z',
      altitude: 1234,
    };
    const action = gpsSlice.actions.updateElevation(actionPayload);
    const newState = gpsSlice.reducer(previousState, action);
    expect(newState.elevationGraph).toHaveLength(1);
    expect(newState.elevationGraph[0]).toEqual(actionPayload);
  });

  it('should handle resetGps action', () => {
    const modifiedState = {
      satelliteWidget: { timestamp: '2024-07-04T10:00:00Z', satellites: '10' },
      latLongWidget: { lat: 12.34, long: 56.78 },
      elevationGraph: [{ timestamp: '2024-07-04T10:00:00Z', altitude: 1234 }],
    };
    expect(gpsSlice.reducer(modifiedState, resetGps())).toEqual(initialState);
  });

  it('should handle fetchGpsData.fulfilled', () => {
    const previousState = { ...initialState };
    const gpsData = [
      { timestamp: '2024-07-04T10:00:00Z', satellites: '10', latitude: 12.34, longitude: 56.78, altitude: 1234 },
      { timestamp: '2024-07-04T10:05:00Z', satellites: '8', latitude: 12.35, longitude: 56.79, altitude: 1235 },
    ];
    const action = { type: fetchGpsData.fulfilled.type, payload: gpsData };
    const newState = gpsSlice.reducer(previousState, action);
    expect(newState.satelliteWidget.timestamp).toEqual('2024-07-04T10:05:00Z');
    expect(newState.satelliteWidget.satellites).toEqual('8');
    expect(newState.latLongWidget.lat).toEqual(12.35);
    expect(newState.latLongWidget.long).toEqual(56.79);
  });
});

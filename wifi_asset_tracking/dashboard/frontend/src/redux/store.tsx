// store/index.ts
import { configureStore } from '@reduxjs/toolkit';
import temperatureSlice from './reducers/temperatureSlice';
import humiditySlice from './reducers/humiditySlice';
import gpsSlice from './reducers/gpsSlice';
import wifiSlice from './reducers/wifiSlice';
import accleroSlice from './reducers/acceleroSlice';
import gyroSlice from './reducers/gyroSlice';
import authSlice from './reducers/authSlice';
import alertSlice from './reducers/alertSlice';
import { thunk } from 'redux-thunk';
import { useDispatch } from 'react-redux';

const store = configureStore({
  reducer: {
    temperature: temperatureSlice.reducer,
    humidity: humiditySlice.reducer,
    gps: gpsSlice.reducer,
    wifi: wifiSlice.reducer,
    accelero: accleroSlice.reducer,
    gyro: gyroSlice.reducer,
    auth: authSlice.reducer,
    alert: alertSlice.reducer,
  },
  middleware: (getDefaultMiddleware) =>
    getDefaultMiddleware({
      serializableCheck: false,
    }).concat(thunk),
});

export type RootState = ReturnType<typeof store.getState>;
export type AppDispatch = typeof store.dispatch;
export const useAppDispatch: () => AppDispatch = useDispatch;

export default store;

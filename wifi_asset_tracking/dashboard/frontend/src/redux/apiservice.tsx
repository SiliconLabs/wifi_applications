import { UnknownAction, createAsyncThunk } from '@reduxjs/toolkit';
import axios from 'axios';
import {
  API_TEMPERATURE,
  API_HUMIDITY,
  API_GPS,
  API_WIFI,
  API_ACCELERO,
  API_GYRO,
  API_GPX,
  API_CSV,
  SOCKET_URL,
  API_LOGOUT,
  API_LOGIN,
} from './constants';
import { Socket, io } from 'socket.io-client';
import { getCookie, processDataWithCurrentTimeStamp } from '../utilities/common';
import { toast } from 'react-toastify';
import 'react-toastify/dist/ReactToastify.css';
import { resetTemperature } from './reducers/temperatureSlice';
import { resetAccelero } from './reducers/acceleroSlice';
import { resetAuth } from './reducers/authSlice';
import { resetGps } from './reducers/gpsSlice';
import { resetHumidity } from './reducers/humiditySlice';
import { resetGyro } from './reducers/gyroSlice';
import { resetWifi } from './reducers/wifiSlice';
import { AppDispatch } from './store';
import {
  hideDevicePopUp,
  hideSessionEndPopUp,
  hideSocketErrorPopUp,
  setError,
  showSocketErrorPopUp,
  updateConnectionStatus,
} from './reducers/alertSlice';

let initialLoad = true;
let socket: Socket | null = null;

export const fetchTemperatureData = createAsyncThunk('temperature/fetchTemperatureData', async () => {
  const response = await axios.get(API_TEMPERATURE, {
    withCredentials: true,
  });
  return response.data.Data;
});

export const fetchHumidityData = createAsyncThunk('humidity/fetchHumidityData', async () => {
  const response = await axios.get(API_HUMIDITY, {
    withCredentials: true,
  });
  return response.data.Data;
});

export const fetchGpsData = createAsyncThunk('gps/fetchGpsData', async () => {
  const response = await axios.get(API_GPS, {
    withCredentials: true,
  });
  return response.data.Data;
});

export const fetchWifiData = createAsyncThunk('wifi/fetchWifiData', async () => {
  const response = await axios.get(API_WIFI, {
    withCredentials: true,
  });
  return response.data.Data;
});

export const fetchAcceleroData = createAsyncThunk('accelero/fetchAcceleroData', async () => {
  const response = await axios.get(API_ACCELERO, {
    withCredentials: true,
  });
  return response.data.Data;
});

export const fetchGyroData = createAsyncThunk('gyro/fetchGyroData', async () => {
  const response = await axios.get(API_GYRO, {
    withCredentials: true,
  });
  return response.data.Data;
});

export const fetchAccessToken = createAsyncThunk('auth/fetchAccessToken', async (response: UnknownAction) => {
  try {
    const responses = await fetch(API_LOGIN, {
      method: 'GET',
      headers: {
        Authorization: `${response.idToken}`,
      },
      credentials: 'include',
    });

    if (!responses.ok) {
      const data = await responses.json();
      toast.error(data.Message, {
        pauseOnHover: false,
        autoClose: 1000,
        position: 'top-center',
      });
      throw new Error(data.Message || 'Failed to fetch access token');
    }

    const data = await responses.json();
    toast.success('Login Successfully !', {
      pauseOnHover: false,
      autoClose: 1000,
      position: 'top-center',
    });
    return data;
  } catch (error: any) {
    if (!error.include('session is already active')) {
      toast.error('Network error: Failed to connect to the server', {
        pauseOnHover: false,
        autoClose: 1000,
        position: 'top-center',
      });
      throw error;
    }
  }
});

export const updateLogin = (token: string) => ({
  type: 'auth/login',
  payload: { token },
});

export const tickSession = (remainingTime: number) => {
  return {
    type: 'auth/ticksession',
    payload: { remainingTime },
  };
};

export const updateDevice = (message: boolean) => ({
  type: 'device/deviceStatus',
  payload: { message },
});

export const logoutSession = createAsyncThunk('auth/logoutSession', async (Message: string, { dispatch }) => {
  const responses = await fetch(API_LOGOUT, {
    method: 'GET',
    credentials: 'include',
  });

  if (!responses.ok) {
    const data = await responses.json();
    throw new Error(data.message || 'Failed to fetch access token');
  }
  toast.success(Message, {
    pauseOnHover: false,
    autoClose: 1000,
    position: 'top-center',
  });
  localStorage.clear();
  document.cookie ="access_token=; expires=Thu, 01 Jan 1970 00:00:00 UTC; path=/;";
  dispatch(hideDevicePopUp());
  dispatch(hideSessionEndPopUp());
  dispatch(resetTemperature());
  dispatch(resetHumidity());
  dispatch(resetAuth());
  dispatch(resetGps());
  dispatch(resetWifi());
  dispatch(resetGyro());
  dispatch(resetAccelero());
  localStorage.setItem('loginStatus', 'logout');
  if (socket) {
    socket.disconnect();
    socket = null;
  }
});

export const updateTemperatureWidget = (timestamp: string, temperature: number, unit: string) => ({
  type: 'temperature/updateWidget',
  payload: { timestamp, temperature, unit },
});

export const addTemperatureDataPoint = (timestamp: string, temperature: number) => ({
  type: 'temperature/addDataPoint',
  payload: { timestamp, temperature },
});

export const updateHumidityWidget = (timestamp: string, humidity: number) => ({
  type: 'humidity/updateWidget',
  payload: { timestamp, humidity },
});

export const updateSatelliteWidget = (timestamp: string, gps: unknown) => ({
  type: 'gps/updateSatellite',
  payload: { timestamp, gps },
});

export const updateLatLongWidget = (timestamp: string, gps: unknown) => ({
  type: 'gps/updateLatLong',
  payload: { timestamp, gps },
});

export const updateElevationGraph = (timestamp: string, altitude: number) => ({
  type: 'gps/updateElevation',
  payload: { timestamp, altitude },
});

export const addHumidityDataPoint = (timestamp: string, humidity: number) => ({
  type: 'humidity/addDataPoint',
  payload: { timestamp, humidity },
});
export const updateWifiWidgetData = (timestamp: string, ssid: string, rssi: number) => ({
  type: 'wifi/updateWifiWidget',
  payload: { timestamp, ssid, rssi },
});

export const addAcceleroDataPoint = (timestamp: string, accelero: unknown) => ({
  type: 'accelero/addDataPointAccelero',
  payload: { timestamp, accelero },
});

export const addGyroDataPoint = (timestamp: string, gyro: unknown) => ({
  type: 'gyro/addDataPointGyro',
  payload: { timestamp, gyro },
});

export const downloadGPXandCSV = createAsyncThunk('', async (_, { dispatch, rejectWithValue }) => {
  try {
    await dispatch(downloadGPX()).unwrap();
    await dispatch(downloadCSV()).unwrap();
    await dispatch(logoutSession('Successfully logout')).unwrap();
  } catch (error: any) {
    console.error('Error during download and logout process:', error);
    return rejectWithValue(error.response?.data || error.message);
  }
});

export const downloadGPX = createAsyncThunk('gpx/download', async () => {
  fetch(API_GPX, {
    credentials: 'include',
  })
    .then((response) => {
      if (response.ok) {
        return response.blob();
      } else {
        return response.json();
      }
    })
    .then((blob) => {
      if (blob.IsSuccess === false) {
        throw new Error(blob?.Message);
      }
      const url = window.URL.createObjectURL(new Blob([blob]));
      const link = document.createElement('a');
      link.href = url;
      link.setAttribute('download', 'GPXInformation.gpx'); // Replace with your file name
      document.body.appendChild(link);
      link.click();
      return Promise.resolve();
    })
    .then(() => {
      // toast.success('File downloaded successfully!');
    })
    .catch((error) => {
      console.error('Error downloading file:', error);
      toast.error(`${error}`, {
        pauseOnHover: false,
        autoClose: 1000,
      });
    });
});

export const downloadCSV = createAsyncThunk('csv/download', async () => {
  try {
    const response = await fetch(API_CSV, {
      credentials: 'include',
    });

    if (!response.ok) {
      const json = await response.json();
      throw new Error(json?.Message);
    }
    const blob = await response.blob();
    const url = window.URL.createObjectURL(new Blob([blob]));
    const link = document.createElement('a');
    link.href = url;
    link.target = '_zblank';
    link.setAttribute('download', 'SessionData.xlsx');
    document.body.appendChild(link);
    link.click();
    return Promise.resolve();
  } catch (error: unknown) {
    console.error('Error downloading file:', error);
    toast.error(`${error}`, {
      pauseOnHover: false,
      autoClose: 1000,
    });
  }
});

export const fetchAllData = () => {
  return (dispatch: AppDispatch) => {
    if (initialLoad) {
      if (!localStorage.getItem('heatDisconnect')) {
        dispatch(fetchTemperatureData());
        dispatch(fetchHumidityData());
      }
      if (!localStorage.getItem('gpsDisconnect')) {
        dispatch(fetchGpsData());
      }
      if (!localStorage.getItem('wifiDisconnect')) {
        dispatch(fetchWifiData());
      }
      if (!localStorage.getItem('imuDisconnect')) {
        dispatch(fetchAcceleroData());
        dispatch(fetchGyroData());
      }
      initialLoad = false;
    } else {
      const token = getCookie('access_token');
      if (socket) {
        socket.disconnect();
        socket = null;
      }
      socket = io(SOCKET_URL, {
        extraHeaders: {
          authentication: `Bearer ${token}`,
        },
      });

      // We are maintaining the networt disconnect pop-up

      let disconnectTimeout: number | null | undefined;

      socket.on('connect', () => {
        if (disconnectTimeout) {
          clearTimeout(disconnectTimeout);
          disconnectTimeout = null;
        }
        localStorage.setItem('loginStatus', '');
        dispatch(hideSocketErrorPopUp());
      });

      socket.on('disconnect', () => {
        const loginStatus = localStorage.getItem('loginStatus');
        disconnectTimeout = setTimeout(() => {
          if (loginStatus !== 'logout') {
            dispatch(showSocketErrorPopUp());
            dispatch(setError('The Backend Server has Disconnected'));
            dispatch(updateConnectionStatus('Disconnected'));
          }
        }, 3000);
      });

      socket.on('receiveMessage', (resp) => {
        const value = processDataWithCurrentTimeStamp(resp?.timestamp);
        localStorage.setItem('disconnect', '');
        localStorage.setItem('network', '');
        localStorage.setItem('iot', '');
        if (value) {
          if (resp.type == 'heat') {
            localStorage.setItem('heatDisconnect', '');
            dispatch(
              updateTemperatureWidget(resp.heat.timestamp, resp.heat.temperature.value, resp.heat.temperature.unit)
            );
            dispatch(addTemperatureDataPoint(resp.heat.timestamp, resp.heat.temperature.value));
            dispatch(updateHumidityWidget(resp.heat.timestamp, resp.heat.humidity));
            dispatch(addHumidityDataPoint(resp.heat.timestamp, resp.heat.humidity));
          }
          if (resp.type == 'gps') {
            localStorage.setItem('gpsDisconnect', '');
            dispatch(updateSatelliteWidget(resp.gps.timestamp, resp.gps));
            dispatch(updateLatLongWidget(resp.gps.timestamp, resp.gps));
            dispatch(updateElevationGraph(resp.gps.timestamp, resp.gps.altitude));
          }
          if (resp.type == 'wifi') {
            localStorage.setItem('wifiDisconnect', '');
            dispatch(updateWifiWidgetData(resp.wifi.timestamp, resp.wifi.ssid, resp.wifi.rssi));
          }
          if (resp.type == 'imu') {
            localStorage.setItem('imuDisconnect', '');
            dispatch(addAcceleroDataPoint(resp.accelGyroData.timestamp, resp.accelGyroData));
            dispatch(addGyroDataPoint(resp.accelGyroData.timestamp, resp.accelGyroData));
          }
        }
      });

      socket.on('onConnectDisconnect', (resp) => {
        if (resp.type == 'deviceDisconnected') {
          dispatch(resetTemperature());
          dispatch(resetHumidity());
          dispatch(resetGps());
          dispatch(resetWifi());
          dispatch(resetGyro());
          dispatch(resetAccelero());
          // if (socket) {
          //   socket.disconnect();
          //   socket = null;
          // }
          const connected = 'Disconnected';
          localStorage.setItem('disconnect', connected);
        }
        if (resp.type == 'network error') {
          const netError = 'networkError';
          localStorage.setItem('network', netError);
        }
        if (resp.type == 'iot connection timeout') {
          const iotError = 'iotError';
          localStorage.setItem('iot', iotError);
        }
      });

      socket.on('onSensorDisconnect', (resp) => {
        if (resp.type == 'heat') {
          localStorage.setItem('heatDisconnect', 'heat');
          dispatch(resetTemperature());
          dispatch(resetHumidity());
        }
        if (resp.type == 'gps') {
          localStorage.setItem('gpsDisconnect', 'gps');
          dispatch(resetGps());
        }
        if (resp.type == 'wifi') {
          localStorage.setItem('wifiDisconnect', 'wifi');
          dispatch(resetWifi());
        }
        if (resp.type == 'imu') {
          localStorage.setItem('imuDisconnect', 'imu');
          dispatch(resetAccelero());
          dispatch(resetGyro());
        }
      });
    }
  };
};

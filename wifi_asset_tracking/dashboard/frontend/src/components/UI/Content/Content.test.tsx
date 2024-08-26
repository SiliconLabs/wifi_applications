import { render } from '@testing-library/react';
import Content from './Content';
import { Provider } from 'react-redux';
import configureStore from 'redux-mock-store';
import { thunk } from 'redux-thunk';

const initialState = {
  temperature: {
    temperatureWidget: {
      timestamp: 1623902592000,
      temperature: 25,
    },
    temperatureGraph: [],
  },
  humidity: {
    humidityWidget: {
      timestamp: 1623902592000,
      humidity: 50,
    },
    humidityGraph: [],
  },
  gps: {
    satelliteWidget: {
      satellites: 8,
      timestamp: 1623902592000,
    },
    latLongWidget: {
      lat: 37.7749,
      long: -122.4194,
    },
    elevationGraph: [],
  },
  accelero: {
    acceleroX: [],
    acceleroY: [],
    acceleroZ: [],
    timestamp: 1623902592000,
  },
  gyro: {
    gyroX: [],
    gyroY: [],
    gyroZ: [],
    timestamp: 1623902592000,
  },
  wifi: {
    timestamp: 1623902592000,
    device: 'WiFi Device',
    strength: 80,
  },
};
const mockStore = configureStore([thunk]);
const store = mockStore(initialState);

// Test case
test('renders Content component', () => {
  render(
    <Provider store={store}>
      <Content />
    </Provider>
  );
});

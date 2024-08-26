import { render, screen } from '@testing-library/react';
import { Provider } from 'react-redux';
import configureMockStore from 'redux-mock-store';
import { thunk } from 'redux-thunk';
import AlertTimeout from './AlertTimout'; // Ensure this import path is correct
import { vi } from 'vitest';
import '@testing-library/jest-dom';

const mockStore = configureMockStore([thunk]);

vi.mock('moment', async () => {
  const actualMoment = await vi.importActual('moment');
  return {
    ...actualMoment,
    unix: vi.fn(() => 1620000000), // mock a fixed unix time for testing
  };
});

vi.mock('../../../redux/reducers/alertSlice', async () => {
  const originalModule = await vi.importActual('../../../redux/reducers/alertSlice');
  return {
    ...originalModule,
    showSessionPopUp: vi.fn(),
    hideSessionPopUp: vi.fn(),
    hideDevicePopUp: vi.fn(),
    showDevicePopUp: vi.fn(),
  };
});

vi.mock('../../../redux/apiservice', async () => {
  const actual = await vi.importActual('../../../redux/apiservice');
  return {
    ...actual,
    logoutSession: vi.fn(),
    fetchTemperatureData: vi.fn(), // ensure this is correctly mocked
  };
});

describe('AlertTimeout', () => {
  let store: ReturnType<typeof mockStore>;

  beforeEach(() => {
    store = mockStore({
      temperature: {}, // Replace with appropriate initial state
      humidity: {}, // Replace with appropriate initial state
      gps: {}, // Replace with appropriate initial state
      wifi: {}, // Replace with appropriate initial state
      accelero: {}, // Replace with appropriate initial state
      gyro: {}, // Replace with appropriate initial state
      auth: {}, // Replace with appropriate initial state
      alert: {
        showAlert: false,
        showDeviceAlert: false,
      },
    });

    // Mock localStorage
    vi.spyOn(Storage.prototype, 'getItem').mockImplementation((key: string) => {
      if (key === 'expireSession') return '1620000300'; // mock expiry time 5 mins later
      if (key === 'disconnect') return 'Disconnected';
      if (key === 'expiryTimeDisconnect') return '1620000180'; // mock expiry time 3 mins later
      return null;
    });
    vi.spyOn(Storage.prototype, 'setItem').mockImplementation(() => {});
  });

  afterEach(() => {
    vi.clearAllMocks();
  });

  test('renders AlertTimeout component', async () => {
    store = mockStore({
      ...store,
      alert: {
        showAlert: true,
        showDeviceAlert: false,
      },
    });
    render(
      <Provider store={store}>
        <AlertTimeout />
      </Provider>
    );
    expect(await screen.findByText(/Reminder/)).toBeInTheDocument();
  });

  test('shows session modal when session is about to expire', async () => {
    store = mockStore({
      ...store,
      alert: {
        showAlert: true,
        showDeviceAlert: false,
      },
    });

    render(
      <Provider store={store}>
        <AlertTimeout />
      </Provider>
    );
    expect(await screen.findByText(/Your session is about to end in/)).toBeInTheDocument();
  });

  test('shows device modal when device is disconnected', async () => {
    store = mockStore({
      ...store,
      alert: {
        showAlert: false,
        showDeviceAlert: true,
      },
    });

    render(
      <Provider store={store}>
        <AlertTimeout />
      </Provider>
    );
    expect(await screen.findByText(/Your Wi-Fi Device is disconnected/)).toBeInTheDocument();
  });

  test('calls logoutSession when session expires', async () => {
    vi.useFakeTimers();

    render(
      <Provider store={store}>
        <AlertTimeout />
      </Provider>
    );
  });
});

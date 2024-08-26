import { render, screen, fireEvent } from '@testing-library/react';
import { Provider } from 'react-redux';
import configureStore from 'redux-mock-store';
import Header from './Header';
import { downloadCSV, downloadGPX, logoutSession } from '../../../redux/apiservice';
import { vi } from 'vitest';
import '@testing-library/jest-dom';
import { BrowserRouter as Router } from 'react-router-dom';

const mockStore = configureStore([]);
vi.mock('../../../redux/apiservice', async () => {
  const actual = await vi.importActual('../../../redux/apiservice');
  return {
    ...actual,
    downloadCSV: vi.fn(),
    downloadGPX: vi.fn(),
    logoutSession: vi.fn(),
  };
});

vi.mock('../../../utilities/common', () => ({
  getCookie: vi.fn(() => 'fake-token'),
  parseJwt: vi.fn(() => ({ payload: { username: 'testuser' } })),
  truncateString: vi.fn((str: string, num: number) => (str.length > num ? `${str.substring(0, num)}...` : str)),
}));

describe('Header component', () => {
  let store: ReturnType<typeof mockStore>;

  beforeEach(() => {
    store = mockStore({
      auth: {
        timeleft: 360, // 6 minutes
      },
    });
    store.dispatch = vi.fn();
  });

  test('renders Header component and checks elements', () => {
    render(
      <Provider store={store}>
        <Router>
          <Header />
        </Router>
      </Provider>
    );

    expect(screen.getByAltText('dashboard-logo')).toBeInTheDocument();
    expect(screen.getByText('Session Timer :')).toBeInTheDocument();
    expect(screen.getByText('06:00')).toBeInTheDocument(); // Check if the session timer is formatted correctly
  });

  test('toggles dropdown menu', () => {
    render(
      <Provider store={store}>
        <Router>
          <Header />
        </Router>
      </Provider>
    );

    const usernameLink = screen.getByTitle('testuser');
    fireEvent.click(usernameLink);
    expect(screen.getByText('Logout')).toBeInTheDocument();
    fireEvent.click(usernameLink);
    expect(screen.queryByText('Logout')).not.toBeInTheDocument();
  });

  test('shows and hides logout modal', () => {
    render(
      <Provider store={store}>
        <Router>
          <Header />
        </Router>
      </Provider>
    );

    const usernameLink = screen.getByTitle('testuser');
    fireEvent.click(usernameLink);
    fireEvent.click(screen.getByText('Logout'));
    expect(screen.getByText('Confirm Logout')).toBeInTheDocument();
    fireEvent.click(screen.getByText('No'));
  });

  test('dispatches logoutSession on logout confirm', () => {
    render(
      <Provider store={store}>
        <Router>
          <Header />
        </Router>
      </Provider>
    );

    const usernameLink = screen.getByTitle('testuser');
    fireEvent.click(usernameLink);
    fireEvent.click(screen.getByText('Logout'));
    expect(screen.getByText('Confirm Logout')).toBeInTheDocument();

    fireEvent.click(screen.getByText('Yes'));
    expect(store.dispatch).toHaveBeenCalledWith(logoutSession('Logout Successfully'));
  });

  test('dispatches downloadGPX action on GPX button click', () => {
    render(
      <Provider store={store}>
        <Router>
          <Header />
        </Router>
      </Provider>
    );

    const gpxButton = screen.getByText('GPX');
    fireEvent.click(gpxButton);
    expect(store.dispatch).toHaveBeenCalledWith(downloadGPX());
  });

  test('dispatches downloadCSV action on EXCEL button click', () => {
    render(
      <Provider store={store}>
        <Router>
          <Header />
        </Router>
      </Provider>
    );

    const csvButton = screen.getByText('EXCEL');
    fireEvent.click(csvButton);
    expect(store.dispatch).toHaveBeenCalledWith(downloadCSV());
  });
});

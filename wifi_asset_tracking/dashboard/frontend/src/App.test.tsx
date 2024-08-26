import { render, screen } from '@testing-library/react';
import { Provider } from 'react-redux';
import store from './redux/store';
import App from './App';
import { vi } from 'vitest';
import { ReactNode } from 'react';

vi.mock('@azure/msal-browser', () => ({
  PublicClientApplication: vi.fn().mockImplementation(() => ({
    loginRedirect: vi.fn(),
    logout: vi.fn(),
  })),
}));

vi.mock('@azure/msal-react', () => ({
  MsalProvider: ({ children }: { children: ReactNode }) => <div>{children}</div>,
}));

vi.mock('../src/container/Root', () => ({
  __esModule: true,
  default: () => <div>Root Component</div>,
}));
vi.mock('./components/UI/Alert/AlertTimout', () => ({
  __esModule: true,
  default: () => <div>Alert Timeout Component</div>,
}));
vi.mock('react-toastify', () => ({
  ToastContainer: () => <div>Toast Container</div>,
}));

describe('App component', () => {
  test('renders without crashing and includes Root and AlertTimeout components', () => {
    render(
      <Provider store={store}>
        <App />
      </Provider>
    );

    expect(screen.getByText('Root Component')).toBeInTheDocument();
    expect(screen.getByText('Alert Timeout Component')).toBeInTheDocument();
    expect(screen.getByText('Toast Container')).toBeInTheDocument();
  });
});

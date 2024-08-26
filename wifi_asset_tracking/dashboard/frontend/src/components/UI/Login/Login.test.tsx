import { render, fireEvent, screen, waitFor } from '@testing-library/react';
import { Provider } from 'react-redux';
import Login from './Login';
import store from '../../../redux/store';
import { BrowserRouter } from 'react-router-dom';
import { PublicClientApplication } from '@azure/msal-browser';
import { MsalProvider } from '@azure/msal-react';
import '@testing-library/jest-dom';

const msalInstance = new PublicClientApplication({
  auth: {
    clientId: 'your-client-id',
    authority: 'your-authority',
    redirectUri: 'your-redirect-uri',
  },
});

describe('Login Component', () => {
  test('renders logo', () => {
    render(
      <Provider store={store}>
        <BrowserRouter>
          <MsalProvider instance={msalInstance}>
            <Login />
          </MsalProvider>
        </BrowserRouter>
      </Provider>
    );
    const logo = screen.getByAltText('Silabs Logo');
    expect(logo).toBeInTheDocument();
  });

  test('handles login submission', async () => {
    render(
      <Provider store={store}>
        <BrowserRouter>
          <MsalProvider instance={msalInstance}>
            <Login />
          </MsalProvider>
        </BrowserRouter>
      </Provider>
    );
    const loginButton = screen.getByText(/Login/i);
    fireEvent.click(loginButton);
    await waitFor(() => {
      expect(screen.getByText(/Log in to/i)).toBeInTheDocument();
    });
  });
});

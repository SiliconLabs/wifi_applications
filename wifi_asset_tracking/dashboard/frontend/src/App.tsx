import { Provider } from 'react-redux';
import store from './redux/store';
import { PublicClientApplication } from '@azure/msal-browser';
import { MsalProvider } from '@azure/msal-react';
import { ToastContainer } from 'react-toastify';
import 'react-toastify/dist/ReactToastify.css';
import Root from '../src/container/Root';
import AlertTimeout from './components/UI/Alert/AlertTimout';

const msalConfig = {
  auth: {
    clientId: import.meta.env.VITE_CLIENT_ID,
    authority: `https://login.microsoftonline.com/${import.meta.env.VITE_TENANT_ID}/token`,
    redirectUri: 'http://localhost:5173',
  },
};

const pca = new PublicClientApplication(msalConfig);

function App() {
  return (
    <Provider store={store}>
      <MsalProvider instance={pca}>
        <ToastContainer />
        <Root />
        <AlertTimeout />
      </MsalProvider>
    </Provider>
  );
}

export default App;

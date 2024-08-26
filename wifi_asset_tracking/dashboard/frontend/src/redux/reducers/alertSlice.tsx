import { createSlice, PayloadAction } from '@reduxjs/toolkit';

interface IAlert {
  showAlert: boolean;
  showDeviceAlert: boolean;
  showSessionEndAlert: boolean;
  showNetworkAlert: boolean;
  showIotAlert: boolean;
  showSocketError: boolean;
  message: string | null;
  backendConnectionStatus: string;
}

const initialState: IAlert = {
  showAlert: false,
  showDeviceAlert: false,
  showSessionEndAlert: false,
  showNetworkAlert: false,
  showIotAlert: false,
  showSocketError: false,
  message: null,
  backendConnectionStatus: '',
};

const alertSlice = createSlice({
  name: 'alert',
  initialState,
  reducers: {
    showSessionPopUp(state) {
      state.showAlert = true;
    },
    hideSessionPopUp(state) {
      state.showAlert = false;
    },
    showDevicePopUp(state) {
      state.showDeviceAlert = true;
    },
    hideDevicePopUp(state) {
      state.showDeviceAlert = false;
      localStorage.setItem('disconnect', '');
    },
    showSesssionEndPopUp(state) {
      state.showSessionEndAlert = true;
    },
    hideSessionEndPopUp(state) {
      state.showSessionEndAlert = false;
    },
    showNetworkPopUp(state) {
      state.showNetworkAlert = true;
    },
    hideNetworkPopUp(state) {
      state.showNetworkAlert = false;
      localStorage.setItem('network', '');
    },
    showIotPopUp(state) {
      state.showIotAlert = true;
    },
    hideIotPopUp(state) {
      state.showIotAlert = false;
      localStorage.setItem('iot', '');
    },
    setError: (state, action: PayloadAction<string>) => {
      state.message = action.payload;
    },
    showSocketErrorPopUp(state) {
      state.showSocketError = true;
    },
    hideSocketErrorPopUp(state) {
      state.showSocketError = false;
    },
    updateConnectionStatus(state, action: PayloadAction<string>) {
      state.backendConnectionStatus = action.payload;
    },
  },
});

export const {
  showSessionPopUp,
  hideSessionPopUp,
  showDevicePopUp,
  hideDevicePopUp,
  showSesssionEndPopUp,
  hideSessionEndPopUp,
  showNetworkPopUp,
  hideNetworkPopUp,
  showIotPopUp,
  hideIotPopUp,
  setError,
  showSocketErrorPopUp,
  hideSocketErrorPopUp,
  updateConnectionStatus,
} = alertSlice.actions;
export default alertSlice;

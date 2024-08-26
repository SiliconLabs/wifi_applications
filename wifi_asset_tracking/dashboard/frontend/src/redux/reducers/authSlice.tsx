import { createSlice } from '@reduxjs/toolkit';
import { fetchAccessToken, logoutSession } from '../apiservice';

interface AuthState {
  token: string;
  isAuthenticated: boolean;
  expiryTime: Date;
  timeleft: number;
}

const initialState: AuthState = {
  token: '',
  isAuthenticated: false,
  expiryTime: new Date(new Date().getTime()),
  timeleft: 0,
};

const authSlice = createSlice({
  name: 'auth',
  initialState,
  reducers: {
    login(state) {
      state.isAuthenticated = true;
    },
    ticksession: (state, action) => {
      const { remainingTime } = action.payload;
      state.timeleft = remainingTime;
    },
    resetAuth: () => {
      return initialState;
    },
  },
  extraReducers: (builder) => {
    builder
      .addCase(fetchAccessToken.fulfilled, (state) => {
        state.isAuthenticated = true;
        state.expiryTime = new Date(new Date().getTime() + 60 * 60 * 1000);
      })
      .addCase(logoutSession.fulfilled, (state) => {
        state.isAuthenticated = false;
        state.expiryTime = new Date(new Date().getTime());
        localStorage.removeItem('expireSession');
      });
  },
});

export default authSlice;
export const { resetAuth } = authSlice.actions;

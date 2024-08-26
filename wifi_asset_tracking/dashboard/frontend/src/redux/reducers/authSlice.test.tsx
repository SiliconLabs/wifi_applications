import authSlice from './authSlice';
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

describe('authSlice', () => {
  it('should handle login action', () => {
    const previousState = { ...initialState, isAuthenticated: false };
    expect(authSlice.reducer(previousState, authSlice.actions.login())).toEqual({
      ...previousState,
      isAuthenticated: true,
    });
  });

  it('should handle ticksession action', () => {
    const previousState = { ...initialState, timeleft: 0 };
    const remainingTime = 30;
    expect(authSlice.reducer(previousState, authSlice.actions.ticksession({ remainingTime }))).toEqual({
      ...previousState,
      timeleft: remainingTime,
    });
  });

  it('should handle fetchAccessToken.fulfilled', () => {
    const previousState = { ...initialState, isAuthenticated: false };
    const action = { type: fetchAccessToken.fulfilled.type };
    const newState = authSlice.reducer(previousState, action);
    expect(newState.isAuthenticated).toBe(true);
    expect(newState.expiryTime.getTime()).toBeGreaterThan(new Date().getTime());
  });

  it('should handle logoutSession.fulfilled', () => {
    const previousState = {
      token: 'abc123',
      isAuthenticated: true,
      expiryTime: new Date(new Date().getTime() + 60 * 60 * 1000),
      timeleft: 60,
    };
    const action = { type: logoutSession.fulfilled.type };
    const newState = authSlice.reducer(previousState, action);
    expect(newState.isAuthenticated).toBe(false);
    expect(newState.expiryTime.getTime()).toBeLessThanOrEqual(new Date().getTime());
    // You can also mock and check if localStorage.removeItem was called
  });
});

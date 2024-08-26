import alertSlice, { showSessionPopUp, hideSessionPopUp, showDevicePopUp, hideDevicePopUp } from './alertSlice';

const initialState = {
  showAlert: false,
  showDeviceAlert: false,
};

describe('alertSlice', () => {
  it('should handle showSessionPopUp action', () => {
    const previousState = { ...initialState, showAlert: false };
    expect(alertSlice.reducer(previousState, showSessionPopUp())).toEqual({
      ...previousState,
      showAlert: true,
    });
  });

  it('should handle hideSessionPopUp action', () => {
    const previousState = { ...initialState, showAlert: true };
    expect(alertSlice.reducer(previousState, hideSessionPopUp())).toEqual({
      ...previousState,
      showAlert: false,
    });
  });

  it('should handle showDevicePopUp action', () => {
    const previousState = { ...initialState, showDeviceAlert: false };
    expect(alertSlice.reducer(previousState, showDevicePopUp())).toEqual({
      ...previousState,
      showDeviceAlert: true,
    });
  });

  it('should handle hideDevicePopUp action', () => {
    const previousState = { ...initialState, showDeviceAlert: true };
    expect(alertSlice.reducer(previousState, hideDevicePopUp())).toEqual({
      ...previousState,
      showDeviceAlert: false,
    });
  });
});

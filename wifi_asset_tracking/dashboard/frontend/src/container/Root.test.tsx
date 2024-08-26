import React from 'react';
import { render } from '@testing-library/react';
import { Provider } from 'react-redux';
import configureStore from 'redux-mock-store';
import Root from './Root';
import * as CommonUtilities from '../utilities/common';
import * as ApiService from '../redux/apiservice';
import { vi } from 'vitest';

const mockStore = configureStore([]);

vi.mock('react-router-dom', () => ({
  RouterProvider: ({ children }: { children: React.ReactNode }) => <div>{children}</div>,
  createBrowserRouter: () => ({
    router: {},
  }),
}));

describe('Root component', () => {
  let store: ReturnType<typeof mockStore>;

  beforeEach(() => {
    store = mockStore({
      auth: {
        isAuthenticated: false,
      },
    });

    vi.spyOn(CommonUtilities, 'getCookie').mockReturnValue('fake-token');
    vi.spyOn(ApiService, 'updateLogin').mockReturnValue({ type: 'auth/updateLogin' });
  });

  afterEach(() => {
    vi.restoreAllMocks();
  });

  test('renders Login component when user is not authenticated', () => {
    render(
      <Provider store={store}>
        <Root />
      </Provider>
    );
  });

  test('renders Dashboard component when user is authenticated', () => {
    store = mockStore({
      auth: {
        isAuthenticated: true,
      },
    });

    render(
      <Provider store={store}>
        <Root />
      </Provider>
    );
  });

  test('dispatches updateLogin action if token exists', () => {
    render(
      <Provider store={store}>
        <Root />
      </Provider>
    );

    expect(ApiService.updateLogin).toHaveBeenCalled();
  });

  test('does not dispatch updateLogin action if token does not exist', () => {
    vi.spyOn(CommonUtilities, 'getCookie').mockReturnValue(null);

    render(
      <Provider store={store}>
        <Root />
      </Provider>
    );

    expect(ApiService.updateLogin).not.toHaveBeenCalled();
  });
});

import { useEffect } from 'react';
import { RouterProvider, createBrowserRouter } from 'react-router-dom';
import Login from '../components/UI/Login/Login';
import Dashboard from '../components/UI/Dashboard/Dashboard';
import { getCookie } from '../utilities/common';
import 'react-toastify/dist/ReactToastify.css';
import { useSelector } from 'react-redux';
import { updateLogin } from '../redux/apiservice';
import { RootState, useAppDispatch } from '../redux/store';

function Root() {
  const token = getCookie('access_token');
  const dispatch = useAppDispatch();

  useEffect(() => {
    // Dispatch the action to update login status only if the token exists
    if (token) {
      dispatch(updateLogin());
    }
  }, [dispatch, token]);

  const isAuth = useSelector((state: RootState) => state.auth.isAuthenticated);

  const router = createBrowserRouter([
    {
      path: '/',
      element: token && isAuth ? <Dashboard /> : <Login />,
    },
  ]);

  return <RouterProvider router={router} />;
}

export default Root;

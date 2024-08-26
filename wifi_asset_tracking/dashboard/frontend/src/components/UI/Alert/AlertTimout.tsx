import { useEffect } from 'react';
import { useSelector } from 'react-redux';
import {
  hideSessionPopUp,
  hideDevicePopUp,
  showDevicePopUp,
  hideSessionEndPopUp,
  showSesssionEndPopUp,
  showNetworkPopUp,
  hideNetworkPopUp,
  hideIotPopUp,
  showIotPopUp,
  hideSocketErrorPopUp,
} from '../../../redux/reducers/alertSlice';
import { Button, Modal } from 'react-bootstrap';
import { logoutSession, tickSession, downloadGPXandCSV } from '../../../redux/apiservice';
import moment from 'moment';
import { RootState, useAppDispatch } from '../../../redux/store';
import { IconDownload, IconInfoCircle } from '@tabler/icons-react';
import { getCookie } from '../../../utilities/common';
import styles from './Alert.module.scss';

const AlertTimeout = () => {
  const dispatch = useAppDispatch();
  const showDeviceModal = useSelector((state: RootState) => state.alert.showDeviceAlert);
  const showSessionEndModal = useSelector((state: RootState) => state.alert.showSessionEndAlert);
  const showNetworkModal = useSelector((state: RootState) => state.alert.showNetworkAlert);
  const showIotModal = useSelector((state: RootState) => state.alert.showIotAlert);
  const { message, showSocketError, backendConnectionStatus } = useSelector((state: RootState) => state.alert);

  // We are maintaining the Session Timer disruption

  useEffect(() => {
    let timeoutId: string | number | NodeJS.Timeout | undefined;

    const checkConnectionStatus = () => {
      const currentTime = moment().unix();
      const expiryTimeData = localStorage.getItem('expireSession');
      const deviceDisconnect = localStorage.getItem('disconnect');
      const networkDisconnect = localStorage.getItem('network');
      const iotDisconnect = localStorage.getItem('iot');

      const expiryTime = expiryTimeData ? parseInt(expiryTimeData) : 0;
      const remainingTime = expiryTime - currentTime;
      if (expiryTimeData && !showSessionEndModal) {
        if (networkDisconnect == 'networkError' && !showNetworkModal && remainingTime > 0) {
          dispatch(showNetworkPopUp());
        } else {
          dispatch(hideNetworkPopUp());
        }
        if (deviceDisconnect == 'Disconnected' && !showDeviceModal && remainingTime > 0) {
          dispatch(showDevicePopUp());
        } else {
          dispatch(hideDevicePopUp());
        }
        if (iotDisconnect == 'iotError' && !showIotModal && remainingTime > 0) {
          dispatch(showIotPopUp());
        } else {
          dispatch(hideIotPopUp());
        }
        dispatch(tickSession(remainingTime));
        if (remainingTime <= 0) {
          dispatch(hideDevicePopUp());
          dispatch(hideSessionPopUp());
          dispatch(showSesssionEndPopUp());
          localStorage.setItem('expireSession', '');
        }
      } else {
        const token = getCookie('access_token');
        if (token) {
          dispatch(showSesssionEndPopUp());
        }
      }
    };

    const startInterval = () => {
      checkConnectionStatus();
      timeoutId = setTimeout(startInterval, 1000);
    };

    startInterval();

    return () => clearTimeout(timeoutId);
  }, []);

  return (
    <>
      <Modal backdrop="static" keyboard={false} show={showDeviceModal} onHide={() => dispatch(hideDevicePopUp())}>
        <Modal.Header>
          <Modal.Title className="card-title">Alert</Modal.Title>
        </Modal.Header>
        <Modal.Body>
          <p>Device Disconnected</p>
          <p style={{ color: '#FF0000', fontSize: '12px' }}>
            <IconInfoCircle color="#FF0000" />
            You can choose to wait for the device to appear online and continue with the session by clicking "Wait for
            reconnect".
          </p>
          <p style={{ color: '#FF0000', fontSize: '12px' }}>
            <IconInfoCircle color="#FF0000" />
            You may choose to download and terminate the session by clicking "Download and Logout".
          </p>
        </Modal.Body>
        <Modal.Footer style={{ display: 'flex', justifyContent: 'center', alignItems: 'center' }}>
          <Button variant="secondary" onClick={() => dispatch(hideDevicePopUp())}>
            Wait for reconnect.
          </Button>
          <Button variant="secondary" onClick={() => dispatch(downloadGPXandCSV())}>
            Download and Logout <IconDownload size={18} stroke={2} />
          </Button>
        </Modal.Footer>
      </Modal>
      <Modal
        backdrop="static"
        keyboard={false}
        show={showSessionEndModal}
        onHide={() => dispatch(hideSessionEndPopUp())}
      >
        <Modal.Header>
          <Modal.Title className="card-title">Alert</Modal.Title>
        </Modal.Header>
        <Modal.Body>
          <p>Session Expired</p>
          <p style={{ color: '#FF0000', fontSize: '12px' }}>
            <IconInfoCircle color="#FF0000" />
            You may choose to download the session data and logout or discard and logout.
          </p>
        </Modal.Body>
        <Modal.Footer style={{ display: 'flex', justifyContent: 'center', alignItems: 'center' }}>
          <Button variant="secondary" onClick={() => dispatch(logoutSession('Successfully logout'))}>
            Discard and Logout
          </Button>
          <Button variant="secondary" onClick={() => dispatch(downloadGPXandCSV())}>
            Download and Logout <IconDownload size={18} stroke={2} />
          </Button>
        </Modal.Footer>
      </Modal>
      <Modal backdrop="static" keyboard={false} show={showNetworkModal} onHide={() => dispatch(hideNetworkPopUp())}>
        <Modal.Header closeButton>
          <Modal.Title className="card-title">Alert</Modal.Title>
        </Modal.Header>
        <Modal.Body>
          <p>Network Error.</p>
          <p style={{ color: '#FF0000', fontSize: '12px' }}>
            <IconInfoCircle color="#FF0000" />
            Please check your Internet connection.
          </p>
        </Modal.Body>
      </Modal>
      <Modal backdrop="static" keyboard={false} show={showIotModal} onHide={() => dispatch(hideIotPopUp())}>
        <Modal.Header closeButton>
          <Modal.Title className="card-title">Alert</Modal.Title>
        </Modal.Header>
        <Modal.Body>
          <p>Connection Error!</p>
          <p style={{ color: '#FF0000', fontSize: '12px' }}>
            <IconInfoCircle color="#FF0000" />
            Dashboard failed to connect to Azure IoT Hub. Please check you Internet and firewall settings.
          </p>
        </Modal.Body>
      </Modal>
      <Modal backdrop="static" keyboard={false} show={showSocketError} onHide={() => dispatch(hideSocketErrorPopUp())}>
        <Modal.Header closeButton>
          <Modal.Title className="card-title">Alert</Modal.Title>
        </Modal.Header>
        <Modal.Body>
          <p>Backend Connection Information!</p>
          <p className={`${backendConnectionStatus === 'Connected' ? styles.successError : styles.failure}`}>
            <IconInfoCircle />
            {message}
          </p>
        </Modal.Body>
      </Modal>
    </>
  );
};

export default AlertTimeout;

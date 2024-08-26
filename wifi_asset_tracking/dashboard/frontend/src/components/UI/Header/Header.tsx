import React, { useState } from 'react';
import { Container, Navbar, Dropdown, Modal, Button } from 'react-bootstrap';
import logo from '../../../assets/dashboard-logo.png';
import './Header.scss';
import { IconUserCircle, IconDownload, IconLogout } from '@tabler/icons-react';
import { downloadCSV, downloadGPX, logoutSession } from '../../../redux/apiservice';
import { Link } from 'react-router-dom';
import { getCookie, parseJwt, truncateString } from '../../../utilities/common';
import { RootState, useAppDispatch } from '../../../redux/store';
import { useSelector } from 'react-redux';
import moment from 'moment';

const Header: React.FC = () => {
  const [showDropdown, setShowDropdown] = useState(false);
  const [showModal, setShowModal] = useState(false);
  const dispatch = useAppDispatch();
  const expiryTime = useSelector((state: RootState) => state.auth.timeleft);
  const handleLogout = () => {
    dispatch(logoutSession('Logout Successfully'));
    setShowModal(false);
  };

  const toggleDropdown = () => {
    setShowDropdown(!showDropdown);
  };

  const downloadGPXFile = () => {
    dispatch(downloadGPX());
  };

  const downloadCSVFile = () => {
    dispatch(downloadCSV(''));
  };

  const token = getCookie('access_token');
  let username = '';
  if (token) {
    const parsedToken = parseJwt(token);
    username = parsedToken.payload.username;
  }
  const formatTime = (seconds: number) => {
    return moment.utc(seconds * 1000).format('mm:ss');
  };

  return (
    <>
      <Container>
        <Navbar.Brand>
          <img alt="dashboard-logo" src={logo} width={300} height={30} />
        </Navbar.Brand>
        <Navbar.Toggle />
        <Navbar.Collapse className="justify-content-end">
          {/* <LogoutButton /> */}
          <div className="session-timer text-muted">
            Session Timer : <span className="fw-bold"> &nbsp;{expiryTime ? formatTime(expiryTime) : '00:00'}</span>
          </div>
          <Navbar.Collapse id="basic-navbar-nav" className="justify-content-end">
            <button className="btn btn-primary primary-button" onClick={downloadGPXFile}>
              GPX <IconDownload size={18} stroke={2} />
            </button>
            <button className="btn btn-primary primary-button" onClick={downloadCSVFile}>
              EXCEL <IconDownload size={18} stroke={2} />
            </button>
            <div className="d-flex\\\ align-items-center">
              <div style={{ position: 'relative', display: 'inline-block' }}>
                <Link to="/" tabIndex={0} className="text-dark ms-1" onClick={toggleDropdown}>
                  <span title={username}>
                    {truncateString(username, 10)}
                    <IconUserCircle />
                  </span>
                </Link>
                {showDropdown && (
                  <Dropdown.Menu
                    show={showDropdown}
                    onClose={() => setShowDropdown(false)}
                    className="dropdown-container"
                    style={{ position: 'absolute', top: '100%', right: 0, width: '10px', zIndex: 1000 }}
                    drop="down"
                  >
                    <Dropdown.Item eventKey="1" onClick={() => setShowModal(true)}>
                      <IconLogout stroke={1} /> Logout
                    </Dropdown.Item>
                  </Dropdown.Menu>
                )}
              </div>
            </div>
          </Navbar.Collapse>
        </Navbar.Collapse>
      </Container>
      <Modal show={showModal} onHide={() => setShowModal(false)}>
        <Modal.Header closeButton>
          <Modal.Title className="card-title">Confirm Logout</Modal.Title>
        </Modal.Header>
        <Modal.Body>
          Session Data will be cleared upon Logout . Click "Yes" to proceed with Logout. Click "No" to download the
          data.
        </Modal.Body>
        <Modal.Footer>
          <Button variant="secondary" onClick={() => setShowModal(false)}>
            No
          </Button>
          <Button variant="danger" onClick={handleLogout}>
            Yes
          </Button>
        </Modal.Footer>
      </Modal>
    </>
  );
};

export default Header;

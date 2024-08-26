import React from 'react';
import styles from './Footer.module.scss';
import logoArrow from '../../../assets/arrows-logo.png';
import eilogo from '../../../assets/eic-logo.svg';
import { Navbar } from 'react-bootstrap';

const Footer: React.FC = () => {
  return (
    <Navbar as="footer" sticky="bottom" className={styles.footer}>
      <div className={styles.centerContent}>
        <div>Copyright © 2024 Silicon Laboratories. All rights reserved.</div>
      </div>
      <div className={styles.centerContent}>
        <div>
          Designed & Developed By <div className={styles.verticalbar}></div> &nbsp;
          <img alt="logo" className={styles.arrowlogo} src={logoArrow} />
          &nbsp;
          <img alt="ei-logo" className={styles.eilogo} src={eilogo} />
        </div>
      </div>
    </Navbar>
  );
};

export default Footer;

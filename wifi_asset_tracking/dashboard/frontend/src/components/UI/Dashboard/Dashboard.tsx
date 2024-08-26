import React from 'react';
import { Container, Navbar } from 'react-bootstrap';
import Header from '../Header/Header';
import Footer from '../Footer/Footer';
import Content from '../Content/Content';
import styles from './Dashboard.module.scss';
import { getCookie, parseJwt } from '../../../utilities/common';
const Dashboard: React.FC = () => {
  const token = getCookie('access_token');
  let name = '';
  if (token) {
    const parsedToken = parseJwt(token);
    name = parsedToken.payload.name;
  }
  return (
    <>
      <Navbar sticky="top" className='bg-white border-bottom'>
        <Header />
      </Navbar>
      <Container className={styles.content}>
        <div className={styles.customText}>
          Welcome,<span className={styles.nameText}>&nbsp;{name}!</span>
        </div>
        <Content />
      </Container>
      <Footer />
    </>
  );
};

export default Dashboard;

import React, { FormEvent } from 'react';
import styles from './Login.module.scss';
import { Container, Row, Col, Form } from 'react-bootstrap';
import logo from '../../../assets/dashboard-logo.png';
import { fetchAccessToken } from './../../../redux/apiservice';
import { useMsal } from '@azure/msal-react';
import { getCookie, parseJwt } from '../../../utilities/common';
import { useAppDispatch } from '../../../redux/store';
import { toast } from 'react-toastify';
import { IconArrowRight } from '@tabler/icons-react';
import Footer from '../Footer/Footer';

const Login: React.FC = () => {
  const dispatch = useAppDispatch();

  const { instance } = useMsal();
  const handleSubmit = (e: FormEvent<HTMLFormElement>) => {
    e.preventDefault(); // Prevent default form submission behavior
    instance
      .loginPopup()
      .then(async (response) => {
        const responses = await dispatch(fetchAccessToken(response));
        if (responses) {
          const token = getCookie('access_token');
          if (token) {
            const parsedToken = parseJwt(token);
            localStorage.setItem('expireSession', parsedToken.payload.sessionTimer);
          }
        }
      })
      .catch((error: string) => {
        toast.error(error, {
          position: 'top-center',
        });
      });
  };

  return (
    <div className={styles.backimage}>
      <section className={styles.fullcontainer}>
        <header className="bg-light mb-3 p-2">
          <img src={logo} width={300} alt="Silabs Logo" />
        </header>
        <Container className={styles.loginContainer}>
          <Row className="mx-3">
            <Col className="d-flex justify-content-start align-items-center">
              <div className="flex-column">
                <p className={styles.loginto}>Log in to</p>
                <p className={styles.silicon}>SILICON LABS</p>
                <Form onSubmit={handleSubmit}>
                  <button className={styles.loginbutton} type="submit">
                    Login
                    <IconArrowRight stroke={1} />
                  </button>
                </Form>
              </div>
            </Col>
          </Row>
        </Container>
        <section className={styles.footerSection}>
          <Footer />
        </section>
      </section>
    </div>
  );
};

export default Login;

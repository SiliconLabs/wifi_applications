import React, { ReactElement } from 'react';
import Card from 'react-bootstrap/Card';
import styles from './WifiWidget.module.scss';

interface WifiWidgetProps {
  title: string;
  device?: string;
  timestamp?: string;
  value?: string;
  icon?: ReactElement;
}
const WifiWidget: React.FC<WifiWidgetProps> = ({ title, device, timestamp, value, icon }) => {
  return (
    <Card className={styles.card}>
      <Card.Body className="d-flex flex-column justify-content-center align-items-center">
        <Card.Title className={styles.cardname}>{title}</Card.Title>
        <Card.Title className={styles.cardtitle}>{icon} </Card.Title>
        {value ? (
          <Card.Title className={styles.cardtext}>RSSI: {value}db</Card.Title>
        ) : (
          <Card.Title className={styles.cardtext}>
            <p></p>
          </Card.Title>
        )}
        {device ? (
          <Card.Title className={styles.cardvalue}>Connected AP: {device}</Card.Title>
        ) : (
          <Card.Title className={styles.cardvalue}>Not connected to AP</Card.Title>
        )}
        {timestamp ? (
          <Card.Text className={styles.cardsmalltext}>{timestamp}</Card.Text>
        ) : (
          <Card.Title className={styles.cardPadding}>
            <p></p>
          </Card.Title>
        )}
      </Card.Body>
    </Card>
  );
};

export default WifiWidget;

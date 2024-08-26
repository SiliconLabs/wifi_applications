import React, { useEffect } from 'react';
import { Row, Col } from 'react-bootstrap';
import { IconTemperatureSun, IconTemperaturePlus, IconWifiOff, IconMapPin, IconSatellite } from '@tabler/icons-react';
import Widget from '../Widget/Widget';
import LineGraph from '../LineGraph/LineGraph';
import CurveGraph from '../CurveGraph/CurveGraph';
import WifiWidget from '../WifiWidget/WifiWidget';
import { useSelector } from 'react-redux';
import { fetchAllData } from '../../../redux/apiservice';
import { formatDate, formatCoordinates, getWifiSignalImage } from '../../../utilities/common';
import {
  TEMPERATURE_TITLE,
  HUMIDITY_TITLE,
  GPS_SATELLITE,
  GPS_COORDINATES,
  ELEVATION_TITLE,
  ACCERO_TITLE,
  GYRO_TITLE,
  YELLOW_COLOR,
} from '../../../constants/index';
import styles from './Content.module.scss';
import { RootState, useAppDispatch } from '../../../redux/store';

const Content: React.FC = () => {
  const dispatch = useAppDispatch();
  const temperatureWidget = useSelector((state: RootState) => state.temperature.temperatureWidget);
  const temperatureData = useSelector((state: RootState) => state.temperature.temperatureGraph);

  const humidityWidget = useSelector((state: RootState) => state.humidity.humidityWidget);
  const humidityData = useSelector((state: RootState) => state.humidity.humidityGraph);

  const satelliteWidget = useSelector((state: RootState) => state.gps.satelliteWidget.satellites);
  const gpstimestamp = useSelector((state: RootState) => state.gps.satelliteWidget.timestamp);

  const latitude = useSelector((state: RootState) => state.gps.latLongWidget.lat);

  const longitude = useSelector((state: RootState) => state.gps.latLongWidget.long);

  const elevationData = useSelector((state: RootState) => state.gps.elevationGraph);

  const accelerXData = useSelector((state: RootState) => state.accelero.acceleroX);
  const accelerYData = useSelector((state: RootState) => state.accelero.acceleroY);
  const accelerZData = useSelector((state: RootState) => state.accelero.acceleroZ);
  const lastUpdatedTimeAccelero = useSelector((state: RootState) => state.accelero.timestamp);

  const gyroXData = useSelector((state: RootState) => state.gyro.gyroX);
  const gyroYData = useSelector((state: RootState) => state.gyro.gyroY);
  const gyroZData = useSelector((state: RootState) => state.gyro.gyroZ);
  const lastUpdatedTimeGyro = useSelector((state: RootState) => state.gyro.timestamp);

  const wifiData = useSelector((state: RootState) => state.wifi);

  const formattedCoordinates = formatCoordinates(latitude, longitude);

  useEffect(() => {
    dispatch(fetchAllData());
  }, [dispatch]);

  return (
    <>
      <Row>
        <Col xxs={12} sm={6} md={6} lg={6} xl={3} xxl={3}>
          <Widget
            title={TEMPERATURE_TITLE}
            timestamp={temperatureWidget.timestamp && formatDate(temperatureWidget.timestamp)}
            value={temperatureWidget.temperature}
            unit=" °C"
            icon={<IconTemperatureSun color={YELLOW_COLOR} className={styles.iconSize} />}
          />
        </Col>
        <Col xxs={12} sm={6} md={6} lg={6} xl={3} xxl={3}>
          <Widget
            title={HUMIDITY_TITLE}
            timestamp={humidityWidget.timestamp && formatDate(humidityWidget.timestamp)}
            value={humidityWidget.humidity}
            unit=" %"
            icon={<IconTemperaturePlus className={styles.iconSize} color="#32D583" />}
          />
        </Col>
        <Col xxs={12} sm={6} md={6} lg={6} xl={3} xxl={3}>
          <Widget
            title={GPS_SATELLITE}
            timestamp={gpstimestamp && formatDate(gpstimestamp)}
            value={satelliteWidget != '0' ? satelliteWidget : '0'}
            icon={<IconSatellite className={styles.iconSize} color="#73A2F2" />}
          />
        </Col>
        <Col xxs={12} sm={6} md={6} lg={6} xl={3} xxl={3}>
          <Widget
            title={GPS_COORDINATES}
            timestamp={gpstimestamp && formatDate(gpstimestamp)}
            value={latitude != 0 && formattedCoordinates}
            icon={<IconMapPin className={styles.iconSize} color="#F59F00" />}
          />
        </Col>
      </Row>
      <Row>
        <Col xxs={12} sm={6} md={6} lg={6} xl={3} xxl={3}>
          <LineGraph
            name="temperature-chart"
            lastUpdated={temperatureWidget.timestamp && formatDate(temperatureWidget.timestamp)}
            title={TEMPERATURE_TITLE}
            color={YELLOW_COLOR}
            unit="°C"
            datatemp={temperatureData}
          />
        </Col>
        <Col xxs={12} sm={6} md={6} lg={6} xl={3} xxl={3}>
          <LineGraph
            name="humidity-chart"
            lastUpdated={humidityWidget.timestamp && formatDate(humidityWidget.timestamp)}
            title={HUMIDITY_TITLE}
            color="#80E67E"
            unit="%"
            datatemp={humidityData}
          />
        </Col>
        <Col xxs={12} sm={6} md={6} lg={6} xl={3} xxl={3}>
          <LineGraph
            name="elevation-chart"
            lastUpdated={gpstimestamp && formatDate(gpstimestamp)}
            title={ELEVATION_TITLE}
            color="#73A2F2"
            unit="mm"
            datatemp={elevationData}
          />
        </Col>
        <Col xxs={12} sm={6} md={6} lg={6} xl={3} xxl={3}>
          <WifiWidget
            title="SSID and RSSI"
            timestamp={wifiData.timestamp && formatDate(wifiData.timestamp)}
            device={wifiData.device}
            value={wifiData.strength}
            icon={
              wifiData.strength ? (
                getWifiSignalImage(parseInt(wifiData.strength))
              ) : (
                <IconWifiOff size={80} color="#73A2F2" />
              )
            }
          />
        </Col>
      </Row>
      <Row>
        <Col xxs={12} sm={12} md={12} lg={12} xl={6} xxl={6}>
          <CurveGraph
            title={ACCERO_TITLE}
            lastupdated={lastUpdatedTimeAccelero && formatDate(lastUpdatedTimeAccelero)}
            accleroX={accelerXData}
            accleroY={accelerYData}
            accleroZ={accelerZData}
            colorX="#73A2F2"
            colorY={YELLOW_COLOR}
            colorZ="#139E60"
          />
        </Col>
        <Col xxs={12} sm={12} md={12} lg={12} xl={6} xxl={6}>
          <CurveGraph
            title={GYRO_TITLE}
            lastupdated={lastUpdatedTimeGyro && formatDate(lastUpdatedTimeGyro)}
            accleroX={gyroXData}
            accleroY={gyroYData}
            accleroZ={gyroZData}
            colorX="#73A2F2"
            colorY={YELLOW_COLOR}
            colorZ="#139E60"
          />
        </Col>
      </Row>
    </>
  );
};

export default Content;

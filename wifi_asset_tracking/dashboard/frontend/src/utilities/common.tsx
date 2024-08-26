import moment from 'moment';
import IconWifi1 from '../assets/svg/IconWifi1.svg';
import IconWifi2 from '../assets/svg/IconWifi2.svg';
import IconWifi3 from '../assets/svg/IconWifi3.svg';
import { IconWifi } from '@tabler/icons-react';

export const formatDate = (dateString: string): string => {
  const utcMoment = moment.utc(dateString);
  const localMoment = utcMoment.local();
  const localTimeString = localMoment.format('YYYY-MM-DD HH:mm:ss');
  return localTimeString;
};

export const convertToDMS = (coordinate: number, isLatitude: boolean): string => {
  const absolute = Math.abs(coordinate);
  const degrees = Math.floor(absolute);
  const minutesNotTruncated = (absolute - degrees) * 60;
  const minutes = Math.floor(minutesNotTruncated);
  const seconds = Math.round((minutesNotTruncated - minutes) * 60);
  const direction = isLatitude ? (coordinate >= 0 ? 'N' : 'S') : coordinate >= 0 ? 'E' : 'W';
  return `${degrees}°${minutes}'${seconds}"${direction}`;
};

export const formatCoordinates = (latitude: number, longitude: number): string => {
  const latitudeDMS = convertToDMS(latitude, true);
  const longitudeDMS = convertToDMS(longitude, false);
  return `${latitudeDMS} ${longitudeDMS}`;
};

export const getCookie = (name: string): string | null => {
  const cookies = document.cookie.split(';');
  for (const cookie of cookies) {
    const [cookieName, cookieValue] = cookie.trim().split('=');
    if (cookieName === name) {
      return cookieValue;
    }
  }
  return null;
};

export const parseJwt = (token: string) => {
  // Split the token into header, payload, and signature
  const [header, payload, signature] = token.split('.');

  // Decode Base64 URL (replace '-' with '+', '_' with '/')
  const base64UrlDecode = (str: string) => {
    const base64 = str.replace(/-/g, '+').replace(/_/g, '/');
    return decodeURIComponent(
      atob(base64)
        .split('')
        .map((c) => {
          return '%' + ('00' + c.charCodeAt(0).toString(16)).slice(-2);
        })
        .join('')
    );
  };

  // Parse JSON
  const parseJSON = (str: string) => {
    try {
      return JSON.parse(str);
    } catch (e) {
      console.error('Invalid JSON', e);
      return null;
    }
  };

  // Decode and parse header and payload
  const decodedHeader = parseJSON(base64UrlDecode(header));
  const decodedPayload = parseJSON(base64UrlDecode(payload));

  return {
    header: decodedHeader,
    payload: decodedPayload,
    signature: signature,
  };
};

export const setCookie = (name: string, value: string, maxAgeInSeconds: number) => {
  const maxAge = 'max-age=' + maxAgeInSeconds;
  document.cookie = name + '=' + value + ';' + maxAge + ';path=/';
};

export const truncateString = (str: string, maxLength: number) => {
  if (str.length > maxLength) {
    return str.substring(0, maxLength) + '...';
  } else {
    return str;
  }
};

export const getWifiSignalImage = (value: number) => {
  if (value <= -70) {
    return <img src={IconWifi1} alt="WiFi Icon" />;
  } else if (value > -70 && value <= -60) {
    return <img src={IconWifi2} alt="WiFi Icon" />;
  } else if (value > -60 && value <= -50) {
    return <img src={IconWifi3} alt="WiFi Icon" />;
  } else if (value > -50) {
    return <IconWifi size={80} color="#73A2F2" />;
  }
};

// We are comparing the data Timestamp with current data Timestamp

export const processDataWithCurrentTimeStamp = (payloadData: any) => {
  const unixDateData = Math.floor(new Date(payloadData).getTime() / 1000);
  const unixDateCurrent = Math.floor(new Date().getTime() / 1000);
  if (unixDateData >= unixDateCurrent) {
    return true;
  }
};

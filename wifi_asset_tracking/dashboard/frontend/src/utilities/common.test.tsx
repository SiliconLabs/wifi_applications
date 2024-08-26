import moment from 'moment';
import { formatDate, convertToDMS, formatCoordinates, getCookie, parseJwt, setCookie, truncateString } from './common'; // Adjust the import path as needed

describe('Utility Functions', () => {
  describe('formatDate', () => {
    it('should format UTC date string to local time string', () => {
      const dateString = '2024-07-04T10:00:00Z';
      const localTimeString = formatDate(dateString);
      expect(localTimeString).toEqual(moment.utc(dateString).local().format('YYYY-MM-DD HH:mm:ss'));
    });
  });

  describe('convertToDMS', () => {
    it('should convert latitude coordinate to DMS format', () => {
      const coordinate = 37.7749;
      const dms = convertToDMS(coordinate, true);
      expect(dms).toEqual('37°46\'30"N');
    });

    it('should convert longitude coordinate to DMS format', () => {
      const coordinate = -122.4194;
      const dms = convertToDMS(coordinate, false);
      expect(dms).toEqual('122°25\'10"W');
    });
  });

  describe('formatCoordinates', () => {
    it('should format latitude and longitude coordinates to DMS format', () => {
      const latitude = 37.7749;
      const longitude = -122.4194;
      const formattedCoordinates = formatCoordinates(latitude, longitude);
      expect(formattedCoordinates).toEqual('37°46\'30"N 122°25\'10"W');
    });
  });

  describe('getCookie', () => {
    beforeAll(() => {
      Object.defineProperty(document, 'cookie', {
        writable: true,
        value: 'name=JohnDoe; sessionToken=abc123',
      });
    });

    it('should return the value of the specified cookie', () => {
      expect(getCookie('name')).toEqual('JohnDoe');
      expect(getCookie('sessionToken')).toEqual('abc123');
    });

    it('should return null if the cookie is not found', () => {
      expect(getCookie('nonexistent')).toBeNull();
    });
  });

  describe('parseJwt', () => {
    it('should parse a JWT token correctly', () => {
      const token =
        'eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzdWIiOiIxMjM0NTY3ODkwIiwibmFtZSI6IkpvaG4gRG9lIiwiaWF0IjoxNTE2MjM5MDIyfQ.SflKxwRJSMeKKF2QT4fwpMeJf36POk6yJV_adQssw5c';
      const parsed = parseJwt(token);
      expect(parsed.header).toEqual({ alg: 'HS256', typ: 'JWT' });
      expect(parsed.payload).toEqual({ sub: '1234567890', name: 'John Doe', iat: 1516239022 });
      expect(parsed.signature).toEqual('SflKxwRJSMeKKF2QT4fwpMeJf36POk6yJV_adQssw5c');
    });
  });

  describe('setCookie', () => {
    it('should set a cookie with the specified name, value, and max age', () => {
      setCookie('testCookie', 'testValue', 3600);
      expect(document.cookie).toContain('testCookie=testValue');
    });
  });

  describe('truncateString', () => {
    it('should return the string as is if it does not exceed the max length', () => {
      const str = 'Short string';
      const truncated = truncateString(str, 20);
      expect(truncated).toEqual(str);
    });
  });
});

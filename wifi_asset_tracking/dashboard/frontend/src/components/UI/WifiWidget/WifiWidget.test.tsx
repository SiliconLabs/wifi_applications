import { render, screen } from '@testing-library/react';
import WifiWidget from './WifiWidget';
import { IconWifi } from '@tabler/icons-react';

describe('WifiWidget', () => {
  it('renders with all props provided', () => {
    render(
      <WifiWidget
        title="SSID and RSSI"
        device="MyWifi"
        timestamp="2024-06-17 10:00:00"
        value="80"
        icon={<IconWifi />}
      />
    );

    expect(screen.getByText('SSID and RSSI')).toBeInTheDocument();
    expect(screen.getByText('RSSI: 80db')).toBeInTheDocument();
    expect(screen.getByText('Connected AP: MyWifi')).toBeInTheDocument();
    expect(screen.getByText('2024-06-17 10:00:00')).toBeInTheDocument();
  });

  it('renders with missing value and device props', () => {
    render(<WifiWidget title="SSID and RSSI" timestamp="2024-06-17 10:00:00" icon={<IconWifi />} />);
    expect(screen.getByText('Not connected to AP')).toBeInTheDocument();
    expect(screen.getByText('2024-06-17 10:00:00')).toBeInTheDocument();
  });

  it('renders with missing timestamp prop', () => {
    render(<WifiWidget title="SSID and RSSI" device="MyWifi" value="80" icon={<IconWifi />} />);

    expect(screen.getByText('SSID and RSSI')).toBeInTheDocument();
    expect(screen.getByText('RSSI: 80db')).toBeInTheDocument();
    expect(screen.getByText('Connected AP: MyWifi')).toBeInTheDocument();
  });

  it('renders with only the title prop', () => {
    render(<WifiWidget title="SSID and RSSI" />);

    expect(screen.getByText('SSID and RSSI')).toBeInTheDocument();
    expect(screen.getByText('Not connected to AP')).toBeInTheDocument();
  });
});

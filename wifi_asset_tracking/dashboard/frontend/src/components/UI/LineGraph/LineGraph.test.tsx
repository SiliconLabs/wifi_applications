import { render, screen } from '@testing-library/react';
import '@testing-library/jest-dom';
import LineGraph from './LineGraph';
import { vi } from 'vitest';

vi.mock('react-apexcharts', () => {
  return {
    default: function MockApexCharts() {
      return <div aria-label="apex-chart"></div>;
    },
  };
});

const mockData = [
  { timestamp: '2024-01-01T00:00:00Z', temperature: 22, humidity: 55, altitude: 150 },
  { timestamp: '2024-01-01T01:00:00Z', temperature: 23, humidity: 55, altitude: 150 },
];

describe('LineGraph Component', () => {
  it('renders the component with temperature data', () => {
    render(
      <LineGraph
        name="TemperatureGraph"
        lastUpdated="2024-01-01T01:00:00Z"
        title="Temperature"
        unit="C"
        color="#FF0000"
        datatemp={mockData}
      />
    );

    // Check if the title is rendered
    expect(screen.getByText('Temperature')).toBeInTheDocument();
    // Check if the last updated text is rendered
    expect(screen.getByText('2024-01-01T01:00:00Z')).toBeInTheDocument();
  });
  it('renders the component with temperature data', () => {
    render(
      <LineGraph
        name="HumidityGraph"
        lastUpdated="2024-01-01T01:00:00Z"
        title="Humidity"
        unit="%"
        color="#FF0000"
        datatemp={mockData}
      />
    );

    // Check if the title is rendered
    expect(screen.getByText('Humidity')).toBeInTheDocument();
    // Check if the last updated text is rendered
    expect(screen.getByText('2024-01-01T01:00:00Z')).toBeInTheDocument();
  });

  it('renders the component with temperature data', () => {
    render(
      <LineGraph
        name="ElevationGraph"
        lastUpdated="2024-01-01T01:00:00Z"
        title="Elevation"
        unit="mm"
        color="#FF0000"
        datatemp={mockData}
      />
    );

    // Check if the title is rendered
    expect(screen.getByText('Elevation')).toBeInTheDocument();
    // Check if the last updated text is rendered
    expect(screen.getByText('2024-01-01T01:00:00Z')).toBeInTheDocument();
  });
  it('renders "No data available" when there is no data', () => {
    render(
      <LineGraph
        name="EmptyGraph"
        lastUpdated="2024-01-01T01:00:00Z"
        title="Temperature"
        unit="C"
        color="#FF0000"
        datatemp={[]}
      />
    );

    // Check if "No data available" message is rendered
    expect(screen.getByText('No data available')).toBeInTheDocument();
  });
});

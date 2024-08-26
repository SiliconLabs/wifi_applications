import { render, screen } from '@testing-library/react';
import '@testing-library/jest-dom';
import CurveGraph from './CurveGraph';

import { vi } from 'vitest';

vi.mock('react-apexcharts', () => {
  return {
    default: function MockApexCharts() {
      return <div aria-label="apex-chart"></div>;
    },
  };
});

describe('CurveGraph Component', () => {
  const props = {
    title: 'Accerometer',
    lastupdated: '2023-06-21 12:34:56',
    accleroX: [
      { timestamp: '2023-06-21T12:34:56Z', x: 1 },
      { timestamp: '2023-06-21T12:35:56Z', x: 2 },
    ],
    accleroY: [
      { timestamp: '2023-06-21T12:34:56Z', y: 3 },
      { timestamp: '2023-06-21T12:35:56Z', y: 4 },
    ],
    accleroZ: [
      { timestamp: '2023-06-21T12:34:56Z', z: 5 },
      { timestamp: '2023-06-21T12:35:56Z', z: 6 },
    ],
    colorX: '#ff0000',
    colorY: '#00ff00',
    colorZ: '#0000ff',
  };

  const propsNew = {
    title: 'Gyroscope',
    lastupdated: '2023-06-21 12:34:56',
    accleroX: [
      { timestamp: '2023-06-21T12:34:56Z', x: 1 },
      { timestamp: '2023-06-21T12:35:56Z', x: 2 },
    ],
    accleroY: [
      { timestamp: '2023-06-21T12:34:56Z', y: 3 },
      { timestamp: '2023-06-21T12:35:56Z', y: 4 },
    ],
    accleroZ: [
      { timestamp: '2023-06-21T12:34:56Z', z: 5 },
      { timestamp: '2023-06-21T12:35:56Z', z: 6 },
    ],
    colorX: '#ff0000',
    colorY: '#00ff00',
    colorZ: '#0000ff',
  };

  it('should render the CurveGraph component with given props', () => {
    render(<CurveGraph {...props} />);

    // Check if the title is rendered
    expect(screen.getByText('Accerometer')).toBeInTheDocument();

    // Check if the last updated text is rendered

    // Check if the unit of measure is correctly set
  });
  it('should render the CurveGraph component with given props', () => {
    render(<CurveGraph {...propsNew} />);

    // Check if the title is rendered
    expect(screen.getByText('Gyroscope')).toBeInTheDocument();

    // Check if the last updated text is rendered

    // Check if the unit of measure is correctly set
  });

  it('should render "No data available" when no data is provided', () => {
    const emptyProps = { ...props, accleroX: [], accleroY: [], accleroZ: [] };
    render(<CurveGraph {...emptyProps} />);

    // Check if the "No data available" message is displayed
    expect(screen.getByText('No data available')).toBeInTheDocument();
  });
});

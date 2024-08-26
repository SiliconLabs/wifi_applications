import { render, screen } from '@testing-library/react';
import Widget from './Widget';
import { IconTemperatureSun } from '@tabler/icons-react';

describe('Widget', () => {
  it('renders with all props provided', () => {
    render(
      <Widget title="Temperature" unit="°C" timestamp="2024-06-17 10:00:00" value="25" icon={<IconTemperatureSun />} />
    );

    expect(screen.getByText('Temperature')).toBeInTheDocument();
    expect(screen.getByText('25°C')).toBeInTheDocument();
    expect(screen.getByText('2024-06-17 10:00:00')).toBeInTheDocument();
  });

  it('renders with missing value and timestamp props', () => {
    render(<Widget title="Temperature" unit="°C" icon={<IconTemperatureSun />} />);

    expect(screen.getByText('Temperature')).toBeInTheDocument();
    expect(screen.getByText('No data available')).toBeInTheDocument();
  });

  it('renders with only the title prop', () => {
    render(<Widget title="Temperature" icon={<IconTemperatureSun />} />);

    expect(screen.getByText('Temperature')).toBeInTheDocument();
    expect(screen.getByText('No data available')).toBeInTheDocument();
  });
});

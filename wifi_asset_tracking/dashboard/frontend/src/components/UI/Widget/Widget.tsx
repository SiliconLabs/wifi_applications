import React, { ReactElement } from 'react';
import Card from 'react-bootstrap/Card';

interface WidgetProps {
  title: string;
  unit?: string;
  timestamp?: string;
  value?: string;
  icon?: ReactElement;
}
const Widget: React.FC<WidgetProps> = ({ title, unit, timestamp, value, icon }) => {
  return (
    <Card className="card">
      <Card.Body>
        <div className="d-flex justify-content-between">
          <div className="align-items-start">
            <Card.Text className="card-title">{title}</Card.Text>
            {title === 'Coordinates'
              ? value && (
                  <Card.Title className="card-coordinate">
                    {value}
                    {unit}
                  </Card.Title>
                )
              : value && (
                  <Card.Title className="card-value">
                    {value}
                    {unit}
                  </Card.Title>
                )}
            {timestamp && <Card.Text className="card-small-text">{timestamp}</Card.Text>}
            {!value && !timestamp && (
              <div
                style={{
                  width: '100%',
                  height: 95,
                  display: 'flex',
                  alignItems: 'top',
                  justifyContent: 'center',
                  fontSize: 18,
                  color: '#555',
                }}
              >
                No data available
              </div>
            )}
          </div>
          <div className="d-flex align-items-center">
            <Card.Title>{icon}</Card.Title>
          </div>
        </div>
      </Card.Body>
    </Card>
  );
};

export default Widget;

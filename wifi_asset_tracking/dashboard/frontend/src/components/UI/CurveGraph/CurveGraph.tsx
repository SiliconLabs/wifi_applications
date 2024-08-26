import React from 'react';
import Card from 'react-bootstrap/Card';
import styles from './CurveGraph.module.scss';
import ReactApexCharts from 'react-apexcharts';

interface CurveGraphProps {
  title: string;
  accleroX: Array<{ timestamp: string; x: number }>;
  accleroY: Array<{ timestamp: string; y: number }>;
  accleroZ: Array<{ timestamp: string; z: number }>;
  lastupdated: string;
  colorX: string;
  colorY: string;
  colorZ: string;
}
const CurveGraph: React.FC<CurveGraphProps> = ({
  title,
  lastupdated,
  accleroX,
  accleroY,
  accleroZ,
  colorX,
  colorY,
  colorZ,
}) => {
  let unitOfMeasure;
  let minValue;
  let maxValue;
  let sizeStep;

  if (title === 'Accelerometer') {
    unitOfMeasure = 'meter / second squared';
    minValue = -20;
    maxValue = 20;
    sizeStep = 20;
  } else if (title === 'Gyroscope') {
    unitOfMeasure = 'degree / second';
    minValue = -2000;
    maxValue = 2000;
    sizeStep = 20;
  }

  const chartData = {
    series: [
      {
        name: 'X',
        type: 'line',
        color: colorX,
        data: accleroX.map((item) => ({ x: new Date(item.timestamp).getTime(), y: item.x })),
      },
      {
        name: 'Y',
        type: 'line',
        color: colorY,
        data: accleroY.map((item) => ({ x: new Date(item.timestamp).getTime(), y: item.y })),
      },
      {
        name: 'Z',
        type: 'line',
        color: colorZ,
        data: accleroZ.map((item) => ({ x: new Date(item.timestamp).getTime(), y: item.z })),
      },
    ],
    options: {
      chart: {
        type: 'line',
        toolbar: {
          show: false,
        },
        zoom: {
          enabled: false, // Disable zoom functionality
        },
      },
      stroke: {
        width: [3, 3, 3],
        curve: 'smooth',
      },
      legend: {
        show: true,
        position: 'top',
        horizontalAlign: 'left',
      },
      xaxis: {
        title: {
          text: 'Time',
          style: {
            marginTop: '50px',
          },
        },
        type: 'datetime',
        labels: {
          datetimeUTC: false,
        },
      },
      yaxis: {
        title: {
          text: unitOfMeasure,
        },
        min: minValue,
        max: maxValue,
        stepSize: sizeStep,
        forceNiceScale: true,
        // tickAmount:10,
      },
      markers: {
        size: 3,
        strokeWidth: 0,
        fillOpacity: 1,
        hover: {
          sizeOffset: 2,
        },
      },
      tooltip: {
        x: {
          format: 'dd MMM yyyy HH:mm:ss',
        },
        y: [
          {
            title: {
              formatter: function (val: number) {
                return val;
              },
            },
          },
          {
            title: {
              formatter: function (val: number) {
                return val;
              },
            },
          },
          {
            title: {
              formatter: function (val: number) {
                return val;
              },
            },
          },
        ],
      },
      grid: {
        borderColor: '#f1f1f1',
      },
    },
  };
  return (
    <Card className={styles.card}>
      <Card.Body className={styles.cardbody}>
        <div className="d-flex justify-content-between align-items-center">
          <div className="align-items-start">
            <Card.Text className="card-title">{title}</Card.Text>
          </div>
          <div className="align-items-end">
            <Card.Text className="card-small-text">{lastupdated}</Card.Text>
          </div>
        </div>
        <div>
          {accleroX.length > 0 ? (
            <ReactApexCharts
              options={chartData.options}
              series={chartData.series}
              type="line"
              width="100%"
              height="350px"
            />
          ) : (
            <div
              style={{
                width: '100%',
                height: 350,
                display: 'flex',
                alignItems: 'center',
                justifyContent: 'center',
                fontSize: 18,
                color: '#555',
              }}
            >
              No data available
            </div>
          )}
        </div>
      </Card.Body>
    </Card>
  );
};

export default CurveGraph;

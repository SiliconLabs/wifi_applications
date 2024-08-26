import React from 'react';
import Card from 'react-bootstrap/Card';
import ReactApexCharts from 'react-apexcharts';

interface LineGraphProps {
  name: string;
  lastUpdated: string;
  title: string;
  unit: string;
  color: string;
  datatemp: Array<{ timestamp: string; temperature?: number; humidity?: number; altitude?: number; unit?: string }>;
}

const LineGraph: React.FC<LineGraphProps> = ({ name, lastUpdated, title, color, unit, datatemp }) => {
  const data = datatemp;
  let seriesData: { x: number; y: number | undefined }[] = [];
  let unitMeasure;
  if (title === 'Temperature') {
    seriesData = data.map((item) => ({ x: new Date(item.timestamp).getTime(), y: item.temperature }));
    unitMeasure = 'Celsius';
  } else if (title === 'Humidity') {
    seriesData = data.map((item) => ({ x: new Date(item.timestamp).getTime(), y: item.humidity }));
    unitMeasure = 'Percentage';
  } else if (title === 'Elevation') {
    seriesData = data.map((item) => ({ x: new Date(item.timestamp).getTime(), y: item.altitude }));
    unitMeasure = 'Height (mm) ';
  }

  const chartData = {
    options: {
      chart: {
        id: name,
        type: 'line',
        toolbar: {
          show: false,
        },
        zoom: {
          enabled: false, // Disable zoom functionality
        },
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
          text: unitMeasure,
        },
        tickAmount: 5,
      },
      stroke: {
        width: [3],
        curve: 'straight',
      },
      markers: {
        size: 2,
        strokeWidth: 0,
        fillOpacity: 1,
        colors: [color],
        hover: {
          size: 3,
        },
      },
      tooltip: {
        x: {
          format: 'dd MMM yyyy HH:mm:ss',
        },
        y: {
          formatter: function (value: number) {
            return value + '(' + unit + ')'; // Append "C" to the tooltip value
          },
        },
      },
    },
    series: [
      {
        name: title,
        color: color,
        data: seriesData,
      },
    ],
  };

  return (
    <Card className="card">
      <Card.Body>
        <div className="d-flex justify-content-between align-items-center">
          <div className="align-items-start">
            <Card.Text className="card-title">{title}</Card.Text>
          </div>
          <div className="align-items-end">
            <Card.Text className="card-small-text">{lastUpdated}</Card.Text>
          </div>
        </div>
        <div>
          {datatemp.length > 0 ? (
            <ReactApexCharts
              options={chartData.options}
              series={chartData.series}
              type="line"
              width="100%"
              height={238}
            />
          ) : (
            <div
              style={{
                width: '100%',
                height: 254,
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

export default LineGraph;

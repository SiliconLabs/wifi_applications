import * as fs from 'fs';
import * as ExcelJS from 'exceljs';

export const addMinutes = (date: Date, minutes: number) => {
  return new Date(date.getTime() + minutes * 60000);
};

export const addSecond = (date: Date, second: number) => {
  const d = new Date(date);
  d.setSeconds(d.getSeconds() + second);
  return d;
};

export const capitalizeFirstWord = (str: string) => {
  const string = str.toLowerCase();

  const modifiedStr = string.charAt(0).toUpperCase() + string.slice(1);

  return modifiedStr.replace(/_/g, ' ');
};

export const replaceText = (filePath: string, time: string) => {
  fs.readFile(filePath, 'utf8', (err, data) => {
    if (err) {
      return;
    }

    // Replace time and topic using regular expressions
    const replacedText = data.replace(/<time>/g, time);

    return replacedText;
  });
};

export function generateXLS(telemetryData): Promise<ArrayBuffer> {
  const workSheetList = Object.keys(telemetryData);
  const workbook = new ExcelJS.Workbook();

  for (let sheet of workSheetList) {
    const worksheet = workbook.addWorksheet(sheet, {
      pageSetup: { paperSize: 9, orientation: 'landscape' },
    });
    const rowIndex = 1;
    const row = worksheet.getRow(rowIndex);
    row.values = telemetryData[sheet][0]
      ? Object.keys(telemetryData[sheet][0]).map((sheet) => sheet.charAt(0).toUpperCase() + sheet.slice(1))
      : [];
    row.font = { bold: true };
    const columnWidths = [20, 20, 20];

    row.eachCell((cell, colNumber) => {
      const columnIndex = colNumber - 1;
      const columnWidth = columnWidths[columnIndex];
      worksheet.getColumn(colNumber).width = columnWidth;
    });

    telemetryData[sheet].forEach((task, index) => {
      const taskList = Object.keys(task);
      const row = worksheet.getRow(rowIndex + index + 1);
      row.getCell('A').value = task[taskList[0]] ? task[taskList[0]] : ' ';
      row.getCell('B').value = task[taskList[1]] ? task[taskList[1]] : ' ';
      row.getCell('C').value = task[taskList[2]] ? task[taskList[2]] : ' ';
      row.getCell('D').value = task[taskList[3]] ? task[taskList[3]] : ' ';
      row.getCell('E').value = task[taskList[4]] ? task[taskList[4]] : ' ';
    });
  }
  return workbook.xlsx.writeBuffer();
}

export function getTimeDifference(currentTime, prevTime): number {
  let firstDate = new Date(currentTime);
  let secondDate = new Date(prevTime);
  let timeDifference = Math.abs(firstDate.getTime() - secondDate.getTime());
  return timeDifference;
}

export function millisToSeconds(millis): number {
  let seconds = (millis / 1000).toFixed(0);
  return Number(seconds);
}

export function millisToMinutes(millis): number {
  var minutes = Math.floor(millis / 60000);
  return minutes;
}

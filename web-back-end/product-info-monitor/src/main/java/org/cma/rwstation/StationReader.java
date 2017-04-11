package org.cma.rwstation;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;

import org.cma.rwstation.struct.ForecastTimeLine;
import org.cma.rwstation.struct.StationFileObj;

public class StationReader {
	private StationFileObj obj;
	private String filename;

	public StationReader(StationFileObj obj, String filename) {
		this.obj = obj;
		this.filename = filename;
	}

	private boolean readHeader(BufferedReader br) throws IOException {
		// 读第一行
		String firstMark = br.readLine();
		if (!firstMark.equals("ZCZC")) {
			return false;
		}

		// 读第二行
		String secondLine = br.readLine();
		String provinceCode = secondLine.split("\\s+")[1];
		obj.getHeader().setProvinceCode(provinceCode);
		String dayHourStamp = secondLine.split("\\s+")[2];
		obj.getHeader().setDayHourStamp(dayHourStamp);

		// 读第三行
		String title = br.readLine();
		obj.getHeader().setTitle(title.trim());

		// 读第四行
		String fourthLine = br.readLine();
		String dataType = fourthLine.split("\\s+")[0];
		obj.getHeader().setDataType(dataType);
		String dateTimeStamp = fourthLine.split("\\s+")[1];
		obj.getHeader().setDateTimeStamp(dateTimeStamp);

		// 判断是否是6H格式的文件
		String fsc = secondLine.split("\\s+")[0];
		if (fsc.equals("FSCI50")) {// 6H
			// 跳过3行
			br.readLine();
			br.readLine();
			br.readLine();

		} else if (fsc.equals("FSC150")) {// none-6H
			String stationCnt = br.readLine();
			obj.getHeader().setStationCount(Integer.valueOf(stationCnt.trim()));
		} else {
			return false;// 未知类型，返回失败
		}

		return true;
	}

	private void readBody(BufferedReader br) throws IOException {
		// 判断6H格式
		if (obj.getHeader().getFsc().equals("FSCI50")) {// 6H

			String stationLine;
			while (!(stationLine = br.readLine()).trim().equals("NNNN")) {
				// 提取station头信息
				String stationID = stationLine.split("\\s+")[0];
				String lon, lat, height;
				lon = stationLine.split("\\s+")[1];
				lat = stationLine.split("\\s+")[2];
				height = stationLine.split("\\s+")[3];

				String forecastCnt = stationLine.split("\\s+")[4];
				int cnt = Integer.valueOf(forecastCnt);
				for (int i = 0; i < cnt; ++i) {

					String forecastLine = br.readLine();
					forecastLine = forecastLine.trim();
					String forecastTime = forecastLine.split("\\s+")[0];

					ForecastTimeLine line = new ForecastTimeLine(forecastTime);
					line.addDataValue(Float.valueOf(forecastLine.split("\\s+")[1]));
					line.addDataValue(Float.valueOf(forecastLine.split("\\s+")[2]));
					line.addDataValue(Float.valueOf(forecastLine.split("\\s+")[3]));
					line.addDataValue(Float.valueOf(forecastLine.split("\\s+")[4]));
					line.addDataValue(Float.valueOf(forecastLine.split("\\s+")[5]));
					line.addDataValue(Float.valueOf(forecastLine.split("\\s+")[6]));

					obj.getBody().addStation(stationID,
							Float.valueOf(lon),
							Float.valueOf(lat),
							Float.valueOf(height),
							line, 6);

				}

			} // end while

		} else if (obj.getHeader().getFsc().equals("FSC150")) {// none6H
			String stationLine;
			while (!(stationLine = br.readLine()).trim().equals("NNNN")) {
				// 提取station头信息
				String stationID = stationLine.split("\\s+")[0];
				String lon, lat, height;
				lon = stationLine.split("\\s+")[1];
				lat = stationLine.split("\\s+")[2];
				height = stationLine.split("\\s+")[3];

				String forecastCnt = stationLine.split("\\s+")[4];
				int cnt = Integer.valueOf(forecastCnt);
				for (int i = 0; i < cnt; ++i) {

					String forecastLine = br.readLine();
					forecastLine = forecastLine.trim();
					String forecastTime = forecastLine.split("\\s+")[0];

					ForecastTimeLine line = new ForecastTimeLine(forecastTime);
					line.addDataValue(Float.valueOf(forecastLine.split("\\s+")[1]));
					line.addDataValue(Float.valueOf(forecastLine.split("\\s+")[2]));
					line.addDataValue(Float.valueOf(forecastLine.split("\\s+")[3]));
					line.addDataValue(Float.valueOf(forecastLine.split("\\s+")[4]));
					line.addDataValue(Float.valueOf(forecastLine.split("\\s+")[5]));
					line.addDataValue(Float.valueOf(forecastLine.split("\\s+")[6]));
					line.addDataValue(Float.valueOf(forecastLine.split("\\s+")[7]));
					line.addDataValue(Float.valueOf(forecastLine.split("\\s+")[8]));
					line.addDataValue(Float.valueOf(forecastLine.split("\\s+")[9]));
					line.addDataValue(Float.valueOf(forecastLine.split("\\s+")[10]));
					line.addDataValue(Float.valueOf(forecastLine.split("\\s+")[11]));
					line.addDataValue(Float.valueOf(forecastLine.split("\\s+")[12]));
					line.addDataValue(Float.valueOf(forecastLine.split("\\s+")[13]));
					line.addDataValue(Float.valueOf(forecastLine.split("\\s+")[14]));
					line.addDataValue(Float.valueOf(forecastLine.split("\\s+")[15]));
					line.addDataValue(Float.valueOf(forecastLine.split("\\s+")[16]));
					line.addDataValue(Float.valueOf(forecastLine.split("\\s+")[17]));
					line.addDataValue(Float.valueOf(forecastLine.split("\\s+")[18]));
					line.addDataValue(Float.valueOf(forecastLine.split("\\s+")[19]));
					line.addDataValue(Float.valueOf(forecastLine.split("\\s+")[20]));
					line.addDataValue(Float.valueOf(forecastLine.split("\\s+")[21]));
					obj.getBody().addStation(stationID,
							Float.valueOf(lon),
							Float.valueOf(lat),
							Float.valueOf(height),
							line, 21);

				}

			} // end while
		}
	}

	public boolean read() {

		boolean bRet = true;
		// 打开文件
		File file = new File(filename);
		FileReader fr = null;
		BufferedReader reader = null;
		try {
			fr = new FileReader(file);
			reader = new BufferedReader(fr);

			bRet = (readHeader(reader)) ? true : false;
			readBody(reader);

		} catch (FileNotFoundException e) {
			e.printStackTrace();
			bRet = false;
		} catch (IOException e) {
			e.printStackTrace();
			bRet = false;
		} finally {

			try {
				reader.close();
				fr.close();
			} catch (IOException e) {
				e.printStackTrace();
				bRet = false;
			}

		}

		return bRet;

	}
}

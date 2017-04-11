package org.cma.rwstation;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

import org.cma.rwstation.struct.ForecastTimeLine;
import org.cma.rwstation.struct.MetaStation;
import org.cma.rwstation.struct.StationFileBody;
import org.cma.rwstation.struct.StationFileHeader;
import org.cma.rwstation.struct.StationFileHeader.CityType;
import org.cma.rwstation.struct.StationFileObj;
import org.cma.rwstation.struct.StationObject;

import cn.org.cma.activemq.message.MessageHeader;
import cn.org.cma.activemq.message.ProductBean;
import cn.org.cma.activemq.message.SCMOC6HBean;
import cn.org.cma.activemq.message.SPCC6HBean;
import cn.org.cma.activemq.message.SPCCBean;

public class Message6HPersistence implements MessagePersistence {

	private static HashMap<String, MetaStation> stations = StationListConfig.getStations();

	@Override
	public boolean save(MessageHeader header,
			ArrayList<ProductBean> lstBeans, String filename) {

		String dataType = header.getProductType();
		String source = header.getSource();
		String yyyymmdd = header.getYyyymmdd();
		String batchID = header.getBatchID();

		StationFileHeader head = new StationFileHeader(CityType.BIG_CITY_6H);

		String title;
		String dateTimeStamp;
		String reportTime;

	
		long baseTime = header.getBaseTime();
		Date date = new Date(baseTime);
		reportTime = String.format("%02d", date.getHours());

		dateTimeStamp = yyyymmdd + reportTime;

		head.init6H(source, dataType, dateTimeStamp);

		StationFileBody body = new StationFileBody();

		// 根据list bean添加站点数据
		for (ProductBean bean : lstBeans) {
			if (dataType.equals("SCMOC6H") && bean instanceof SCMOC6HBean) {
				SCMOC6HBean pb = (SCMOC6HBean) bean;

				String stationID = pb.getStation();
				String forecastTime = String.format("%03d",
						pb.getMinutesValid() / 60);

				ForecastTimeLine line = new ForecastTimeLine(forecastTime);
				line.addDataValue(pb.getWeather());
				line.addDataValue(pb.gettMax());
				line.addDataValue(pb.gettMin());
				line.addDataValue(pb.getWindDirection());
				line.addDataValue(pb.getWindLevel());
				line.addDataValue(pb.getPrecipitation());

				if (stations.containsKey(stationID)) {
					MetaStation inf = stations.get(stationID);
					body.addStation(stationID, inf.getLon(), inf.getLat(),
							inf.getHeight(),
							line, 6);
				}else{
					body.addStation(stationID, -99.9f, -99.9f,
							-99.9f,
							line, 6);
				}

			} else if (dataType.equals("SPCC6H")&& bean instanceof SPCC6HBean) {
				SPCC6HBean pb = (SPCC6HBean) bean;

				String stationID = pb.getStation();
				String forecastTime = String.format("%d",
						pb.getMinutesValid() / 60);

				ForecastTimeLine line = new ForecastTimeLine(forecastTime);
				line.addDataValue(pb.getWeather());
				line.addDataValue(pb.getTMax());
				line.addDataValue(pb.getTMin());
				line.addDataValue(pb.getWindDirection());
				line.addDataValue(pb.getWindLevel());
				line.addDataValue(pb.getPrecipitation());

				if (stations.containsKey(stationID)) {
					MetaStation inf = stations.get(stationID);
					body.addStation(stationID, inf.getLon(), inf.getLat(),
							inf.getHeight(),
							line, 6);
				}else{
					body.addStation(stationID, -99.9f, -99.9f,
							-99.9f,
							line, 6);
				}

			}
		}
		
		//替换filename的[FFF]pattern
		String maxFFF = "";
		for(Map.Entry<String, StationObject> entry: body.getStationMap().entrySet()) {
			int size = entry.getValue().getForecastLineList().size();
			maxFFF = entry.getValue().getForecastLineList().get(size - 1).getForecastTime();
			break;
		}
		
		int i_fff = Integer.valueOf(maxFFF);
		maxFFF = String.format("%03d", i_fff);
		filename = filename.replace("[FFF]", maxFFF);
		
		Date currentTime = new Date();
		SimpleDateFormat formatter = new SimpleDateFormat("yyyyMMddHHmmss");
		String stamp = formatter.format(currentTime);
		filename = filename.replace("yyyymmddhhmiss", stamp);
		
		StationFileObj obj = new StationFileObj(head, body);
		StationWriter writer = new StationWriter(obj, filename);

		if (!writer.save()) {
			return false;
		}

		return true;

	}

	@Override
	public boolean getMessage(MessageHeader header,
			ArrayList<ProductBean> beans,
			String filename) {
		StationFileHeader head = new StationFileHeader(CityType.BIG_CITY_6H);

		StationFileBody body = new StationFileBody();
		StationFileObj obj = new StationFileObj(head, body);
		StationReader reader = new StationReader(obj, filename);

		if (!reader.read()) {
			return false;
		}

		// 设置Header
		header.setSource(head.getProvinceCode());
		header.setProductType(head.getDataType());
		header.setLifeCycle(3600);
		header.setFileName("");
		header.setFilePath("");

		String dateTimeStamp = head.getDateTimeStamp();
		String yyyymmdd = dateTimeStamp.substring(0, dateTimeStamp.length() - 2);
		header.setYyyymmdd(yyyymmdd);
		
		String batchID = dateTimeStamp.substring(dateTimeStamp.length()-2, 
				dateTimeStamp.length());
		if(batchID.equals("12")){
			header.setBatchID("1400");
		}else if(batchID.equals("00")){
			header.setBatchID("1000");
		}
		
		header.setForecastType(0);

		// 把dateTimeStamp转换成baseTime
		SimpleDateFormat simpleDateFormat = new SimpleDateFormat("yyyyMMddHHmmss");
		Date date;
		try {
			date = simpleDateFormat.parse(dateTimeStamp + "0000");// 0000是分和秒
			long baseTime = date.getTime();
			header.setBaseTime(baseTime);
		} catch (ParseException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			return false;
		}

		
		String dataID = head.getProvinceCode() + dateTimeStamp;
		header.setDataID(dataID);

		// 根据Body的内容填充BeanList
		LinkedHashMap<String, StationObject> map = body.getStationMap();

		// 遍历map
		LinkedHashMap<String, StationObject> stations = obj.getBody().getStationMap();

		Iterator<Map.Entry<String, StationObject>> iter = stations.entrySet().iterator();

		String dataType = head.getDataType();
		while (iter.hasNext()) {
			Map.Entry<String, StationObject> entry = iter.next();
			String stationID = entry.getKey();
			StationObject item = entry.getValue();

			if (dataType.equals("SCMOC6H") ) {
				List<ForecastTimeLine> list = item.getForecastLineList();

				for (int i = 0; i < list.size(); ++i) {

					ForecastTimeLine line = list.get(i);

					// 创建一个空的Bean
					SCMOC6HBean pb = new SCMOC6HBean("",
							0, 0, 0, 0, 0, 0, 0, 0);

					// 设置基本参数
					pb.setStation(stationID); // 站点ID
					String foreTime = line.getForecastTime();
					int forecastTime = Integer.valueOf(foreTime);
					pb.setMinutesValid(forecastTime * 60);
					pb.setMinutesInterval(3);// 暂时为3

					// 设置预报数据
					List<Float> datalist = line.getDataList();

					if (datalist.size() == 6) {
						pb.setWeather(datalist.get(0));
						pb.settMax(datalist.get(1));
						pb.settMin(datalist.get(2));
						pb.setWindDirection(datalist.get(3));
						pb.setWindLevel(datalist.get(4));
						pb.setPrecipitation(datalist.get(5));

						beans.add(pb);
					}

				}

			} else if (dataType.equals("SPCC6H")) {

				List<ForecastTimeLine> list = item.getForecastLineList();
				for (int i = 0; i < list.size(); ++i) {

					ForecastTimeLine line = list.get(i);
					SPCC6HBean pb = new SPCC6HBean("",
							0, 0, 0, 0, 0, 0, 0, 0);

					// 设置基本参数
					pb.setStation(stationID); // 站点ID
					String foreTime = line.getForecastTime();
					int forecastTime = Integer.valueOf(foreTime);
					pb.setMinutesValid(forecastTime * 60);
					pb.setMinutesInterval(3);// 暂时为3

					// 设置预报数据
					List<Float> datalist = line.getDataList();
					if (datalist.size() == 6) {
						pb.setWeather(datalist.get(0));
						pb.setTMax(datalist.get(1));
						pb.setTMin(datalist.get(2));
						pb.setWindDirection(datalist.get(3));
						pb.setWindLevel(datalist.get(4));
						pb.setPrecipitation(datalist.get(5));

						beans.add(pb);
					}

				}

			}

		}

		return true;
	}
}

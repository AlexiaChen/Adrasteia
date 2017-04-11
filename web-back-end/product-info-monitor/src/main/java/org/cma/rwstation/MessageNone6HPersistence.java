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
import cn.org.cma.activemq.message.SCMOCBean;
import cn.org.cma.activemq.message.SPCCBean;

public class MessageNone6HPersistence implements MessagePersistence {

	private static HashMap<String, MetaStation> stations = StationListConfig.getStations();

	@Override
	public boolean save(MessageHeader header, ArrayList<ProductBean> lstBeans, String filename) {
		// TODO Auto-generated method stub

		String dataType = header.getProductType();
		String source = header.getSource();
		String yyyymmdd = header.getYyyymmdd();
		String batchID = header.getBatchID();

		StationFileHeader head = new StationFileHeader(CityType.NONE_6H);

		String title;
		String dateTimeStamp;
		String reportTime;

		
		long baseTime = header.getBaseTime();
		Date date = new Date(baseTime);
		reportTime = String.format("%02d", date.getHours());

		dateTimeStamp = yyyymmdd + reportTime;

		
		int stationCnt = lstBeans.size() / 28;

		head.init(source, dataType, dateTimeStamp, stationCnt);

		StationFileBody body = new StationFileBody();

		// 根据list bean添加站点数据
		for (ProductBean bean : lstBeans) {
			if (dataType.equals("SCMOC") && bean instanceof SCMOCBean) {
				SCMOCBean pb = (SCMOCBean) bean;

				String stationID = pb.getStation();
				String forecastTime = String.format("%d",
						pb.getMinutesValid() / 60);

				ForecastTimeLine line = new ForecastTimeLine(forecastTime);
				line.addDataValue(pb.getTemperature());
				line.addDataValue(pb.getHumidity());// 相对湿度
				line.addDataValue(pb.getWindDirection());
				line.addDataValue(pb.getWindSpeed());
				line.addDataValue(pb.getAirPressure());
				line.addDataValue(pb.getPrecipitation());// 降水量
				line.addDataValue(pb.getCloud());// 总云量
				line.addDataValue(pb.getlCloud());// 低运量
				line.addDataValue(pb.getWeather());// 天气现象
				line.addDataValue(pb.getVisibility());// 能见度
				line.addDataValue(pb.gettMax());// 最高温度
				line.addDataValue(pb.gettMin());// 最低温度
				line.addDataValue(pb.gethMax());// 最大相对湿度
				line.addDataValue(pb.gethMin());// 最小相对湿度
				line.addDataValue(pb.getPrecipitation24H());// 24小时降水量
				line.addDataValue(pb.getrProbability12H());// 12小时降水量
				line.addDataValue(pb.getCloud12H());
				line.addDataValue(pb.getlCloud12H());
				line.addDataValue(pb.getWeather12H());
				line.addDataValue(pb.getWindDirection12H());
				line.addDataValue(pb.getWindSpeed12H());

				if (stations.containsKey(stationID)) {
					MetaStation inf = stations.get(stationID);

					body.addStation(stationID, inf.getLon(), inf.getLat(),
							inf.getHeight(),
							line, 21);
				}else{
					body.addStation(stationID, -99.9f, -99.9f,
							-99.9f,
							line, 21);
				}

			} else if (dataType.equals("SPCC") && bean instanceof SPCCBean) {
				SPCCBean pb = (SPCCBean) bean;

				String stationID = pb.getStation();
				String forecastTime = String.format("%d",
						pb.getMinutesValid() / 60);

				ForecastTimeLine line = new ForecastTimeLine(forecastTime);
				line.addDataValue(999.9f);
				line.addDataValue(999.9f);// 相对湿度
				line.addDataValue(pb.getWindDirection());
				line.addDataValue(pb.getWindLevel());
				line.addDataValue(999.9f);
				line.addDataValue(999.9f);// 降水量
				line.addDataValue(999.9f);// 总云量
				line.addDataValue(999.9f);// 低运量
				line.addDataValue(pb.getWeather());// 天气现象
				line.addDataValue(999.9f);// 能见度
				line.addDataValue(pb.getTMax());// 最高温度
				line.addDataValue(pb.getTMin());// 最低温度
				line.addDataValue(999.9f);// 最大相对湿度
				line.addDataValue(999.9f);// 最小相对湿度
				line.addDataValue(999.9f);// 24小时降水量
				line.addDataValue(999.9f);// 12小时降水量
				line.addDataValue(999.9f);
				line.addDataValue(999.9f);
				line.addDataValue(999.9f);
				line.addDataValue(999.9f);
				line.addDataValue(999.9f);

				if (stations.containsKey(stationID)) {
					MetaStation inf = stations.get(stationID);

					body.addStation(stationID, inf.getLon(), inf.getLat(),
							inf.getHeight(),
							line, 21);
				}else{
					body.addStation(stationID, -99.9f, -99.9f,
							-99.9f,
							line, 21);
				}

			}
		}

		//计算站点数量和替换filename的[FFF]pattern
		int lineCntPerStaion = 0;
		String maxFFF = "";
		for(Map.Entry<String, StationObject> entry: body.getStationMap().entrySet()) {
			lineCntPerStaion = entry.getValue().getForecastLineCnt();
			int size = entry.getValue().getForecastLineList().size();
			maxFFF = entry.getValue().getForecastLineList().get(size - 1).getForecastTime();
			break;
		}
		//站点数量
		stationCnt = lstBeans.size() / lineCntPerStaion;
		head.init(source, dataType, dateTimeStamp, stationCnt);
		
		//filename的[FFF]pattern
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

		StationFileHeader head = new StationFileHeader(CityType.NONE_6H);

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

			if (dataType.equals("SCMOC")) {
				List<ForecastTimeLine> list = item.getForecastLineList();

				for (int i = 0; i < list.size(); ++i) {

					ForecastTimeLine line = list.get(i);

					// 创建一个空的Bean
					SCMOCBean pb = new SCMOCBean("",
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

					// 设置基本参数
					pb.setStation(stationID); // 站点ID
					String foreTime = line.getForecastTime();
					int forecastTime = Integer.valueOf(foreTime);
					pb.setMinutesValid(forecastTime * 60);
					pb.setMinutesInterval(3);// 暂时为3

					// 设置预报数据
					List<Float> datalist = line.getDataList();

					if (datalist.size() == 21) {
						pb.setTemperature(datalist.get(0));
						pb.setHumidity(datalist.get(1));// 相对湿度
						pb.setWindDirection(datalist.get(2));
						pb.setWindSpeed(datalist.get(3));
						pb.setAirPressure(datalist.get(4));
						pb.setPrecipitation(datalist.get(5));// 降水量
						pb.setCloud(datalist.get(6));// 总云量
						pb.setlCloud(datalist.get(7));// 低运量
						pb.setWeather(datalist.get(8));// 天气现象
						pb.setVisibility(datalist.get(9));// 能见度
						pb.settMax(datalist.get(10));// 最高温度
						pb.settMin(datalist.get(11));// 最低温度
						pb.sethMax(datalist.get(12));// 最大相对湿度
						pb.sethMin(datalist.get(13));// 最小相对湿度
						pb.setPrecipitation24H(datalist.get(14));// 24小时降水量
						pb.setrProbability12H(datalist.get(15));// 12小时降水量
						pb.setCloud12H(datalist.get(16));
						pb.setlCloud12H(datalist.get(17));
						pb.setWeather12H(datalist.get(18));
						pb.setWindDirection12H(datalist.get(19));
						pb.setWindSpeed12H(datalist.get(20));

						beans.add(pb);
					}

				}

			} else if (dataType.equals("SPCC")) {

				List<ForecastTimeLine> list = item.getForecastLineList();
				for (int i = 0; i < list.size(); ++i) {

					ForecastTimeLine line = list.get(i);
					SPCCBean pb = new SPCCBean("",
							0, 0, 0, 0, 0, 0, 0);

					// 设置基本参数
					pb.setStation(stationID); // 站点ID
					String foreTime = line.getForecastTime();
					int forecastTime = Integer.valueOf(foreTime);
					pb.setMinutesValid(forecastTime * 60);
					pb.setMinutesInterval(3);// 暂时为3

					// 设置预报数据
					List<Float> datalist = line.getDataList();
					if (datalist.size() == 21) {

						pb.setWindDirection(datalist.get(2));
						pb.setWindLevel(datalist.get(3));
						pb.setWeather(datalist.get(8));
						pb.setTMax(datalist.get(10));
						pb.setTMin(datalist.get(11));

						beans.add(pb);
					}

				}

			}

		}

		return true;
	}

}

package org.cma.rwstation.struct;

import java.util.LinkedHashMap;

public class StationFileBody {
	private LinkedHashMap<String,StationObject> stationMap;
	
	public StationFileBody(){
		stationMap = new LinkedHashMap<String,StationObject>();
	}
	
	public void addStation(String stationID,float lon,float lat,float height,
			ForecastTimeLine line, int columnCnt){
		if(stationMap.containsKey(stationID)){
			stationMap.get(stationID).addForecastTimeLine(line);
		}
		else{
			
			StationObject obj = new StationObject(lon,lat,height);
			obj.addForecastTimeLine(line);
			obj.setdataColumn(columnCnt);
			stationMap.put(stationID, obj);
		}
	}

	public LinkedHashMap<String, StationObject> getStationMap() {
		return stationMap;
	}
	
	
	
	
}

package org.cma.rwstation.struct;

import java.util.ArrayList;
import java.util.List;

public class StationObject {
	private float lon,lat,height;
	private int forecastLineCnt = 0;
	private int dataColumn = 0; /*数据列数*/
	
	private List<ForecastTimeLine> list;
	
	public StationObject(float lon, float lat, float height){
		this.lon = lon;
		this.lat = lat;
		this.height = height;
		
		list = new ArrayList<ForecastTimeLine>();
	}
	
	public void addForecastTimeLine(ForecastTimeLine line){
		list.add(line);
		forecastLineCnt = list.size();
	}
	
	public List<ForecastTimeLine> getForecastLineList(){
		return list;
	}
	
	public void setdataColumn(int columnCnt){
		this.dataColumn = columnCnt;
	}

	public float getLon() {
		return lon;
	}

	public float getLat() {
		return lat;
	}

	public float getHeight() {
		return height;
	}

	public int getForecastLineCnt() {
		return forecastLineCnt;
	}

	public int getDataColumn() {
		return dataColumn;
	}
	
	
	
}

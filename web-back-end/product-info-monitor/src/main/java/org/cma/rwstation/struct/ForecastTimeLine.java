package org.cma.rwstation.struct;

import java.util.ArrayList;
import java.util.List;

public class ForecastTimeLine {
	private String forecastTime;
	private List<Float> list;
	
	public ForecastTimeLine(String time){
		list = new ArrayList<Float>();
		forecastTime = time;
	}
	
	
	public void addDataValue(Float value){
		list.add(value);	
	}
	
	public List<Float> getDataList(){
		return list;
	}


	public String getForecastTime() {
		return forecastTime;
	}
	
	
	
	
}

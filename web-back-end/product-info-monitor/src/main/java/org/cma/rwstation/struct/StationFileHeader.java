package org.cma.rwstation.struct;

public class StationFileHeader {
	private static String firstMark="ZCZC";
	private String fsc;  /*6H:FSCI50  none-6H:FSC150*/
	private String provinceCode;
	private String dayHourStamp;
	private String title;
	private String dataType;
	private String dateTimeStamp;
	private String firstTime; /* for 6H*/
	private String numSequece;/*for 6H  like 621 622 ... 626*/
	private String lastTime; /*for 6H*/
	private int stationCount; /*for none-6H*/
	
	
	private void init6H(){
		
		this.fsc = "FSCI50";
		this.firstTime = "006";
		this.numSequece = "621 622 623 624 625 626";
		this.lastTime = "0036";
	}
	
	private void init(){
		this.fsc = "FSC150";
		this.firstTime = "";
		this.numSequece="";
		this.lastTime="";
	}
	
	public enum CityType{
		NONE_6H,  BIG_CITY_6H
	}
	
	public StationFileHeader(CityType type){
		switch(type)
		{
		case NONE_6H:
			init();
			break;
		case BIG_CITY_6H:
			init6H();
			break;
		
		}
	}
	
	public void init6H(String provinceCode,String dataType,
		String dateTimeStamp
		){
		this.provinceCode = provinceCode;
		//取最后四个字符
		int startIndex = dateTimeStamp.length() - 4;
		this.dayHourStamp = dateTimeStamp.substring(startIndex) + "00";
		
		String title="";
		if(dataType.equals("SCMOC6H"))
		{
			title = dateTimeStamp + "时中央台6小时指导产品 ";
		}
		else if(dataType.equals("SPCC6H"))
		{
			title = dateTimeStamp + "时省6小时订正产品 ";
		}
		
		
		this.title = title;
		this.dataType = dataType;
		this.dateTimeStamp = dateTimeStamp;
		this.stationCount = 0;
	}
	
	public void init(String provinceCode,String dataType,
			String dateTimeStamp,int stationCnt){
		this.provinceCode = provinceCode;
		
		//取最后四个字符
		int startIndex = dateTimeStamp.length() - 4;
		this.dayHourStamp = dateTimeStamp.substring(startIndex) + "00";
		
		String title="";
		if(dataType.equals("SCMOC"))
		{
			title = dateTimeStamp + "时中央台指导产品 ";
		}
		else if(dataType.equals("SPCC"))
		{
			title = dateTimeStamp + "时省订正产品 ";
		}
	
		this.title = title;
		this.dataType = dataType;
		this.dateTimeStamp = dateTimeStamp;
		this.stationCount = stationCnt;
	}

	public static String getFirstMark() {
		return firstMark;
	}

	public String getFsc() {
		return fsc;
	}

	public String getProvinceCode() {
		return provinceCode;
	}

	public String getDayHourStamp() {
		return dayHourStamp;
	}

	public String getTitle() {
		return title;
	}

	public String getDataType() {
		return dataType;
	}

	public String getDateTimeStamp() {
		return dateTimeStamp;
	}

	public String getFirstTime() {
		return firstTime;
	}

	public String getNumSequece() {
		return numSequece;
	}

	public String getLastTime() {
		return lastTime;
	}

	public int getStationCount() {
		return stationCount;
	}

	public static void setFirstMark(String firstMark) {
		StationFileHeader.firstMark = firstMark;
	}

	public void setFsc(String fsc) {
		this.fsc = fsc;
	}

	public void setProvinceCode(String provinceCode) {
		this.provinceCode = provinceCode;
	}

	public void setDayHourStamp(String dayHourStamp) {
		this.dayHourStamp = dayHourStamp;
	}

	public void setTitle(String title) {
		this.title = title;
	}

	public void setDataType(String dataType) {
		this.dataType = dataType;
	}

	public void setDateTimeStamp(String dateTimeStamp) {
		this.dateTimeStamp = dateTimeStamp;
	}

	public void setFirstTime(String firstTime) {
		this.firstTime = firstTime;
	}

	public void setNumSequece(String numSequece) {
		this.numSequece = numSequece;
	}

	public void setLastTime(String lastTime) {
		this.lastTime = lastTime;
	}

	public void setStationCount(int stationCount) {
		this.stationCount = stationCount;
	}

	
	
	
	
}

package org.cma.database.models;

public class NwfdStationSend {
	private int id;
	private String status;
	private String dataType;
	private String cityType;
	
	private int year;
	private int month;
	private int day;
	private int hour;
	private int minute;
	private int second;
	private int forecasttime;
	private int timerange;

	private int clientID;
	private String createTime;
	private String startTime;
	private String finishTime;
	private String errReason;
	private String errLevel;
	
	private String station;
	private float temperature;
	private float humidity;
	private float windDirection;
	private float windSpeed;
	private float airPressure;
	private float precipitation;
	private float cloud;
	private float lCloud;
	private float weather;
	private float visibility;
	private float tMax;
	private float tMin;
	private float hMax;
	private float hMin;
	private float precipitation24H;
	private float rProbability12H;
	private float cloud12H;
	private float lCloud12H;
	private float weather12H;
	private float windDirection12H;
	private float windSpeed12H;

	public int getId() {
		return id;
	}

	public void setId(int id) {
		this.id = id;
	}

	public String getStatus() {
		return status;
	}

	public void setStatus(String status) {
		this.status = status;
	}

	public String getDataType() {
		return dataType;
	}

	public void setDataType(String dataType) {
		this.dataType = dataType;
	}

	public String getCityType() {
		return cityType;
	}

	public void setCityType(String cityType) {
		this.cityType = cityType;
	}

	public int getYear() {
		return year;
	}

	public void setYear(int year) {
		this.year = year;
	}

	public int getMonth() {
		return month;
	}

	public void setMonth(int month) {
		this.month = month;
	}

	public int getDay() {
		return day;
	}

	public void setDay(int day) {
		this.day = day;
	}

	public int getHour() {
		return hour;
	}

	public void setHour(int hour) {
		this.hour = hour;
	}

	public int getMinute() {
		return minute;
	}

	public void setMinute(int minute) {
		this.minute = minute;
	}

	public int getSecond() {
		return second;
	}

	public void setSecond(int second) {
		this.second = second;
	}

	public int getForecasttime() {
		return forecasttime;
	}

	public void setForecasttime(int forecasttime) {
		this.forecasttime = forecasttime;
	}

	public int getTimerange() {
		return timerange;
	}

	public void setTimerange(int timerange) {
		this.timerange = timerange;
	}

	public int getClientID() {
		return clientID;
	}

	public void setClientID(int clientID) {
		this.clientID = clientID;
	}

	public String getCreateTime() {
		return createTime;
	}

	public void setCreateTime(String createTime) {
		this.createTime = createTime;
	}

	public String getStartTime() {
		return startTime;
	}

	public void setStartTime(String startTime) {
		this.startTime = startTime;
	}

	public String getFinishTime() {
		return finishTime;
	}

	public void setFinishTime(String finishTime) {
		this.finishTime = finishTime;
	}

	public String getErrReason() {
		return errReason;
	}

	public void setErrReason(String errReason) {
		this.errReason = errReason;
	}

	public String getErrLevel() {
		return errLevel;
	}

	public void setErrLevel(String errLevel) {
		this.errLevel = errLevel;
	}

	public String getStation() {
	  return this.station;
	}
	
	public void setStation(String station) {
	  this.station = station;
	}
	
	public float getTemperature() {
	  return this.temperature;
	}
	
	public void setTemperature(float temperature) {
	  this.temperature = temperature;
	}
	
	public float getHumidity() {
	  return this.humidity;
	}
	
	public void setHumidity(float humidity) {
	  this.humidity = humidity;
	}
	
	public float getWindDirection() {
	  return this.windDirection;
	}
	
	public void setWindDirection(float windDirection) {
	  this.windDirection = windDirection;
	}
	
	public float getWindSpeed() {
	  return this.windSpeed;
	}
	
	public void setWindSpeed(float windSpeed) {
	  this.windSpeed = windSpeed;
	}
	
	public float getAirPressure() {
	  return this.airPressure;
	}
	
	public void setAirPressure(float airPressure) {
	  this.airPressure = airPressure;
	}
	
	public float getPrecipitation() {
	  return this.precipitation;
	}
	
	public void setPrecipitation(float precipitation) {
	  this.precipitation = precipitation;
	}
	
	public float getCloud() {
	  return this.cloud;
	}
	
	public void setCloud(float cloud) {
	  this.cloud = cloud;
	}
	
	public float getlCloud() {
	  return this.lCloud;
	}
	
	public void setlCloud(float lCloud) {
	  this.lCloud = lCloud;
	}
	
	public float getWeather() {
	  return this.weather;
	}
	
	public void setWeather(float weather) {
	  this.weather = weather;
	}
	
	public float getVisibility() {
	  return this.visibility;
	}
	
	public void setVisibility(float visibility) {
	  this.visibility = visibility;
	}
	
	public float gettMax() {
	  return this.tMax;
	}
	
	public void settMax(float tMax) {
	  this.tMax = tMax;
	}
	
	public float gettMin() {
	  return this.tMin;
	}
	
	public void settMin(float tMin) {
	  this.tMin = tMin;
	}
	
	public float gethMax() {
	  return this.hMax;
	}
	
	public void sethMax(float hMax) {
	  this.hMax = hMax;
	}
	
	public float gethMin() {
	  return this.hMin;
	}
	
	public void sethMin(float hMin) {
	  this.hMin = hMin;
	}
	
	public float getPrecipitation24H() {
	  return this.precipitation24H;
	}
	
	public void setPrecipitation24H(float precipitation24h) {
	  this.precipitation24H = precipitation24h;
	}
	
	public float getrProbability12H() {
	  return this.rProbability12H;
	}
	
	public void setrProbability12H(float rProbability12H) {
	  this.rProbability12H = rProbability12H;
	}
	
	public float getCloud12H() {
	  return this.cloud12H;
	}
	
	public void setCloud12H(float cloud12h) {
	  this.cloud12H = cloud12h;
	}
	
	public float getlCloud12H() {
	  return this.lCloud12H;
	}
	
	public void setlCloud12H(float lCloud12H) {
	  this.lCloud12H = lCloud12H;
	}
	
	public float getWeather12H() {
	  return this.weather12H;
	}
	
	public void setWeather12H(float weather12h) {
	  this.weather12H = weather12h;
	}
	
	public float getWindDirection12H() {
	  return this.windDirection12H;
	}
	
	public void setWindDirection12H(float windDirection12H) {
	  this.windDirection12H = windDirection12H;
	}
	
	public float getWindSpeed12H() {
	  return this.windSpeed12H;
	}
	
	public void setWindSpeed12H(float windSpeed12H) {
	  this.windSpeed12H = windSpeed12H;
	}
}

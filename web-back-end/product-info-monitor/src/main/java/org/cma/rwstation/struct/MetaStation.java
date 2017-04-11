package org.cma.rwstation.struct;

public class MetaStation {
	private float lon;
	private float lat;
	private float height;
	
	public MetaStation(float lon,float lat, float height){
		this.lon = lon;
		this.lat = lat;
		this.height = height;
	}

	public float getLon() {
		return lon;
	}

	public void setLon(float lon) {
		this.lon = lon;
	}

	public float getLat() {
		return lat;
	}

	public void setLat(float lat) {
		this.lat = lat;
	}

	public float getHeight() {
		return height;
	}

	public void setHeight(float height) {
		this.height = height;
	}
	
	
}

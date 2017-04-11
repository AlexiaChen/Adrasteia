package org.cma.product;

import java.io.Serializable;
import java.util.List;

import org.cma.product.files.SendProductFileSet;

public class SendMonProduct implements Serializable {

	private String key;
	private String name;
	private String type;
	private float di, dj;
	private String range;
	private String maxforecast;
	private String sendfolder;
	private String sendoverfolder;
	private String sendtime;
	private String fileformat;
	private List<String> reporttimes;

	private SendProductFileSet fileSet;

	public String getKey() {
		return key;
	}

	public void setKey(String key) {
		this.key = key;
	}

	public String getName() {
		return name;
	}

	public void setName(String name) {
		this.name = name;
	}

	public String getType() {
		return type;
	}

	public void setType(String type) {
		this.type = type;
	}

	public float getDi() {
		return di;
	}

	public void setDi(float di) {
		this.di = di;
	}

	public float getDj() {
		return dj;
	}

	public void setDj(float dj) {
		this.dj = dj;
	}

	public String getRange() {
		return range;
	}

	public void setRange(String range) {
		this.range = range;
	}

	public String getMaxforecast() {
		return maxforecast;
	}

	public void setMaxforecast(String maxforecast) {
		this.maxforecast = maxforecast;
	}

	public String getSendfolder() {
		return sendfolder;
	}

	public void setSendfolder(String sendfolder) {
		this.sendfolder = sendfolder;
	}

	public String getSendoverfolder() {
		return sendoverfolder;
	}

	public void setSendoverfolder(String sendoverfolder) {
		this.sendoverfolder = sendoverfolder;
	}

	public String getSendtime() {
		return sendtime;
	}

	public void setSendtime(String sendtime) {
		this.sendtime = sendtime;
	}

	public String getFileformat() {
		return fileformat;
	}

	public void setFileformat(String fileformat) {
		this.fileformat = fileformat;
	}

	public List<String> getReporttimes() {
		return reporttimes;
	}

	public void setReporttimes(List<String> reporttimes) {
		this.reporttimes = reporttimes;
	}

	public SendProductFileSet getFileSet() {
		return fileSet;
	}

	public void setFileSet(SendProductFileSet fileSet) {
		this.fileSet = fileSet;
	}

}

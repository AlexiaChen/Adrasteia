package org.cma.product;

import java.io.Serializable;
import java.util.List;

import org.cma.product.files.RecvProductFileSet;

public class RecvMonProduct implements Serializable {

	private String key;
	private String name;
	private String cccc;
	private String ccccName;
	private String type;
	private float di, dj;
	private String range;
	private String maxforecast;
	private String srcfolder;
	private String recvtime;
	private String actualRecvTime;
	private String fileformat;
	private List<String> reporttimes;

	private RecvProductFileSet fileSet;

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

	public String getCccc() {
		return cccc;
	}

	public void setCccc(String cccc) {
		this.cccc = cccc;
	}

	public String getCcccName() {
		return ccccName;
	}

	public void setCcccName(String ccccName) {
		this.ccccName = ccccName;
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

	public String getSrcfolder() {
		return srcfolder;
	}

	public void setSrcfolder(String srcfolder) {
		this.srcfolder = srcfolder;
	}

	public String getRecvtime() {
		return recvtime;
	}

	public void setRecvtime(String recvtime) {
		this.recvtime = recvtime;
	}

	public String getActualRecvTime() {
		return actualRecvTime;
	}

	public void setActualRecvTime(String actualRecvTime) {
		this.actualRecvTime = actualRecvTime;
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

	public RecvProductFileSet getFileSet() {
		return fileSet;
	}

	public void setFileSet(RecvProductFileSet fileSet) {
		this.fileSet = fileSet;
	}

}

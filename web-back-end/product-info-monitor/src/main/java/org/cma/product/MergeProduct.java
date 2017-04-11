package org.cma.product;

import java.io.Serializable;
import java.util.List;

import org.cma.product.files.MergeProductFile;
import org.cma.product.files.MergeProductFileSet;

public class MergeProduct implements Serializable {
	private String productKey;
	private String cccc;
	private String ccccName;
	private String name;
	private String type;
	private float di, dj;
	private String range;
	private String maxForecast;
	private String shouldTime;
	private List<String> reporttimes;

	private MergeProductFileSet fileSet;

	public String getProductKey() {
		return productKey;
	}

	public void setProductKey(String productKey) {
		this.productKey = productKey;
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

	public String getMaxForecast() {
		return maxForecast;
	}

	public void setMaxForecast(String maxForecast) {
		this.maxForecast = maxForecast;
	}

	public String getShouldTime() {
		return shouldTime;
	}

	public void setShouldTime(String shouldTime) {
		this.shouldTime = shouldTime;
	}

	public List<String> getReporttimes() {
		return reporttimes;
	}

	public void setReporttimes(List<String> reporttimes) {
		this.reporttimes = reporttimes;
	}

	public MergeProductFileSet getFileSet() {
		return fileSet;
	}

	public void setFileSet(MergeProductFileSet fileSet) {
		this.fileSet = fileSet;
	}

}

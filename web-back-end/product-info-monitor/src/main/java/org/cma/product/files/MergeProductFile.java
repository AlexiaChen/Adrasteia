package org.cma.product.files;

import java.io.Serializable;

public class MergeProductFile implements Serializable {
	private String status;
	private String reportTime;
	private String fileName;
	private String shouldTime;
	private String practicalTime;
	private String errReason;
	private boolean Result;

	public String getStatus() {
		return status;
	}

	public void setStatus(String status) {
		this.status = status;
	}

	public String getReportTime() {
		return reportTime;
	}

	public void setReportTime(String reportTime) {
		this.reportTime = reportTime;
	}

	public String getFileName() {
		return fileName;
	}

	public void setFileName(String fileName) {
		this.fileName = fileName;
	}

	public String getShouldTime() {
		return shouldTime;
	}

	public void setShouldTime(String shouldTime) {
		this.shouldTime = shouldTime;
	}

	public String getPracticalTime() {
		return practicalTime;
	}

	public void setPracticalTime(String practicalTime) {
		this.practicalTime = practicalTime;
	}

	public String getErrReason() {
		return errReason;
	}

	public void setErrReason(String errReason) {
		this.errReason = errReason;
	}

	public boolean isResult() {
		return Result;
	}

	public void setResult(boolean result) {
		Result = result;
	}

}

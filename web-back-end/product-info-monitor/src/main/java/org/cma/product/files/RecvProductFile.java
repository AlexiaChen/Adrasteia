package org.cma.product.files;

import java.io.Serializable;

public class RecvProductFile implements Serializable {
	private String reportTime;
	private String fileName;
	private String shouldRecvTime;
	private String recvTime;
	private boolean result;

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

	public String getShouldRecvTime() {
		return shouldRecvTime;
	}

	public void setShouldRecvTime(String shouldRecvTime) {
		this.shouldRecvTime = shouldRecvTime;
	}

	public String getRecvTime() {
		return recvTime;
	}

	public void setRecvTime(String recvTime) {
		this.recvTime = recvTime;
	}

	public boolean isResult() {
		return result;
	}

	public void setResult(boolean result) {
		this.result = result;
	}

}

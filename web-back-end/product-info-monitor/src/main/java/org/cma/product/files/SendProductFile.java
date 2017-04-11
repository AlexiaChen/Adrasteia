package org.cma.product.files;

import java.io.Serializable;

public class SendProductFile implements Serializable {

	private String reportTime;
	private String fileName;
	private String shouldSendTime;
	private String sendTime;
	private boolean Result;

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

	public String getShouldSendTime() {
		return shouldSendTime;
	}

	public void setShouldSendTime(String shouldSendTime) {
		this.shouldSendTime = shouldSendTime;
	}

	public String getSendTime() {
		return sendTime;
	}

	public void setSendTime(String sendTime) {
		this.sendTime = sendTime;
	}

	public boolean isResult() {
		return Result;
	}

	public void setResult(boolean result) {
		Result = result;
	}

}

package org.cma.utils;

public class SendProduct {
	private String productKey;
	private String scanFolder;
	private String sendoverFolder;
	private String queuekey;
	private String fileformat;
	private String reporttime;

	public String getProductKey() {
		return productKey;
	}

	public void setProductKey(String productKey) {
		this.productKey = productKey;
	}

	public String getScanFolder() {
		return scanFolder;
	}

	public void setScanFolder(String scanFolder) {
		this.scanFolder = scanFolder;
	}

	public String getSendoverFolder() {
		return sendoverFolder;
	}

	public void setSendoverFolder(String sendoverFolder) {
		this.sendoverFolder = sendoverFolder;
	}

	public String getQueuekey() {
		return queuekey;
	}

	public void setQueuekey(String queuekey) {
		this.queuekey = queuekey;
	}

	public String getFileformat() {
		return fileformat;
	}

	public void setFileformat(String fileformat) {
		this.fileformat = fileformat;
	}

	public String getReporttime() {
		return reporttime;
	}

	public void setReporttime(String reporttime) {
		this.reporttime = reporttime;
	}

}

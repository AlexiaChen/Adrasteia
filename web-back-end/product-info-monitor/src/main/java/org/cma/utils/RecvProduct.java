package org.cma.utils;

import java.util.HashMap;

public class RecvProduct {
	private String queueKey; // 接收队列名
	private String saveFolder; // 保存文件夹

	public String getQueueKey() {
		return queueKey;
	}

	public void setQueueKey(String queueKey) {
		this.queueKey = queueKey;
	}

	public String getSaveFolder() {
		return saveFolder;
	}

	public void setSaveFolder(String saveFolder) {
		this.saveFolder = saveFolder;
	}
}

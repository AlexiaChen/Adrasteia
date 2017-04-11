package org.cma.utils;

import java.io.File;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import org.dom4j.Document;
import org.dom4j.Node;
import org.dom4j.io.SAXReader;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
*
* @author zhangl
* 
* 获取MQ的XML配置
*/

public class MQConfig {
	private static transient final Logger logger = LoggerFactory.getLogger(MQConfig.class);
	private static MQConfig _instance = new MQConfig();
	private String  serverIP;  // MQServerIP
	private Integer sendPort;  // 发送端口
	private Integer recvPort;  // 接收端口
	private String  source;     // MQServer所在地编码
	private ArrayList<SendProduct> listSendProduct = new ArrayList<SendProduct>();  // 文件发送配置
	private ArrayList<RecvProduct> listRecvProduct = new ArrayList<RecvProduct>();  // 文件接收配置
	
	private MQConfig() {
		try {
			readConfig();
		} catch (Exception e) {
			logger.error(e.getMessage(), e);
		}
	}

	public static MQConfig getInstance() {
		return _instance;
	}

	// 读取配置文件
	private void readConfig() {
		try {
			String configFile = MQConfig.class.getClassLoader().getResource("mqConfig.xml").getPath();

			SAXReader reader = new SAXReader();
			Document document = reader.read(new File(configFile));
			
			// 基本配置
			serverIP = document.selectSingleNode("/config/mqserver/serverip").getText();
			sendPort = Integer.valueOf(document.selectSingleNode("/config/mqserver/sendport").getText());
			recvPort = Integer.valueOf(document.selectSingleNode("/config/mqserver/recvport").getText());
			source = document.selectSingleNode("/config/mqserver/source").getText();
			
			// 文件发送配置
			List<Node> send = document.selectNodes("/config/send/product");
			for (Iterator<Node> iterator = send.iterator(); iterator.hasNext();) {
				Node product = iterator.next();
				
				String queueKey = product.selectSingleNode("queuekey").getText();
				String productkey = product.selectSingleNode("productkey").getText();
				String fileFormat = product.selectSingleNode("fileformat").getText();
				String reporttimes = product.selectSingleNode("reporttime").getText();
				String scanfolder = product.selectSingleNode("scanfolder").getText();
				String sendoverfolder = product.selectSingleNode("sendoverfolder").getText();
				
				SendProduct sendProduct = new SendProduct();
				sendProduct.setProductKey(productkey);
				sendProduct.setScanFolder(scanfolder);
				sendProduct.setSendoverFolder(sendoverfolder);
				sendProduct.setQueuekey(queueKey);
				sendProduct.setFileformat(fileFormat);
				sendProduct.setReporttime(reporttimes);
				
				listSendProduct.add(sendProduct);
			}
			
			
			// 文件接收配置
			List<Node> recv = document.selectNodes("/config/recv/product");
			for (Iterator<Node> iterator = recv.iterator(); iterator.hasNext();) {
				Node product = iterator.next();
				String queuekey = product.selectSingleNode("queuekey").getText();
				String savefolder = product.selectSingleNode("savefolder").getText();
				
				RecvProduct recvProduct = new RecvProduct();
				recvProduct.setQueueKey(queuekey);
				recvProduct.setSaveFolder(savefolder);
				
				listRecvProduct.add(recvProduct);
			}
			
		} catch (Exception e) {
			logger.error(e.getMessage(), e);
		}
	}

	public String getServerIP() {
		return serverIP;
	}

	public Integer getSendPort() {
		return sendPort;
	}

	public Integer getRecvPort() {
		return recvPort;
	}

	public String getSource() {
		return source;
	}

	public List<SendProduct> getListSendProduct() {
		return listSendProduct;
	}

	public List<RecvProduct> getListRecvProduct() {
		return listRecvProduct;
	}
}


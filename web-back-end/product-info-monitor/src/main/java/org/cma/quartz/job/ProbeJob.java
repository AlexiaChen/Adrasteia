package org.cma.quartz.job;

import java.io.File;
import java.sql.Connection;
import java.sql.ResultSet;
import java.sql.Statement;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.List;

import org.cma.database.connect.DBConnectionPoolSingle;
import org.cma.database.operation.DataBase;
import org.cma.utils.MQConfig;
import org.cma.utils.ScanFiles;
import org.cma.utils.SendProduct;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import cn.org.cma.activemq.message.MessageHeader;
import cn.org.cma.activemq.xwfd.Sender;

public class ProbeJob implements IJob {
	private static transient final Logger logger = LoggerFactory.getLogger(ProbeJob.class);
	private static boolean running = false;

	private String severIP;
	private Integer sendPort;
	private String source;
	private Integer LifeCycle = 3600;
	private Integer ForecastType = 0;
	private List<SendProduct> listSendProduct;

	// 构造函数初始化
	public ProbeJob() {
		severIP = MQConfig.getInstance().getServerIP();
		sendPort = MQConfig.getInstance().getSendPort();
		source = MQConfig.getInstance().getSource();
		listSendProduct = MQConfig.getInstance().getListSendProduct();

	}

	private boolean status() {
		return running;
	}

	private void running() {
		running = true;
	}

	private void runover() {
		running = false;
	}

	@Override
	public void run() {
		if (status()) {
			// 正在运行，则本次不执行
			return;
		}
		// 开始执行
		running();

		try {
			boolean bRet = ExecTask();
		} catch (Exception e) {
			logger.error(e.getMessage(), e);
		}

		// 执行结束
		runover();
	}

	private boolean ExecTask() {
		try {
			// 建立连接
			Sender sf = new Sender(severIP, sendPort);
			String strConnResult = sf.connect();
			if (strConnResult == "connect success") {
				logger.info("Connect MQ Server: " + severIP + ":" + sendPort + " Success");
			} else {
				logger.info("Connect MQ Server: " + severIP + ":" + sendPort + " Failed");

				 return false;
			}

			// 按照配置的产品，文件夹进行处理
			for (SendProduct product : listSendProduct) {
				// 处理文件发送
				ProcessProduct(sf, product);
			}

			// 处理完毕，断开连接
			String strDisResult = sf.disconnect();
			if (strDisResult == "disconnect success") {
				logger.info("Disconnect MQ Server Success");
			} else {
				logger.info("Disconnect MQ Server Failed");
			}

		} catch (Exception e) {
			logger.error(e.getMessage(), e);
			return false;
		}

		return true;
	}

	private void ProcessFiles(Sender sf, File[] files, String prodkey,
			String scanfolder,
			String sendoverfolder,
			String queuekey,
			String reporttime) {
		if (files.length != 0) {
			for (File file : files) {
				if (!DataBase.getInstance().isFileSent(file.getName())) {
					// 开始发送文件
					logger.info("Start Send File: " + file.getName());

					// 时间
					SimpleDateFormat sdf = new SimpleDateFormat("yyyyMMdd");
					Date date = new Date();
					String yyyymmdd = sdf.format(date);

					Calendar now = Calendar.getInstance();
					int year = now.get(Calendar.YEAR);
					int month = now.get(Calendar.MONTH) + 1;
					int day = now.get(Calendar.DAY_OF_MONTH);

					// 信息头
					MessageHeader header = new MessageHeader(source, LifeCycle, queuekey, 0, "0", ForecastType, "dataID", yyyymmdd, file.getName(), scanfolder);

					// 发送文件
					String strResult = sf.sendMessage(header, "FILE", file);
					if (strResult == "SendFile success") {
						logger.info("Send File: " + file.getName() + " Success");

						// 检测文件名是否在DB中存在，如果存在只用更新status字段，如果不存在，需要插入新的一行
						if (DataBase.getInstance().isFileExistMqSend(file.getName())) {

							DataBase.getInstance().updateToMqSend("SUCCESS", file.getName());
						} else {
							DataBase.getInstance().InsertMqSend("SUCCESS", queuekey, prodkey,
									scanfolder, file.getName(), reporttime,
									year, month, day, "", "", "", "");
						}
					} else {
						logger.info("Send File: " + file.getName() + " Failed");

						// 同上
						if (DataBase.getInstance().isFileExistMqSend(file.getName())) {
							DataBase.getInstance().updateToMqSend("FAILURE", file.getName());
						} else {
							DataBase.getInstance().InsertMqSend("FAILURE", queuekey, prodkey,
									scanfolder, file.getName(), reporttime,
									year, month, day, "", "", "", "");
						}

					}

					// 发送完毕后的处理
					if (sendoverfolder != "") {
						// 检测文件夹
						File dir = new File(sendoverfolder);
						if (!dir.exists()) {
							dir.mkdirs();
						}

						// 移动文件
						File completeFile = new File(sendoverfolder, file.getName());
						if (file.renameTo(completeFile)) {
							logger.info("Remove File " + file.getName() + "Success");
						} else {
							logger.info("Remove File" + file.getName() + "Failed");
						}
					}

				}

			}
		}
	}

	// 发送文件
	private void ProcessProduct(Sender sf, SendProduct product) {

		String productkey = product.getProductKey();
		String scanfolder = ScanFiles.replaceWithDate(product.getScanFolder(), "", "");
		String sendoverfolder = product.getSendoverFolder();
		String queueKey = product.getQueuekey();
		String fileformat = product.getFileformat();
		String reporttime = product.getReporttime();

		logger.info("Scan Disk: " + scanfolder);

		for (String reportTime : reporttime.split(",")) {
			// 替换文件格式
			String fileFormat = ScanFiles.replaceWithDate(fileformat,
					"", reportTime);

			// 根据文件格式扫描路径
			File[] files = ScanFiles.isFileExists(scanfolder, fileFormat);
			if (files != null) {
				ProcessFiles(sf, files, productkey, scanfolder, sendoverfolder,
						queueKey, reportTime);
			}

		}

		

	}

}

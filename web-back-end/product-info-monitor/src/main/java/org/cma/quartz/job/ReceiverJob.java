package org.cma.quartz.job;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.HashMap;
import java.util.List;

import javax.jms.MessageConsumer;

import org.apache.commons.io.IOUtils;
import org.cma.utils.Config;
import org.cma.utils.MQConfig;
import org.cma.utils.RecvProduct;
import org.cma.utils.SendProduct;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import cn.org.cma.activemq.message.CompositeMessage;
import cn.org.cma.activemq.message.MessageHeader;
import cn.org.cma.activemq.xwfd.Receiver;
import cn.org.cma.activemq.xwfd.Sender;

/**
 * 该作业是接收消息的作业。从MQ中接收消息后，将其写入到磁盘上，然后C++程序读取磁盘上的文件，进行后续处理
 * 
 * @author Jeff
 * 
 */
public class ReceiverJob implements IJob {
	private static transient final Logger logger = LoggerFactory.getLogger(ReceiverJob.class);
	private static boolean running = false;
	
	private String  severIP  = MQConfig.getInstance().getServerIP();
	private Integer recvPort = MQConfig.getInstance().getRecvPort();
	private List<RecvProduct> listRecvProduct = MQConfig.getInstance().getListRecvProduct();
	private String otherFolder = Config.GetConfig().getProperty("mq_recv_defualt");
	
	private boolean status()
	{
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
		if(status()) {
			// 正在运行，则本次不执行
			return;			
		}
	
		// 开始执行
		running();
		
		try {
			// 执行接收任务
			ExecTask();
		}catch(Exception e) {
			logger.error(e.getMessage(), e);
		}
		
		// 执行结束
		runover();
	}

	// 执行接收任务
	private boolean ExecTask() {
		try {
			// 创建接收
			Receiver rf = new Receiver(severIP, recvPort);
			String strConnResult = rf.connect();
			if (strConnResult == "connect success") {
				logger.info("Connect MQ Server: " + severIP + ":" + recvPort + " Success");
			} else {
				logger.info("Connect MQ Server: " + severIP + ":" + recvPort + " Failed");
				return false;
			}

			// 按照配置的产品，遍历处理
            for(RecvProduct product:listRecvProduct) {
            	// 处理文件接收
				ProcessProduct(rf, product);
            }

			// 断开连接
			String strDisResult = rf.disconnect();
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
	
	// 按照配置的queue接收文件
	private boolean ProcessProduct(Receiver rf, RecvProduct product) {
		String strQueueKey = product.getQueueKey();
		String saveFolder = product.getSaveFolder();
		if(strQueueKey.isEmpty() ) {
			return false;
		}
		
		try {
			// 创建消费对象
			MessageConsumer consumer = rf.createConsumer(strQueueKey);
			CompositeMessage comMsg = null;
			int nCount = 0;
			while (true) {
				// 延时
				Thread.sleep(500);
				
				// 接收数据
				if ((comMsg = rf.recvMessage(consumer)) != null) {
					MessageHeader header = comMsg.getHeader();
					logger.info("Recv Message Success:" + header.getSource() + "," + header.getYyyymmdd() + "," + header.getFileName());
					InputStream instream = comMsg.getStream();
					
					if (instream != null) {
						if ( saveFile(strQueueKey, instream, header, saveFolder)== "SUCCESS" ) {
							logger.info("Recv File: [" + header.getSource() + "]:" + header.getFileName() + " Success");
						} else {
							logger.info("Recv File: [" + header.getSource() + "]:" + header.getFileName() + " Failed");
						}
					}
					nCount = 0;
				} else {
					// 连续3次收不到数据，结束本次接收作业
					if (nCount++ >= 3)
						break;
				}
			}

		} catch (Exception e) {
			logger.error(e.getMessage(), e);
			return false;
		}
		
		return true;
	}

	// 保存接收的文件信息到本地
	private String saveFile(String strQueueKey, InputStream instream, MessageHeader header, String saveFolder) {
		String strResult = "SUCCESS";
		
		if(saveFolder.isEmpty()) {
			// 配置的文件夹为空时，不保存数据，只消费数据
			logger.info("File not be saved：" + header.getFileName());
			strResult = "保存文件夹为空";

			return strResult;
		}
		
		// 当前日期
		/*
		SimpleDateFormat df = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss.z");// 设置日期格式
		String current = df.format(new Date());
		Calendar c = Calendar.getInstance();//可以对每个时间域单独修改
		int year = c.get(Calendar.YEAR); 
		int month = c.get(Calendar.MONTH); 
		int date = c.get(Calendar.DATE); 
		int hour = c.get(Calendar.HOUR_OF_DAY); 
		*/
		
		// 保存数据文件
		try {
			String strFileName = header.getFileName();
			String strCCCC;
			String strType;
			String strTime;
			String strYear, strMonth, strDay, strHour;
			
			// 解析文件名
			if(strQueueKey.indexOf("NWGD") >= 0) {
				// 格点数据
				// 文件格式：Z_NWGD_C_BABJ_20150831224736_P_RFFC_SCMOC-EFG_201509010800_02424.GRB2
				String fileFormat[] = strFileName.split("_|-");  
				if(fileFormat.length != 11) {
					// 正常格式的文件能分成11个数据
					// strResult = "文件名格式不正确";
					logger.info("文件名格式不正确：" + header.getFileName());
					if(!otherFolder.isEmpty()) {
						strResult = saveStream2File(instream, otherFolder, header.getFileName());
					}
					
					return strResult;
				} 
				
				strCCCC = fileFormat[3];
				strType = fileFormat[8];
				strTime = fileFormat[9];
				strYear = strTime.substring(0, 4);
				strMonth = strTime.substring(4, 6);
				strDay = strTime.substring(6, 8);
				strHour = strTime.substring(8, 10);
				
				// 文件夹替换字符
				String strRecvPath = saveFolder.replace("[TYPE]", strType);
				strRecvPath = strRecvPath.replace("[CCCC]", strCCCC);
				strRecvPath = strRecvPath.replace("[YYYY]", strYear);
				strRecvPath = strRecvPath.replace("[MM]", strMonth);
				strRecvPath = strRecvPath.replace("[DD]", strDay);
				strRecvPath = strRecvPath.replace("[HH]", strHour);
				
				// 保存字符流到文件
				strResult = saveStream2File(instream, strRecvPath, strFileName);
			} else {
				// 站点数据，不进行解析-要求保存站点文件的文件夹不要有替换符
				strResult = saveStream2File(instream, saveFolder, strFileName);
			}
			
		} catch (Exception e) {
			logger.error(e.getMessage(), e);
			strResult = "异常" + e.getMessage();
			return strResult;
		}

		return strResult;
	}
	
	// 保存字符流到文件
	private String saveStream2File(InputStream instream, String strRecvPath, String strFileName){
		String strResult = "SUCCESS";
		
		if(strFileName.isEmpty()){
			strResult = "文件名为空";
			return strResult;
		}
		try {
			// create a directory if it not exists
			File newDir = new File(strRecvPath);
			newDir.mkdirs();
			
			// 保存文件
			String strTmpFileName = strFileName + ".tmp";
			File tmpfile = new File(strRecvPath, strTmpFileName);

			OutputStream outstream = new FileOutputStream(tmpfile);
			IOUtils.copy(instream, outstream);
			instream.close();
			outstream.close();

			logger.info("保存文件成功："+ strRecvPath + " " + strTmpFileName);

			// 保存文件成功，重命名文件
			File recvFile = new File(strRecvPath, strFileName);
			boolean bFlg = tmpfile.renameTo(recvFile);
			if (!bFlg) {
				// 重命名失败
				logger.info("File Rename Failed：" + strRecvPath + strFileName);
				strResult = "重命名失败";
				return strResult;
			}
		} catch (Exception e) {
			logger.error(e.getMessage(), e);
			strResult = "异常" + e.getMessage();
		}

		return strResult;
	}
	
}

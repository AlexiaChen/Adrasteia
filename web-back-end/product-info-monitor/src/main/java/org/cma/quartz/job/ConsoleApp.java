package org.cma.quartz.job;

import org.cma.test.DataBaseTest;
import org.cma.test.OracleTest;
import org.cma.utils.Config;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * 控制台方式运行程序，调试用方便。生产环境上是通过Tomcat容器里加载Quartz启动的。
 * 
 * @author Jeff
 *
 */

public class ConsoleApp {

	private static transient final Logger logger = LoggerFactory.getLogger(ConsoleApp.class);
	
	
	public static void main(String[] args) {

		logger.info("开始逐个调度作业");
/*
		logger.info("开始发送作业");
		ProbeJob j = new ProbeJob();
		j.run();
*/
		logger.info("开始接收作业");
		ReceiverJob r = new ReceiverJob();
		r.run();
       
		/*
		logger.info("开始发送消息作业");
		SendMsgJob s = new SendMsgJob();
		s.run();
		*/
		System.out.println("Oracle test start");
		String url = Config.GetConfig().getProperty("url");
		String driverName = Config.GetConfig().getProperty("driver");
		
		DataBaseTest dbTest = new OracleTest();
		dbTest.setDriver(driverName);
		dbTest.setDataBaseURL(url);
		dbTest.setUser("C##mathxh");
		dbTest.setPassWord("753951");
		
		if(dbTest.open()){
			System.out.println("open oracle success");
			
			final long startTime = System.currentTimeMillis();
			
			dbTest.testInsert("NwfdStationSend", 100000);
			
			final long endTime = System.currentTimeMillis();

			System.out.println("Total execution time: " + (endTime - startTime) );
		
		}else{
			System.out.println("open failed");
		}
	}

}

package org.cma.quartz.job;

import java.sql.Connection;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.List;

import org.cma.cached.ICommonsVars;
import org.cma.config.JobConfig;
import org.cma.database.connect.DBConnectionPoolSingle;
import org.cma.product.SendMonProduct;
import org.cma.product.files.SendProductFile;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class SendMonitor implements IJob {

	// 日志
	private static transient final Logger logger = LoggerFactory.getLogger(SendMonitor.class);

	private List<SendMonProduct> m_products = JobConfig.getInstance().getSendMonProductList();

	private static boolean running = false;
	private static String yesterdayDate;

	static {
		SimpleDateFormat df = new SimpleDateFormat("yyyyMMDD");// 设置日期格式
		yesterdayDate = df.format(new Date());
	}

	public SendMonitor() {
		ICommonsVars.globalSendProducts = m_products;
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

	private void cleanYesterdayData() {
		// 得到当前日期
		SimpleDateFormat df = new SimpleDateFormat("yyyyMMDD");// 设置日期格式
		String currentDate = df.format(new Date());

		if (!currentDate.equals(yesterdayDate)) {
			for (SendMonProduct prod : ICommonsVars.globalSendProducts) {
				prod.getFileSet().clearAll();
			}

			// 更新状态
			yesterdayDate = currentDate;
		}
	}

	private boolean processInfoByKey(SendMonProduct prod, String productKey) {

		boolean bRet = false;
		Connection dbconnect = null;

		// 获取数据库连接
		dbconnect = DBConnectionPoolSingle.getInstance().getConnection();
		if (dbconnect == null) {
			return false;
		}

		// 执行方法
		try {
			Statement stmt = dbconnect.createStatement();

			Calendar now = Calendar.getInstance();
			int currYear = now.get(Calendar.YEAR);
			int currMonth = now.get(Calendar.MONTH) + 1;
			int currDay = now.get(Calendar.DAY_OF_MONTH);

			String select = "SELECT * FROM Tbl_MqSend WHERE productkey="
					+ "'" + productKey + "'"
					+ " AND year=" + currYear
					+ " AND month=" + currMonth
					+ " AND day=" + currDay;

			ResultSet rs = stmt.executeQuery(select);

			while (rs.next()) {
				String status = rs.getString("status");
				String filename = rs.getString("filename");
				String reporttime = rs.getString("reporttime");

				SendProductFile file = new SendProductFile();
				file.setFileName(filename);
				file.setReportTime(reporttime);
				file.setShouldSendTime(prod.getSendtime());
				file.setSendTime("");

				if (status.equals("SUCCESS")) {

					file.setResult(true);
				} else {
					file.setResult(false);
				}
				
				//检查是否在List存在同名文件，如果存在，则不添加
				if(!prod.getFileSet().isExist(filename)){
					prod.getFileSet().add(file);
				}
				
				

			}

		} catch (SQLException e) {
			// 释放数据库连接
			if (dbconnect != null) {
				DBConnectionPoolSingle.getInstance().freeConnection(dbconnect);
			}

			return false;
		}

		// 释放数据库连接
		if (dbconnect != null) {
			DBConnectionPoolSingle.getInstance().freeConnection(dbconnect);
		}

		return bRet;

	}

	private boolean ExecTask() {

		for (SendMonProduct prod : m_products) {

			String productKey = prod.getKey();

			processInfoByKey(prod, productKey);

		}

		return false;
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

}

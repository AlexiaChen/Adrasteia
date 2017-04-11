package org.cma.database.operation;

import java.sql.Connection;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.Calendar;
import java.util.List;

import org.cma.database.connect.DBConnectionPool;
import org.cma.database.connect.DBConnectionPoolSingle;
import org.cma.product.MergeProduct;
import org.cma.product.files.MergeProductFile;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 *
 * @author zhangl
 * 
 *         数据库操作
 */

public class DataBase {
	private static transient final Logger logger = LoggerFactory.getLogger(DataBase.class);
	private static DataBase _instance = new DataBase();
	private DBConnectionPool dbConnectPool = null;

	private DataBase() {
		dbConnectPool = DBConnectionPoolSingle.getInstance();
	}

	public static DataBase getInstance() {
		return _instance;
	}

	/* insert Tabl_MQRecv */
	public boolean InsertMqRecv(String status, String queueKey, String fileName, String saveFolder, String provinceCode, String forecastType,
			String dataType, int year, int month, int day, int hour, String createTime, String errReason) {
		if (dbConnectPool == null) {
			return false;
		}
		boolean bRet = false;
		Connection dbconnect = null;

		// 获取数据库连接
		dbconnect = dbConnectPool.getConnection();
		if (dbconnect == null) {
			return false;
		}

		// 执行方法
		try {
			Statement stmt = dbconnect.createStatement();
			String sql = "";
			if (sql == "SUCCESS") {
				sql = "Insert into Tbl_MQRecv(status,queueKey,fileName,saveFolder,provinceCode,forecastType,dataType,year,month,day,hour,createTime) values('SUCCESS','" +
						queueKey + "','" + fileName + "','" + saveFolder + "','" + provinceCode + "','" + forecastType + "','" + dataType + "'," +
						year + "," + month + "," + day + "," + hour + ",'" + createTime + "')";
			} else {
				sql = "Insert into Tbl_MQRecv(status,fileName,saveFolder,errReason) values('FAILURE','" + fileName + "','" + saveFolder + "','" + errReason + "')";
			}

			// 执行Insert语句
			bRet = stmt.execute(sql);

		} catch (SQLException e) {
			bRet = false;
		} finally {
			// 释放数据库连接
			if (dbconnect != null) {
				dbConnectPool.freeConnection(dbconnect);
			}

		}

		return bRet;
	}

	// 检查文件是否发送
	public boolean isFileSent(String filename) {
		if (dbConnectPool == null) {
			return false;
		}
		boolean bRet = false;
		Connection dbconnect = null;

		// 获取数据库连接
		dbconnect = dbConnectPool.getConnection();
		if (dbconnect == null) {
			return false;
		}

		// 执行方法
		try {
			Statement stmt = dbconnect.createStatement();
			String query = "select * from Tbl_MqSend where filename=" +
					"\"" + filename + "\"" + ";";

			ResultSet rs = stmt.executeQuery(query);

			while (rs.next()) {
				String fileName = rs.getString("filename");

				if (fileName.equals(filename)) {
					String status = rs.getString("status");
					if (status.equals("SUCCESS")) {
						bRet = true;
					} else if (status.equals("FAILURE")) {
						bRet = false;
					}
				}
			}

		} catch (SQLException e) {

			bRet = false;

		} finally {
			// 释放数据库连接
			if (dbconnect != null) {
				dbConnectPool.freeConnection(dbconnect);
			}
		}

		return bRet;
	}

	// 检查文件名是否在DataBase中
	public boolean isFileExistMqSend(String filename) {
		if (dbConnectPool == null) {
			return false;
		}
		boolean bRet = false;
		Connection dbconnect = null;

		// 获取数据库连接
		dbconnect = dbConnectPool.getConnection();
		if (dbconnect == null) {
			return false;
		}

		// 执行方法
		try {
			Statement stmt = dbconnect.createStatement();
			String query = "select * from Tbl_MqSend where filename=" +
					"\"" + filename + "\"" + ";";

			ResultSet rs = stmt.executeQuery(query);

			if (rs.next()) {

				bRet = true;

			}

		} catch (SQLException e) {

			bRet = false;
		} finally {
			// 释放数据库连接
			if (dbconnect != null) {
				dbConnectPool.freeConnection(dbconnect);
			}
		}

		return bRet;
	}

	public boolean isProductExistInTask(String fileName, String reportTime) {
		if (dbConnectPool == null) {
			return false;
		}
		boolean bRet = false;
		Connection dbconnect = null;

		// 获取数据库连接
		dbconnect = dbConnectPool.getConnection();
		if (dbconnect == null) {
			return false;
		}

		// 得到当前日期
		Calendar c = Calendar.getInstance();// 可以对每个时间域单独修改
		int year = c.get(Calendar.YEAR);
		int month = c.get(Calendar.MONTH) + 1;
		int day = c.get(Calendar.DAY_OF_MONTH);

		// 执行方法
		try {
			Statement stmt = dbconnect.createStatement();
			String query = String.format("select * from Tbl_Task where filename='%s' and "
					+ "reporttime=%d and year=%d and month=%d and day=%d", fileName, Integer.parseInt(reportTime), year, month, day);

			ResultSet rs = stmt.executeQuery(query);

			if (rs.next()) {

				bRet = true;

			}

		} catch (SQLException e) {

			bRet = false;
		} finally {
			// 释放数据库连接
			if (dbconnect != null) {
				dbConnectPool.freeConnection(dbconnect);
			}
		}

		return bRet;
	}

	public boolean InsertMqSend(String status, String queueKey,
			String productKey, String folder,
			String filename, String reporttime, int year, int month,
			int day, String starttime, String finishTime,
			String errReason, String errLevel) {

		if (dbConnectPool == null) {
			return false;
		}
		boolean bRet = false;
		Connection dbconnect = null;

		// 获取数据库连接
		dbconnect = dbConnectPool.getConnection();
		if (dbconnect == null) {
			return false;
		}

		// 执行方法
		try {
			Statement stmt = dbconnect.createStatement();
			String insert = "INSERT INTO Tbl_MqSend(status,queuekey,productkey,"
					+ "folder,"
					+ "filename,reporttime,year,month,day,"
					+ "startTime,finishTime,errReason,errLevel) "
					+ "VALUES('" + status + "','"
					+ queueKey + "','"
					+ productKey + "','"
					+ folder + "','"
					+ filename + "','"
					+ reporttime + "',"
					+ String.valueOf(year) + ","
					+ String.valueOf(month) + ","
					+ String.valueOf(day) + ",'"
					+ starttime + "','"
					+ finishTime + "','"
					+ errReason + "','"
					+ errLevel
					+ "')";

			if (stmt.execute(insert)) {
				bRet = true;
			}

		} catch (SQLException e) {
			bRet = false;
		} finally {
			// 释放数据库连接
			if (dbconnect != null) {
				dbConnectPool.freeConnection(dbconnect);
			}
		}

		return bRet;
	}

	public boolean updateToMqSend(String status, String filename) {

		if (dbConnectPool == null) {
			return false;
		}
		boolean bRet = false;
		Connection dbconnect = null;

		// 获取数据库连接
		dbconnect = dbConnectPool.getConnection();
		if (dbconnect == null) {
			return false;
		}

		// 执行方法
		try {
			Statement stmt = dbconnect.createStatement();
			String update = "UPDATE Tbl_MqSend SET "
					+ "status=" + "'" + status + "'"
					+ " WHERE filename="
					+ "'" + filename + "'";

			if (stmt.execute(update)) {
				bRet = true;
			}

		} catch (SQLException e) {
			bRet = false;
		} finally {
			// 释放数据库连接
			if (dbconnect != null) {
				dbConnectPool.freeConnection(dbconnect);
			}
		}

		return bRet;
	}

	/* Tbl_Task */
	public boolean getCurrentTaskStatus(List<MergeProduct> products) {

		if (dbConnectPool == null) {
			return false;
		}
		boolean bRet = false;
		Connection dbconnect = null;

		// 获取数据库连接
		dbconnect = dbConnectPool.getConnection();
		if (dbconnect == null) {
			return false;
		}
		// 得到当前日期
		Calendar c = Calendar.getInstance();// 可以对每个时间域单独修改
		int year = c.get(Calendar.YEAR);
		int month = c.get(Calendar.MONTH) + 1;
		int day = c.get(Calendar.DAY_OF_MONTH);

		// 执行方法
		try {

			Statement stmt = dbconnect.createStatement();

			for (MergeProduct prod : products) {
				String key = prod.getProductKey();
				String role = "merge";
				prod.getFileSet().clearAll();

				for (String reportTime : prod.getReporttimes()) {

					String query = String.format("SELECT id,status,role,"
							+ "year,month,day,folder,filename,"
							+ "reporttime,forecasttime,"
							+ "startTime,finishTime,errReason "
							+ "FROM Tbl_Task "
							+ "WHERE year=%d and month=%d and day=%d"
							+ " and productkey='%s' and role='%s' and "
							+ "reporttime=%d",
							year, month, day, key, role,Integer.parseInt(reportTime));

					ResultSet rs = stmt.executeQuery(query);

					if (rs.next()) {

						MergeProductFile file = new MergeProductFile();
						file.setStatus(rs.getString("status"));
						file.setFileName(rs.getString("filename"));
						file.setShouldTime(prod.getShouldTime());
						file.setPracticalTime(rs.getString("finishTime"));
						file.setErrReason(rs.getString("errReason"));
						file.setReportTime(rs.getString("reporttime"));

						if (rs.getString("status").equals("complete")) {
							file.setResult(true);
						} else {
							file.setResult(false);
						}
						
						prod.getFileSet().add(file);

					}else{
						MergeProductFile file = new MergeProductFile();
						file.setStatus("");
						file.setFileName("");
						file.setShouldTime(prod.getShouldTime());
						file.setPracticalTime("");
						file.setErrReason("");
						file.setReportTime(reportTime);

						
						file.setResult(false);
						
						prod.getFileSet().add(file);
						
					}

				}

			}

		} catch (SQLException e) {
			bRet = false;
		} finally {
			// 释放数据库连接
			if (dbconnect != null) {
				dbConnectPool.freeConnection(dbconnect);
			}
		}

		return bRet;
	}

	public boolean UpdateToTaskStatus(String productkey, String reportTime) {

		if (dbConnectPool == null) {
			return false;
		}
		boolean bRet = false;
		Connection dbconnect = null;

		// 获取数据库连接
		dbconnect = dbConnectPool.getConnection();
		if (dbconnect == null) {
			return false;
		}
		// 得到当前日期
		Calendar c = Calendar.getInstance();// 可以对每个时间域单独修改
		int year = c.get(Calendar.YEAR);
		int month = c.get(Calendar.MONTH) + 1;
		int day = c.get(Calendar.DAY_OF_MONTH);

		boolean flag = false;

		try {
			Statement is_stmt = dbconnect.createStatement();
			String query = String.format("select * from Tbl_Task where productkey='%s' and "
					+ "reporttime=%d and year=%d and month=%d and day=%d", productkey, Integer.parseInt(reportTime), year, month, day);

			ResultSet rs = is_stmt.executeQuery(query);

			if (rs.next()) {

				flag = true;

			} else {
				flag = false;
			}
		} catch (SQLException e) {
			bRet = false;
		}

		// 如果任务表中存在产品信息就更新，没有就重新插入一条
		if (flag) {

			try {
				Statement stmt = dbconnect.createStatement();
				String update = String.format("UPDATE Tbl_Task SET "
						+ "status='%s' WHERE productkey='%s' and "
						+ "reporttime=%d and year=%d"
						+ " and month=%d"
						+ " and day=%d", "reprocess", productkey, Integer.parseInt(reportTime), year, month, day);

				if (stmt.execute(update)) {
					bRet = true;
				}

			} catch (SQLException e) {
				bRet = false;
			} finally {
				// 释放数据库连接
				if (dbconnect != null) {
					dbConnectPool.freeConnection(dbconnect);
				}
			}
		} else {

			try {
				Statement stmt = dbconnect.createStatement();
				String insert = String.format("INSERT INTO Tbl_Task"
						+ " (productkey,reporttime,status,year,month,day,role)"
						+ " VALUES ('%s',%d,'%s',%d,%d,%d)",
						productkey, Integer.parseInt(reportTime), "reprocess", year, month, day, "merge");

				if (stmt.execute(insert)) {
					bRet = true;
				}

			} catch (SQLException e) {
				bRet = false;
			} finally {
				// 释放数据库连接
				if (dbconnect != null) {
					dbConnectPool.freeConnection(dbconnect);
				}
			}

		}

		return bRet;
	}

}

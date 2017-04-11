package org.cma.test;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.Random;

public class OracleTest implements DataBaseTest {

	private String dbURL;
	private String user;
	private String password;
	private String driverName;
	private static Connection connect = null;

	public void setDriver(String name) {
		this.driverName = name;
	}

	public void setDataBaseURL(String url) {
		this.dbURL = url;
	}

	public boolean open() {

		// 反射Oracle数据库驱动程序类
		try {
			Class.forName(driverName);

			// 获取数据库连接
			connect = DriverManager.getConnection(dbURL, user, password);

			if (connect == null) {
				return false;
			}

		} catch (Exception e) {

			e.printStackTrace();
			return false;
		}

		return true;
	}

	public void setUser(String user) {
		this.user = user;
	}

	public void setPassWord(String pswd) {
		this.password = pswd;
	}

	public void testInsert(String tablename, int numRepeat) {

		Random random = new Random();

		try {

			Statement stmt = connect.createStatement();

			for (int i = 1; i <= numRepeat; ++i) {

				String insertStr = String.format(
						"INSERT INTO %s(id,status,airPressure) VALUES(%d,'%s',%f)",
						tablename,
						i, "SUCCESS", random.nextDouble() * 100 + 10);

				stmt.execute(insertStr);

			}
			
			connect.close();

		} catch (SQLException e) {
			e.printStackTrace();
           
		}

	}
}

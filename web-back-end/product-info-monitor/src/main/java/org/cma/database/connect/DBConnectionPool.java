package org.cma.database.connect;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.Iterator;

public class DBConnectionPool {
	private Connection con = null;
	private int inUsed = 0; // 使用的连接数
	private ArrayList<Connection> freeConnections = new ArrayList<Connection>();// 容器，空闲连接
	private int minConn; // 最小连接数
	private int maxConn; // 最大连接
	private String name; // 连接池名字
	private String password; // 密码
	private String url; // 数据库连接地址
	private String driver; // 驱动
	private String user; // 用户名

	/**
	 * 创建连接池
	 * 
	 * @param driver
	 * @param name
	 * @param URL
	 * @param user
	 * @param password
	 * @param maxConn
	 */
	public DBConnectionPool(String name, String driver, String URL, String user, String password, int maxConn) {
		this.name = name;
		this.driver = driver;
		this.url = URL;
		this.user = user;
		this.password = password;
		this.maxConn = maxConn;

		for (int i = 0; i < maxConn; ++i) {
			Connection con = newConnection();
			if (con != null) {
				freeConnections.add(con);
			}
		}
	}

	/**
	 * 用完，释放连接
	 * 
	 * @param con
	 */
	public void freeConnection(Connection con) {

		synchronized (this) {
			if (con != null) {
				if (freeConnections.size() < maxConn) {
					this.freeConnections.add(con);// 添加到空闲连接的末尾
					this.inUsed--;
				}
			}

		}

	}

	/**
	 * 
	 * 从连接池里得到连接
	 * 
	 * @return
	 */
	public Connection getConnection() {

		synchronized (this) {
			if (this.freeConnections.size() > 0) {
				Connection con = this.freeConnections.get(0);
				this.freeConnections.remove(0);
				this.inUsed++;

				System.out.println("得到　" + this.name + "　的连接，现有" + inUsed + "个连接在使用!");

				return con;
			}
		}

		return null;
	}

	/**
	 * 释放全部连接
	 * 
	 */
	public synchronized void release() {
		Iterator<Connection> allConns = this.freeConnections.iterator();
		while (allConns.hasNext()) {
			Connection con = (Connection) allConns.next();
			try {
				con.close();
			} catch (SQLException e) {
				e.printStackTrace();
			}

		}
		this.freeConnections.clear();

	}

	/**
	 * 创建新连接
	 * 
	 * @return
	 */
	private Connection newConnection() {
		try {
			Class.forName(driver);
			con = DriverManager.getConnection(url, user, password);
		} catch (ClassNotFoundException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			System.out.println("sorry can't find db driver!");
		} catch (SQLException e1) {
			// TODO Auto-generated catch block
			e1.printStackTrace();
			System.out.println("sorry can't create Connection!");
		}
		return con;

	}

	/**
	 * @return the driver
	 */
	public String getDriver() {
		return driver;
	}

	/**
	 * @param driver
	 *            the driver to set
	 */
	public void setDriver(String driver) {
		this.driver = driver;
	}

	/**
	 * @return the maxConn
	 */
	public int getMaxConn() {
		return maxConn;
	}

	/**
	 * @param maxConn
	 *            the maxConn to set
	 */
	public void setMaxConn(int maxConn) {
		this.maxConn = maxConn;
	}

	/**
	 * @return the minConn
	 */
	public int getMinConn() {
		return minConn;
	}

	/**
	 * @param minConn
	 *            the minConn to set
	 */
	public void setMinConn(int minConn) {
		this.minConn = minConn;
	}

	/**
	 * @return the name
	 */
	public String getName() {
		return name;
	}

	/**
	 * @param name
	 *            the name to set
	 */
	public void setName(String name) {
		this.name = name;
	}

	/**
	 * @return the password
	 */
	public String getPassword() {
		return password;
	}

	/**
	 * @param password
	 *            the password to set
	 */
	public void setPassword(String password) {
		this.password = password;
	}

	/**
	 * @return the url
	 */
	public String getUrl() {
		return url;
	}

	/**
	 * @param url
	 *            the url to set
	 */
	public void setUrl(String url) {
		this.url = url;
	}

	/**
	 * @return the user
	 */
	public String getUser() {
		return user;
	}

	/**
	 * @param user
	 *            the user to set
	 */
	public void setUser(String user) {
		this.user = user;
	}
}
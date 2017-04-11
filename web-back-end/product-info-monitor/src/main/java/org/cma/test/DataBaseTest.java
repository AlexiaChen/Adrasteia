package org.cma.test;

public interface DataBaseTest {
	public void setDriver(String name);
	public void setDataBaseURL(String url);
	public void setUser(String user);
	public void setPassWord(String pswd);
	public boolean open();
	public void testInsert(String tablename, int numRepeat);
}

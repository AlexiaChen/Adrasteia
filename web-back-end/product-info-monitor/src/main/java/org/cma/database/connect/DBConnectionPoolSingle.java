package org.cma.database.connect;

import org.cma.utils.Config;

public class DBConnectionPoolSingle {
     private static DBConnectionPool instance = null;
     
     
     public static DBConnectionPool getInstance(){
    	 if(instance == null){

    			Integer maxConn = Integer.valueOf(Config.GetConfig().getProperty("max_conn"));
    			 String driver = Config.GetConfig().getProperty("driver");
    			 String url = Config.GetConfig().getProperty("url");
    			
    			instance =  new DBConnectionPool("pool",driver,url,null,null,
    					maxConn.intValue());
    	 }
    	 
    	 return instance;
     }
}

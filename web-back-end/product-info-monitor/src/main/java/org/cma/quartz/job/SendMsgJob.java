package org.cma.quartz.job;

import java.io.Reader;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import org.apache.ibatis.io.Resources;
import org.apache.ibatis.session.SqlSession;
import org.apache.ibatis.session.SqlSessionFactory;
import org.apache.ibatis.session.SqlSessionFactoryBuilder;
import org.cma.database.models.NwfdStationSend;
import org.cma.utils.Config;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import cn.org.cma.activemq.message.MessageHeader;
import cn.org.cma.activemq.xwfd.Sender;
import cn.org.cma.activemq.message.ProductBean;
import cn.org.cma.activemq.message.SPCCBean;
import cn.org.cma.activemq.message.SPCC6HBean;

public class SendMsgJob implements IJob {
    // 日志
	private static transient final Logger logger = LoggerFactory.getLogger(SendMsgJob.class);
	
	private String send_SeverIP = Config.GetConfig().getProperty("MQServerIP");
	private Integer send_Port = Integer.valueOf(Config.GetConfig().getProperty("send_Port"));
	private String Source = Config.GetConfig().getProperty("Source");
	private String ProductKey = Config.GetConfig().getProperty("ProductKey");
	private Integer LifeCycle = Integer.valueOf(Config.GetConfig().getProperty("LifeCycle"));
	private Integer ForecastType = Integer.valueOf(Config.GetConfig().getProperty("ForecastType"));

	// 数据库连接
    private static SqlSessionFactory sqlSessionFactory;

    private static Reader reader; 
    static{
        try{
            reader = Resources.getResourceAsReader("Configuration.xml");
            sqlSessionFactory = new SqlSessionFactoryBuilder().build(reader);
            //sqlSessionFactory.getConfiguration().addMapper(IDBOperation.class);
        }catch(Exception e){
            e.printStackTrace();
        }
    }
    public static SqlSessionFactory getSession(){
        return sqlSessionFactory;
    }

	@Override
	public void run() {
        SqlSession session = sqlSessionFactory.openSession();
        try {
			// 配置发送的IP地址为空时，不执行接收作业
			if(send_SeverIP.isEmpty()) {
				return;
			}
			
			// 建立连接
			Sender sf = new Sender (send_SeverIP, send_Port);
			String strConnResult = sf.connect();
			if(strConnResult == "connect success"){
				logger.info("Connect MQ Server: " + send_SeverIP + ":" +send_Port + " Success");
			}else {
				logger.info("Connect MQ Server: " + send_SeverIP + ":" +send_Port + " Failed");
				return;
			}
			
        	// 获取未处理的列表
        	//IDBOperation db = session.getMapper(IDBOperation.class);
            List<NwfdStationSend> lstStationMsg;// = db.getStationMsgList();
            
			// 时间
			SimpleDateFormat sdf=new SimpleDateFormat("yyyyMMdd");  
			Date date=new Date();  
			String yyyymmdd=sdf.format(date);  
			
			// 信息头
			MessageHeader header = new MessageHeader(Source, LifeCycle, ProductKey, 0, "0", ForecastType, "dataID", yyyymmdd);
			ArrayList<ProductBean> beans = new ArrayList<ProductBean>();
			
            // 遍历列表，构造bean
			/*
            for(NwfdStationSend msg:lstStationMsg) {
            	if(msg.getCityType() == "bigcity"){
            		// 大城市
            		ProductBean pb = new SPCC6HBean(msg.getStation(), 3600, 3600, msg.getWeather(), msg.gettMax(), msg.gettMin(), msg.getWindDirection(), msg.getWindSpeed(),msg.getPrecipitation());
            		beans.add(pb);
            	} else {
            		// 城镇
            		ProductBean pb = new SPCCBean(msg.getStation(), 3600, 3600, msg.getWeather(), msg.gettMax(), msg.gettMin(), msg.getWindDirection(), msg.getWindSpeed());
            		beans.add(pb);
            	}
            }
            */
			
			// 发送消息
            String strStatus;
			String strResult = sf.sendMessage(header, "OBJECT", beans);
			if(strResult == "SendFile success"){
				logger.info("Send Station Msg Success");
				strStatus = "complete";
			}else {
				logger.info("Send Station Msg Failed");
				strStatus = "abnormal";
			}
			
			// 更新数据库状态
            //for(NwfdStationSend msg:lstStationMsg) {
            //	db.updateResult(strStatus, msg.getId());
            //}
            
			// 处理完毕，断开连接
			String strDisResult = sf.disconnect();
			if(strDisResult == "disconnect success") {
				logger.info("Disconnect MQ Server Success");
			} else {
				logger.info("Disconnect MQ Server Failed");
			}
			
        } catch (Exception e) {
			logger.error(e.getMessage(), e);
		} finally {
        	session.close();
        } 
	}
}

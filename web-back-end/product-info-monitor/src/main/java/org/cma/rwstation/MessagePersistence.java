package org.cma.rwstation;

import java.util.ArrayList;

import cn.org.cma.activemq.message.MessageHeader;
import cn.org.cma.activemq.message.ProductBean;

public interface MessagePersistence {
	public boolean save( MessageHeader header, 
			ArrayList<ProductBean> lstBeans,String filename);
	
	
	public boolean getMessage(MessageHeader header, ArrayList<ProductBean> beans,
			String  filename);
	
}

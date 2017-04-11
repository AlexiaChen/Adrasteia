package org.cma.config;

import java.io.File;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import org.cma.product.LocalMicpProduct;
import org.cma.product.MergeProduct;
import org.cma.product.RecvMonProduct;
import org.cma.product.SendMonProduct;
import org.cma.product.files.MergeProductFileSet;
import org.cma.product.files.MicapsProductFileSet;
import org.cma.product.files.RecvProductFileSet;
import org.cma.product.files.SendProductFileSet;
import org.dom4j.Document;
import org.dom4j.Element;
import org.dom4j.io.SAXReader;

public class JobConfig {
	private static JobConfig cfg = null;
	private List<SendMonProduct> m_sendproducts;
	private List<RecvMonProduct> m_recvproducts;
	private List<LocalMicpProduct> m_mipproducts;
	private List<MergeProduct> m_mergeproducts;

	private void readSendMonConfig() {
		m_sendproducts = new ArrayList<SendMonProduct>();

		try {

			String configFile = JobConfig.class.getClassLoader().getResource("sendConfig.xml").getPath();

			SAXReader reader = new SAXReader();
			Document document = reader.read(new File(configFile));
			Element root = document.getRootElement();

			List<Element> products = root.elements("product");
			Iterator itProd = products.iterator();
			
			while (itProd.hasNext()){
				SendMonProduct prod = new SendMonProduct();
				Element element = (Element) itProd.next();
				
				prod.setKey(element.attribute("key").getValue());
				prod.setName(element.attribute("name").getValue());
				prod.setType(element.attribute("type").getValue());
				prod.setDi(Float.parseFloat(element.attribute("di").getValue()));
				prod.setDj(Float.parseFloat(element.attribute("dj").getValue()));
				prod.setRange(element.attribute("range").getValue());
				prod.setMaxforecast(element.attribute("maxforecast").getValue());
				
				prod.setSendfolder(element.element("send").getText());
				prod.setSendoverfolder(element.element("sendover").getText());
				
				prod.setSendtime(element.element("sendtime").getText());
				prod.setFileformat(element.element("fileformat").getText());
				
				/* 产品的起报时列表 */
				List<Element> reportTimes = element.element("reporttimes").elements("reporttime");
				List<String> reportTimesList = new ArrayList<String>();
				
				Iterator itReportTimes = reportTimes.iterator();

				while (itReportTimes.hasNext()) {
					Element reportTime = (Element) itReportTimes.next();

					reportTimesList.add(reportTime.attribute("hour").getValue());
				}
				
				prod.setReporttimes(reportTimesList);
				
				
				SendProductFileSet spdfSet = new SendProductFileSet();
			
				prod.setFileSet(spdfSet);
				
				m_sendproducts.add(prod);
			}

		} catch (Exception e) {
			e.printStackTrace();
		}

	}

	private void readRecvMonConfig() {
		m_recvproducts = new ArrayList<RecvMonProduct>();

		try {

			String configFile = JobConfig.class.getClassLoader().getResource("recvConfig.xml").getPath();

			SAXReader reader = new SAXReader();
			Document document = reader.read(new File(configFile));
			Element root = document.getRootElement();

			List<Element> products = root.elements("product");
			Iterator itProd = products.iterator();
			
			while (itProd.hasNext()){
				RecvMonProduct prod = new RecvMonProduct();
				Element element = (Element) itProd.next();
				
				prod.setKey(element.attribute("key").getValue());
				prod.setName(element.attribute("name").getValue());
				prod.setType(element.attribute("type").getValue());
				prod.setDi(Float.parseFloat(element.attribute("di").getValue()));
				prod.setDj(Float.parseFloat(element.attribute("dj").getValue()));
				prod.setRange(element.attribute("range").getValue());
				prod.setMaxforecast(element.attribute("maxforecast").getValue());
				
				prod.setCccc(element.attribute("cccc").getValue());
				prod.setCcccName(element.attribute("ccccname").getValue());
				
				prod.setSrcfolder(element.element("srcfolder").getText());
				prod.setRecvtime(element.element("recvtime").getText());
				prod.setFileformat(element.element("fileformat").getText());
				
				
				/* 产品的起报时列表 */
				List<Element> reportTimes = element.element("reporttimes").elements("reporttime");
				List<String> reportTimesList = new ArrayList<String>();
				
				Iterator itReportTimes = reportTimes.iterator();

				while (itReportTimes.hasNext()) {
					Element reportTime = (Element) itReportTimes.next();

					reportTimesList.add(reportTime.attribute("hour").getValue());
				}
				
				prod.setReporttimes(reportTimesList);
				
				
				RecvProductFileSet pdfSet = new RecvProductFileSet();
				
				prod.setFileSet(pdfSet);
				
				m_recvproducts.add(prod);
			}

		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	private void readMicapsConfig() {
		m_mipproducts = new ArrayList<LocalMicpProduct>();

		try {

			String configFile = JobConfig.class.getClassLoader().getResource("micapsConfig.xml").getPath();

			SAXReader reader = new SAXReader();
			Document document = reader.read(new File(configFile));
			Element root = document.getRootElement();

			List<Element> products = root.elements("product");
			Iterator itProd = products.iterator();
			
			while (itProd.hasNext()){
				LocalMicpProduct prod = new LocalMicpProduct();
				Element element = (Element) itProd.next();
				
				prod.setKey(element.attribute("key").getValue());
				prod.setName(element.attribute("name").getValue());
				prod.setType(element.attribute("type").getValue());
				prod.setDi(Float.parseFloat(element.attribute("di").getValue()));
				prod.setDj(Float.parseFloat(element.attribute("dj").getValue()));
				prod.setRange(element.attribute("range").getValue());
				prod.setMaxforecast(element.attribute("maxforecast").getValue());
				
				prod.setSrcfolder(element.element("srcfolder").getText());
				prod.setRecvtime(element.element("recvtime").getText());
				prod.setFileformat(element.element("fileformat").getText());
				
				
				/* 产品的起报时列表 */
				List<Element> reportTimes = element.element("reporttimes").elements("reporttime");
				List<String> reportTimesList = new ArrayList<String>();
				
				Iterator itReportTimes = reportTimes.iterator();

				while (itReportTimes.hasNext()) {
					Element reportTime = (Element) itReportTimes.next();

					reportTimesList.add(reportTime.attribute("hour").getValue());
				}
				
				prod.setReporttimes(reportTimesList);
				
				
				MicapsProductFileSet fileset = new MicapsProductFileSet();
				prod.setFileSet(fileset);
				
				
				
				m_mipproducts.add(prod);
			}

		} catch (Exception e) {
			e.printStackTrace();
		}
	}
	
	private void readMergeConfig(){
		m_mergeproducts = new ArrayList<MergeProduct>();
		try {
			String configFile = JobConfig.class.getClassLoader().getResource("taskMonConfig.xml").getPath();

			SAXReader reader = new SAXReader();
			Document document = reader.read(new File(configFile));
			Element root = document.getRootElement();

			List<Element> products = root.elements("product");
			Iterator itProd = products.iterator();
			
			while (itProd.hasNext()){
				MergeProduct prod = new MergeProduct();
				Element element = (Element) itProd.next();
				
				prod.setProductKey(element.attribute("key").getValue());
				prod.setCccc(element.attribute("cccc").getValue());
				prod.setCcccName(element.attribute("ccccname").getValue());
				prod.setName(element.attribute("name").getValue());
				prod.setType(element.attribute("type").getValue());
				
				prod.setDi(Float.parseFloat(element.attribute("di").getValue()));
				prod.setDj(Float.parseFloat(element.attribute("dj").getValue()));
				prod.setRange(element.attribute("range").getValue());
				prod.setMaxForecast(element.attribute("maxforecast").getValue());
				
				prod.setShouldTime(element.element("shouldtime").getText());
				
				
				
				/* 产品的起报时列表 */
				List<Element> reportTimes = element.element("reporttimes").elements("reporttime");
				List<String> reportTimesList = new ArrayList<String>();
				
				Iterator itReportTimes = reportTimes.iterator();

				while (itReportTimes.hasNext()) {
					Element reportTime = (Element) itReportTimes.next();

					reportTimesList.add(reportTime.attribute("hour").getValue());
				}
				
				prod.setReporttimes(reportTimesList);
				
				
				MergeProductFileSet fileSet = new MergeProductFileSet();
				
				prod.setFileSet(fileSet);
				
				
				m_mergeproducts.add(prod);
		
			}
		}catch(Exception e){
			e.printStackTrace();
		}
		
	}

	private JobConfig() {

		readSendMonConfig();
		readRecvMonConfig();
		readMicapsConfig();
		readMergeConfig();

	}

	public static JobConfig getInstance() {

		if (cfg == null) {
			cfg = new JobConfig();
		}

		return cfg;
	}
	
	public List<SendMonProduct> getSendMonProductList(){
		return m_sendproducts;
	}
	
	public List<RecvMonProduct> getRecvMonProductList(){
		return m_recvproducts;
	}
	
	public List<LocalMicpProduct> getLocalMicpProductList(){
		return m_mipproducts;
	}
	
	
	public List<MergeProduct> getMergeProductLits(){
		return m_mergeproducts;
	}
	

}

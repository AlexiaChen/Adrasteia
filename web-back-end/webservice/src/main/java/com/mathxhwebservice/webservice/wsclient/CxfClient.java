package com.mathxhwebservice.webservice.wsclient;

import com.mathxhwebservice.webservice.mtom.BinaryFile;
import com.mathxhwebservice.webservice.service.CommonService;
import org.apache.cxf.jaxws.JaxWsProxyFactoryBean;

import javax.activation.DataHandler;
import javax.activation.DataSource;
import javax.activation.FileDataSource;
import java.io.*;

public class CxfClient {
    public static void main(String[] args) {
        //cl1();
       // downloadTest();
        uploadTest();
    }

    /**
     * 方式1.代理类工厂的方式,需要拿到对方的接口
     */
    public static void cl1() {
        try {
            // 接口地址
            String address = "http://localhost:8080/services/CommonService?wsdl";
            // 代理工厂
            JaxWsProxyFactoryBean jaxWsProxyFactoryBean = new JaxWsProxyFactoryBean();
            // 设置代理地址
            jaxWsProxyFactoryBean.setAddress(address);
            // 设置接口类型
            jaxWsProxyFactoryBean.setServiceClass(CommonService.class);
            // 创建一个代理接口实现
            CommonService cs = (CommonService) jaxWsProxyFactoryBean.create();
            // 数据准备
            String userName = "MathxH Chen";
            // 调用代理接口的方法调用并返回结果
            String result = cs.sayHello(userName);
            System.out.println("返回结果:" + result);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static void uploadTest(){
        try{
            // 接口地址
            String address = "http://localhost:8080/services/CommonService?wsdl";
            // 代理工厂
            JaxWsProxyFactoryBean jaxWsProxyFactoryBean = new JaxWsProxyFactoryBean();
            // 设置代理地址
            jaxWsProxyFactoryBean.setAddress(address);
            // 设置接口类型
            jaxWsProxyFactoryBean.setServiceClass(CommonService.class);
            // 创建一个代理接口实现
            CommonService cs = (CommonService) jaxWsProxyFactoryBean.create();

            BinaryFile file = new BinaryFile();
            file.setTitle("uploaded.png");
            DataSource source = new FileDataSource(new File("d:" + File.separator + "downloaded.png"));
            file.setBinaryData(new DataHandler(source));
            if(cs.uploadFile(file)) {
                System.out.println("upload success");
            }else{
                System.out.println("upload failed");
            }

        }catch (Exception e){
            e.printStackTrace();
        }
    }

    public static void downloadTest(){
        try{
            // 接口地址
            String address = "http://localhost:8080/services/CommonService?wsdl";
            // 代理工厂
            JaxWsProxyFactoryBean jaxWsProxyFactoryBean = new JaxWsProxyFactoryBean();
            // 设置代理地址
            jaxWsProxyFactoryBean.setAddress(address);
            // 设置接口类型
            jaxWsProxyFactoryBean.setServiceClass(CommonService.class);
            // 创建一个代理接口实现
            CommonService cs = (CommonService) jaxWsProxyFactoryBean.create();

           BinaryFile file = cs.downloadFile("test.png");
           String title = file.getTitle();
           DataHandler binaryData = file.getBinaryData();

            try (
                    InputStream is = binaryData.getInputStream();
                    OutputStream os = new FileOutputStream(new File("d:" + File.separator + "downloaded.png"));
                    BufferedOutputStream bos = new BufferedOutputStream(os))
            {

                byte[] buffer = new byte[100000];
                int bytesRead = 0;
                while ((bytesRead = is.read(buffer)) != -1) {
                    bos.write(buffer, 0, bytesRead);
                }

                bos.flush();
            } catch (IOException e) {
                e.printStackTrace();

            }

        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}

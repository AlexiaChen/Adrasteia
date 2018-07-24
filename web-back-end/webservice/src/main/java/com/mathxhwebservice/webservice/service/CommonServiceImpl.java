package com.mathxhwebservice.webservice.service;

import com.mathxhwebservice.webservice.mtom.BinaryFile;
import org.springframework.stereotype.Component;

import javax.activation.DataHandler;
import javax.activation.DataSource;
import javax.activation.FileDataSource;
import javax.jws.WebService;
import javax.xml.ws.soap.MTOM;
import java.io.*;


@WebService(serviceName = "CommonService", // 与接口中指定的name一致
        targetNamespace = "http://service.webservice.mathxhwebservice.com/", // 与接口中的命名空间一致,一般是接口的包名倒
        endpointInterface = "com.mathxhwebservice.webservice.service.CommonService"// 接口地址
)
@MTOM(threshold = 1024)
@Component
public class CommonServiceImpl implements CommonService{

    @Override
    public String sayHello(String name) {
        return "Hello ," + name;
    }

    @Override
    public BinaryFile downloadFile(String fileName){
        BinaryFile file = new BinaryFile();
        file.setTitle(fileName);
        DataSource source = new FileDataSource(new File("d:" + File.separator + fileName));
        file.setBinaryData(new DataHandler(source));
        return file;
    }

    @Override
    public  boolean uploadFile(BinaryFile file){
        DataHandler dataHandler = file.getBinaryData();
        String fileTitle = file.getTitle();

        try (
                InputStream is = dataHandler.getInputStream();
                OutputStream os = new FileOutputStream(new File("d:" + File.separator + fileTitle));
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
            return false;
        }
        return true;
    }
}



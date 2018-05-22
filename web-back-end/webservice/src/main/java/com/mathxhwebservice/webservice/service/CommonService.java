package com.mathxhwebservice.webservice.service;


/**
 * 接口
 *
 * @author leftso
 *
 */

import javax.jws.WebMethod;
import javax.jws.WebParam;
import javax.jws.WebResult;
import javax.jws.WebService;

@WebService(name = "CommonService", // 暴露服务名称
        targetNamespace = "http://service.webservice.mathxhwebservice.com/")// 命名空间,一般是接口的包名倒序
public interface CommonService {

    @WebMethod
    @WebResult(name = "String", targetNamespace = "")
    String sayHello(@WebParam(name = "userName") String name);
}

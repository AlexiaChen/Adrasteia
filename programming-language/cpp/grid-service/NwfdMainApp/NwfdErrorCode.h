/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* 文件名: NwfdErrorCode.h
* 作  者: zhangl  		版本：1.0		日  期：2015/06/23
* 描  述：错误编码定义
* 其  他： =0 : 编码表示正常
*          <0 : 编码表示错误 - error
*          >0 : 编码表示异常 - warning
* 功能列表:
* 修改日志：
*   No.   日期		  作者		       修改内容
*    1    2015.11.10  MathxH Chen
* ------------------------------------------------------------------------
*************************************************************************/
#ifndef  NWFD_ERROR_CODE_H
#define  NWFD_ERROR_CODE_H

#include <string>
#include <map>
#include <cassert>

// 中文
#define ERR_CN 
#pragma execution_character_set("utf-8")

class Error
{
public:
	Error(int value, const std::string& str)
	{
		m_value = value;
		m_message = str;
#ifdef DEBUG
		ErrorMap::iterator found = GetErrorMap().find(value);
		if (found != GetErrorMap().end())
			assert(found->second == m_message);
#endif
		GetErrorMap()[m_value] = m_message;
	}

	// auto-cast Error to integer error code
	operator int() { return m_value; }

private:
	int m_value;
	std::string m_message;

	typedef std::map<int, std::string> ErrorMap;
	static ErrorMap& GetErrorMap()
	{
		static ErrorMap errMap;
		return errMap;
	}

public:

	static std::string GetErrorString(int value)
	{
		ErrorMap::iterator found = GetErrorMap().find(value);
		if (found == GetErrorMap().end())
		{
			assert(false);
			return "Unkown Error Description";
		}
		else
		{
			return found->second;
		}
	}
};

#ifdef ERR_CN
static Error SUCCESS(0, "成功");

static Error INIT_REPEATED(1, "重复初始化");
static Error INIT_FAIL(2, "初始化失败");
static Error NOT_FOUND(3, "查找失败");
static Error NO_AUTHORITY(4, "没有认证");
static Error NOT_COMPLETE(5, "数据不完整，还有未取得的数据");
static Error LOGIN_FAIL(6, "登录失败");

static Error LOAD_LIB_FAIL(11, "加载动态库失败");
static Error GET_LIB_FUN_FAIL(12, "获取动态库方法失败");
static Error GET_PROVIDER_FAIL(13, "获取动态库接口方法失败");

static Error CACHED_INIT_FAIL(21, "缓存初始化失败");
static Error CACHED_CONNECT_ABNORMAL(22, "缓存连接异常");
static Error CACHED_CONNECT_FAIL(23, "缓存连接失败");
static Error CACHED_GETVALUE_FAIL(24, "获取缓存数据失败");
static Error CACHED_GETALLKEY_FAIL(25, "获取所有产品Key失败");
static Error CACHED_GETPRODUCTINFO_FAIL(26, "获取产品信息失败");
static Error CACHED_GETPRODUCTDATA_FAIL(27, "获取产品数据失败");
static Error CACHED_LOCK_TIMEOUT(28, "save data to cache is timeout");
static Error CACHED_SETVALUE_FAIL(29, "save data to cache failed");
static Error CACHED_DELVALUE_FAIL(30, "delete data from cache failed");
static Error CACHED_DELCLIENTDATA_FAIL(31, "delete client data failed");
static Error CACHED_SAVEPRODUCTKEY_FAIL(32, "save product key to cache failed");
static Error CACHED_SAVEPRODUCTINFO_FAIL(33, "save product information to cache failed");
static Error CACHED_SAVEPRODUCTTIME_FAIL(34, "save product time to cache failed");
static Error CACHED_SAVEPRODUCTDATA_FAIL(35, "save product data to cache failed");

static Error TCP_CONNECT_ABNORMAL(51, "network connection is abnormal");
static Error SEND_FAIL(52, "发送数据失败");
static Error RECV_EMPTY(53, "没有接收到数据");
static Error UNPACK_SIZE_ABNORAML(54, "数据解包长度异常");
static Error UNPACK_FAIL(56, "数据解包失败");
static Error SERVER_ACK_FAIL(57, "消息响应确认码错误");
static Error RECV_DATA_ABNORMAL(58, "接收数据异常");
static Error RECV_DATA_OVER_LEN(59, "接收数据太长");
static Error SUBSCRIBE_FAIL(60, "订阅失败");

static Error AUTH_FAIL(100, "认证失败");
static Error FUN_NOT_IMPL(1000, "方法未实现");
static Error FUNCTION_DISABLED(150, "功能禁用");
static Error NO_AVAILABLE_DATA(151, "没有可用数据");

static Error ERR_PARAM(-1, "参数错误");
static Error ERR_PARAM_NULL(-2, "参数空指针");
static Error ERR_CREATE_CACHED(-3, "创建缓存失败");
static Error ERR_CACHED_TYPE(-4, "缓存类型错误");
static Error ERR_MEM_MALLOC(-5, "申请内存空间失败");
static Error ERR_SEARCHTIME_TYPE(-6, "检索时间类型失败");
static Error ERR_DATASRCFLG(-9, "数据类型错误");
static Error ERR_DATA_LEN(-10, "数据长度错误");
static Error ERR_CREATE_SOCKETCLIENT(-7, "客户端套接字创建失败");
static Error ERR_PARAM_OVER_LEN(-8, "参数数据的长度过长");
static Error ERR_VERIFCODE(-100, "认证码错误");
static Error ERR_CATCH(-1000, "抛出未知异常");

static Error ERR_MEMTEMP_FAILED(-20, "创建临时空间失败");

static Error GET_DATA_IS_NULL(101, "获取的数据为空");
static Error GET_DATAINFO_IS_NULL(102, "获取的数据描述信息为空");
static Error GET_DATA_CNT_ERROR(103, "获取的数据个数与配置不一致");
static Error GET_DATA_NINJ_ERROR(104, "获取的经纬度方向数据个数与配置不一致");
static Error GET_DATA_FAILED(105, "获取数据失败");

static Error NO_DATAPROC_CHAINS(106, "没有数据加工处理对象");
static Error DATAPROC_IS_NULL(107, "数据加工处理对象指针为空");
static Error NO_REPORTHOUR_CFG(108, "没有该起报时配置");
static Error FORECAT_RANGE_CFG_ERROR(109, "起止预报时次配置错误");

static Error FILE_FORMAT_UNKOWN(110, "未知文件类型");
static Error FILE_ISNOT_EXISTS(111, "文件不存在");
static Error READ_FILE_FAILED(112, "读取文件内容失败");
static Error GET_GRIBINFO_FAILED(113, "获取GRIB2文件信息失败");

#else  // ERR_EN
static Error SUCCESS(0, "function success");

static Error ERR_PARAM(-1, "function input parameter error");
static Error ERR_PARAM_NULL(-2, "function input parameter is null pointer");
static Error ERR_CREATE_CACHED(-3, "create cached error");
static Error ERR_CACHED_TYPE(-4, "cached type is error");
static Error ERR_MEM_MALLOC(-5, "memory malloc fail");
static Error ERR_SEARCHTIME_TYPE(-6, "serach time type error");
static Error ERR_DATASRCFLG(-9, "data's data source error");
static Error ERR_DATA_LEN(-10, "data length error");
static Error ERR_CREATE_SOCKETCLIENT(-7, "socket client create fail");
static Error ERR_PARAM_OVER_LEN(-8, "parameter'data is over length");
static Error ERR_VERIFCODE(-100, "Verify Code is Error");
static Error ERR_CATCH(-1000, "throwed exception");

static Error AUTH_FAIL(100, " authentication failed");
static Error FUN_NOT_IMPL(1000, "function not implemented");


static Error TCP_CONNECT_ABNORMAL(51, "network connection is abnormal");
static Error SEND_FAIL(52, "send data failed");
static Error RECV_EMPTY(53, "not received data");
static Error UNPACK_SIZE_ABNORAML(54, "unpack abnormal");
static Error UNPACK_FAIL(56, "unpack failed");
static Error SERVER_ACK_FAIL(57, "message acknownledge failed");
static Error RECV_DATA_ABNORMAL(58, "received data is abnormal");
static Error RECV_DATA_OVER_LEN(59, "received data is over length");
static Error SUBSCRIBE_FAIL(60, "subscribe failed");

static Error CACHED_INIT_FAIL(21, "cache initialized failed");
static Error CACHED_CONNECT_ABNORMAL(22, "cache connection is abnormal");
static Error CACHED_CONNECT_FAIL(23, "cache connect failed");
static Error CACHED_GETVALUE_FAIL(24, "get value from cache failed");
static Error CACHED_GETALLKEY_FAIL(25, "get all product keys from cache failed");
static Error CACHED_GETPRODUCTINFO_FAIL(26, "get product information failed");
static Error CACHED_GETPRODUCTDATA_FAIL(27, "get product data failed");
static Error CACHED_LOCK_TIMEOUT(28, "save data to cache is timeout");
static Error CACHED_SETVALUE_FAIL(29, "save data to cache failed");
static Error CACHED_DELVALUE_FAIL(30, "delete data from cache failed");
static Error CACHED_DELCLIENTDATA_FAIL(31, "delete client data failed");
static Error CACHED_SAVEPRODUCTKEY_FAIL(32, "save product key to cache failed");
static Error CACHED_SAVEPRODUCTINFO_FAIL(33, "save product information to cache failed");
static Error CACHED_SAVEPRODUCTTIME_FAIL(34, "save product time to cache failed");
static Error CACHED_SAVEPRODUCTDATA_FAIL(35, "save product data to cache failed");

static Error LOAD_LIB_FAIL(11, "Load Library failed");
static Error GET_LIB_FUN_FAIL(12, "Get Library function failed");
static Error GET_PROVIDER_FAIL(13, "Get Provider failed");

static Error INIT_REPEATED(1, "initialize repeated");
static Error INIT_FAIL(2, "initialize failed");
static Error NOT_FOUND(3, "not found");
static Error NO_AUTHORITY(4, "no authority");
static Error NOT_COMPLETE(5, "data is not complete and have the rest of data");
static Error LOGIN_FAIL(6, "login failed");

#endif //


#endif // NWFD_ERROR_CODE_H


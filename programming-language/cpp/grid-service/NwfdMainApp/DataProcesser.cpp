#include "DataProcesser.h"
#include "NwfdErrorCode.h"
#include <string>
#include <malloc.h>
#include <QDateTime>
#include <QString>

/************************************************************************/
/* 构造函数                                                             */
/************************************************************************/
DataProcesser::DataProcesser(char * request, int nlen)
{
	_request = NULL;
	m_nDataLen = nlen;
	this->_request = (char *)malloc(m_nDataLen);
	if (this->_request)
	{
		memcpy(this->_request, request, m_nDataLen);
	}
}

/************************************************************************/
/* 析构函数                                                             */
/************************************************************************/
DataProcesser::~DataProcesser()
{
	// 释放空间
	if (this->_request)
	{
		free(this->_request);
		this->_request = NULL;
	}
}

/************************************************************************/
/* 数据解析处理，返回处理结果                                           */
/************************************************************************/
int DataProcesser::process(QString strClientIP, QByteArray & response)
{
	// 没有收到数据
	if (this->_request == NULL)
	{
		return -1;
	}

	// 解析数据
	ST_MSG stMsg;
	int nErr = UnPack(this->_request, m_nDataLen, stMsg);
	if (nErr != SUCCESS)
	{
		// 数据解析失败,不处理
		return -2;
	}

	// 验证IP地址和ClientID
	if (!NetworkManager::getClass().IsLegalClient(stMsg.stMsgHead.ADR, strClientIP))
	{
		// 与配置绑定的IP和ClientID不一致，不做处理
		return -3;
	}

	int nRet = 0;
	// 数据处理
	switch (stMsg.stMsgHead.CMD)
	{
	case CMD_HEARTBEAT: // 连接确认（心跳）
		nRet = HandleHeartbeat(stMsg.stMsgHead.ADR, response);
		break;

	case CMD_LOGIN: // 登录注册
		nRet = HandleLogin(stMsg, response);
		break;

	case CMD_SENDDATA:  // 客户端发送格点数据
		nRet = HandleGridData(stMsg, response);
		break;

	case CMD_SENDSTATION:  // 客户端发送站点数据
		nRet = HandleStationData(stMsg, response);
		break;

	case CMD_SENDSTATION_FILE:  // 站点文件
		nRet = HandleStationFile(stMsg, response);
		break;

	case CMD_SEND_MULT_STATION: // 多站点数据
		nRet = HandleMultiStationData(stMsg, response);
		break;

	case CMD_GRID2STATION: // 格点转站点
		nRet = HandleGrid2Station(stMsg, response);
		break;

	case CMD_STATION2GRID: // 站点转格点
		nRet = HandleStation2Grid(stMsg, response);
		break;

	case CMD_RAIN_PROCESS: // 降水处理
		nRet = HandleRainProcess(stMsg, response);
		break;

	case CMD_UP_STATIONCFG: // 站点配置信息同步
		nRet = HandleStationCfg(stMsg, response);
		break;

	default:
		nRet = HandleErrorCmd(stMsg.stMsgHead.ADR, response);
	}

	return nRet;
}

/************************************************************************/
/* 处理心跳命令                                                         */
/************************************************************************/
int DataProcesser::HandleHeartbeat(int nClientID, QByteArray & response)
{
	// 心跳响应
	unsigned char szSendMsg[20];
	int nOff = 0;
	szSendMsg[nOff++] = STD_SOI;           // 起始标志位
	szSendMsg[nOff++] = MSG_VERSION;       // 版本号
	TxI4(nClientID, (unsigned char *)szSendMsg, nOff); // 客户端标识
	szSendMsg[nOff++] = CMD_HEARTBEAT_ACK; // 命令码：心跳响应
	szSendMsg[nOff++] = 0;                 // 数据包序号
	szSendMsg[nOff++] = 0;                 // 数据格式类别
	TxI4(0, (unsigned char *)szSendMsg, nOff); // 数据内容长度
	szSendMsg[nOff++] = STD_EOI;           // 终止标志位

	QByteArray byte((char*)&szSendMsg, nOff);
	response = byte;

	return 0;
}

/************************************************************************/
/* 处理登录命令                                                         */
/************************************************************************/
int DataProcesser::HandleLogin(ST_MSG &stMsg, QByteArray & response)
{
	if (stMsg.stMsgHead.LEN != MSG_LONGIN_LENGHET)
	{
		// 信息长度异常
		return HandleErrorCmd(stMsg.stMsgHead.ADR, response);
	}

	char szValue[26];
	QString strUserName;
	QString strPassword;
	bool  bAllowUpdate;
	unsigned char  cResponseInfo;

	// 用户名
	memset(szValue, 0x00, sizeof(szValue));
	memcpy(szValue, stMsg.szMsgInfo, MSG_LONGIN_USERNAME);
	strUserName = QString::fromLocal8Bit(szValue);
	//密码
	memset(szValue, 0x00, sizeof(szValue));
	memcpy(szValue, stMsg.szMsgInfo + MSG_LONGIN_USERNAME, MSG_LONGIN_PASSWORD);
	strPassword = QString::fromLocal8Bit(szValue);

	// 登录验证
	bool bIsLogin = NetworkManager::getClass().IsLogin(stMsg.stMsgHead.ADR, strUserName, strPassword, bAllowUpdate);
	if (!bIsLogin)
	{
		// 没有登录验证成功
		cResponseInfo = MSG_LOGIN_FAILED;
	}
	else
	{
		// 写权限
		cResponseInfo = bAllowUpdate ? RIGHT_WRITE : RIGHT_NOTWRITE;
	}

	// 响应命名
	unsigned char szSendMsg[20];
	int nOff = 0;
	szSendMsg[nOff++] = STD_SOI;             // 起始标志位
	szSendMsg[nOff++] = MSG_VERSION;       // 版本号
	TxI4(stMsg.stMsgHead.ADR, (unsigned char *)szSendMsg, nOff); // 客户端标识
	szSendMsg[nOff++] = CMD_LOGIN_ACK;       // 命令码：登录响应
	szSendMsg[nOff++] = 0;                   // 数据包序号
	szSendMsg[nOff++] = 0;                   // 数据格式类别
	TxI4(1, (unsigned char *)szSendMsg, nOff); // 数据内容长度
	szSendMsg[nOff++] = cResponseInfo;     // 响应数据信息内容
	szSendMsg[nOff++] = STD_EOI;           // 终止标志位

	QByteArray byte((char*)&szSendMsg, nOff);
	response = byte;

	return 0;
}

/************************************************************************/
/* 处理发送数据命令                                                     */
/************************************************************************/
int DataProcesser::HandleGridData(ST_MSG &stMsg, QByteArray & response)
{
	if (stMsg.stMsgHead.LEN <= 0)
	{
		// 信息长度异常
		return HandleErrorCmd(stMsg.stMsgHead.ADR, response);
	}

	int nDataLen;
	int nYear, nMonth, nDay, nHour, nMinute, nSecond, nForecastTime, nTimeRange;
	float fLon1, fLon2, fLat1, fLat2;
	QString strProductKey;

	// 数据信息格式非第三类格式不处理
	if (stMsg.stMsgHead.TYPE != TYPE_FORMAT_3)
	{
		// 暂不处理第三类格式的数据
		return -1;
	}

	// 解析数据信息部分
	int nOff = 5;  // 从年份开始获取
	unsigned short ival;
	RxI2(ival, (unsigned char *)stMsg.szMsgInfo, nOff); // 年
	nYear = ival;
	nMonth = stMsg.szMsgInfo[nOff++]; // 月
	nDay   = stMsg.szMsgInfo[nOff++]; // 日
	nHour  = stMsg.szMsgInfo[nOff++]; // 时
	nMinute = stMsg.szMsgInfo[nOff++]; // 分
	nSecond = stMsg.szMsgInfo[nOff++]; // 秒
	nForecastTime = (unsigned char)stMsg.szMsgInfo[nOff++]; // 预报时间
	nTimeRange    = stMsg.szMsgInfo[nOff++]; // 时间间隔
	RxF4(fLon1, (unsigned char *)stMsg.szMsgInfo, nOff); // lon1
	RxF4(fLon2, (unsigned char *)stMsg.szMsgInfo, nOff); // lon2
	RxF4(fLat1, (unsigned char *)stMsg.szMsgInfo, nOff); // lat1
	RxF4(fLat2, (unsigned char *)stMsg.szMsgInfo, nOff); // lat2
	nOff += 9;  // 数据类型 非DATA_TYPE_2不处理...
	// 长度
	RxI4(nDataLen, (unsigned char *)stMsg.szMsgInfo, nOff); // 数据内容长度
	// 具体信息-上传的产品Key
	strProductKey = QString::fromLocal8Bit(stMsg.szMsgInfo + nOff, nDataLen);

	int nErr = NetworkManager::getClass().HandleGridData(stMsg.stMsgHead.ADR, strProductKey, nYear, nMonth, nDay, nHour, nMinute, nSecond,
		nForecastTime, nTimeRange,fLon1, fLon2, fLat1, fLat2);
	if (nErr != 0)
	{
		// todo 处理失败，不对客户端进行响应
		return -2;
	}

	// 处理成功（异步），响应命令
	unsigned char szSendMsg[20];
	nOff = 0;
	szSendMsg[nOff++] = STD_SOI;                // 起始标志位
	szSendMsg[nOff++] = MSG_VERSION;            // 版本号
	TxI4(stMsg.stMsgHead.ADR, (unsigned char *)szSendMsg, nOff); // 客户端标识
	szSendMsg[nOff++] = CMD_SENDDATA_ACK;       // 命令码：发送数据响应
	szSendMsg[nOff++] = 0;                      // 数据包序号
	szSendMsg[nOff++] = 0;                      // 数据格式类别
	TxI4(0, (unsigned char *)szSendMsg, nOff);  // 数据内容长度
	szSendMsg[nOff++] = STD_EOI;                // 终止标志位

	QByteArray byte((char*)&szSendMsg, nOff);
	response = byte;

	return 0;
}

/************************************************************************/
/* 处理发送站点数据命令                                                 */
/************************************************************************/
int DataProcesser::HandleStationData(ST_MSG &stMsg, QByteArray & response)
{
	if (stMsg.stMsgHead.LEN <= 0)
	{
		// 信息长度异常
		return HandleErrorCmd(stMsg.stMsgHead.ADR, response);
	}

	int nYear, nMonth, nDay, nHour, nMinute, nSecond, nForecastTime, nTimeRange;
	float fLon, fLat, fHeight;
	QString strCityType, strStationNo;
	char szCityType[10];
	char szStationNo[10];
	// 数据信息格式非第五类格式不处理
	if (stMsg.stMsgHead.TYPE != TYPE_FORMAT_5)
	{
		// 暂不处理第三类格式的数据
		return -1;
	}

	// 解析数据信息部分
	int nOff = 0;  
	// 城市类型
	//strCityType = QString::fromLocal8Bit(stMsg.szMsgInfo + nOff, 10);
	memcpy(szCityType, stMsg.szMsgInfo + nOff, 10);
	strCityType = QString::fromLocal8Bit(szCityType);
	nOff += 10;
	// 站点号
	//strStationNo = QString::fromLocal8Bit(stMsg.szMsgInfo + nOff, 10);
	memcpy(szStationNo, stMsg.szMsgInfo + nOff, 10);
	strStationNo = QString::fromLocal8Bit(szStationNo);
	nOff += 10;

	unsigned short ival;
	RxI2(ival, (unsigned char *)stMsg.szMsgInfo, nOff); // 年
	nYear = ival;
	nMonth = stMsg.szMsgInfo[nOff++]; // 月
	nDay = stMsg.szMsgInfo[nOff++]; // 日
	nHour = stMsg.szMsgInfo[nOff++]; // 时
	nMinute = stMsg.szMsgInfo[nOff++]; // 分
	nSecond = stMsg.szMsgInfo[nOff++]; // 秒
	nForecastTime = (unsigned char)stMsg.szMsgInfo[nOff++]; // 预报时间
	nTimeRange = stMsg.szMsgInfo[nOff++]; // 时间间隔
	RxF4(fLon, (unsigned char *)stMsg.szMsgInfo, nOff); // lon
	RxF4(fLat, (unsigned char *)stMsg.szMsgInfo, nOff); // lat
	RxF4(fHeight, (unsigned char *)stMsg.szMsgInfo, nOff); // height

	int nErr = NetworkManager::getClass().HandleStationData(stMsg.stMsgHead.ADR, strCityType, strStationNo, nYear, nMonth, nDay, nHour, nMinute, nSecond,
		nForecastTime, nTimeRange, fLon, fLat, fHeight);
	if (nErr != 0)
	{
		// todo 处理失败，不对客户端进行响应
		return -2;
	}

	// 处理成功（异步），响应命令
	unsigned char szSendMsg[20];
	nOff = 0;
	szSendMsg[nOff++] = STD_SOI;                // 起始标志位
	szSendMsg[nOff++] = MSG_VERSION;            // 版本号
	TxI4(stMsg.stMsgHead.ADR, (unsigned char *)szSendMsg, nOff); // 客户端标识
	szSendMsg[nOff++] = CMD_SENDSTATION_ACK;    // 命令码：发送数据响应
	szSendMsg[nOff++] = 0;                      // 数据包序号
	szSendMsg[nOff++] = 0;                      // 数据格式类别
	TxI4(0, (unsigned char *)szSendMsg, nOff);  // 数据内容长度
	szSendMsg[nOff++] = STD_EOI;                // 终止标志位

	QByteArray byte((char*)&szSendMsg, nOff);
	response = byte;

	return 0;
}

/************************************************************************/
/* 处理发送站点数据命令                                                 */
/************************************************************************/
int DataProcesser::HandleStationFile(ST_MSG &stMsg, QByteArray & response)
{
	if (stMsg.stMsgHead.LEN <= 0)
	{
		// 信息长度异常
		return HandleErrorCmd(stMsg.stMsgHead.ADR, response);
	}

	int nYear, nMonth, nDay, nHour, nMinute, nSecond, nTimeRange;
	
	QString strCityType, strStationList;

	// 数据信息格式非第五类格式不处理
	if (stMsg.stMsgHead.TYPE != TYPE_FORMAT_5)
	{
		// 暂不处理第三类格式的数据
		return -1;
	}

	// 解析数据信息部分
	int nOff = 0;
	// 城市类型
	char szCityType[10];
	memcpy(szCityType, stMsg.szMsgInfo + nOff, 10);
	strCityType = QString::fromLocal8Bit(szCityType);
	nOff += 10;
	// 时间间隔
	nTimeRange = stMsg.szMsgInfo[nOff++];
	unsigned short ival;
	RxI2(ival, (unsigned char *)stMsg.szMsgInfo, nOff); // 年
	nYear = ival;
	nMonth = stMsg.szMsgInfo[nOff++]; // 月
	nDay = stMsg.szMsgInfo[nOff++]; // 日
	nHour = stMsg.szMsgInfo[nOff++]; // 时
	nMinute = stMsg.szMsgInfo[nOff++]; // 分
	nSecond = stMsg.szMsgInfo[nOff++]; // 秒

	int nErr = NetworkManager::getClass().HandleStationFile(stMsg.stMsgHead.ADR, strCityType, nTimeRange, nYear, nMonth, nDay, nHour, nMinute, nSecond);
	if (nErr != 0)
	{
		// todo 处理失败，不对客户端进行响应
		return -2;
	}

	// 处理成功（异步），响应命令
	unsigned char szSendMsg[20];
	nOff = 0;
	szSendMsg[nOff++] = STD_SOI;                // 起始标志位
	szSendMsg[nOff++] = MSG_VERSION;            // 版本号
	TxI4(stMsg.stMsgHead.ADR, (unsigned char *)szSendMsg, nOff); // 客户端标识
	szSendMsg[nOff++] = CMD_SENDSTATION_FILE_ACK;// 命令码：发送数据响应
	szSendMsg[nOff++] = 0;                      // 数据包序号
	szSendMsg[nOff++] = 0;                      // 数据格式类别
	TxI4(0, (unsigned char *)szSendMsg, nOff);  // 数据内容长度
	szSendMsg[nOff++] = STD_EOI;                // 终止标志位

	QByteArray byte((char*)&szSendMsg, nOff);
	response = byte;

	return 0;
}

/************************************************************************/
/* 处理发送站点数据命令                                                 */
/************************************************************************/
int DataProcesser::HandleMultiStationData(ST_MSG &stMsg, QByteArray & response)
{
	if (stMsg.stMsgHead.LEN <= 0)
	{
		// 信息长度异常
		return HandleErrorCmd(stMsg.stMsgHead.ADR, response);
	}

	int nYear, nMonth, nDay, nHour, nMinute, nSecond, nEndForecast, nTimeRange;
	
	QString strDataType;

	// 数据信息格式非第五类格式不处理
	if (stMsg.stMsgHead.TYPE != TYPE_FORMAT_5)
	{
		// 暂不处理第三类格式的数据
		return -1;
	}

	// 解析数据信息部分
	int nOff = 0;

	char szDataType[10];
	memcpy(szDataType, stMsg.szMsgInfo + nOff, 10);
	strDataType = QString::fromLocal8Bit(szDataType);
	nOff += 10;

	// 时间间隔
	nTimeRange = stMsg.szMsgInfo[nOff++];
	nEndForecast = (unsigned char)stMsg.szMsgInfo[nOff++]; // 预报时间
	// 时间
	unsigned short ival;
	RxI2(ival, (unsigned char *)stMsg.szMsgInfo, nOff); // 年
	nYear = ival;
	nMonth = stMsg.szMsgInfo[nOff++]; // 月
	nDay = stMsg.szMsgInfo[nOff++]; // 日
	nHour = stMsg.szMsgInfo[nOff++]; // 时
	nMinute = stMsg.szMsgInfo[nOff++]; // 分
	nSecond = stMsg.szMsgInfo[nOff++]; // 秒

	int nErr = NetworkManager::getClass().HandleMultiStationData(stMsg.stMsgHead.ADR, szDataType, nTimeRange, nEndForecast, nYear, nMonth, nDay, nHour, nMinute, nSecond);
	if (nErr != 0)
	{
		// todo 处理失败，不对客户端进行响应
		return -2;
	}

	// 处理成功（异步），响应命令
	unsigned char szSendMsg[20];
	nOff = 0;
	szSendMsg[nOff++] = STD_SOI;                // 起始标志位
	szSendMsg[nOff++] = MSG_VERSION;            // 版本号
	TxI4(stMsg.stMsgHead.ADR, (unsigned char *)szSendMsg, nOff); // 客户端标识
	szSendMsg[nOff++] = CMD_SEND_MULT_STATION_ACK;// 命令码：发送数据响应
	szSendMsg[nOff++] = 0;                      // 数据包序号
	szSendMsg[nOff++] = 0;                      // 数据格式类别
	TxI4(0, (unsigned char *)szSendMsg, nOff);  // 数据内容长度
	szSendMsg[nOff++] = STD_EOI;                // 终止标志位

	QByteArray byte((char*)&szSendMsg, nOff);
	response = byte;

	return 0;
}

/************************************************************************/
/* 处理格点转站点                                                       */
/************************************************************************/
int DataProcesser::HandleGrid2Station(ST_MSG &stMsg, QByteArray & response)
{
	if (stMsg.stMsgHead.LEN <= 0)
	{
		// 信息长度异常
		return HandleErrorCmd(stMsg.stMsgHead.ADR, response);
	}

	int nDataLen;
	int nYear, nMonth, nDay, nHour, nMinute, nSecond, nStartForecast, nEndForecast;
	
	QString strProductKey;

	// 解析数据信息部分
	int nOff = 0;

	// 时间
	unsigned short ival;
	RxI2(ival, (unsigned char *)stMsg.szMsgInfo, nOff); // 年
	nYear = ival;
	nMonth = stMsg.szMsgInfo[nOff++]; // 月
	nDay = stMsg.szMsgInfo[nOff++]; // 日
	nHour = stMsg.szMsgInfo[nOff++]; // 时
	nMinute = stMsg.szMsgInfo[nOff++]; // 分
	nSecond = stMsg.szMsgInfo[nOff++]; // 秒
	nStartForecast = (unsigned char)stMsg.szMsgInfo[nOff++];
	nEndForecast = (unsigned char)stMsg.szMsgInfo[nOff++]; // 预报时间
	// 长度
	RxI4(nDataLen, (unsigned char *)stMsg.szMsgInfo, nOff); // 数据内容长度
	// 具体信息-上传的产品Key
	strProductKey = QString::fromLocal8Bit(stMsg.szMsgInfo + nOff, nDataLen);

	int nErr = NetworkManager::getClass().HandleGrid2Station(stMsg.stMsgHead.ADR, strProductKey, nYear, nMonth, nDay, nHour, nMinute, nSecond, nStartForecast, nEndForecast);
	if (nErr != 0)
	{
		// todo 处理失败，不对客户端进行响应
		return -2;
	}

	// 处理成功（异步），响应命令
	unsigned char szSendMsg[20];
	nOff = 0;
	szSendMsg[nOff++] = STD_SOI;                // 起始标志位
	szSendMsg[nOff++] = MSG_VERSION;            // 版本号
	TxI4(stMsg.stMsgHead.ADR, (unsigned char *)szSendMsg, nOff); // 客户端标识
	szSendMsg[nOff++] = CMD_GRID2STATION_ACK;   // 命令码：发送数据响应
	szSendMsg[nOff++] = 0;                      // 数据包序号
	szSendMsg[nOff++] = 0;                      // 数据格式类别
	TxI4(0, (unsigned char *)szSendMsg, nOff);  // 数据内容长度
	szSendMsg[nOff++] = STD_EOI;                // 终止标志位

	QByteArray byte((char*)&szSendMsg, nOff);
	response = byte;

	return 0;
}

/************************************************************************/
/* 处理站点转格点                                                       */
/************************************************************************/
int DataProcesser::HandleStation2Grid(ST_MSG &stMsg, QByteArray & response)
{
	if (stMsg.stMsgHead.LEN <= 0)
	{
		// 信息长度异常
		return HandleErrorCmd(stMsg.stMsgHead.ADR, response);
	}

	int nYear, nMonth, nDay, nHour, nMinute, nSecond,nTimeRange, nStartForecast, nEndForecast;
	
	QString strDataType;

	// 解析数据信息部分
	int nOff = 0;

	char szDataType[10];
	memcpy(szDataType, stMsg.szMsgInfo + nOff, 10);
	strDataType = QString::fromLocal8Bit(szDataType);
	nOff += 10;

	nTimeRange = stMsg.szMsgInfo[nOff++]; 
	nStartForecast = (unsigned char)stMsg.szMsgInfo[nOff++];
	nEndForecast = (unsigned char)stMsg.szMsgInfo[nOff++]; // 预报时间

	// 时间
	unsigned short ival;
	RxI2(ival, (unsigned char *)stMsg.szMsgInfo, nOff); // 年
	nYear = ival;
	nMonth = stMsg.szMsgInfo[nOff++]; // 月
	nDay = stMsg.szMsgInfo[nOff++]; // 日
	nHour = stMsg.szMsgInfo[nOff++]; // 时
	nMinute = stMsg.szMsgInfo[nOff++]; // 分
	nSecond = stMsg.szMsgInfo[nOff++]; // 秒

	int nErr = NetworkManager::getClass().HandleStation2Grid(stMsg.stMsgHead.ADR, strDataType, nTimeRange, nYear, nMonth, nDay, nHour, nMinute, nSecond, nStartForecast, nEndForecast);
	if (nErr != 0)
	{
		// todo 处理失败，不对客户端进行响应
		return -2;
	}

	// 处理成功（异步），响应命令
	unsigned char szSendMsg[20];
	nOff = 0;
	szSendMsg[nOff++] = STD_SOI;                // 起始标志位
	szSendMsg[nOff++] = MSG_VERSION;            // 版本号
	TxI4(stMsg.stMsgHead.ADR, (unsigned char *)szSendMsg, nOff); // 客户端标识
	szSendMsg[nOff++] = CMD_STATION2GRID_ACK;   // 命令码：发送数据响应
	szSendMsg[nOff++] = 0;                      // 数据包序号
	szSendMsg[nOff++] = 0;                      // 数据格式类别
	TxI4(0, (unsigned char *)szSendMsg, nOff);  // 数据内容长度
	szSendMsg[nOff++] = STD_EOI;                // 终止标志位

	QByteArray byte((char*)&szSendMsg, nOff);
	response = byte;

	return 0;
}

/************************************************************************/
/* 处理降水                                                             */
/************************************************************************/
int DataProcesser::HandleRainProcess(ST_MSG &stMsg, QByteArray & response)
{
	if (stMsg.stMsgHead.LEN <= 0)
	{
		// 信息长度异常
		return HandleErrorCmd(stMsg.stMsgHead.ADR, response);
	}

	int nDataLen,nRelatedDataLen;
	int nYear, nMonth, nDay, nHour, nMinute, nSecond, nStartForecast, nEndForecast;
	QString strProductKey,strRelatedProductKey;


	// 解析数据信息部分
	int nOff = 0;

	// 时间
	unsigned short ival;
	RxI2(ival, (unsigned char *)stMsg.szMsgInfo, nOff); // 年
	nYear = ival;
	nMonth = stMsg.szMsgInfo[nOff++]; // 月
	nDay = stMsg.szMsgInfo[nOff++]; // 日
	nHour = stMsg.szMsgInfo[nOff++]; // 时
	nMinute = stMsg.szMsgInfo[nOff++]; // 分
	nSecond = stMsg.szMsgInfo[nOff++]; // 秒
	nStartForecast = (unsigned char)stMsg.szMsgInfo[nOff++];
	nEndForecast = (unsigned char)stMsg.szMsgInfo[nOff++]; // 预报时间
	// 具体信息-上传的产品Key
	RxI4(nDataLen, (unsigned char *)stMsg.szMsgInfo, nOff); // 数据内容长度
	strProductKey = QString::fromLocal8Bit(stMsg.szMsgInfo + nOff, nDataLen);
	nOff += nDataLen;

	// 具体信息-上传的产品Key
	RxI4(nRelatedDataLen, (unsigned char *)stMsg.szMsgInfo, nOff); // 数据内容长度
	strRelatedProductKey = QString::fromLocal8Bit(stMsg.szMsgInfo + nOff, nRelatedDataLen);
	nOff += nRelatedDataLen;

	int nErr = NetworkManager::getClass().HandleRainProcess(stMsg.stMsgHead.ADR, strProductKey, strRelatedProductKey, nYear, nMonth, nDay, nHour, nMinute, nSecond, nStartForecast, nEndForecast);
	if (nErr != 0)
	{
		// todo 处理失败，不对客户端进行响应
		return -2;
	}

	// 处理成功（异步），响应命令
	unsigned char szSendMsg[20];
	nOff = 0;
	szSendMsg[nOff++] = STD_SOI;                // 起始标志位
	szSendMsg[nOff++] = MSG_VERSION;            // 版本号
	TxI4(stMsg.stMsgHead.ADR, (unsigned char *)szSendMsg, nOff); // 客户端标识
	szSendMsg[nOff++] = CMD_RAIN_PROCESS_ACK;   // 命令码：发送数据响应
	szSendMsg[nOff++] = 0;                      // 数据包序号
	szSendMsg[nOff++] = 0;                      // 数据格式类别
	TxI4(0, (unsigned char *)szSendMsg, nOff);  // 数据内容长度
	szSendMsg[nOff++] = STD_EOI;                // 终止标志位

	QByteArray byte((char*)&szSendMsg, nOff);
	response = byte;

	return 0;
}

/************************************************************************/
/* 处理站点配置信息                                                     */
/************************************************************************/
int DataProcesser::HandleStationCfg(ST_MSG &stMsg, QByteArray & response)
{
	if (stMsg.stMsgHead.LEN <= 0)
	{
		// 信息长度异常
		return HandleErrorCmd(stMsg.stMsgHead.ADR, response);
	}
	
	//处理客户端上传的站点配置
	int nErr = NetworkManager::getClass().HandleUploadStationCfg(stMsg.stMsgHead.ADR);
	if (nErr != 0)
	{
		// todo 处理失败，不对客户端进行响应
		return -2;
	}

	//发送响应消息
	unsigned char szSendMsg[20];
	int nOff = 0;
	szSendMsg[nOff++] = STD_SOI;                // 起始标志位
	szSendMsg[nOff++] = MSG_VERSION;            // 版本号
	TxI4(stMsg.stMsgHead.ADR, (unsigned char *)szSendMsg, nOff); // 客户端标识
	szSendMsg[nOff++] = CMD_UP_STATIONCFG_ACK;    // 命令码：发送数据响应
	szSendMsg[nOff++] = 0;                      // 数据包序号
	szSendMsg[nOff++] = 0;                      // 数据格式类别
	TxI4(0, (unsigned char *)szSendMsg, nOff);  // 数据内容长度
	szSendMsg[nOff++] = STD_EOI;                // 终止标志位

	QByteArray byte((char*)&szSendMsg, nOff);
	response = byte;
	
	
	return 0;
}

/************************************************************************/
/* 接收到的错误命令                                                     */
/************************************************************************/
int DataProcesser::HandleErrorCmd(int nClientID, QByteArray & response)
{
	unsigned char szSendMsg[20];
	int nOff = 0;
	szSendMsg[nOff++] = STD_SOI;           // 起始标志位
	szSendMsg[nOff++] = MSG_VERSION;       // 版本号
	TxI4(nClientID, (unsigned char *)szSendMsg, nOff); // 客户端标识
	szSendMsg[nOff++] = CMD_ERROR;         // 命令码：错误命令
	szSendMsg[nOff++] = 0;                 // 数据包序号
	szSendMsg[nOff++] = 0;                 // 数据格式类别
	TxI4(0, (unsigned char *)szSendMsg, nOff); // 数据内容长度
	szSendMsg[nOff++] = STD_EOI;           // 终止标志位

	QByteArray byte((char*)&szSendMsg, nOff);
	response = byte;

	return 0;
}

/************************************************************************/
/* 打包数据                                                             */
/************************************************************************/
int DataProcesser::Pack()
{
	return FUN_NOT_IMPL;
}

/************************************************************************/
/* 解包数据-只解包信息头                                                */
/************************************************************************/
int DataProcesser::UnPack(const char *pOrigData, int nOrigSize, STD_MSG_HEAD &stMsgHead)
{
	// 参数判断
	if (pOrigData == NULL || nOrigSize < 10)
	{
		return UNPACK_SIZE_ABNORAML;
	}

	// 查找开始标志位
	int iPos = 0;
	while (pOrigData[iPos] != STD_SOI)
	{
		iPos++;
	}
	if (iPos + 10 > nOrigSize)
	{
		//discard it
		return UNPACK_SIZE_ABNORAML;
	}
	// 开始解析位置
	const char *pData = pOrigData + iPos;
	int nOff = 1;

	stMsgHead.VER = pData[nOff++];    // 版本号        1Byte
	RxI4(stMsgHead.ADR, (unsigned char *)pData, nOff);// 客户端标识    4Byte
	stMsgHead.CMD = pData[nOff++];    // 命令码        1Byte
	stMsgHead.SEQ = pData[nOff++];    // 数据包序号    1Byte
	stMsgHead.TYPE = pData[nOff++];   // 数据格式类别  1Byte
	// 数据长度      4Bytes
	RxI4(stMsgHead.LEN, (unsigned char *)pData, nOff);
	// dataInfo

	return SUCCESS;
}

/************************************************************************/
/* 解包数据                                                             */
/************************************************************************/
int DataProcesser::UnPack(const char *pOrigData, int nOrigSize, ST_MSG &stMsg)
{
	// 参数判断
	if (pOrigData == NULL || nOrigSize < 10)
	{
		return UNPACK_SIZE_ABNORAML;
	}

	// 查找开始标志位
	int iPos = 0;
	while (pOrigData[iPos] != STD_SOI)
	{
		iPos++;
	}
	if (iPos + 10 > nOrigSize)
	{
		//discard it
		return UNPACK_SIZE_ABNORAML;
	}

	// 开始解析位置
	const char *pData = pOrigData + iPos;
	int nOff = 1;

	stMsg.stMsgHead.VER = pData[nOff++];    // 版本号        1Byte
	RxI4(stMsg.stMsgHead.ADR, (unsigned char *)pData, nOff);// 客户端标识    4Byte
	stMsg.stMsgHead.CMD = pData[nOff++];    // 命令码        1Byte
	stMsg.stMsgHead.SEQ = pData[nOff++];    // 数据包序号    1Byte
	stMsg.stMsgHead.TYPE = pData[nOff++];   // 数据格式类别  1Byte

	// 数据长度      4Bytes
	RxI4(stMsg.stMsgHead.LEN, (unsigned char *)pData, nOff);

	// dataInfo
	if (stMsg.stMsgHead.LEN > MAX_MSG_INFO_LEN || stMsg.stMsgHead.LEN < 0)
	{
		return RECV_DATA_OVER_LEN;
	}

	// 消息内容
	memcpy(stMsg.szMsgInfo, pData + nOff, stMsg.stMsgHead.LEN);

	return SUCCESS;
}


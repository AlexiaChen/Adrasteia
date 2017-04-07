/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* 文件名: CachedDef.h
* 作  者: zhangl		版本：1.0		日  期：2015/05/06
* 描  述：缓存定义
* 其  他：
* 功能列表:
*   1.
* 修改日志：
*   No. 日期		作者		修改内容
*   1  2015-08-28   zhangl   去除QT类型，使用STL，修改缓存存储结构
*************************************************************************/
#ifndef CACHED_DEF_H
#define CACHED_DEF_H

#include <string>
#include <list>
using namespace std;

/************************************************************************/
/* 1. 缓存中Key名称宏定义                                               */
/************************************************************************/
#define PRODUCT_ROOT  "Product"                         // 产品缓存的根
#define PRODUCT_INFO  "ProductInfo_%s"                  // 产品信息
#define PRODUCT_TIME  "ProductTime_%s"                  // 产品时间
#define PRODUCT_DATA  "ProductData_%s"                  // 产品数据 
#define PRODUCT_MEM_DATA      "ProductData_%s_%s"       // memcached产品数据
													    
#define PRODUCT_ROOT_LOCK  "Product_Locker"             // 产品根锁
#define PRODUCT_INFO_LOCK  "ProductInfo_Locker_%s"      // 产品信息锁   
#define PRODUCT_TIME_LOCK  "ProductTime_Locker_%s"      // 产品时间锁  
#define PRODUCT_DATA_LOCK  "ProductData_Locker_%s"      // 产品数据锁   
#define PRODUCT_MEM_DATA_LOCK "ProductData_Locker_%s_%s"// 产品数据锁   

/************************************************************************/
/* 2. 缓存中数据合并Key名称宏定义                                       */
/************************************************************************/
#define MERGE_ROOT  "Merge"                          // 拼图缓存的根
#define MERGE_INFO  "MergeInfo_%s"                   // 拼图信息
#define MERGE_TIME  "MergeTime_%s"                   // 拼图时间
#define MERGE_DATA  "MergeData_%s"                   // 拼图数据
#define MERGE_MEM_DATA      "MergeData_%s_%s"        // memcached的拼图数据

#define MERGE_ROOT_LOCK  "Merge_Locker"              // 拼图根锁
#define MERGE_INFO_LOCK  "MergeInfo_Locker_%s"       // 拼图信息锁
#define MERGE_TIME_LOCK  "MergeTime_Locker_%s"       // 拼图时间锁
#define MERGE_DATA_LOCK  "MergeData_Locker_%s"       // 拼图数据锁
#define MERGE_MEM_DATA_LOCK "MergeData_Locker_%s_%s" // 拼图数据锁

/************************************************************************/
/* 3. 客户端临时缓存中Key名称宏定义                                     */
/************************************************************************/
// 客户端数据以每个客户端编号进行管理
#define CLIENT_ROOT  "Client_%d"                     // 某客户端数据的根
#define CLIENT_INFO  "ClientInfo_%d_%s"              // 某客户端信息
#define CLIENT_TIME  "ClientTime_%d_%s"              // 某客户端时间
#define CLIENT_DATA  "ClientData_%d_%s"              // 某客户端数据
#define CLIENT_MEM_DATA      "ClientData_%d_%s_%s"   // memcached的拼图数据

#define CLIENT_ROOT_LOCK  "Client_Locker_%d"         // 某客户端根锁
#define CLIENT_INFO_LOCK  "ClientInfo_Locker_%d_%s"  // 某客户端信息锁
#define CLIENT_TIME_LOCK  "ClientTime_Locker_%d_%s"  // 某客户端时间锁
#define CLIENT_DATA_LOCK  "ClientData_Locker_%d_%s"  // 某客户端数据锁
#define CLIENT_MEM_DATA_LOCK "ClientData_Locker_%d_%s_%s" // 某客户端数据锁

/************************************************************************/
/* 4. 缓存中时间的定义-ProductTime                                      */
/************************************************************************/
#define CACHED_TIME_FORMAT   "%04d%02d%02d%02d0000.%03d_%02d"  //eg.20150829080000.012_03
#define CACHED_TIME_FORMAT2  "%04d%02d%02d%02d.%03d"           //eg.2015082908.012

/************************************************************************/
/* 5. Memcached中使用的宏定义                                           */
/************************************************************************/
#define POWER_BLOCK             1024000  // memcached中每个Data最大数据长度1M1048576
#define MEMCACHE_KEY_NOT_FOUND  16       // memcached中未找到Key的返回码是16

/************************************************************************/
/* 6. 缓存中站点数据定义                                                */
/************************************************************************/
#define STATION_ROOT  "Station_%s_%s_%02d"          // eg. Station_town_SCMOC_01
#define STATION_TIME  "StationTime_%s_%s_%02d_%s"   // eg. Station_bigcity_SPCC_12_55011              
#define STATION_INFO  "StationInfo_%s_%s_%02d_%s"   // eg. 解释：城镇/大城市_类型_间隔_站点  
#define STATION_DATA  "StationData_%s_%s_%02d_%s_%s"

#define STATION_ROOT_LOCK  "Station_Locker_%s_%s_%02d"
#define STATION_INFO_LOCK  "StationInfo_Locker_%s_%s_%02d_%s"    
#define STATION_TIME_LOCK  "StationTime_Locker_%s_%s_%02d_%s"    
#define STATION_DATA_LOCK  "StationData_Locker_%s_%s_%02d_%s_%s"    

#define CLIENT_STATION       "CLT%d"       // 客户端上传类型： %d:客户端编号
#define CLIENT_STATION_TYPE  "CLT%d_%s"    // 客户端上传类型： %d:客户端编号 %s：某类数据
#define CLIENT_STATION_TYPE_DIFF  "CLT%d_%s_DIFF" // 客户端上传类型： %d:客户端编号 %s：某类数据,用于存差值

/************************************************************************/
/* 8. 站点全部数据定义（站点文件整体内容）                              */
/************************************************************************/
#define STATION_FILE_ROOT  "StationFile"                     // eg. StationFile
#define STATION_FILE_ROOT_VAL "%s_%s_%02d"                   // eg. town_SCMOC_12
#define STATION_FILE_TIME  "StationFileTime_%s_%s_%02d"      // eg. StationFileTime_bigcity_SPCC_06        
#define STATION_FILE_DATA  "StationFileData_%s_%s_%02d_%s"   // eg. StationFileData_bigcity_SPCC_06_201510220800.000_12

#define STATION_FILE_ROOT_LOCK  "StationFile_Locker"
#define STATION_FILE_TIME_LOCK  "StationFileTime_Locker_%s_%s_%02d"
#define STATION_FILE_DATA_LOCK  "StationFileData_Locker_%s_%s_%02d_%s"

/************************************************************************/
/* 9. 某类型多站点多时效数据                                            */
/************************************************************************/
#define STATION_MULTI_ROOT  "StationMulti"                     // eg. StationFile
#define STATION_MULTI_ROOT_VAL "%s_%02d"                       // eg. town_SCMOC_12
#define STATION_MULTI_TIME  "StationMultiTime_%s_%02d"         // eg. StationMultiTime_TMP_03        
#define STATION_MULTI_DATA  "StationMultiData_%s_%02d_%s"      // eg. StationMultiData_ER24_24_2015102208.000_24

#define STATION_MULTI_ROOT_LOCK  "StationMulti_Locker"
#define STATION_MULTI_TIME_LOCK  "StationMultiTime_Locker_%s_%02d"
#define STATION_MULTI_DATA_LOCK  "StationMultiData_Locker_%s_%02d_%s"

/************************************************************************/
/* 10. 站点配置信息                                                     */
/************************************************************************/
#define STATION_CFG_ROOT     "StationCfg"        // 值分为：SVR表示服务器上的配置，CLTxxx:客户端长传配置                                              
#define STATION_CFG_DATA     "StationCfgData_%s" // eg.StationCfgData_SVR,StationCfgData_CLT1300

#define STATION_CFG_ROOT_LOCK  "StationCfg_Locker"
#define STATION_CFG_DATA_LOCK  "StationCfgData_Locker_%s"

/************************************************************************/
/* 7. 缓存配置定义                                                      */
/************************************************************************/
// 7.1 缓存类型
typedef enum _CACHED_TYPE
{
	_CACHED_UNKNOW = 0,    // 未知类型
	_CACHED_REDIS = 1,     // Redis
	_CACHED_MEMCACHED = 2  // MemCached
}EM_CACHED_TYPE;

// 7.2 缓存配置
typedef struct _CACHED
{
	EM_CACHED_TYPE emType; // 类型
	char  strSvrAddr[32];  // 服务地址
	int   nPortNo;         // 服务端口 
	bool  bDisabled;       // 缓存存储
}ST_CACHED;

#endif //CACHED_DEF_H

/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* 文件名: DataBaseDef.h
* 作  者: zhangl		版本：1.0		日  期：2015/07/06
* 描  述: 数据库宏定义头文件
* 其  他:
* 功能列表:
*   1.
* 修改日志：
*   No. 日期		作者		修改内容
*
*************************************************************************/
#ifndef DATABASEDEF_H
#define DATABASEDEF_H

#include <QString>
#include <QList>

/************************************************************************/
/* 1.数据库配置                                                         */
/************************************************************************/
// 1.1 配置文件中所使用的数据库字符串
#define DB_SQLITE        "sqlite"   // SQLite第三版
#define DB_MYSQL         "mysql"    // MySql数据库
#define DB_SQLSERVER     "sqlserver"// SqlServer数据库
#define DB_ORACLE        "oracle"   // Oracle数据库

// 1.2 数据库类型定义 - 目前要支持的数据库类型
typedef enum _DBTYPE
{
	NODEF = 0,     // 类型没有定义
	SQLITE = 1,     // Sqlite第三版
	MYSQL = 2,     // MySql数据库5.0
	SQLSERVER = 3,  // SqlServer数据库
	ORACLE = 4      // Oracle数据库
} ENUM_DBTYPE;

// 1.3 数据库配置结构体
typedef struct _DATABASE
{
	ENUM_DBTYPE emDBType; // 数据库类型
	QString strServer;    // 数据库服务IP地址
	QString strDBName;    // 数据库名称
	QString strUser;      // 数据库登录用户名
	QString strPassword;  // 数据库登录密码
	int  nPort;           // 数据库端口号
} ST_DATABASE, *LPST_DATABASE;

// 1.4 连接池配置结构体
typedef struct _CONNPOOL
{
	bool bDisabled;          // 功能是否禁用
	int nMinConnectNum;      // 最小连接数
	int nMaxConnectNum;      // 最大连接数
	ST_DATABASE  stDataBase; // 数据库连接配置
} ST_CONNPOOL;

/************************************************************************/
/* 2. 数据库各类型驱动定义                                              */
/************************************************************************/
#define DRIVER_DB2             "QDB2"     // IBM DB2
#define DRIVER_IBASE           "QIBASE"   // Borland InterBase Driver
#define DRIVER_MYSQL           "QMYSQL"   // MySQL Driver
#define DRIVER_ORACLE          "QOCI"     // Oracle Call Interface Driver
#define DRIVER_ODBC            "QODBC"    // ODBC Driver (includes Microsoft SQL Server)
#define DRIVER_PSQL            "QPSQL"    // PostgreSQL Driver
#define DRIVER_SQLITE          "QSQLITE"  // SQLite version 3 or above
#define DRIVER_SQLITE2         "QSQLITE2" // SQLite version 2
#define DRIVER_TDS             "QTDS"     // Sybase Adaptive Server

/************************************************************************/
/* 3. 数据库连接池用宏定义                                              */
/************************************************************************/
// 3.1 数据库连接名称基名定义
#define CONNECT_NAME           "DBConnection%1"

// 3.2 数据库连接池最大/最小连接数
#define DB_CONNECT_MIN_COUNT   1          // 最小连接数1个
#define DB_CONNECT_MAX_COUNT   10         // 最大连接数10

// 3.3 数据库连接结构体
typedef struct _DB_CONNECT
{
	QString strConnectName;  // 连接名称
	int     nConnectID;      // 连接ID
	bool    bConnectState;   // 连接状态 true:正常；false:断开
	bool    bUseState;       // 利用状态 true:正在使用；false:未被使用
} ST_DB_CONNECT;

// 3.4 数据库连接管理列表
typedef QList<ST_DB_CONNECT> LIST_DB_CONNECT;


/************************************************************************/
/* 4. 数据库表状态定义                                              */
/************************************************************************/
#define WAITTING    "WaitProcess"     // 等待处理
#define PROCESSING  "processing"      // 处理中
#define COMPLETE    "complete"        // 完成处理
#define ABNORMAL    "abnormal"        // 异常
#define REPROCESS   "reprocess"       //重新处理


#endif // DATABASEDEF_H


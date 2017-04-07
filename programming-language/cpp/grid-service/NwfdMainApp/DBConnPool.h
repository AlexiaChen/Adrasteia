/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* 文件名: DBConnPool.h
* 作  者: zhangl		版本：1.0		日  期：2015/07/06
* 描  述: 连接池管理类定义 - 单例
* 其  他:
* 功能列表:
*   1.
* 修改日志：
*   No. 日期		作者		修改内容
*
*************************************************************************/
#ifndef DBCONNPOOL_H
#define DBCONNPOOL_H

#include "DataBaseDef.h"

#include <QSqlDatabase>
#include <QObject>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>

/*************************************************************************/
/*  类  名: CDBConnPool                                                  */
/*  描  述: 连接池管理类--使用懒汉式单例                                      */
/*************************************************************************/
class CDBConnPool : public QThread
{
    Q_OBJECT
public:
    static CDBConnPool * GetInstance(); // 单例调用方法
    virtual ~CDBConnPool();

    // 初始化步骤  Init → Open
    bool Init(ST_DATABASE &stDBConnectParam, int nMinNum, int nMaxNum);
    bool StopThread();

    int  GetAConnection(QSqlDatabase &dbConnect);     // 获取一个空闲连接
    void RestoreAConnection(int nConnectID);          // 释放一个使用的连接
    int  GetDBOpenCount();                            // 获取连接池成功打开的连接数
    bool GetDBPoolState();                            // 获取连接池的状态

private:
    CDBConnPool();                                         // 构造函数私有
	bool Open();                                           // 打开数据库
	bool SetDBConnectParam(ST_DATABASE &stDBConnectParam); // 设置连接参数
    void SetConnectNum(int nMinNum, int nMaxNum);          // 设置连接池连接数目

    int  InitAllDBConnect();    // 初始化所有连接
    void DestroyAllDBConnect(); // 关闭所有连接

    void OpenADBConnect();                            // 初始化一个数据库连接
    bool OpenAConnection(QString strConnectName);     // 创建一个连接
    bool CloseAConnection(QString strConnectName);    // 关闭一个连接

    void CheckDBPoolState();
    void HandleDBReOpen();

protected:
    void run(); // 维护线程

signals:
    void SignalOpenNewConEvent();

private slots:
    void SlotOpenNewConnect();

private:
    friend class ConnGuard;            // 将守卫类作为连接池类的友元类
    QMutex m_MutexConnect;             // 连接互斥锁
    QMutex m_MutexNewConn;             // 创建新连接时的互斥锁

private:
    bool          m_bInitFlg;          // 初始化成功与否标志 true:成功；false:失败
    bool          m_bStopFlg;          // 线程停止标志
    int           m_nMinNum;           // 连接池最小连接数
    int           m_nMaxNum;           // 连接池最大连接数
    int           m_nCurConnNum;       // 当前连接数，同时用于连接名称，进行连接管理
    int           m_nDBOpenCount;      // 数据库管理列表中打开成功的数据库连接的个数
    int           m_nUnUseCount;       // 未被使用的连接数
    QString       m_strDBDriver;       // 数据库驱动名称

    LIST_DB_CONNECT m_lstDBConnect;    // 数据库管理连接列表
	ST_DATABASE     m_stDBConnectParam;// 数据库连接参数
};

/*************************************************************************/
/*  类  名: DBConnGuard                                                  */
/*  描  述: 连接池守卫类--使用懒汉式单例                                 */
/*         利用构造和析构函数保证连接取出和归还必须成对，防止资源泄露    */
/*************************************************************************/
class  DBConnGuard
{
public:
    DBConnGuard(QSqlDatabase& dbConnect)
    {
        m_nConnectID = CDBConnPool::GetInstance()->GetAConnection(dbConnect);
    }

    // 数据库操作当前所使用的守护方法
    DBConnGuard(QSqlDatabase& dbConnect, int &nConnectID)
    {
        nConnectID = CDBConnPool::GetInstance()->GetAConnection(dbConnect);
        m_nConnectID = nConnectID;
    }

    virtual ~DBConnGuard()
    {
        // 通过连接管理编号进行管理
        CDBConnPool::GetInstance()->RestoreAConnection(m_nConnectID);
    }

private:
    int m_nConnectID;  // 数据库连接管理ID
};

#endif // DBCONNPOOL_H

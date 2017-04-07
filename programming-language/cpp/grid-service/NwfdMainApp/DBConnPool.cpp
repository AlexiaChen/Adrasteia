/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* 文件名: DBConnPool.cpp
* 作  者: zhangl		版本：1.0		日  期：2015/07/06
* 描  述: 连接池管理类实现
* 其  他:
* 功能列表:
*   1.
* 修改日志：
*   No. 日期		作者		修改内容
*
*************************************************************************/
#include "log.h"
#include "DBConnPool.h"
#include "DataBaseSQLDef.h"
#include <QSqlQuery>
#include <QSqlError>

/*************************************************************************/
/*  类  名: CDBConnPool                                                  */
/*  方法名: CDBConnPool                                                  */
/*  功  能: 构造方法，初始化成员变量                                     */
/*  返回值:                                                              */
/*  参  数:                                                              */
/*************************************************************************/
CDBConnPool::CDBConnPool()
{
    m_bInitFlg     = false;
    m_bStopFlg     = false;
    m_nCurConnNum  = 0;
    m_nDBOpenCount = 0;
    m_nMinNum      = 0;
    m_nMaxNum      = 0;
    m_strDBDriver  = "";

    m_stDBConnectParam.emDBType    = NODEF;
    m_stDBConnectParam.strServer   = "";
    m_stDBConnectParam.strDBName   = "";
    m_stDBConnectParam.strUser     = "";
    m_stDBConnectParam.strPassword = "";
    m_stDBConnectParam.nPort       = 0;

    m_lstDBConnect.clear();
}

CDBConnPool::~CDBConnPool()
{
    // 停止线程
    StopThread();
}
/*************************************************************************/
/*  类  名: CDBConnPool                                                  */
/*  方法名: GetInstance                                                  */
/*  功  能: 获取数据库连接池单例指针的方法                               */
/*  返回值: CDBConnPool对象单例指针                                      */
/*  参  数:                                                              */
/*************************************************************************/
CDBConnPool * CDBConnPool::GetInstance()
{
    static CDBConnPool g_ConnPoolInstance;
    return &g_ConnPoolInstance;
}

/*************************************************************************/
/*  类  名: CDBConnPool                                                  */
/*  方法名: Init                                                         */
/*  功  能: 连接池初始化                                                 */
/*  返回值: 成功与否： true:成功；false:失败                             */
/*  参  数:                                                              */
/*************************************************************************/
bool CDBConnPool::Init(ST_DATABASE &stDBConnectParam, int nMinNum, int nMaxNum)
{
    // 设置最小，最大连接数
    SetConnectNum(nMinNum, nMaxNum);

    // 设置数据库连接参数
    if( !SetDBConnectParam(stDBConnectParam) )
    {
        m_bInitFlg = false;
        //CLog::GetInstance()->WriteErrorLog(LOG_DEGREE_LEVEL_1, QString("[连接池]数据库类型配置错误/目前不支持-{驱动=%1}").arg(m_strDBDriver));
        return false;
    }

    // 创建新数据库连接信号槽连接
    connect(this, SIGNAL(SignalOpenNewConEvent()), this, SLOT(SlotOpenNewConnect()));

    return true;
}

/*************************************************************************/
/*  类  名: CDBConnPool                                                   */
/*  方法名: SetDBConnectParam                                             */
/*  功  能: 设置连接参数                                                    */
/*  返回值: CDBConnPool对象单例指针                                         */
/*  参  数:                                                               */
/*************************************************************************/
bool CDBConnPool::SetDBConnectParam(ST_DATABASE &stDBConnectParam)
{
    m_stDBConnectParam = stDBConnectParam;
    // 根据数据库类型设置驱动名称
    switch(m_stDBConnectParam.emDBType)
    {
		case SQLITE:
			m_strDBDriver = QString::fromLocal8Bit(DRIVER_SQLITE);
			break;
        case MYSQL:
            m_strDBDriver = QString::fromLocal8Bit(DRIVER_MYSQL);
            break;
        case SQLSERVER:
            m_strDBDriver = QString::fromLocal8Bit(DRIVER_ODBC);
            break;
        case ORACLE:
            m_strDBDriver = QString::fromLocal8Bit(DRIVER_ORACLE);
            break;
        case NODEF:
            m_strDBDriver = "";
            return false;
            break;
        default:
            m_strDBDriver = "";
            return false;
    }

    return true;
}

void CDBConnPool::SetConnectNum(int nMinNum,int nMaxNum)
{
    m_nMinNum  = nMinNum;
    m_nMaxNum  = nMaxNum;

    if( m_nMinNum < DB_CONNECT_MIN_COUNT )
    {
        m_nMinNum = DB_CONNECT_MIN_COUNT;
    }
    else if( m_nMinNum > DB_CONNECT_MAX_COUNT )
    {
        m_nMinNum = DB_CONNECT_MAX_COUNT;
    }

    if( m_nMaxNum > DB_CONNECT_MAX_COUNT )
    {
        m_nMaxNum = DB_CONNECT_MAX_COUNT;
    }
    else if( m_nMaxNum < m_nMinNum )
    {
        m_nMaxNum = m_nMinNum;
    }
}

/*************************************************************************/
/*  类  名: CDBConnPool                                                   */
/*  方法名: Open                                                          */
/*  功  能: 打开数据库连接                                                  */
/*  返回值: 成功与否： true:成功；false:失败                                  */
/*  参  数:                                                               */
/*************************************************************************/
bool CDBConnPool::Open()
{
    // 初始化所有连接
    if( InitAllDBConnect() < DB_CONNECT_MIN_COUNT )
    {
        m_bInitFlg = false;            // 初始化失败没有连接
        LOG_(LOGID_INFO, LOG_F("[DataBase] 打开连接失败，连接个数(%d)没有达到最小连接数(%d)."), m_nDBOpenCount, DB_CONNECT_MIN_COUNT);
	}
    else
    {
        m_bInitFlg = true;             // 初始化成功

        // 初始化成功启动维护线程
        //start();
		LOG_(LOGID_INFO, LOG_F("[DataBase] 打开数据库连接成功."));
    }

    return m_bInitFlg;
}

bool CDBConnPool::StopThread()
{
    // 停止线程
    m_bStopFlg = true;
    wait();

    // 销毁所有连接
    DestroyAllDBConnect();

    return true;
}

/*************************************************************************/
/*  类  名: CDBConnPool                                                   */
/*  方法名: InitAllDBConnect                                              */
/*  功  能: 初始化所有连接                                                  */
/*  返回值: 初始化成功的个数                                                 */
/*  参  数:                                                               */
/*************************************************************************/
int CDBConnPool::InitAllDBConnect()
{
    if( m_strDBDriver.isEmpty() )
    {
        return 0;
    }

    // 销毁所有连接
    DestroyAllDBConnect();

    m_nCurConnNum  = 0;   // 连接管理ID-连接个数
    m_nDBOpenCount = 0;   // 打开成功的数据库个数
    m_nUnUseCount  = 0;   // 未被使用的连接个数

    // 以最小连接数进行建立
    for( int i = 0; i < m_nMinNum; i++ )
    {
        // 打开一个数据库连接
        OpenADBConnect();
    }

    return m_nDBOpenCount;
}

void CDBConnPool::DestroyAllDBConnect()
{
    // Log-Print 销毁所有数据库连接
	//LOG_(LOGID_INFO, LOG_F("[DataBase] 销毁所有数据库连接开始."));
    if( m_lstDBConnect.size() == 0 )
    {
		//LOG_(LOGID_DEBUG, LOG_F("[DataBase] 连接池连接数为0."));
        return;
    }

    LIST_DB_CONNECT::iterator iter = m_lstDBConnect.begin();
    for( ; iter != m_lstDBConnect.end(); iter++ )
    {
        ST_DB_CONNECT stDBConnect = (*iter);
        if( stDBConnect.bUseState )
        {
            // 连接正在使用中
        }

        // 关闭一个连接
        CloseAConnection(stDBConnect.strConnectName);
        // 从数据库连接列表中移除此连接
        QSqlDatabase::removeDatabase(stDBConnect.strConnectName);

        // Log-Print 成功从数据库列表中移除连接<连接名>
        //CLog::GetInstance()->WriteDebugLog(LOG_DEGREE_LEVEL_5, QString(PRINT_DEBUG_DBPOOL_REMOVE_CONNECT).arg(stDBConnect.strConnectName));
    }

    m_lstDBConnect.clear();

    m_nCurConnNum  = 0;   // 连接管理ID-连接个数
    m_nDBOpenCount = 0;   // 打开成功的数据库个数
    m_nUnUseCount  = 0;   // 未被使用的连接个数
	//LOG_(LOGID_INFO, LOG_F("[DataBase] 销毁所有数据库结束."));
}

/*************************************************************************/
/*  类  名: CDBConnPool                                                   */
/*  方法名: GetAConnection                                                */
/*  功  能: 获取一个空闲连接                                                 */
/*  返回值: 连接名称                                                        */
/*  参  数:                                                               */
/*************************************************************************/
int CDBConnPool::GetAConnection(QSqlDatabase &dbConnect)
{
    int nConnectID;
    nConnectID = -1;

    int nCount = 0;

    // 未被利用数=0 ，延时等待释放
    while( m_nUnUseCount == 0)
    {
        // 发送建立新连接的信号
        emit(SignalOpenNewConEvent());

        msleep(100);       // 100毫秒
        if(nCount++ == 10) // 共等待1秒
        {
            break;
        }
    }

    QMutexLocker locker(&m_MutexConnect);

    // 从连接管理列表中找到一个未被使用的连接
    LIST_DB_CONNECT::iterator iter = m_lstDBConnect.begin();
    for( ; iter != m_lstDBConnect.end(); iter++ )
    {
        if( !(*iter).bUseState && (*iter).bConnectState )
        {
            dbConnect = QSqlDatabase::database((*iter).strConnectName); // 数据库连接
            nConnectID = (*iter).nConnectID; // 数据库连接的ID号
            (*iter).bUseState = true;        // 更改标记为使用
            m_nUnUseCount--;                 // 未被利用数
            break;
        }
    }

    return nConnectID;
}

/*************************************************************************/
/*  类  名: CDBConnPool                                                   */
/*  方法名: RestoreAConnection                                            */
/*  功  能: 释放一个使用的连接                                               */
/*  返回值:                                                               */
/*  参  数: strConnectName:数据库连接名                                     */
/*************************************************************************/
void CDBConnPool::RestoreAConnection(int nConnectID)
{
    QMutexLocker locker(&m_MutexConnect);

    // 从连接管理列表中找到一个未被使用的连接
    LIST_DB_CONNECT::iterator iter = m_lstDBConnect.begin();
    for( ; iter != m_lstDBConnect.end(); iter++ )
    {
        if( (*iter).nConnectID == nConnectID )
        {
            (*iter).bUseState = false; // 更改标记为未被利用
            m_nUnUseCount++;           // 未被利用数
            break;
        }
    }
}

/*************************************************************************/
/*  类  名: CDBConnPool                                                   */
/*  方法名: OpenADBConnect                                                */
/*  功  能: 初始化一个数据库连接                                              */
/*  返回值:                                                               */
/*  参  数:                                                               */
/*************************************************************************/
void CDBConnPool::OpenADBConnect()
{
    QMutexLocker locker(&m_MutexNewConn);

    // 当前连接个数
    m_nCurConnNum++;
    // 连接名称
    QString strConnectName = QString(CONNECT_NAME).arg(QString::number(m_nCurConnNum));

    ST_DB_CONNECT stDBConnect;

    if( OpenAConnection(strConnectName) )
    {
        // 打开一个连接成功
        stDBConnect.strConnectName = strConnectName; // 连接名称
        stDBConnect.nConnectID     = m_nCurConnNum;  // 连接管理ID
        stDBConnect.bConnectState  = true;           // 连接状态正常
        stDBConnect.bUseState      = false;          // 使用状态未被使用
        // 正常连接的个数
        m_nDBOpenCount++;

    }
    else
    {
        // 打开一个连接失败
        stDBConnect.strConnectName = strConnectName; // 连接名称
        stDBConnect.nConnectID     = m_nCurConnNum;  // 连接管理ID
        stDBConnect.bConnectState  = false;          // 连接状态断开
        stDBConnect.bUseState      = false;          // 使用状态未被使用
    }

    // 未被使用的数据库连接数
    m_nUnUseCount++;

    // 插入到数据库连接管理列表中
    m_lstDBConnect.push_back(stDBConnect);
}

/*************************************************************************/
/*  类  名: CDBConnPool                                                   */
/*  方法名: OpenAConnection                                               */
/*  功  能: 创建一个连接                                                    */
/*  返回值:  连接是否打开成功                                                */
/*  参  数:  strConnectName：连接的名称                                     */
/*************************************************************************/
bool CDBConnPool::OpenAConnection(QString strConnectName)
{
    // 如果驱动为空或者连接名称为空，返回失败
    if( m_strDBDriver.isEmpty() || strConnectName.isEmpty() )
    {
        return false;
    }

    try{
        if( QSqlDatabase::contains(strConnectName) )
        {
            // 如果连接名称已经存在，返回失败
            //CLog::GetInstance()->WriteErrorLog(LOG_DEGREE_LEVEL_2,QString("[连接池]数据库连接已存在<连接名%1>").arg(strConnectName));
            return false;
        }

        QSqlDatabase dbConnect = QSqlDatabase::addDatabase(m_strDBDriver, strConnectName);

        dbConnect.setHostName(m_stDBConnectParam.strServer);
        //dbConnect.setPort(m_stDBConnectParam.nPort);
        dbConnect.setDatabaseName(m_stDBConnectParam.strDBName);
        dbConnect.setUserName(m_stDBConnectParam.strUser);
        dbConnect.setPassword(m_stDBConnectParam.strPassword);

        if( dbConnect.open() )
        {
            // 连接打开成功
			LOG_(LOGID_INFO, LOG_F("[DataBase] 打开数据库连接成功<连接名%s>."), LOG_STR(strConnectName));
            return true;
        }
        else
        {
            // 连接打开失败
			LOG_(LOGID_INFO, LOG_F("[DataBase] 打开数据库连接失败<连接名%s>:%s."), LOG_STR(strConnectName), LOG_STR(dbConnect.lastError().text()));
			LOG_(LOGID_ERROR, LOG_F("[DataBase] 打开数据库连接失败<连接名%s>:%s."), LOG_STR(strConnectName), LOG_STR(dbConnect.lastError().text()));
            return false;
        }
    }
    catch(...)
    {
        // 异常
        //CLog::GetInstance()->WriteErrorLog(LOG_DEGREE_LEVEL_1,QString("[连接池]打开数据库连接异常<连接名%1>").arg(strConnectName));
		LOG_(LOGID_ERROR, LOG_F("[DataBase] 打开数据库连接异常<连接名%s>."), LOG_STR(strConnectName));

    }

    return false;
}
/*************************************************************************/
/*  类  名: CDBConnPool                                                   */
/*  方法名: CloseAConnection                                              */
/*  功  能: 关闭一个连接                                                    */
/*  返回值:  连接是否关闭成功                                                */
/*  参  数:  strConnectName：连接的名称                                     */
/*************************************************************************/
bool CDBConnPool::CloseAConnection(QString strConnectName)
{
    // 连接名称不在数据库连接列表中
    if( !QSqlDatabase::contains(strConnectName) )
    {
        //CLog::GetInstance()->WriteErrorLog(LOG_DEGREE_LEVEL_2,QString("[连接池]关闭连接：%1不在数据库列表中").arg(strConnectName));
        return false;
    }
    try{
        QSqlDatabase dbConnect = QSqlDatabase::database(strConnectName);
        if( !dbConnect.isOpen() )
        {
            return true;
        }
        else
        {
            // 关闭连接
            dbConnect.close();
            //CLog::GetInstance()->WriteDebugLog(LOG_DEGREE_LEVEL_3,QString("[连接池]关闭数据库连接成功<连接名%1>").arg(strConnectName));
        }
    }
    catch(...)
    {
        //CLog::GetInstance()->WriteErrorLog(LOG_DEGREE_LEVEL_1,QString("[连接池]关闭连接异常，连接名%1").arg(strConnectName));
        return false;
    }

    return true;
}

/*************************************************************************/
/*  类  名: CDBConnPool                                                   */
/*  方法名: run                                                           */
/*  功  能: 维护线程，检测数据库的连接是否正常                                  */
/*  返回值:                                                               */
/*  参  数:                                                               */
/*************************************************************************/
void CDBConnPool::run()
{
	m_bStopFlg = false;

	// 1. 初始化数据库连接池
	int nDBConnCount = 0;
	while (!m_bStopFlg)
	{
		if (Open())
		{
			// 如果打开成功，结束连接数据库
			LOG_(LOGID_INFO, LOG_F("[DataBase] 打开数据库成功."));
			break;
		}

		if (nDBConnCount++ > 10)
		{
			nDBConnCount = 10;
		}
		// 连接池初始化失败, 延时执行
		sleep(10 * nDBConnCount);
	}

	//// 2. 执行数据库连接检查
 //   while( !m_bStopFlg )
 //   {
	//	// todo
 //       if( !m_bInitFlg )
 //       {
 //           msleep(500);
 //           continue;
 //       }

 //       // 执行维护任务
 //       // 检测数据库连接状态
 //       CheckDBPoolState();

 //       //断开的连接尝试建立连接
 //       HandleDBReOpen();

 //       // 延时5秒
 //       sleep(5);
 //   }
}


/*************************************************************************/
/*  类  名: CDBConnPool                                                   */
/*  方法名: CheckDBPoolState                                              */
/*  功  能: 检测数据库连接状态                                               */
/*  返回值:                                                               */
/*  参  数:                                                               */
/*************************************************************************/
void CDBConnPool::CheckDBPoolState()
{
    QMutexLocker locker(&m_MutexConnect);
    try
    {
        LIST_DB_CONNECT::iterator iter;
        for( iter = m_lstDBConnect.begin(); iter != m_lstDBConnect.end(); iter++ )
        {
            ST_DB_CONNECT stDBConnect = (*iter);

            if( !stDBConnect.bConnectState || stDBConnect.bUseState )
            {
                // 数据库连接正在使用中或者数据库连接断开，跳过检测
                continue;
            }

            // 数据库连接
            QSqlDatabase dbConnect = QSqlDatabase::database(stDBConnect.strConnectName);

            QSqlQuery query(dbConnect);
            bool bStatus = query.exec(CHECK_CONNECT_STATE);
            //bool bStatus = query.next();
            if( !bStatus )
            {
                //连接断开
                iter->bConnectState = false; // 连接断开
                m_nDBOpenCount--;            // 打开的连接数减一

                //CLog::GetInstance()->WriteErrorLog(LOG_DEGREE_LEVEL_2,QString("[连接池]数据库连接断开<连接名%1>").arg(stDBConnect.strConnectName));
            }
        }
    }
    catch(...)
    {
        //CLog::GetInstance()->WriteErrorLog(LOG_DEGREE_LEVEL_2,QString("[连接池]数据库连接检测异常"));
    }
}

/*************************************************************************/
/*  类  名: CDBConnPool                                                   */
/*  方法名: HandleDBReOpen                                                */
/*  功  能: 数据库断开连接重连                                               */
/*  返回值:                                                               */
/*  参  数:                                                               */
/*************************************************************************/
void CDBConnPool::HandleDBReOpen()
{
    LIST_DB_CONNECT::iterator iter;
    for( iter = m_lstDBConnect.begin(); iter != m_lstDBConnect.end(); iter++ )
    {
        ST_DB_CONNECT stDBConnect = (*iter);
        if( !stDBConnect.bConnectState )
        {
            // 如果连接断开，重新建立连接
            QSqlDatabase dbConnect = QSqlDatabase::database(stDBConnect.strConnectName);
            if( dbConnect.open() )
            {
                dbConnect.close();
            }

            //CLog::GetInstance()->WriteDebugLog(LOG_DEGREE_LEVEL_5,QString("[连接池]数据库尝试重新建立连接<连接名%1>").arg(stDBConnect.strConnectName));

            if( dbConnect.open() )
            {
                iter->bConnectState = true;  // 连接正常
                m_nDBOpenCount++;            // 打开的连接数加一
                //CLog::GetInstance()->WriteInfoLog(LOG_DEGREE_LEVEL_2,QString("[连接池]数据库连接重连成功<连接名%1>").arg(stDBConnect.strConnectName));
            }
        }
    }
}

/*************************************************************************/
/*  类  名: CDBConnPool                                                   */
/*  方法名: SlotOpenNewConnect                                            */
/*  功  能: 创建新连接信号槽                                                */
/*  返回值:                                                               */
/*  参  数:                                                               */
/*************************************************************************/
void CDBConnPool::SlotOpenNewConnect()
{
    m_MutexNewConn.lock();
    if( m_nCurConnNum >= m_nMaxNum )
    {
        m_MutexNewConn.unlock();
        // 建立的连接数已经超过最大连接数-不能再建立
        //CLog::GetInstance()->WriteDebugLog(LOG_DEGREE_LEVEL_4,QString("[连接池]当前连接数已达到最大连接数，不能再建立新的连接"));
        return;
    }
    m_MutexNewConn.unlock();

    // 打开一个数据库连接
    OpenADBConnect();
}

/*************************************************************************/
/*  类  名: CDBConnPool                                                   */
/*  方法名: GetDBOpenCount                                                */
/*  功  能: 获取成功打开的数据库连接个数                                       */
/*  返回值: 成功打开的数据库连接个数                                           */
/*  参  数:                                                               */
/*************************************************************************/
int  CDBConnPool::GetDBOpenCount()
{
    return m_nDBOpenCount;
}

/*************************************************************************/
/*  类  名: CDBConnPool                                                   */
/*  方法名: GetDBPoolState                                                */
/*  功  能: 获取连接池的状态                                                 */
/*  返回值: 当所有连接都断开时，返回false;否则，返回true                         */
/*  参  数:                                                               */
/*************************************************************************/
bool CDBConnPool::GetDBPoolState()
{
    if(m_nDBOpenCount <= 0 )
    {
        return false;
    }

    return true;
}

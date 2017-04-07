#include "nwfdmainapp.h"
#include "Common.h"
#include <QSystemSemaphore>
#include <QSharedMemory>
#include <QMessageBox>
#include <QFile>
#include <QTextCodec>

#ifndef WIN32
#include <signal.h>
#endif

#pragma execution_character_set("utf-8")

/************************************************************************/
/* main函数                                                             */
/************************************************************************/
int main(int argc, char *argv[])
{
	QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

	/* 单例程序检测-使用共享内存的方法 */
	//QSystemSemaphore sema("NwfdServer_APP", 1, QSystemSemaphore::Open);
	//sema.acquire();
	//QSharedMemory mem("Nwfd");
	//if (!mem.create(1))
	//{
	//	//QMessageBox::information(0, "NWFD", "Nwfd服务程序已经在运行中！\n 本次启动不能执行！.");
	//	sema.release();
	//	return 0;
	//}
	//sema.release();// 临界区

	/*  开始执行  */
	NwfdMainApp mwNwfd;
    
    //忽略SIGPIPE信号
#ifndef WIN32
    signal(SIGPIPE, SIG_IGN);
#endif
	// 防止程序执行完毕退出
	while (1)
	{
		delay(10000);
		printf("Service is running... \n");
	}

	return 0;
}

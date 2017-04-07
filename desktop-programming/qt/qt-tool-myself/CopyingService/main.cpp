
//#include <QtCore/QCoreApplication>
#include <QString>
#include <Windows.h>
#include <QStringList>
#include <QDir>
#include <QFileInfoList>
#include <QDateTime>

#include "CopyingTask.h"


static QFileInfoList getLocalFiles(const QString& dir){
    QFileInfoList ret;

    QDir srcDir;
    srcDir.setPath(dir);
    srcDir.setFilter(QDir::Files);   // 列出目录中的文件
    srcDir.setSorting(QDir::Name);   // 按照文件名排序 

    // 获取产品文件
    QFileInfoList files = srcDir.entryInfoList();

    //筛选出当天特定的文件
    QDateTime date = QDateTime::currentDateTime();
    int nYear = date.date().year();
    int nMonth = date.date().month();
    int nDay = date.date().day();
    
    QString pattern(QString("").sprintf("*%04d%02d%02d*.GRB2",nYear,nMonth,nDay));

    QRegExp rx(pattern.toStdString().c_str());
    rx.setPatternSyntax(QRegExp::Wildcard);

    for (int i = 0; i < files.size(); ++i)
    {
        QString filename = files.at(i).fileName();
        if (rx.exactMatch(filename))
        {
            ret.append(files.at(i));
        }
    }

    return ret;

}

int main(int argc, char *argv[])
{
  //  QCoreApplication a(argc, argv);

    QStringList srcDirs;
    srcDirs << "X:/fsol/rrh_05";
    srcDirs << "X:/fsol/rhx_05";
    srcDirs << "X:/fsol/rhi_05";
    srcDirs << "X:/fsol/ttt_05";
    srcDirs << "X:/fsol/tmx_05";
    srcDirs << "X:/fsol/tmi_05";

    srcDirs << "X:/forecast/QPF_V2/gridrain03";
    srcDirs << "X:/forecast/QPF_V2/gridrain24";

    srcDirs << "X:/forecast/WARNING/hot";
    srcDirs << "X:/forecast/WARNING/fog";

    QString SnapShotDir("D:/CMISS-NWFD/bak");
    QString UploadWaitDir("D:/CMISS-NWFD/Upload/wait");

    CopyingTask *pTask = new CopyingTask();

   
    pTask->setSnapShotDir(SnapShotDir);
    pTask->setUploadWaitDir(UploadWaitDir);
    
    while (true)
    {
        Sleep(3000);

        for (int i = 0; i < srcDirs.size(); ++i)
        {
            QString srcDir = srcDirs.at(i);
            
            pTask->setSrcDir(srcDir);
            
            QFileInfoList files = getLocalFiles(srcDir);
            pTask->setFileList(files);
            pTask->start();
        }
        
      
    }

    delete pTask;


    return 0;
}

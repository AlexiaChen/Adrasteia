#include "CopyingTask.h"

#include "HandleCommon.h"

CopyingTask::CopyingTask()
{

}

CopyingTask::~CopyingTask()
{

}

void CopyingTask::setSrcDir(const QString& dir)
{
    m_srcDir = dir;
}


void CopyingTask::setSnapShotDir(const QString& dir)
{
    m_snapShotDir = dir;
    QDir tmp(m_snapShotDir);

    if (!tmp.exists())
    {
        tmp.mkpath(dir);
    }
}

void CopyingTask::setUploadWaitDir(const QString& dir)
{
    m_uploadWaitDir = dir;
}

void CopyingTask::start()
{
    
    for (int i = 0; i < m_fileList.size(); ++i)
    {
        QFileInfo file = m_fileList.at(i);
        QString fileName = file.fileName();


        QString timeStamp = HandleCommon::GetFileCreatedTime(file.absoluteFilePath());

        if (isLastModified(fileName, timeStamp))
        {
            //step 1: copy to snapshot
            QFile snap(m_snapShotDir + QString("/") + fileName);
            if (snap.exists())
            {
                QFile::remove(m_snapShotDir + QString("/") + fileName);
            }
           
            if (QFile::copy(file.absoluteFilePath(), m_snapShotDir + QString("/") + fileName))
            {
                //step 2: copy to uploadWait  and rename it
                QString type, reportTime, maxForcast, range;
                parseFileName(fileName, type, reportTime, maxForcast, range);

                QString createTimeStamp = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
                
                QString waitFiletmp = QString("").sprintf("Z_NWGD_C_BABJ_%s_P_RFFC_SCMOC-%s_%s_%s%s.GRB2.tmp",
                createTimeStamp,type,reportTime,maxForcast,range);
                
                QString waitFile = QString("").sprintf("Z_NWGD_C_BABJ_%s_P_RFFC_SCMOC-%s_%s_%s%s.GRB2",
                    createTimeStamp, type, reportTime, maxForcast, range);
                
                if (QFile::copy(m_snapShotDir + QString("/") + fileName, m_uploadWaitDir + QString("/") + waitFiletmp))
                {
                    QFile::rename(m_uploadWaitDir + QString("/") + waitFiletmp, m_uploadWaitDir + QString("/") + waitFile);
                }
                

            }

           

        }

    }
}

void CopyingTask::setFileList(const QFileInfoList& list)
{
    m_fileList = list;
}


bool CopyingTask::isLastModified(const QString& fileName, const QString& timeStamp)
{
    if (m_hasFileLastModified.contains(fileName))
    {
        
        QString oldtimeStamp = m_hasFileLastModified.value(fileName);
        if (oldtimeStamp != timeStamp)
        {
            m_hasFileLastModified[fileName] = timeStamp;
            return true;
        }
        else
        {
            return false;
        }
        
    }
    else
    {
        
        m_hasFileLastModified[fileName] = timeStamp;
        
        return true;
    }

}

void CopyingTask::parseFileName(const QString& fileName, QString & type, QString& reportTime, QString & maxForcast, QString& range)
{
    //GDFS_NMC_AMEL_QPF_R03_ACHN_LNO_G01_YYYYMMDDHHmm07203.GRB2

    QString tmpType = fileName.split("_")[4];
    if (tmpType == "RRH")
    {
        type = "ERH";
    }
    else if (tmpType == "RHMX")
    {
        type = "ERHA";
    }
    else if (tmpType == "RHMI")
    {
        type = "ERHI";
    }
    else if (tmpType == "R03")
    {
        type = "ER03";
    }
    else if (tmpType == "R06")
    {

    }
    else if (tmpType == "R12")
    {

    }
    else if (tmpType == "R24")
    {
        type = "ER24";
    }
    else if (tmpType == "ETM")
    {
        type = "ETH";
    }
    else
    {
        type = tmpType;
    }

    QString tmpStamp = fileName.split("_")[8];

    reportTime = tmpStamp.mid(0, 12);
    maxForcast = tmpStamp.mid(12, 3);
    range = tmpStamp.mid(15, 2);

}

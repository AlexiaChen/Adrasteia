#ifndef COPYING_TASK_H
#define COPYING_TASK_H

#include <QString>
#include <QStringList>
#include <QFileInfoList>
#include <QHash>
class CopyingTask
{
public:
    CopyingTask();
    ~CopyingTask();

    void setSrcDir(const QString& dir);
    void setSnapShotDir(const QString& dir);
    void setUploadWaitDir(const QString& dir);

    void setFileList(const QFileInfoList& list);
    void start();

private:
   

    bool isLastModified(const QString& fileName, const QString& timeStamp);
    void parseFileName(const QString& fileName, QString & type, QString& reportTime, QString & maxForcast, QString& range);

private:

    QString m_srcDir;
    QString m_snapShotDir;
    QString m_uploadWaitDir;
    QFileInfoList m_fileList;
    QHash<QString, QString> m_hasFileLastModified;


};







#endif
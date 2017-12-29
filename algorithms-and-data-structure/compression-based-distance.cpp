#include <QtCore/QCoreApplication>
#include <QByteArray>
#include <QDebug>

// https://stackoverflow.com/questions/451884/similar-string-algorithm#

static bool isSimilarlyEqualFromScore(int sliceNum, int score){
    
    if (sliceNum == 1 && score > 28 && score <= 36)
    {
        return true;
    }

    if (sliceNum == 2 && score > 38 && score <= 46)
    {
        return true;
    }

    if (sliceNum == 3 && score >= 44 && score <= 56)
    {
        return true;
    }

    if (sliceNum == 4 && score >= 54 && score <= 78)
    {
        return true;
    }
    
    
    return false;
}

static bool isSimilarlyEqual(const QString& source, const QString& target){


    QString st = source + target;
    QString ts = target + source;
    
    int score = 2 * (qCompress(source.toLower().toLatin1(), 9).size() + qCompress(target.toLower().toLatin1(), 9).size())
        - qCompress(st.toLower().toLatin1(), 9).size() - qCompress(ts.toLower().toLatin1(), 9).size();

    qDebug() << source<< "and" <<  target << " string's compression-based distance is:" << score;

    return isSimilarlyEqualFromScore(source.split(QRegExp("\\s+")).count(),score);

}



int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);


    isSimilarlyEqual("Hello", "Hello");
    isSimilarlyEqual("Hello", "Hllo");
    isSimilarlyEqual("HeLlLo", "hello");
    isSimilarlyEqual("HeLlO", "hEllo");


    isSimilarlyEqual("Fastest", "fastest");
    isSimilarlyEqual("Fastest", "Fast");

    isSimilarlyEqual("QQ", "QQ Game");
    isSimilarlyEqual("TIM", "Tencent TIM");
  
    isSimilarlyEqual("Hello World", "Hello world");
    isSimilarlyEqual("Hello World", "Hello Word");
    isSimilarlyEqual("Hello World", "hllo world");
    isSimilarlyEqual("Hello World", "Hello");

    isSimilarlyEqual("WireShark 1.0.1", "WireShark 1.0.0");
    isSimilarlyEqual("WireShark", "Wire Shark");
    isSimilarlyEqual("Adobe Flash     ", "Adobe Flash Player 1.0.5");
    isSimilarlyEqual("Adobe AIR", "Adobe Flash Player");

    isSimilarlyEqual("WPS Office 2005", "WPS Office 2010");
    isSimilarlyEqual("WPS Excel 2010", "WPS PowerPoint");
    isSimilarlyEqual("WPS PowerPoint 2017", "WPS PowerPoint 2015");
    isSimilarlyEqual("WPS PowerPoint 2017", "WPS PowerPoint Professional");
    isSimilarlyEqual("WPS PowerPoint 2017", "WPS PowerPoint 2017");


    isSimilarlyEqual("Rockstar Games Social Club", "Rockstar Games Social Club");
    isSimilarlyEqual("Rockstar Games Social Club", "Rockstar Games Social Room");
    isSimilarlyEqual("Rockstar Games Social Club", "Rockstar Games Social  ");
    isSimilarlyEqual("Microsoft Visio Professional 2013", "Microsoft Visio Professional 2010");
    isSimilarlyEqual("Microsoft Visio Professional 2013", "Microsoft Visio Professional 2013");
    isSimilarlyEqual("Microsoft Visio Professional 2013", "Microsoft Visio Enterprise 2010");
    isSimilarlyEqual("Microsoft Visio Professional 2013", "Microsoft Visio  2016");
    isSimilarlyEqual("Microsoft Visio Professional 2013", "Microsoft Visual Studio");


    isSimilarlyEqual("Microsoft Visio Professional 2013", "Microsoft Visio Professional 2010");
   
    
    return a.exec();
}

#ifndef PRODUCTCFG_H
#define PRODUCTCFG_H

#include <QObject>
#include <QString>
#include <QDomDocument>
#include <QDomElement>
#include <QVector>
#include <QFile>

typedef struct _Time{
	QString hour;
	QString min;
	QString second;
}Time;

typedef QVector<Time> TimeList;

typedef struct _ScanTime{
	QString interval;
	TimeList timeList;
}ScanTime;

typedef struct _File{
	QString forcastTime;
	QString timeRange;
}File;

typedef QVector<File> FileList;

typedef struct _ForeCast{
	QString hourTime;
	QString filecount;
	QString RangeTime;
	QString startTime;
	QString endTime;
	QString latestTime;
	FileList fileList;
}ForeCast;

typedef QVector<ForeCast> ForeCastList;

typedef struct _ForeCastCfg{
	ForeCastList forecastList;
}ForeCastCfg;


typedef struct _Template{
	QString type;
	QString value;
}Template;

typedef struct _Mask{
	QString lon1;
	QString lon2;
	QString lat1;
	QString lat2;
	QString di;
	QString dj;
	QString filePath;
}Mask;

typedef struct _Item{
	QString clientID;
	QString lon1;
	QString lon2;
	QString lat1;
	QString lat2;
	QString srcFolder;
	QString srcfileformat;
}Item;

typedef QVector<Item> ItemList;

typedef struct _DataSrc{
	ItemList itemList;
}DataSrc;

typedef struct  _SaveFile{
	bool bDisable;
	QString savepath;
	QString filename;
	QString dds;
}SaveFile;


typedef struct _FileMerge{
	bool bDisable;
	QString savepath;
	QString filename;
	QString dds;
}FileMerge;

typedef struct _Chain{
	QString params;
	QString libName;
}Chain;

typedef QVector<Chain> ChainList;

typedef struct _Chains{
	QString ID;
	QString name;
	bool bDisable;
	QString savepath;
	QString filename;
	QString dds;
	ChainList chainList;
}Chains;

typedef QVector<Chains> ChainsList;

typedef struct _DataProc{
	ChainsList chainsList;
}DataProc;


typedef struct _CachedSave{
	bool bDisable;
	QString validtime;
}CachedSave;

typedef struct _Product{
	QString key;
	QString name;
	QString role;
	QString type;
	QString publisher;
	QString category;
	QString element;
	QString statistical;
	QString status;
	QString lon1;
	QString lon2;
	QString lat1;
	QString lat2;
	QString di;
	QString dj;
	QString offset;
	QString TimeRange;
	QString MaxForecastTime;
	QString BusinessStatus;
	bool isWarning;
	QString srcfolder;
	QString srcfileformat;
	ScanTime scanTime;
	ForeCastCfg forecastCfg;
	CachedSave savecached;
	SaveFile savefile;
	QString relatedProduct;
	Template template_t;
	Mask mask;
	DataSrc dataSrc;
	FileMerge fileMerge;
	DataProc dataProc;
	bool isGridtoStat;
}Product;

typedef QVector<Product> ProductList;

class ProductCfg : public QObject
{
	Q_OBJECT

public:
	ProductCfg(QString& path, QObject *parent);
	~ProductCfg();

	bool isInvalid() const;
	void saveToFile();
	void openFile();

	//getter
	void getProductList(ProductList &raw,ProductList& merge);
	//setter
	void setProductList(ProductList &list);
	void setProductListNew(ProductList &raw, ProductList& merge);

private:
	void addRawToDomTree(ProductList& raw, QDomElement &productsEle);
	void addMergeToDomTree(ProductList& merge, QDomElement &productsEle);
	void addProductHeader(QDomElement &productEle,Product & prod);
	void addProductScanTime(QDomElement &productEle, Product & prod);
	void addProductSrc(QDomElement &productEle, Product & prod);
	void addProductForeCastCfg(QDomElement &productEle, Product & prod);
	void addProductDataProc(QDomElement &productEle, Product & prod);
	void addSavedCached(QDomElement &productEle, Product & prod);
	void addGridToStation(QDomElement &productEle, Product & prod);
	void addFileMerge(QDomElement &productEle, Product & prod);
	void addRelatedProduct(QDomElement &productEle, Product & prod);
	void addTemplate(QDomElement &productEle, Product & prod);
	void addMask(QDomElement &productEle, Product & prod);
	void addDataSrc(QDomElement &productEle, Product & prod);

private:
	QString cfgPath;
	bool m_isInvalid;
	QDomDocument m_doc;
	QDomElement m_root;
	QFile m_xmlFile;
	
};

#endif // PRODUCTCFG_H

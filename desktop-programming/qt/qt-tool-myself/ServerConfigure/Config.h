#ifndef SERVERCONFIG_H
#define SERVERCONFIG_H

#include <cmath>
#include <QtWidgets/QDialog>
#include "ui_serverconfig.h"

#include "ServerCfg.h"
#include "ClientCfg.h"
#include "ProductCfg.h"
#include "StationCfg.h"
#include "updateProductdlg.h"
#include "addProductDlg.h"
#include "PublisherConfig.h"
#include "TypeConfig.h"
#include  "updateStatDlg.h"
#include "DataType.h"
#include "updateClientDlg.h"

class ConfigParser;

class Config : public QDialog
{
	Q_OBJECT

public:
	Config(QString& path, QWidget *parent = 0);
	~Config();

	static int findProductIndex(ProductList& list,QString &type, QString &dij, QString &publisher,
		QString &timerange, QString &maxtime);

	static int findProductIndex2(ProductList& list, QString &type, QString &dij, QString &publisher,
		QString &timerange);

	static int findStationIndex(StationList& list, QString &citytype,
		QString &timerange);

	static int findClientIndex(ClientList& list, QString& id);

public:
	inline void initLanLat(float lon1,float lon2, float lat1, float lat2)
	{
		 m_lon1 = lon1;
		 m_lon2 = lon2;
		 m_lat1 = lat1;
		 m_lat2 = lat2;
	}

public slots:
	void loadCurrentChange(int index);
	void init();
	
	
	void loadClientCfg();
	void loadStationCfg();
	void loadProductCfg();


	void saveProductCfg();
	void saveClientCfg();
	void saveStationCfg();
	
	
	void updateStationCfg(int raw_index);
	void updateProductCfg(int raw_index, int merge_index);
	void updateClientCfg(int index);

	void selectProductOneRow(int row, int column);
	void selectStationOneRow(int row, int column);
	void selectClientOneRow(int row, int column);

	void addProduct();
	void addProductInfo(QString&type,QString&dij,QString&publisher,QString& timerange,QString&maxtime);
	void deleteProduct();
	void updateProduct();

	void addStation();
	void deleteStation();
	void updateStation();
	
	void updateClient();
	
	void refreshProductCfg();
	void refreshStationCfg();
	void refreshClientCfg();

private:
	void createConnectBind();
	

private:
	// product private
	void insertProductRow(int row, const QString &type, const QString &dij, const QString &publisher,
		const QString &timerange, const QString &maxtime);
	void getProductInfoByRow(int row, QString &type, QString &dij, QString &publisher,
		QString &timerange, QString &maxtime);

	
	void createNewMergeProduct(Product& merge, Product& raw);
	void createNewRawProduct(Product &raw,QString &type,QString& dij,QString& publisher,
		QString& timerange,QString &maxtime);

	void createMergeProductHeader(Product& merge, Product& raw);
	void createRawProductHeader(Product &raw, QString &type, QString& dij, QString& publisher,
		QString& timerange, QString &maxtime);
	
	void addMergeProductScanTime(Product& merge);
	void addMergeProductForeCfg(Product& merge);
	void addProductSaveCached(Product &prod);
	void addMergeProductFileMerge(Product& merge);
	void addMergeProductTemplate(Product& merge);
	void addMergeProductMask(Product& merge);
	void addMergeProductDataSrc(Product& merge);

	void addRawProductScanTime(Product& raw);
	void addRawProductForeCfg(Product& raw);
	void addRawProductDataProc(Product& raw);

	inline bool isEqual(float a, float b) {
		return std::fabs(a - b) < std::numeric_limits<float>::epsilon();
	}

	void fillDataProcParamsWithLonLat();

	//station private
	void insertStationRow(int row, const QString& citytype,const QString &timerange);
	void getStationInfoByRow(int row, QString &citytype,
		QString &timerange);
	void addDataSrcItem(DataSrcItemList& list,QString& datatype, QString& srcfolder, QString& srcfileformat, QString& cityType, QString&timerange);


	//client private
	void insertClientRow(int row, const QString& id, const QString& name, const QString&username,
		const QString& password);
	void getClientInfoByRow(int row, QString& id, QString& name, QString&username,
		QString& password);
	void setClientInfoByRow(int row, const QString&username, const QString& password);
	
	
private:
	Ui::ServerConfigClass ui;
	ConfigParser* m_parser;
	DDSServerList m_ddsServerList;
	ClientList m_clientList;
	ProductList m_rawProductList;
	ProductList m_mergeProductList;
	ForeCastList m_forecastList;
	ItemList m_itemList;
	
	
	StationList m_rawStationlist, m_mergeStationlist;
	
	int m_stationCurrentRow;

	QStringList typeList;

	updateProductDlg* m_updateProductDlg;
	updateStatDlg * m_updateStatDlg;
	addProductDlg* m_addDlg;
	updateClientDlg *m_updateClientDlg;

	//hash table for type and publisher
	PublisherConfig::PublisherHas m_publisherHas;
	TypeConfig::TypeHas m_typeHas;
	DataTypeConfig::DataTypeHas m_datatypeHas;
	
	float m_lon1;
	float m_lon2;
	float m_lat1;
	float m_lat2;

};

#endif // SERVERCONFIG_H

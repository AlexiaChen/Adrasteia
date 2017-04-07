#include "ProductCfg.h"

#include <QDir>
#include <QDomNode>
#include <QDomText>
#include <QTextStream>

ProductCfg::ProductCfg(QString& path, QObject *parent)
	: cfgPath(path),QObject(parent), m_isInvalid(false)
{

}

ProductCfg::~ProductCfg()
{

}

bool ProductCfg::isInvalid() const
{
	return m_isInvalid;
}

void ProductCfg::saveToFile()
{
	m_xmlFile.resize(0);
	QTextStream out(&m_xmlFile);
	out.setCodec("UTF-8");
	
	int size = m_doc.firstChildElement("config").firstChildElement("products").childNodes().size();
	m_doc.save(out, 4, QDomNode::EncodingFromTextStream);

	m_xmlFile.close();
	m_isInvalid = true;

	openFile();
}

void ProductCfg::openFile()
{
	QString xmlPath = cfgPath + QString("/ProductConfig.xml");
	m_xmlFile.setFileName(xmlPath);

	if (!m_xmlFile.open(QFile::ReadWrite | QFile::Text)) {
		printf("open server config file failed\n");
		m_isInvalid = true;
	}

	QString         strError;
	int             errLin = 0, errCol = 0;
	if (!m_doc.setContent(&m_xmlFile, false, &strError, &errLin, &errCol)) {
		printf("parse file failed at line %d column %d, error: %s !\n", errLin, errCol, strError);
		m_isInvalid = true;
	}

	if (m_doc.isNull()) {
		printf("document is null !\n");
		m_isInvalid = true;
	}

	m_root = m_doc.documentElement();
	if (m_root.tagName().compare(QString("config"), Qt::CaseSensitive) != 0)
	{
		m_isInvalid = true;
	}
}

void ProductCfg::getProductList(ProductList &raw, ProductList& merge)
{
	QDomElement productElement = m_root.firstChildElement("products").firstChildElement("product");

	while (!productElement.isNull())
	{
		
		if (productElement.attribute("role") == "raw")
		{
			Product product;

			product.key = productElement.attribute("key");
			product.name = productElement.attribute("name");
			product.role = productElement.attribute("role");
			product.type = productElement.attribute("type");
			product.publisher = productElement.attribute("publisher");

			product.category = productElement.attribute("category");
			product.element = productElement.attribute("element");

			product.statistical = productElement.attribute("statistical");
			product.status = productElement.attribute("status");


			product.lon1 = productElement.attribute("lon1");
			product.lon2 = productElement.attribute("lon2");
			product.lat1 = productElement.attribute("lat1");
			product.lat2 = productElement.attribute("lat2");
			product.di = productElement.attribute("di");
			product.dj = productElement.attribute("dj");

			product.offset = productElement.attribute("offset");
			product.TimeRange = productElement.attribute("TimeRange");
			product.MaxForecastTime = productElement.attribute("MaxForecastTime");
			product.BusinessStatus = productElement.attribute("BusinessStatus");
			product.isWarning = productElement.attribute("IsWarning").toLower() == "yes";

			QDomElement scanTime = productElement.firstChildElement("scantime");
			product.scanTime.interval = scanTime.attribute("interval");

			product.srcfolder = productElement.firstChildElement("srcfolder").text();
			product.srcfileformat = productElement.firstChildElement("srcfileformat").text();


			QDomElement forecastElement = productElement.firstChildElement("forecastCfg").firstChildElement("forecast");
			while (!forecastElement.isNull())
			{
				ForeCast forecast_t;
				forecast_t.hourTime = forecastElement.attribute("hourtime");
				forecast_t.filecount = forecastElement.attribute("filecount");
				forecast_t.RangeTime = forecastElement.attribute("RangeTime");
				forecast_t.startTime = forecastElement.attribute("startForecast");
				forecast_t.endTime = forecastElement.attribute("endForecast");
				forecast_t.latestTime = forecastElement.attribute("latestTime");

				product.forecastCfg.forecastList.push_back(forecast_t);

				forecastElement = forecastElement.nextSiblingElement();
			}

			QDomElement cachedsave = productElement.firstChildElement("savecached");
			if (!cachedsave.isNull())
			{
				product.savecached.bDisable = cachedsave.attribute("disabled").toLower() == "yes";
				product.savecached.validtime = cachedsave.attribute("validtime");
			}
			else
			{
				product.savecached.bDisable = true;
			}

			QDomElement dataprocElement = productElement.firstChildElement("dataproc");
			QDomElement chainsElement = dataprocElement.firstChildElement("chains");
			while (!chainsElement.isNull())
			{
				Chains chains;
				chains.ID = chainsElement.attribute("id");
				chains.name = chainsElement.attribute("name");
				chains.bDisable = chainsElement.attribute("disabled").toLower() == "yes";
				chains.savepath = chainsElement.attribute("savepath");
				chains.filename = chainsElement.attribute("filename");
				chains.dds = chainsElement.attribute("dds");

				QDomElement chainElement = chainsElement.firstChildElement("chain");
				while (!chainElement.isNull())
				{
					Chain chain;
					chain.params = chainElement.attribute("params");
					chain.libName = chainElement.text();

					chains.chainList.push_back(chain);

					// 下一条数据处理dll
					chainElement = chainElement.nextSiblingElement();
				}

				product.dataProc.chainsList.push_back(chains);

				chainsElement = chainsElement.nextSiblingElement();
			}

			QDomElement grid2stat = productElement.firstChildElement("grid2station");
			if (!grid2stat.isNull())
			{
				product.isGridtoStat = true;
			}
			else
			{
				product.isGridtoStat = false;
			}



			raw.push_back(product);
		}
		else if (productElement.attribute("role") == "merge")
		{
			Product product;

			product.key = productElement.attribute("key");
			product.name = productElement.attribute("name");
			product.role = productElement.attribute("role");
			product.type = productElement.attribute("type");
			product.publisher = productElement.attribute("publisher");

			product.category = productElement.attribute("category");
			product.element = productElement.attribute("element");

			product.statistical = productElement.attribute("statistical");
			product.status = productElement.attribute("status");


			product.lon1 = productElement.attribute("lon1");
			product.lon2 = productElement.attribute("lon2");
			product.lat1 = productElement.attribute("lat1");
			product.lat2 = productElement.attribute("lat2");
			product.di = productElement.attribute("di");
			product.dj = productElement.attribute("dj");

			product.offset = productElement.attribute("offset");
			product.TimeRange = productElement.attribute("TimeRange");
			product.MaxForecastTime = productElement.attribute("MaxForecastTime");
			product.BusinessStatus = productElement.attribute("BusinessStatus");
			product.isWarning = productElement.attribute("IsWarning").toLower() == "yes";
			
			
			QDomElement scanTime = productElement.firstChildElement("scantime");
			product.scanTime.interval = scanTime.attribute("interval");

			QDomElement timeElement = scanTime.firstChildElement("time");
			while (!timeElement.isNull())
			{
				Time time;
				time.hour = timeElement.attribute("hour");
				time.min = timeElement.attribute("min");
				time.second = timeElement.attribute("second");

				product.scanTime.timeList.push_back(time);

				timeElement = timeElement.nextSiblingElement();
			}

			QDomElement forecastElement = productElement.firstChildElement("forecastCfg").firstChildElement("forecast");
			while (!forecastElement.isNull())
			{
				ForeCast forecast_t;
				forecast_t.hourTime = forecastElement.attribute("hourtime");
				forecast_t.filecount = forecastElement.attribute("filecount");
				forecast_t.RangeTime = forecastElement.attribute("RangeTime");
				forecast_t.startTime = forecastElement.attribute("startForecast");
				forecast_t.endTime = forecastElement.attribute("endForecast");
				forecast_t.latestTime = forecastElement.attribute("latestTime");

				product.forecastCfg.forecastList.push_back(forecast_t);

				forecastElement = forecastElement.nextSiblingElement();
			}

			QDomElement cachedsave = productElement.firstChildElement("savecached");
			if (!cachedsave.isNull())
			{
				product.savecached.bDisable = cachedsave.attribute("disabled").toLower() == "yes";
				product.savecached.validtime = cachedsave.attribute("validtime");
			}
			else
			{
				product.savecached.bDisable = true;
			}


			QDomElement filemergeElement = productElement.firstChildElement("filemerge");
			if (!filemergeElement.isNull())
			{
				product.fileMerge.bDisable = filemergeElement.attribute("disabled").toInt();
				product.fileMerge.savepath = filemergeElement.attribute("savepath");
				product.fileMerge.filename = filemergeElement.attribute("filename");
				product.fileMerge.dds = filemergeElement.attribute("dds");

			}
			else
			{
				product.fileMerge.bDisable = true;
			}





			product.relatedProduct = productElement.firstChildElement("relatedproduct").text();

			product.template_t.type = productElement.firstChildElement("template").attribute("type");
			product.template_t.value = productElement.firstChildElement("template").text();

			// MASK
			product.mask.filePath = productElement.firstChildElement("mask").text();
			product.mask.lon1 = productElement.firstChildElement("mask").attribute("lon1");
			product.mask.lon2 = productElement.firstChildElement("mask").attribute("lon2");
			product.mask.lat1 = productElement.firstChildElement("mask").attribute("lat1");
			product.mask.lat2 = productElement.firstChildElement("mask").attribute("lat2");
			product.mask.di = productElement.firstChildElement("mask").attribute("di");
			product.mask.dj = productElement.firstChildElement("mask").attribute("dj");



			QDomElement datasrcElemnt = productElement.firstChildElement("datasrc");
			QDomElement itemElement = datasrcElemnt.firstChildElement("item");
			while (!itemElement.isNull())
			{
				Item item_t;
				item_t.clientID = itemElement.attribute("clientid");
				item_t.lon1 = itemElement.attribute("lon1");
				item_t.lon2 = itemElement.attribute("lon2");
				item_t.lat1 = itemElement.attribute("lat1");
				item_t.lat2 = itemElement.attribute("lat2");
				item_t.srcFolder = itemElement.attribute("srcfolder");
				item_t.srcfileformat = itemElement.attribute("srcfileformat");

				product.dataSrc.itemList.push_back(item_t);

				itemElement = itemElement.nextSiblingElement();
			}

			QDomElement grid2stat = productElement.firstChildElement("grid2station");
			if (!grid2stat.isNull())
			{
				product.isGridtoStat = true;
			}
			else
			{
				product.isGridtoStat = false;
			}


			merge.push_back(product);
		}
		
		// next product
		productElement = productElement.nextSiblingElement();
	}

}

void ProductCfg::addRawToDomTree(ProductList& raw, QDomElement &productsEle)
{
	for (int i = 0; i < raw.size(); ++i)
	{
		
		QDomElement newProductEle = m_doc.createElement("product");

		
		// product header
		addProductHeader(newProductEle, raw.value(i));


		//扫描时间
		addProductScanTime(newProductEle, raw.value(i));
		
		// src foler and fileformat
		addProductSrc(newProductEle, raw.value(i));

		// add forecast Config
		addProductForeCastCfg(newProductEle, raw.value(i));
		
		//add data proc
		if (!raw.at(i).dataProc.chainsList.isEmpty())
		{
			addProductDataProc(newProductEle, raw.value(i));
		}
		
		//add save cached
		if (!raw.at(i).savecached.bDisable)
		{
			addSavedCached(newProductEle, raw.value(i));
		}
		

		//add grid to station
		if (raw.at(i).isGridtoStat)
		{
			addGridToStation(newProductEle, raw.value(i));
		}
		

		productsEle.appendChild(newProductEle);
	}
}

void ProductCfg::addMergeToDomTree(ProductList& merge, QDomElement &productsEle)
{
	for (int i = 0; i < merge.size(); ++i)
	{
		QDomElement newProductEle = m_doc.createElement("product");

		// product header
		addProductHeader(newProductEle, merge.value(i));

		//scan Time
		if (!merge.at(i).scanTime.timeList.isEmpty())
		{
			
			addProductScanTime(newProductEle, merge.value(i));
			
		}

		//add forecast cfg
		addProductForeCastCfg(newProductEle, merge.value(i));

		//add saved cached
		addSavedCached(newProductEle, merge.value(i));
		

		//add file merge
		if (!merge.at(i).fileMerge.bDisable)
		{
			addFileMerge(newProductEle, merge.value(i));
		}
        
		//add related product
		addRelatedProduct(newProductEle, merge.value(i));

		//add template
		addTemplate(newProductEle, merge.value(i));

		
		//add mask
		addMask(newProductEle, merge.value(i));

		
		//add data src
		if (!merge.at(i).dataSrc.itemList.isEmpty())
		{
			addDataSrc(newProductEle, merge.value(i));
		}

		//add grid to station
		if (merge.at(i).isGridtoStat)
		{
			addGridToStation(newProductEle, merge.value(i));
		}

		
		
		productsEle.appendChild(newProductEle);
	}
}

void  ProductCfg::addProductScanTime(QDomElement &productEle, Product & prod)
{
	QDomElement scantimeEle = m_doc.createElement("scantime");
	scantimeEle.setAttribute("interval", prod.scanTime.interval);
	
	for (int scantime_index = 0; scantime_index < prod.scanTime.timeList.size(); ++scantime_index)
	{
		QDomElement timeEle = m_doc.createElement("time");
		timeEle.setAttribute("hour", prod.scanTime.timeList.at(scantime_index).hour);
		timeEle.setAttribute("min", prod.scanTime.timeList.at(scantime_index).min);
		timeEle.setAttribute("second", prod.scanTime.timeList.at(scantime_index).second);
		scantimeEle.appendChild(timeEle);
	}
	
	productEle.appendChild(scantimeEle);

}

void  ProductCfg::addProductSrc(QDomElement &productEle, Product & prod)
{
	QDomElement srcfolderEle = m_doc.createElement("srcfolder");
	QDomText srcfolerText = m_doc.createTextNode(prod.srcfolder);
	srcfolderEle.appendChild(srcfolerText);
	productEle.appendChild(srcfolderEle);

	QDomElement srcfileformatEle = m_doc.createElement("srcfileformat");
	QDomText srcfileformatText = m_doc.createTextNode(prod.srcfileformat);
	srcfileformatEle.appendChild(srcfileformatText);
	productEle.appendChild(srcfileformatEle);
	
}

void ProductCfg::addProductForeCastCfg(QDomElement &productEle, Product & prod)
{
	QDomElement forecastCfgEle = m_doc.createElement("forecastCfg");

	for (int forecast_index = 0; forecast_index < prod.forecastCfg.forecastList.size(); ++forecast_index)
	{
		QDomElement forecastEle = m_doc.createElement("forecast");
		forecastEle.setAttribute("hourtime", prod.forecastCfg.forecastList.at(forecast_index).hourTime);
		forecastEle.setAttribute("RangeTime", prod.forecastCfg.forecastList.at(forecast_index).RangeTime);
		forecastEle.setAttribute("filecount", prod.forecastCfg.forecastList.at(forecast_index).filecount);
		forecastEle.setAttribute("startHour", prod.forecastCfg.forecastList.at(forecast_index).startTime);
		forecastEle.setAttribute("endHour", prod.forecastCfg.forecastList.at(forecast_index).endTime);
		forecastEle.setAttribute("latestTime", prod.forecastCfg.forecastList.at(forecast_index).latestTime);
		forecastCfgEle.appendChild(forecastEle);

	}
	productEle.appendChild(forecastCfgEle);
}


void ProductCfg::addProductDataProc(QDomElement &productEle, Product & prod)
{
	QDomElement dataprocEle = m_doc.createElement("dataproc");
	QDomElement chainsEle = m_doc.createElement("chains");
	chainsEle.setAttribute("id", prod.dataProc.chainsList.first().ID);
	chainsEle.setAttribute("name", prod.dataProc.chainsList.first().name);
	chainsEle.setAttribute("savepath", prod.dataProc.chainsList.first().savepath);
	chainsEle.setAttribute("filename", prod.dataProc.chainsList.first().filename);
	chainsEle.setAttribute("dds", prod.dataProc.chainsList.first().dds);

	for (int chain_index = 0; chain_index < prod.dataProc.chainsList.first().chainList.size(); ++chain_index)
	{
		QDomElement chainEle = m_doc.createElement("chain");
		chainEle.setAttribute("params", prod.dataProc.chainsList.first().chainList.at(chain_index).params);
		QDomText chainText = m_doc.createTextNode(prod.dataProc.chainsList.first().chainList.at(chain_index).libName);
		chainEle.appendChild(chainText);

		chainsEle.appendChild(chainEle);
	}

	dataprocEle.appendChild(chainsEle);

	productEle.appendChild(dataprocEle);
}


void ProductCfg::addSavedCached(QDomElement &productEle, Product & prod)
{
	QDomElement savecachedEle = m_doc.createElement("savecached");
	savecachedEle.setAttribute("validtime", prod.savecached.validtime);
	productEle.appendChild(savecachedEle);
}

void ProductCfg::addGridToStation(QDomElement &productEle, Product & prod)
{
	QDomElement grid2stateEle = m_doc.createElement("grid2station");
	QDomText text = m_doc.createTextNode("yes");
	grid2stateEle.appendChild(text);
	productEle.appendChild(grid2stateEle);
}

void ProductCfg::addFileMerge(QDomElement &productEle, Product & prod)
{
	QDomElement fielMergeEle = m_doc.createElement("filemerge");
	fielMergeEle.setAttribute("savepath", prod.fileMerge.savepath);
	fielMergeEle.setAttribute("filename", prod.fileMerge.filename);
	fielMergeEle.setAttribute("dds", prod.fileMerge.dds);

	productEle.appendChild(fielMergeEle);
}

void ProductCfg::addRelatedProduct(QDomElement &productEle, Product & prod)
{
	QDomElement relatedproductEle = m_doc.createElement("relatedproduct");
	QDomText relatedproductText = m_doc.createTextNode(prod.relatedProduct);
	relatedproductEle.appendChild(relatedproductText);
	productEle.appendChild(relatedproductEle);
}

void ProductCfg::addTemplate(QDomElement &productEle, Product & prod)
{
	QDomElement templateEle = m_doc.createElement("template");
	templateEle.setAttribute("type", prod.template_t.type);
	QDomText templateText = m_doc.createTextNode(prod.template_t.value);
	templateEle.appendChild(templateText);
	productEle.appendChild(templateEle);
}

void ProductCfg::addMask(QDomElement &productEle, Product & prod)
{
	QDomElement maskEle = m_doc.createElement("mask");
	maskEle.setAttribute("lon1", prod.mask.lon1);
	maskEle.setAttribute("lon2", prod.mask.lon2);
	maskEle.setAttribute("lat1", prod.mask.lat1);
	maskEle.setAttribute("lat2", prod.mask.lat2);
	maskEle.setAttribute("di", prod.mask.di);
	maskEle.setAttribute("dj", prod.mask.dj);
	QDomText maskText = m_doc.createTextNode(prod.mask.filePath);
	maskEle.appendChild(maskText);
	productEle.appendChild(maskEle);
}

void ProductCfg::addDataSrc(QDomElement &productEle, Product & prod)
{
	QDomElement datasrc = m_doc.createElement("datasrc");

	for (int item_index = 0; item_index < prod.dataSrc.itemList.size(); ++item_index)
	{
		QDomElement itemEle = m_doc.createElement("item");
		itemEle.setAttribute("clientid", prod.dataSrc.itemList.at(item_index).clientID);
		itemEle.setAttribute("lon1", prod.dataSrc.itemList.at(item_index).lon1);
		itemEle.setAttribute("lon2", prod.dataSrc.itemList.at(item_index).lon2);
		itemEle.setAttribute("lat1", prod.dataSrc.itemList.at(item_index).lat1);
		itemEle.setAttribute("lat2", prod.dataSrc.itemList.at(item_index).lat2);
		itemEle.setAttribute("srcfolder", prod.dataSrc.itemList.at(item_index).srcFolder);
		itemEle.setAttribute("srcfileformat", prod.dataSrc.itemList.at(item_index).srcfileformat);

		datasrc.appendChild(itemEle);
	}

	productEle.appendChild(datasrc);
}

void ProductCfg::addProductHeader(QDomElement &productEle, Product & prod)
{
	productEle.setAttribute("key", prod.key);
	productEle.setAttribute("name", prod.name);
	productEle.setAttribute("role", prod.role.toLower());
	productEle.setAttribute("type", prod.type);
	productEle.setAttribute("publisher", prod.publisher);
	productEle.setAttribute("category", prod.category);
	productEle.setAttribute("element", prod.element);
	productEle.setAttribute("statistical", prod.statistical);
	productEle.setAttribute("status", prod.status);
	productEle.setAttribute("lon1", prod.lon1);
	productEle.setAttribute("lon2", prod.lon2);
	productEle.setAttribute("lat1", prod.lat1);
	productEle.setAttribute("lat2", prod.lat2);
	productEle.setAttribute("di", prod.di);
	productEle.setAttribute("dj", prod.dj);
	productEle.setAttribute("offset", prod.offset);
	productEle.setAttribute("TimeRange", prod.TimeRange);
	productEle.setAttribute("MaxForecastTime", prod.MaxForecastTime);
	productEle.setAttribute("BusinessStatus", prod.BusinessStatus);
	productEle.setAttribute("IsWarning", prod.isWarning ? "yes" : "no");

}

void ProductCfg::setProductListNew(ProductList &raw, ProductList& merge)
{
	
	//删除Dom
	QDomElement cfgEle = m_doc.firstChildElement("config");
	m_doc.removeChild(cfgEle);

	cfgEle = m_doc.createElement("config");
	m_doc.appendChild(cfgEle);


	QDomElement productsElement = m_doc.createElement("products");
	QDomText productsText = m_doc.createTextNode("");
	productsElement.appendChild(productsText);
	cfgEle.appendChild(productsElement);
	
	
	//重新建立Dom Tree
	//先添加raw配置
	addRawToDomTree(raw, productsElement);

	//再添加merge配置
	addMergeToDomTree(merge, productsElement);

				
}

void ProductCfg::setProductList(ProductList &list)
{
	QDomElement productsElement = m_root.firstChildElement("products");

	if (!productsElement.isNull())
	{
		for (int i = 0; i < list.count(); ++i)
		{
			QDomElement productElement = productsElement.firstChildElement("product");

			while (!productElement.isNull())
			{

				if (list.at(i).key == productElement.attribute("key"))
				{
					productElement.setAttribute("name", list.at(i).name);

					if (list.at(i).role == "Raw")
					{
						productElement.setAttribute("role", "raw");
					}
					else if (list.at(i).role == "Merge")
					{
						productElement.setAttribute("role", "merge");
					}

					productElement.setAttribute("type", list.at(i).type);
					productElement.setAttribute("publisher", list.at(i).publisher);
					productElement.setAttribute("category", list.at(i).category);
					productElement.setAttribute("element", list.at(i).element);
					productElement.setAttribute("statistical", list.at(i).statistical);
					productElement.setAttribute("status", list.at(i).status);
					productElement.setAttribute("lon1", list.at(i).lon1);
					productElement.setAttribute("lon2", list.at(i).lon2);
					productElement.setAttribute("lat1", list.at(i).lat1);
					productElement.setAttribute("lat2", list.at(i).lat2);

					productElement.setAttribute("di", list.at(i).di);
					productElement.setAttribute("dj", list.at(i).dj);

					productElement.setAttribute("TimeRange", list.at(i).TimeRange);
					productElement.setAttribute("MaxForecastTime", list.at(i).MaxForecastTime);
					productElement.setAttribute("BusinessStatus", list.at(i).BusinessStatus);
					productElement.setAttribute("IsWarning", list.at(i).isWarning ? "yes" : "no");


					QDomElement srcFolderEle = productElement.firstChildElement("srcfolder");

					if (!srcFolderEle.isNull())
					{
						QDomElement newFolderEle = m_doc.createElement("srcfolder");
						QDomText newText = m_doc.createTextNode(list.at(i).srcfolder);

						newFolderEle.appendChild(newText);
						productElement.replaceChild(newFolderEle, srcFolderEle);

					}

					QDomElement srcfileformatEle = productElement.firstChildElement("srcfileformat");

					if (!srcfileformatEle.isNull())
					{
						QDomElement newFileFormatEle = m_doc.createElement("srcfileformatEle");
						QDomText newText = m_doc.createTextNode(list.at(i).srcfileformat);

						newFileFormatEle.appendChild(newText);
						productElement.replaceChild(newFileFormatEle, srcfileformatEle);

					}


					QDomElement scanTimeEle = productElement.firstChildElement("scantime");

					if (!scanTimeEle.isNull())
					{
						scanTimeEle.setAttribute("interval", list.at(i).scanTime.interval);

						//扫描时间列表，需要之后修正
						//to do
					}

					QDomElement forecastCfgEle = productElement.firstChildElement("forecastCfg");

					if (!forecastCfgEle.isNull())
					{
						QDomElement forecastEle = forecastCfgEle.firstChildElement("forecast");

						while (!forecastEle.isNull())
						{
							if (forecastEle.attribute("hourtime") == list.at(i).forecastCfg.forecastList.front().hourTime)
							{
								forecastEle.setAttribute("hourtime", list.at(i).forecastCfg.forecastList.front().hourTime);
								forecastEle.setAttribute("filecount", list.at(i).forecastCfg.forecastList.front().filecount);
								forecastEle.setAttribute("RangeTime", list.at(i).forecastCfg.forecastList.front().RangeTime);
								forecastEle.setAttribute("starttime", list.at(i).forecastCfg.forecastList.front().startTime);
								forecastEle.setAttribute("endtime", list.at(i).forecastCfg.forecastList.front().endTime);
								forecastEle.setAttribute("latestTime", list.at(i).forecastCfg.forecastList.front().latestTime);
							}

							forecastEle = forecastEle.nextSiblingElement();
						}
					}

					QDomElement savecachedEle = productElement.firstChildElement("savecached");

					if (!savecachedEle.isNull())
					{
						savecachedEle.setAttribute("validtime", list.at(i).savecached.validtime);
					}


					if (list.at(i).role == "Merge")
					{
						QDomElement savefileEle = productElement.firstChildElement("savefile");
						if (!savefileEle.isNull())
						{
							savefileEle.setAttribute("savepath", list.at(i).savefile.savepath);
							savefileEle.setAttribute("filename", list.at(i).savefile.filename);
							savefileEle.setAttribute("dds", list.at(i).savefile.dds);

						}
						else
						{
							QDomElement newsaveFileEle = m_doc.createElement("savefile");
							productElement.appendChild(newsaveFileEle);

							newsaveFileEle.setAttribute("savepath", list.at(i).savefile.savepath);
							newsaveFileEle.setAttribute("filename", list.at(i).savefile.filename);
							newsaveFileEle.setAttribute("dds", list.at(i).savefile.dds);
						}


						QDomElement relatedProductEle = productElement.firstChildElement("relatedproduct");
						if (!relatedProductEle.isNull())
						{
							QDomElement newRelatedProductEle = m_doc.createElement("relatedproduct");
							QDomText newText = m_doc.createTextNode(list.at(i).relatedProduct);

							newRelatedProductEle.appendChild(newText);


							productElement.replaceChild(newRelatedProductEle, relatedProductEle);
						}
						else
						{
							QDomElement newRelatedProductEle = m_doc.createElement("relatedproduct");
							QDomText newText = m_doc.createTextNode(list.at(i).relatedProduct);
							newRelatedProductEle.appendChild(newText);
							productElement.appendChild(newRelatedProductEle);

						}

						QDomElement templateEle = productElement.firstChildElement("template");
						if (!templateEle.isNull())
						{
							QDomElement newtemplateEle = m_doc.createElement("template");
							newtemplateEle.setAttribute("type", list.at(i).template_t.type);
							
							QDomText newText = m_doc.createTextNode(list.at(i).template_t.value);

							newtemplateEle.appendChild(newText);


							productElement.replaceChild(newtemplateEle, templateEle);
						}
						else
						{
							QDomElement newtemplateEle = m_doc.createElement("template");
							newtemplateEle.setAttribute("type", list.at(i).template_t.type);

							QDomText newText = m_doc.createTextNode(list.at(i).template_t.value);

							newtemplateEle.appendChild(newText);
							productElement.appendChild(newtemplateEle);

						}

						QDomElement maskEle = productElement.firstChildElement("mask");
						if (!maskEle.isNull())
						{
							QDomElement newMaskEle = m_doc.createElement("mask");
							newMaskEle.setAttribute("lon1", list.at(i).mask.lon1);
							newMaskEle.setAttribute("lon2", list.at(i).mask.lon2);
							newMaskEle.setAttribute("lat1", list.at(i).mask.lat1);
							newMaskEle.setAttribute("lat2", list.at(i).mask.lat2);
							newMaskEle.setAttribute("di", list.at(i).mask.di);
							newMaskEle.setAttribute("dj", list.at(i).mask.dj);

							QDomText newText = m_doc.createTextNode(list.at(i).mask.filePath);

							newMaskEle.appendChild(newText);


							productElement.replaceChild(newMaskEle, maskEle);
						}
						else
						{
							QDomElement newMaskEle = m_doc.createElement("mask");
							newMaskEle.setAttribute("lon1", list.at(i).mask.lon1);
							newMaskEle.setAttribute("lon2", list.at(i).mask.lon2);
							newMaskEle.setAttribute("lat1", list.at(i).mask.lat1);
							newMaskEle.setAttribute("lat2", list.at(i).mask.lat2);
							newMaskEle.setAttribute("di", list.at(i).mask.di);
							newMaskEle.setAttribute("dj", list.at(i).mask.dj);

							QDomText newText = m_doc.createTextNode(list.at(i).mask.filePath);

							newMaskEle.appendChild(newText);
							productElement.appendChild(newMaskEle);

						}


						QDomElement dataSrcEle = productElement.firstChildElement("datasrc");
						if (!dataSrcEle.isNull())
						{
							QDomElement itemEle = dataSrcEle.firstChildElement("item");

							while (!itemEle.isNull())
							{
								if (list.at(i).dataSrc.itemList.front().clientID == itemEle.attribute("clientid"))
								{
									itemEle.setAttribute("lon1", list.at(i).dataSrc.itemList.front().lon1);
									itemEle.setAttribute("lon2", list.at(i).dataSrc.itemList.front().lon2);
									itemEle.setAttribute("lat1", list.at(i).dataSrc.itemList.front().lat1);
									itemEle.setAttribute("lat2", list.at(i).dataSrc.itemList.front().lat2);
									itemEle.setAttribute("srcfolder", list.at(i).dataSrc.itemList.front().srcFolder);
									itemEle.setAttribute("srcfileformat", list.at(i).dataSrc.itemList.front().srcfileformat);
								}
								
								itemEle = itemEle.nextSiblingElement();
							}
						}
						else
						{
							
							QDomElement newDataSrcEle = m_doc.createElement("datasrc");

							QDomElement  newItemEle = m_doc.createElement("item");

							newItemEle.setAttribute("lon1", list.at(i).dataSrc.itemList.front().lon1);
							newItemEle.setAttribute("lon2", list.at(i).dataSrc.itemList.front().lon2);
							newItemEle.setAttribute("lat1", list.at(i).dataSrc.itemList.front().lat1);
							newItemEle.setAttribute("lat2", list.at(i).dataSrc.itemList.front().lat2);
							newItemEle.setAttribute("srcfolder", list.at(i).dataSrc.itemList.front().srcFolder);
							newItemEle.setAttribute("srcfileformat", list.at(i).dataSrc.itemList.front().srcfileformat);

							newDataSrcEle.appendChild(newItemEle);

							productElement.appendChild(newDataSrcEle);

						}

					}


				}



				productElement = productElement.nextSiblingElement();
			}
		}
	}

	
}

#include "JSON.h"
#include "cJSON.h"
#include "stdio.h"
#include "string.h"

/************************************************************************/
/* 创建产品信息JSON数据                                                 */
/************************************************************************/
char * JSON::CreateProductInfoJson(ST_PRODUCT stProduct)
{
	return NULL;
}

/************************************************************************/
/* 解析产品信息JSON数据                                                 */
/************************************************************************/
bool JSON::ParserProductInfo(char * strProductInfo, ST_PRODUCT & stProduct)
{
	cJSON *json;

	// 解析JSON数据
	json = cJSON_Parse(strProductInfo);

	if (!json) 
	{
		// 解析错误
		printf("Error before: [%s]\n", cJSON_GetErrorPtr());
		return false;
	}

	cJSON * jsonValue = json;
	while (jsonValue)
	{
		switch ((jsonValue->type) & 0xff)
		{
		case cJSON_NULL:			break;
		case cJSON_False:			break;
		case cJSON_True:			break;
		case cJSON_Number:			break;
		case cJSON_String:			break;
		case cJSON_Array:			break;
		case cJSON_Object:
			jsonValue = jsonValue->child;
			continue;

			break;
		}

		//  解析数据
		if (strcmp(jsonValue->string, "Name") == 0)
		{
			stProduct.strName = JSTR(jsonValue->valuestring);
		}
		else if (strcmp(jsonValue->string, "Role") == 0)
		{
			stProduct.strRole = JSTR(jsonValue->valuestring);
		}
		else if(strcmp(jsonValue->string, "Type") == 0)
		{
			stProduct.strType = JSTR(jsonValue->valuestring);
		}
		else if (strcmp(jsonValue->string, "Publisher") == 0)
		{
			stProduct.strPublisher = JSTR(jsonValue->valuestring);
		}
		else if (strcmp(jsonValue->string, "Category") == 0)
		{
			stProduct.nCategory = atoi(jsonValue->valuestring);
		}
		else if (strcmp(jsonValue->string, "Element") == 0)
		{
			stProduct.nElement[0] = -1;
			stProduct.nElement[1] = -1;

			string strElement = string(jsonValue->valuestring);
			StringList lstElement;
			split(lstElement, strElement, ",");
			StringList::iterator iter;
			int i = 0;
			for (iter = lstElement.begin(), i = 0; iter != lstElement.end(); iter++, i++)
			{
				if (i >= 2)
				{
					break;
				}
				strElement = *iter;
				stProduct.nElement[i] = atoi(strElement.data());
			}
		}
		else if (strcmp(jsonValue->string, "Statistical") == 0)
		{
			stProduct.nStatistical = atoi(jsonValue->valuestring);
		}
		else if (strcmp(jsonValue->string, "Status") == 0)
		{
			stProduct.nStatus = atoi(jsonValue->valuestring);
		}
		else if (strcmp(jsonValue->string, "OffSet") == 0)
		{
			stProduct.fOffSet = atof(jsonValue->valuestring);
		}
		else if (strcmp(jsonValue->string, "Lon1") == 0)
		{
			stProduct.fLon1 = atof(jsonValue->valuestring);
		}
		else if (strcmp(jsonValue->string, "Lon2") == 0)
		{
			stProduct.fLon2 = atof(jsonValue->valuestring);
		}
		else if (strcmp(jsonValue->string, "Lat1") == 0)
		{
			stProduct.fLat1 = atof(jsonValue->valuestring);
		}
		else if (strcmp(jsonValue->string, "Lat2") == 0)
		{
			stProduct.fLat2 = atof(jsonValue->valuestring);
		}
		else if (strcmp(jsonValue->string, "Di") == 0)
		{
			stProduct.fDi = atof(jsonValue->valuestring);
		}
		else if (strcmp(jsonValue->string, "Dj") == 0)
		{
			stProduct.fDj = atof(jsonValue->valuestring);
		}
		else if (strcmp(jsonValue->string, "TimeRange") == 0)
		{
			stProduct.nTimeRange = atoi(jsonValue->valuestring);
		}
		else if (strcmp(jsonValue->string, "MaxForecastTime") == 0)
		{
			stProduct.nMaxForecastTime = atoi(jsonValue->valuestring);
		}
		else if (strcmp(jsonValue->string, "BusinessStatus") == 0)
		{
			stProduct.strBusinessStatus = JSTR(jsonValue->valuestring);
		}
		else if (strcmp(jsonValue->string, "Warning") == 0)
		{
			stProduct.bIsWarning = (string(jsonValue->valuestring) == "yes");
		}

		// 移向下一个
		jsonValue = jsonValue->next;
	}

	// 删除JSON格式化数据
	cJSON_Delete(json);

	return true;
}


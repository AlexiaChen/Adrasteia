#ifndef HTTP_DEF_H
#define HTTP_DEF_H

#include <string>
#include <vector>
#include "ProductDef.h"

typedef struct _Http
{
	std::string url;
	std::string savePath;
	bool  bDisabled;
	ST_SCANTIME scanTime;
	std::vector<std::string> reportTimes;
}ST_HTTP;




#endif
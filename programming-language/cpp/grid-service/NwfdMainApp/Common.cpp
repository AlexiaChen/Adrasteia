#include "Common.h"

/************************************************************************/
/* 分割字符串                                                           */
/************************************************************************/
void split(StringList& stringlist, const string& source, const string& separator)
{
	int start = 0;
	int end = source.find_first_of(separator);

	while (end != string::npos)
	{
		stringlist.push_back(source.substr(start, end - start));
		start = end + 1;
		end = source.find(separator, start);
	}

	stringlist.push_back(source.substr(start, source.length() - start));
}

/************************************************************************/
/* 从字符串中获取数据的方法                                             */
/************************************************************************/
string getdata(const string &str, const string &separator, string::size_type &nOff)
{
	string substring;
	string::size_type  index;

	index = str.find_first_of(separator, nOff);
	if (index != string::npos)
	{
		substring = str.substr(nOff, index - nOff);

		nOff = str.find_first_not_of(separator, index);
		//if (nOff == string::npos)
		//{
		//	return "";
		//}
	}
	else
	{
		substring = str.substr(nOff);
		//return "";
	}

	return substring;
}

/************************************************************************/
/* 字符流转换                                                           */
/************************************************************************/
void RxF4(float &v, unsigned char *pBuf, int &nOff)
{
	memcpy(&v, pBuf + nOff, 4);
	nOff += 4;
}

void TxF4(float v, unsigned char *pBuf, int &nOff)
{
	memcpy(pBuf + nOff, &v, 4);
	nOff += 4;
}

void RxI2(unsigned short &v, unsigned char *pBuf, int &nOff)
{
	v = pBuf[nOff++];
	v = (v << 8) | pBuf[nOff++];
}

void TxI2(short v, unsigned char *pBuf, int &nOff)
{
	pBuf[nOff++] = (v >> 8) & 0xff;
	pBuf[nOff++] = v & 0xff;
}

void RxI4(int &v, unsigned char *pBuf, int &nOff)
{
	v = pBuf[nOff++];
	v = (v << 8) | pBuf[nOff++];
	v = (v << 8) | pBuf[nOff++];
	v = (v << 8) | pBuf[nOff++];
}

void TxI4(int v, unsigned char *pBuf, int &nOff)
{
	pBuf[nOff++] = (v >> 24) & 0xff;
	pBuf[nOff++] = (v >> 16) & 0xff;
	pBuf[nOff++] = (v >> 8) & 0xff;
	pBuf[nOff++] = v & 0xff;
}



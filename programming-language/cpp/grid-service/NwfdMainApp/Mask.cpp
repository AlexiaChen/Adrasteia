#include "log.h"
#include "Mask.h"
#include "HandleNwfdLib.h"

/************************************************************************/
/* 构造函数                                                             */
/************************************************************************/
CMask::CMask()
{
}

/************************************************************************/
/* 析构函数                                                             */
/************************************************************************/
CMask::~CMask()
{
	ClearMaskData();
}

/************************************************************************/
/* 单例                                                                 */
/************************************************************************/
CMask & CMask::getClass()
{
	static CMask objMask;
	return objMask;
}

/************************************************************************/
/* 添加Mask信息                                                         */
/************************************************************************/
bool CMask::AddMaskInfo(ST_MASK stMask)
{
	if (m_hasMaskInfo.contains(stMask.strMaskFile))
	{
		return true;
	}

	// 添加到列表中
	m_hasMaskInfo.insert(stMask.strMaskFile, stMask);

	return true;
}

/************************************************************************/
/* 获取Mask数据                                                         */
/************************************************************************/
float * CMask::GetMaskData(QString strMaskFile)
{
	if (!m_hasMaskData.contains(strMaskFile))
	{
		// 没有此数据，或者构建此数据失败
		return NULL;
	}

	return m_hasMaskData.value(strMaskFile);
}

/************************************************************************/
/* 构建Mask数据                                                         */
/************************************************************************/
bool CMask::InitMaskData()
{
	// 先清空原先的数据
	ClearMaskData();

	// 遍历构建数据
	HASH_MASK_INFO::iterator iter;
	for (iter = m_hasMaskInfo.begin(); iter != m_hasMaskInfo.end(); iter++)
	{
		ST_MASK stMask = iter.value();
		// 通过Mask文件名读取数据
		float * fMaskData = ReadMaskDataFromFile(stMask);
		if (fMaskData != NULL)
		{
			m_hasMaskData.insert(stMask.strMaskFile, fMaskData);
		}
	}

	return true;
}

/************************************************************************/
/* 从Mask文件中读取数据                                                 */
/************************************************************************/
float * CMask::ReadMaskDataFromFile(ST_MASK stMask)
{
	// 调用 HandleNwfdLib的方法，获取micaps4数据
	grdpts stGrid;
	float * fMaskData = HandleNwfdLib::getClass().nwfd_openm4file(stMask.strMaskFile.toLocal8Bit().data(), &stGrid);
	if (fMaskData == NULL)
	{
		// todo获取数据失败
		LOG_(LOGID_ERROR, LOG_F("[Mask]Mask文件%s读取失败,将导致拼图无法正确执行."), LOG_STR(stMask.strMaskFile));
	}

	return fMaskData;
}

/************************************************************************/
/* 从Mask文件中读取数据                                                 */
/************************************************************************/
void CMask::ClearMaskData()
{
	HASH_MASK_DATA::iterator iter;
	for (iter = m_hasMaskData.begin(); iter != m_hasMaskData.end(); iter++)
	{
		float * fMaskData = iter.value();
		if (fMaskData)
		{
			HandleNwfdLib::getClass().nwfd_freefld(fMaskData);
			fMaskData = NULL;
		}
	}

	m_hasMaskData.clear();
}
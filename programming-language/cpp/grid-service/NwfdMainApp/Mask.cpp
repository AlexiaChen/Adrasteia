#include "log.h"
#include "Mask.h"
#include "HandleNwfdLib.h"

/************************************************************************/
/* ���캯��                                                             */
/************************************************************************/
CMask::CMask()
{
}

/************************************************************************/
/* ��������                                                             */
/************************************************************************/
CMask::~CMask()
{
	ClearMaskData();
}

/************************************************************************/
/* ����                                                                 */
/************************************************************************/
CMask & CMask::getClass()
{
	static CMask objMask;
	return objMask;
}

/************************************************************************/
/* ���Mask��Ϣ                                                         */
/************************************************************************/
bool CMask::AddMaskInfo(ST_MASK stMask)
{
	if (m_hasMaskInfo.contains(stMask.strMaskFile))
	{
		return true;
	}

	// ��ӵ��б���
	m_hasMaskInfo.insert(stMask.strMaskFile, stMask);

	return true;
}

/************************************************************************/
/* ��ȡMask����                                                         */
/************************************************************************/
float * CMask::GetMaskData(QString strMaskFile)
{
	if (!m_hasMaskData.contains(strMaskFile))
	{
		// û�д����ݣ����߹���������ʧ��
		return NULL;
	}

	return m_hasMaskData.value(strMaskFile);
}

/************************************************************************/
/* ����Mask����                                                         */
/************************************************************************/
bool CMask::InitMaskData()
{
	// �����ԭ�ȵ�����
	ClearMaskData();

	// ������������
	HASH_MASK_INFO::iterator iter;
	for (iter = m_hasMaskInfo.begin(); iter != m_hasMaskInfo.end(); iter++)
	{
		ST_MASK stMask = iter.value();
		// ͨ��Mask�ļ�����ȡ����
		float * fMaskData = ReadMaskDataFromFile(stMask);
		if (fMaskData != NULL)
		{
			m_hasMaskData.insert(stMask.strMaskFile, fMaskData);
		}
	}

	return true;
}

/************************************************************************/
/* ��Mask�ļ��ж�ȡ����                                                 */
/************************************************************************/
float * CMask::ReadMaskDataFromFile(ST_MASK stMask)
{
	// ���� HandleNwfdLib�ķ�������ȡmicaps4����
	grdpts stGrid;
	float * fMaskData = HandleNwfdLib::getClass().nwfd_openm4file(stMask.strMaskFile.toLocal8Bit().data(), &stGrid);
	if (fMaskData == NULL)
	{
		// todo��ȡ����ʧ��
		LOG_(LOGID_ERROR, LOG_F("[Mask]Mask�ļ�%s��ȡʧ��,������ƴͼ�޷���ȷִ��."), LOG_STR(stMask.strMaskFile));
	}

	return fMaskData;
}

/************************************************************************/
/* ��Mask�ļ��ж�ȡ����                                                 */
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
#include "DDSBase.h"

CDDSBase::CDDSBase()
{
	m_lstJob = NULL;
}

CDDSBase::~CDDSBase()
{

}

void CDDSBase::SetJobList(LIST_JOB  *lstJob)
{
	m_lstJob = lstJob;

	m_bComplete = false;
}

bool CDDSBase::IsComplete()
{
	return m_bComplete;
}


#include "DDSBase.h"

#include "HTTPSender.h"

class CDDSHttp : public CDDSBase{

	Q_OBJECT

public:
	CDDSHttp();
	virtual ~CDDSHttp();

protected:
	virtual void run();

private:
	bool m_bReady;
	ST_DDS   m_stDDS;    // ¥´ ‰≈‰÷√
	HttpSender* m_sender;
};
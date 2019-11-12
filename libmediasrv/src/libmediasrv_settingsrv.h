


#if !defined(_libmediasrv_settingsrv_h_)
#define _libmediasrv_settingsrv_h_

typedef enum tagStreamStatus
{
	StreamStatusStoped,
	StreamStatusRunning,
}StreamStatus;
class  CSettingSrvEvent :
	public CObjEvent
{

public:
	_CPP_UTIL_DYNAMIC(CSettingSrvEvent)
	_CPP_UTIL_CLASSNAME(CSettingSrvEvent);
	_CPP_UTIL_QUERYOBJ(CObjEvent);


public:
	CSettingSrvEvent();
	virtual ~CSettingSrvEvent();
	virtual int OnExecute(CObj *pThreadContext = NULL);
	virtual BOOL Post(BOOL bNotify = TRUE);

public:
	char m_szId[_MAX_SESSION];
	BOOL m_bSysId;
	int m_status;
	int m_hlsNameType;
};



#endif





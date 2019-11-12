


#include "libmediasrv_def.h"


#define _TAG_NAME_SETSRV "settingsrv"

CSettingSrvEvent::CSettingSrvEvent()
{
	m_szId[0] = 0 ;
	m_bSysId = FALSE ;
	m_status = 0 ;
	m_hlsNameType = 0;
#if defined(_HLS_SUPPORT_)
	m_hlsNameType = _AppConfig.m_hlsCfg.nameType;
#endif
}
CSettingSrvEvent::~CSettingSrvEvent()
{

}
int CSettingSrvEvent::OnExecute(CObj *pThreadContext )
{
	if (m_status == StreamStatusStoped)
	{
		if (IsExistMdSource(NULL,m_szId))
		{
			return 0;
		}
	}
	UpdateStreamSettingStatus(m_szId, NULL, m_bSysId, m_status, m_hlsNameType);
	return 0;
}
BOOL CSettingSrvEvent::Post(BOOL bNotify )
{
	return _MediasrvApp->m_queueSettingSrvNotify->PostEvent(this, bNotify);
}
BOOL UpdateHlsNameType2StreamSetting( const char *szIp,int hlsNameType)
{
	
	return  TRUE;
}

BOOL UpdateStreamSettingStatus(const char *szId, const char *szIp, BOOL bSysId, int status, int hlsNameType)
{
	

	return  TRUE;
}

BOOL GetStreamSetting(CStreamSetting &setting, const char *szId, const char *szIp, BOOL bSysId,const char *szUsr ,const char *szPwd)
{
	
	return TRUE;


}






































































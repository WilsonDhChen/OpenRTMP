
#ifndef _libmediasrv_session_h_
#define _libmediasrv_session_h_




void SessionInit();

CMdSource *GetMdSource(const char *szApp, const char *urlSuffix);
BOOL RemoveMdSource(const char *szApp, const char *urlSuffix, CMdSource * mdsrc);
BOOL AddMdSource(const char *szApp, const char *urlSuffix, CMdSource * mdsrc);
BOOL IsExistMdSource(const char *szApp, const char *urlSuffix);
BOOL RemoveMdSourceClose(const char *szApp, const char *urlSuffix, BOOL removeByPlugin = FALSE);

#if defined(_RTMP_SUPPORT_)
CRTMPNetSession *GetRTMPNetSession(const char *szId);
void RemoveRTMPNetSession(const char *szId ,  CRTMPNetSession * ss);
void AddRTMPNetSession(const char *szId ,  CRTMPNetSession * ss);
#endif

#endif













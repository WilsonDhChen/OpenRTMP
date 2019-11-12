
#if !defined(_PROXY_PLUGIN_H_)

#define _PROXY_PLUGIN_H_     1


#define _FILTER_ALLOW_			0
#define _FILTER_FORBID_			1
#define _FILTER_CALL_NEXT_		2

#define _PROXY_USERTYPE_SKS5		0
#define _PROXY_USERTYPE_SKS4		1
#define _PROXY_USERTYPE_HTTP		2

#define _SESSION_TYPE_SOCKS5		0
#define _SESSION_TYPE_SOCKS4		1
#define _SESSION_TYPE_UDP			2
#define _SESSION_TYPE_BIND			3
#define _SESSION_TYPE_HTTP			4
#define _SESSION_TYPE_HTTPS			5

#define _MONITOR_TYPE_C2S			0
#define _MONITOR_TYPE_S2C			1
/***************User verify plugin************/

/****************filter plugin****************/
typedef int ( *_ProxyInit)();
typedef int ( *_ProxyFilter)(const char *szSrcIp,int nSrcPort,
								  const char *szDstIp,int nDstPort,
								  const char *szUser,const char *szPass,
								  int nSessionType,const  char *szUrl);
typedef int ( *_ProxyFilterClose)(const char *szSrcIp,int nSrcPort,
								  const char *szDstIp,int nDstPort,
								  const char *szUser,const char *szPass,
								  int nSessionType);
/****************monitor plugin****************/
typedef int ( *_ProxyMonitorStart)(void ** pvHandle,SOCKET socClient,SOCKET socRemote);
typedef int ( *_ProxyMonitorEnd)(void * vHandle);
typedef int ( *_ProxyMonitorData)(void * vHandle,void *pData,int nLenData,int nMonitorType);
#endif


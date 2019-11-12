
#ifndef _libmediasrv_app_h_
#define _libmediasrv_app_h_ 


#define _MAX_NETWORKS 20


class CAppLogLock
{
public:
    CAppLogLock();
    ~CAppLogLock();
};

class  CMediaSrvApp :
	public CObj
{
	_CPP_UTIL_DYNAMIC(CMediaSrvApp)  ;
	_CPP_UTIL_CLASSNAME(CMediaSrvApp) ;
	_CPP_UTIL_QUERYOBJ(CObj) ;
public:
	CMediaSrvApp();
	virtual ~CMediaSrvApp();
	void ReloadSetting();
    void CleanUp();
	virtual int  Run();
	virtual void Stop();
public:
	BOOL StartNetworkAndQueue();
	void StopNetworkAndQueue();
	void ParseCmdline(const char *szCmdline);


	CObjNetAsync *GetCdnNet();

    int GetRtmpTotalConnections();
	int GetRtmpConnections();
	int GetEtsConnections();
	int GetHttpConnections();
	int GetRtspTsConnections();
    int GetOutTotalConnections();

	void LoadETSNetwork();
	void LoadETSNetworkTcpServer();

	void LoadRTMPNetwork();
	void LoadRTMPNetworkTcpServer();

	void LoadHTTPDNetwork();
	void LoadHTTPDNetworkTcpServer();

	void LoadHTTPTSNetwork();
	void LoadHTTPTSNetworkTcpServer();

	void LoadRTSPNetwork();
	void LoadRTSPNetworkTcpServer();

public:
	

//////////////RTMP//////////////////////
#if defined(_RTMP_SUPPORT_)
#if defined(_USE_TCPSERVER)
	CObjTcpServerRTMP *m_rtmpServer[_MAX_NETWORKS];
#else
	CRTMPNetworkSrv *m_rtmpNetwork[_MAX_NETWORKS];
#endif
	CRTMPNetworkSrv *m_rtmpLocal;
	CObjEventQueue  *m_queueNetworkEventRTMP;
#endif
	int m_rtmpCount;



//////////////ETS//////////////////////
#if defined(_ETS_SUPPORT_)
#if defined(_USE_TCPSERVER)
	CObjTcpServerETS *m_etsServer[_MAX_NETWORKS];
#else
	CETSNetwork *m_etsNetwork[_MAX_NETWORKS];
#endif
	CObjEventQueue  *m_queueNetworkEventETS;
#endif
	int m_etsCount;

//////////////HTTPD HLS//////////////////////
#if defined(_ENABLE_HTTPD_)
#if defined(_USE_TCPSERVER)
	CObjTcpServerHttpd *m_httpdServer[_MAX_NETWORKS];
#else
    CHttpdNetworkSrv *m_httpdNetwork[ _MAX_NETWORKS ];
#endif
	CObjEventQueue *m_queueHttpd;
#endif
	int m_httpdCount;


//////////////HTTP-TS//////////////////////
#if defined(_HTTPTS_SUPPORT_)

#if defined(_USE_TCPSERVER)
	CObjTcpServerHttpTs *m_httpTsServer[_MAX_NETWORKS];
#else
	CHttpTsNetworkSrv *m_httptsNetwork[ _MAX_NETWORKS ];
#endif

	CObjEventQueue *m_queueHttpTs;
#endif
	int m_httptsCount;

//////////////RTSP//////////////////////
#if defined(_RTSP_SUPPORT_)
	CRtspUDP m_udp[_MAX_TRACKS_];
#if defined(_USE_TCPSERVER)
	CObjTcpServerRTSP *m_rtspTsServer[_MAX_NETWORKS];
#else
	CRTSPNetworkSrv  *m_rtsp_tsNetwork[_MAX_NETWORKS];
#endif
	CObjEventQueue  *m_queueNetworkEventRTSP_TS;
#endif
	int m_rtspCount;

    

	CObjEventQueue *m_queueDiskIo;
    
	CObjEventQueue *m_queueDataProcess;

	CObjEventQueue *m_queueSettingSrvNotify;



	CObjEventQueue *m_queueNetworkClose;



#if defined(_MDSOURCE_MT)
	CObjEventQueue *m_queueSendThread;
#endif

	
	CCallbackProcessor *m_queueCallback;
	

	

#if defined(_CDNNET_SUPPORT_)
    BOOL SendRequestStreamSource(const char *szApp, const char *szStreamId);
    void CopyNetInterfaces(CObjArray *interfaces);
    void AddCdnCtx(CCdnNetContext   *ctx);
    void RemoveCdnCtx(CCdnNetContext   *ctx);
    CObjArray m_cdnnetInterfaces;
    CCdnNetwork *m_cdnNetwork;
    MTX_OBJ m_mtxNetInterfaces;
    MTX_OBJ m_mtxCdnCtxs;
    CObj m_cdnCtxs;
#endif

};



#endif


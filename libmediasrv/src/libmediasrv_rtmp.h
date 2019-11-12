 

#ifndef    _libmediasrv_rtmp_h_
#define   _libmediasrv_rtmp_h_

#if defined(_RTMP_SUPPORT_)

#define RTMP_AUDIO_CHANNEL 0x14 
#define RTMP_VIDEO_CHANNEL 0x15 
#define RTMP_METADATA_CHANNEL 0x03

typedef struct tagRTMPCFG
{
		BOOL enable ;
		BOOL enable_rtmp ;
		BOOL enable_rtmpt ;
		char bind_addr[80] ;
		char bind_addr6[80];
		int  port ;

		int  chunksize;
		BOOL publish ;
		BOOL playback ;
		bool force_playback;
		char app[80];

		BOOL force_hls_setting;
		BOOL output_hls;
        BOOL memory_file;
		char hls_dir[400];
		char hls_ts_prefix[400];

		BOOL record_ts;
		char record_ts_dir[400];
		BOOL record_mp4;
		char record_mp4_dir[400];
		BOOL record_flv;
		char record_flv_dir[400];

		BOOL use_setting_srv;
		BOOL exclude_connections;
		BOOL enable_virtual_live;
		int  network_threads;

}RTMPCFG;

class CMdSource;
class CETS2RTMPVar
{
public:
	CETS2RTMPVar();
	CETS2RTMPVar(CMdSource *mdsrc, INT64 tick, int  cts , RTMPPacketType pktType);
	virtual ~CETS2RTMPVar();
	void SetPar(CMdSource *mdsrc, INT64 tick, int  cts, RTMPPacketType pktType);
	BOOL CreateRtmpH264MetaData();
	BOOL ETSCreateH264IoBuffers(UINT8 *buf ,int bodysize );
	CObjNetIOBufferSharedMemory *CopySpsPps();
	CObjNetIOBufferSharedMemory *CopyAACHeader();

	BOOL ETSCreateAACIoBuffers(UINT8 *buf ,int bodysize );
private:
	void Write( UINT8 *buf , int size);
	int  Write2IoBuffer( UINT8 *buf,int Length, UINT8 channelId );
public:
	CObjNetIOBufferSharedMemory *m_cur_ioBuf ;
	CObjNetIOBufferSharedMemory *m_header_ioBuf ;
	RTMPPacketType m_packetType;
	UINT32 m_timestamp  ;
	UINT32 m_timestampDelta  ;
	CObj m_ioList ;
	BOOL m_bInitedBuffer;
	CMdSource *m_mdsrc;
	int  m_cts;
	BOOL m_bKeyFrame;
private:
	int m_amfsize;
	BOOL m_bDeltaTimestamp;
	int m_nDataLength;
};



class  CRTMPNetworkSrv;

class   CRTMPNetSession :
	public CRTMPCtx ,
	public CCtx 
{
public:
	_CPP_UTIL_DYNAMIC(CRTMPNetSession)  ;
	_CPP_UTIL_CLASSNAME(CRTMPNetSession) ;
	_CPP_UTIL_QUERYOBJ(CRTMPCtx) ;
	CRTMPNetSession();
	virtual ~CRTMPNetSession();
	virtual void Close();
	virtual void CloseByRTMPT();



	BOOL Send(CObjNetIOBuffer *pBuf);

	
	void OnClose(CMediasrvCallback *callback);
	void CloseMdSource();
    BOOL NeedSendPing(int &indexPing);
public:

	BOOL m_bSendCodecInfo;

	BOOL m_bInSessionList;
	BOOL m_bOnCloseCalled;
	BOOL m_bUseSettingSrv;
	UINT64 m_iAudioFrameQueue;
	UINT64 m_iVideoFrameQueue;
	BOOL m_bNoCache;
	BOOL m_bVideoOnly;
	BOOL m_bAudioOnly;
    BOOL m_bFlashPlayer;
    int  m_indexPing;
    TIME_T m_tmCreateStream;
    BOOL m_bPublishCalled;

    int m_resultNumber;
    UINT32  m_windowAck;
    UINT64 m_nextReceivedBytesCountReport;
    UINT32 m_nBytesReceived;

};


class   CRTMPConnNetCtx :
	public CRTMPConnContext,
	public CCtx
{
public:
	_CPP_UTIL_DYNAMIC(CRTMPConnNetCtx)  ;
	_CPP_UTIL_CLASSNAME(CRTMPConnNetCtx) ;
	_CPP_UTIL_QUERYOBJ(CRTMPConnContext) ;
	CRTMPConnNetCtx();
	virtual ~CRTMPConnNetCtx();
	virtual BOOL InitLater();
	virtual CRTMPCtx *CreateRTMPCtx();
	BOOL SendOnMetaData(CMdSource *mdsrc);

	void Close();
	virtual void OnClose();
	void OnReallyClose(CMediasrvCallback *callback);

	BOOL m_bCloseAfterSend;
	BOOL m_bOnCloseCalled;
	BOOL m_bOnReallyCloseCalled;
	TIME_T m_tmBeginSession;
public:

	const CObj *m_queueThread;
	CObjEventQueue *m_queue;

};


class CRTMPEvent :
	public CObjEvent
{
public:
	typedef enum tagRTMPEvent
	{
		RTMPEventUnknown ,
		RTMPEventConnect ,
		RTMPEventCreateStream,
		RTMPEventPlay ,
		RTMPEventFCPublish,
		RTMPEventPublish ,
		RTMPEventSetDataFrame,
		RTMPEventReleaseStream,
		RTMPEventVideo,
		RTMPEventAudio,
		RTMPEventCloseByCtx,
		RTMPEventCloseBySession,

	}RTMPEvent;
public:
	static CRTMPEvent * CreateObj(CRTMPConnNetCtx *pContext)
	{
		if( pContext == NULL )
		{
			//return NULL ;
		}
		CRTMPEvent *pObj = new CRTMPEvent(pContext);
		if( pObj == NULL )
		{
			return NULL ;
		}
		pObj->m_bAlloced = TRUE ;
		pObj->m_nObjSize = sizeof(CRTMPEvent) ;
		pObj->AddObjRef();
		return pObj ;

	}
	_CPP_UTIL_CLASSNAME(CRTMPEvent) ;
	_CPP_UTIL_QUERYOBJ(CObjEvent) ;

public:
	CRTMPEvent(CRTMPConnNetCtx *ctx);
	virtual ~CRTMPEvent();
	virtual BOOL Post(BOOL bNotify = TRUE);
	virtual int OnExecute(CObj *pThreadContext = NULL);
	BOOL Post2MdSource(BOOL bNotify = TRUE);
	BOOL Post2Close(BOOL bNotify = TRUE);
public:
	CObjPtr<CAMFObject> m_objAmf;
	CObjPtr<CRTMPConnNetCtx> m_ctx;
	CObjPtr<CRTMPNetSession> m_ss;
	CObjPtr<CMdSource>  m_mdsrc;

	CRTMPHeader m_rtmpHeader;
	CObjPtr<CObjNetIOBuffer> m_buf;

	RTMPEvent m_event;
	CMediasrvCallback *m_callback;
};

class  CRTMPCfg
{
public:
	CRTMPCfg();
	~CRTMPCfg();

	BOOL m_bPublish;
	BOOL m_bPlayback;
	char m_szApp[80];
	int  m_ChunkSize;
	
	BOOL m_bOutput_hls;
	BOOL m_bHlsOutputMemory;
	BOOL m_bRecordTs;
	BOOL m_bRecordMp4;
	BOOL m_bRecordFlv;
	BOOL m_bUseSettingSrv;
	BOOL m_bForcePlayback;
	BOOL m_bForceHlsSetting;
	BOOL m_bExcludeConnections;
	BOOL m_bEnable_virtual_live;


	CObjVarRef *m_hls_dir;
	CObjVarRef *m_hls_ts_prefix;
	CObjVarRef *m_record_ts_dir;
	CObjVarRef *m_record_mp4_dir;
	CObjVarRef *m_record_flv_dir;
};
class  CRTMPNetworkSrv :
	public CObj ,
	public CRTMPNetAsync
{
	_CPP_UTIL_DYNAMIC_DECLARE(CRTMPNetworkSrv)  ;
	_CPP_UTIL_CLASSNAME_DECLARE(CRTMPNetworkSrv) ;
	_CPP_UTIL_QUERYOBJ_DECLARE(CRTMPNetworkSrv) ;
public:

	static void InitHandshake();
	CRTMPNetworkSrv(int nWaitTimeoutSeconds = -1, int nMaxConnections = _MAX_CONNECTIONS);
	virtual ~CRTMPNetworkSrv();

	virtual CObjConnContext  * AllocConnContext(CObj *par);
	virtual int OnNewConnectionIncoming(CObjConnContext *pContext);
	virtual int OnRTMPReadCompleted(CRTMPConnContext *pContext, CRTMPHeader *header,CObjNetIOBuffer *pBuffer);
			int OnRTMPCtrl(CRTMPConnContext *pContext ,CRTMPHeader *header, CObjNetIOBuffer *pBuffer);
			int OnInvoke(CRTMPConnNetCtx *ctx,const UINT8 *buf , int nLen );
            int OnOther(CRTMPConnContext *ctx, CRTMPHeader *header, CObjNetIOBuffer *pBuffer);
			int OnVideoData(CRTMPConnNetCtx *ctx,CRTMPHeader *header,CObjNetIOBuffer *pBuffer);
			int OnAudioData(CRTMPConnNetCtx *ctx,CRTMPHeader *header,CObjNetIOBuffer *pBuffer);


	virtual int OnWriteCompleted(CObjConnContext *pContext, CObjNetIOBuffer *pBuffer);
	virtual int OnReadCompleted(CObjConnContext *pContext, CObjNetIOBuffer *pBuffer);

	virtual int OnConnectionClosed(CObjConnContext *pContext);
	virtual int OnMaxConnectionsLimited(CObjConnContext *pContext);
	virtual int OnError(CObjConnContext *pContext, const char * szTxt);
    virtual int OnSessionTimeout(CObjConnContext *pContext);

	virtual int OnReceivedHandshake1(CRTMPConnContext *pContext ,const unsigned char * data, int length);
	virtual int OnReceivedHandshake2(CRTMPConnContext *pContext ,const unsigned char * data, int length);
	
public:


	CRTMPCfg m_cfg;
	CRTMPCfg *m_pCfg;
	int  m_index;
};

#if defined(_USE_TCPSERVER)
class CObjTcpServerRTMP :
	public CObjTcpServer,
	public CObj
{
public:
	static CObjTcpServerRTMP * CreateObj(int nMaxInstance, CObjTcpServer::BlanceMode mode = BlanceModeMinimum)
	{

		CObjTcpServerRTMP *pObj = new CObjTcpServerRTMP(nMaxInstance, mode);
		if (pObj == NULL)
		{
			return NULL;
		}
		pObj->m_bAlloced = TRUE;
		pObj->m_nObjSize = sizeof(CObjTcpServerRTMP);
		pObj->AddObjRef();
		return pObj;

	}
	_CPP_UTIL_CLASSNAME(CObjTcpServerRTMP);
	_CPP_UTIL_QUERYOBJ(CObj);
public:
	CObjTcpServerRTMP(int nMaxInstance = -1, CObjTcpServer::BlanceMode mode = BlanceModeMinimum);
	~CObjTcpServerRTMP();
	virtual CObjNetAsync *CreateNetInstance();
	virtual int OnNewConnectionIncoming(CObjConnContext *pContext);

	CRTMPCfg m_cfg;
	CRTMPCfg *m_pCfg;
	int  m_index;
	unsigned int m_Protocol;
};
#endif















#endif

#endif












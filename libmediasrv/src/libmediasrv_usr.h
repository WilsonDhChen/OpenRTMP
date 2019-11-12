 

#ifndef _libmediasrv_usr_h_
#define _libmediasrv_usr_h_

#include "libcam.h"
#define _FLV_TAGHEADER_LEN  11
#define _FLV_CODECID_H264 7

#if defined(_AUDIO_CONVERT_)
#include "audio_convert.h"
#endif



BOOL IsH264KeyFrame(UINT8 *data, int nDatalen, BOOL *findAud);

class CRTSPNetIOBuffer;
class CRTSPNetContext;
class CMdSource;
class CMP4Ctx;
class CClientList;
class   CFlvCtx;
class CHLSCtx;
class CETSConnContext;

class CETS2SharedMemVar
{
public:
	CETS2SharedMemVar(CMdSource *mdsrc, int  cts, INT64 tick, UINT8 *buf, int nLen, XCodec codec);
	virtual ~CETS2SharedMemVar();

	BOOL Init();
public:
	UINT8 *m_buf;
	int m_bufLen;
	int  m_cts;
	INT64 m_tick;
	XCodec m_codec;

	void *m_sharedMem;
};




class    CObjNetIOBufferSharedMemoryTSSource :
	public CObjNetIOBufferSharedMemory
{
public:
	static CObjNetIOBufferSharedMemoryTSSource * CreateObj(void *memShared, int memSize, BOOL bMalloc2 = TRUE);
	_CPP_UTIL_CLASSNAME(CObjNetIOBufferSharedMemoryTSSource);
	_CPP_UTIL_QUERYOBJ(CObjNetIOBufferSharedMemoryTSSource);

	CObjNetIOBufferSharedMemoryTSSource(void *memShared, int memSize, BOOL bMalloc2 = TRUE);
	virtual ~CObjNetIOBufferSharedMemoryTSSource();

};


class    CObjNetIOBufferSharedMemoryVideoFrame :
	public CObjNetIOBufferSharedMemory
{
public:
	static CObjNetIOBufferSharedMemoryVideoFrame * CreateObj(void *memShared, int memSize, BOOL bMalloc2 = TRUE);
	_CPP_UTIL_CLASSNAME(CObjNetIOBufferSharedMemoryVideoFrame);
	_CPP_UTIL_QUERYOBJ(CObjNetIOBufferSharedMemoryVideoFrame);

	CObjNetIOBufferSharedMemoryVideoFrame(void *memShared, int memSize, BOOL bMalloc2 = TRUE);
	virtual ~CObjNetIOBufferSharedMemoryVideoFrame();

	CObjNetIOBufferSharedMemoryVideoFrame *Copy();
public:
	UINT64 m_iVideoFrameQueue;
	UINT64 m_iAudioFrameQueue;
	INT64 m_tick;
	int   m_cts;
	BOOL m_bKeyFrame;
	BOOL m_bAudio;
};

#define _MAX_VIDEO_FRAMES 1000 


class CRTMPAudioConfig :public LibAudioConfig
{
public:
    CRTMPAudioConfig();
    BOOL Parse(UINT8 type);
public:
    double  aac_frame_dur;
    int  inited;
};


class    CMdSource :
	public CObj 

#if defined(_RTSPTS_SUPPORT_) || defined(_HLS_SUPPORT_) || defined(_HTTPTS_SUPPORT_) 
	,  public CTSIOCtx
#endif

{

public:
	_CPP_UTIL_CLASSNAME_DECLARE(CMdSource) ;
	_CPP_UTIL_QUERYOBJ_DECLARE(CMdSource) ;
public:
	void * operator new( size_t cb );
	void operator delete( void* p );
public:
    static CMdSource * CreateObj(const char *szApp,const char *szId);
    CMdSource(const char *szApp, const char *szId);
	~CMdSource();
	void ReOpen();
    void RelaseMediaKeyInfoAudio();
    void RelaseMediaKeyInfoVideo();
#if defined(_RTSP_SUPPORT_)
	CRTSPNetIOBuffer  * GetSDP(CRTSPNetContext *ctx ,char const* cseq) ;
	int GetRtspSDP(XCodec codec,int trackId,char *buffer,int len);
	RTSPSTREAM*  GetSetupStream();
	int  GetTrackIndex(const char *track);
	BOOL ParseSdp(char *szBuf);
	unsigned int SendRtspData(CRTSPNetIOBuffer *ioBuf);
#endif

    void AttachNewSrcCtx(CCtx *ctx);
	BOOL IsOpened();
	BOOL AddClientSession( CCtx *ctx );
	void RemoveClientSession( CCtx *ctx );
    

    void DelayRemoveMdSourceIfMatchSrcCtx(CCtx *ctx,BOOL bSettingSrv = FALSE);
    void ClearSrcCtxIfMatch(CCtx *ctx);
    void CloseSrcCtx();
    void CopySrcCid2Ctx(CCtx *ctx);
	
	int GetConnections();
	void CloseAllClients();
	static char *FindNextMtag(char *szBuf);
	
	
	void GenAACConfigStr();


	
	unsigned int SendETSData(CObjNetIOBuffer *ioBuf, BOOL bDeltaTime, CETSConnContext *ctx);



	unsigned int SendRTMP_H264(CRTMPHeader *header,CObjNetIOBuffer *pBuffer);
	unsigned int SendRTMP_AAC(CRTMPHeader *header,CObjNetIOBuffer *pBuffer);
    unsigned int SendRTMP_AUDIO(CRTMPHeader *header, CObjNetIOBuffer *pBuffer);


	unsigned int SendETS_AAC(UINT8 *buf, int nLen, INT64 tick, UINT64 iAudioFrameQueue = 0);
	unsigned int SendETS_H264(UINT8 *buf, int nLen, INT64 tick, int cts, int flagsKeyFrame = 0, UINT64 iVideoFrameQueue = 0 );
	unsigned int SendETS_H265(UINT8 *buf, int nLen, INT64 tick, int cts, int flagsKeyFrame = 0, UINT64 iVideoFrameQueue = 0);
	unsigned int SendETS_TEXT(UINT8 *buf, int nLen, INT64 tick);

	
	int  GetRTMPChunkSize();
	int  GetRTMPStreamId();


    CHLSCtx *RemoveHLSSession();
    void CreateHLSSession(const char *szName, CHLSCtx *oldCtx);

	void CreateMP4Session(const char *szName);
	void CreateFlvSession(const char *szName);
public:
	BOOL m_bOutRTSP;
	BOOL m_bOutRTMP;
	BOOL m_bOutHTTP_TS;
	BOOL m_bOutHTTP_FLV;
	BOOL m_bClosedAll;





	CObj m_clientList;
	MTX_OBJ m_mtxList;


	

    CObjVar m_stream;
    CObjVar m_app;
    CStrKey m_remveKey;

	

	
	CCtx *m_ctxSrc;
	CtxType m_srcType;
    BOOL m_bNewSrcCtx;
    MTX_OBJ m_mtxSrcCtx;

	
	XCodec m_audioType;
	XCodec m_videoType;

	/************* RTSP begin *****************/
#if defined(_RTSP_SUPPORT_)
	RTSPSTREAM m_streams[ _RTSP_MAX_STREAM_ ];
#endif

	char m_szSdp[SIZE_2KB];
	char m_szContentType[200];
	char m_szGlobalTimestamp[8];
	int  m_nSdpLength;
	
	int  m_MaxPacketSize;
	int  m_StreamCount;
	int  m_interleavedNext;

	/************* RTSP END *****************/




	/************* TS begin *****************/
	virtual CObjNetIOBuffer *AllocTSIOBuffer();

	/************* TS end *****************/


	/************* flv begin *****************/
	BOOL CreateFlvHeader();
	MTX_OBJ m_mtxFlvHeader;
	CObjNetIOBufferSharedMemory *m_flvHeader;
	CObjNetIOBufferSharedMemory *m_flvHeaderVideo;
	CObjNetIOBufferSharedMemory *m_flvHeaderAudio;

	CObjNetIOBufferSharedMemory *m_flvSpsPpsTag;
	CObjNetIOBufferSharedMemory *m_flvAACHeaderTag;
	UINT32 m_flvTimePreVideo  ;
	UINT32 m_flvTimePreAudio  ;
	TIME_T m_firstFlvFrameTime;
	/************* flv end *****************/
	

	/************* RTMP begin *****************/
	BOOL CreateRtmpH264MetaData();
	MTX_OBJ m_mtxMetaData;
	CObjNetIOBufferSharedMemory *m_rtmpMetaDataBuf;
	CObjNetIOBufferSharedMemory *m_rtmpSpsPpsBuf;
	CObjNetIOBufferSharedMemory *m_rtmpAACHeaderBuf;
	int m_rtmpChunkSize;
	UINT32 m_rtmpTimePreVideo  ;
	UINT32 m_rtmpTimePreAudio  ;
	TIME_T m_firstRTMPFrameTime;

	int m_width ;
	int m_height  ;
	int m_framerate  ;
	int m_avclevel;
	int m_avcprofile;
	int m_videokeyframe_frequency;
	BOOL m_bCreatedSpsPps;

	int m_audiosamplerate;
	int m_audiosamplesize;
	int m_audiocodecid;
	int m_audiochannels;




	TIME_T m_tmChannel[_RTMP_MAX_CHANNELS];
	TIME_T m_tmETSVideo;
	TIME_T m_tmETSAudio;
	TIME_T m_tmETSText;
    TIME_T m_timestampDelta;
    TIME_T m_timestampMax;


	BOOL m_bRtmpSrcInited;
	TIME_T m_tmRTMPPublish;
    TIME_T m_rtmpTimeVideo;
    //TIME_T m_rtmpTimeAudio;

	/************* RTMP end *****************/
	
	/************* H264 begin *****************/
	CObjBuffer m_sps;
	CObjBuffer m_pps;
	CObjBuffer m_sei;
	/************* H264 end *****************/

	/************* aac begin *****************/
	CADTSHeader m_adtsHeader;
	double m_audioAbsTime;
    TIME_T m_tmAudioAbsTimeUpdate;
	UINT8  m_szAACConfig[4];
	char   m_szAACConfigStr[20];

    
	/************* aac end *****************/


	/************* amr begin *****************/
	AudioProp m_amrProp;
	/************* amr end *****************/



	/************* rtsp begin *****************/
#if defined(_RTSP_SUPPORT_)
	CRTSPTrack m_tracks[_MAX_TRACKS_];
#endif
	/************* rtsp end *****************/


	/************* hls begin *****************/
#if defined(_HLS_SUPPORT_)
	CObjPtr<CHLSCtx>  m_hls;
#endif
	/************* hls end *****************/
	

	/************* mp4 begin *****************/
#if defined(_MP4_SUPPORT_)
    CObjPtr<CMP4Ctx> m_mp4;
#endif
	/************* mp4 end *****************/
	
#if defined(_FLVREC_SUPPORT_)
    CObjPtr<CFlvCtx> m_flv;
#endif

#if defined(_AUDIO_CONVERT_)
    CObjPtr<CAudioConvert> m_audioConvert;
    BOOL m_bAudioUseRTMPTime;
#endif

    BOOL m_bConvertAudio;



	TIME_T m_tmFirstCachedFrame;
	MTX_OBJ m_mtxVideoFrames;


    CRTMPAudioConfig m_rtmpAudioConfig;

    BOOL m_bAudioNotSupport;

	


	BOOL m_bHaveVideo;
	BOOL m_bHaveAudio;


    TIME_T m_tmSendGps;
    
	BOOL m_bVirtual;
    BOOL m_bVideoUseAudioTime;
	UINT64 m_iVideoFrameQueue;
	UINT64 m_iAudioFrameQueue;
	TIME_T m_tmCacheBegin;
	BOOL m_bCacheStarted;
    BOOL m_fromCdn;
	const CObj *m_queueThread;
	CObjEventQueue *m_queue;
};



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class    CMdUser :
	public CObj 
{

public:
	_CPP_UTIL_CLASSNAME_DECLARE(CMdUser) ;
	_CPP_UTIL_QUERYOBJ_DECLARE(CMdUser) ;
public:
	void * operator new( size_t cb );
	void operator delete( void* p );
public:
	static CMdUser * CreateObj(const char *szUserName);
	CMdUser(const char *szUserName);
	~CMdUser();




public:


};












#endif




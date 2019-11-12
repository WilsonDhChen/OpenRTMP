
#ifndef _libmediasrv_ctx_h_
#define _libmediasrv_ctx_h_

#define _MAX_URI    400
#define _MAX_SESSION    80

#include "map_ext.h"

typedef enum tagCtxType
{
	CtxTypeUnknown,
	CtxTypeRTSP,
	CtxTypeRTSP_HTTP,



	CtxTypeETS,
	CtxTypeRTMP,
	CtxTypeRTMPT,

	CtxTypeHLS,

	CtxTypeFLV,

	CtxTypeEnd ,
}CtxType;

typedef enum tagSessionMgrStatus
{
	SessionMgrStatusUnknown ,
	SessionMgrStatusHold,
	SessionMgrStatusAttached

}SessionMgrStatus;

typedef enum tagSessionAuthType
{
	SessionAuthTypeDefault ,
	SessionAuthTypeBasic,
	SessionAuthTypeDigest

}SessionAuthType;

class CClientList;
class CMdSource ;

class CInetAddrStr :public INET_ADDR_STR
{
public:
    CInetAddrStr();
};

class CCtx
{
public:
	CCtx();
	virtual ~CCtx();

	virtual void Close();
	virtual BOOL SendIoBuffer(CObjNetIOBuffer *pBuf);

	long AddRef()  ;
	long ReleaseRef()  ;

	BOOL IsDied();

	void UpdateBeginTime();
	void UpdateEndTime();
public:
	CObjConnContext *m_objCtx;

	CObjPtr<CMdSource> m_mdsrc;
	

	CtxType m_typeCtx;
	BOOL    m_bSrcCtx;
	CObj    m_mdItem;
	BOOL    m_bInMdSource;
	BOOL    m_bCtxClosing;
	BOOL    m_bCtxError;
    BOOL    m_bCloseByPlugin;
	BOOL    m_bForceCloseMdsource;
	SessionMgrStatus m_sessionManagerStatus;
	


    

    CRefString m_user;
    CRefString m_pwd;




	CObjVar m_app;
	CObjVar m_stream;

    CRefString m_uri;
	CRefString m_query;

	
	char m_szTimeBegin[24];
	char m_szTimeEnd[24];
	TIME_T m_tmBegin;
	TIME_T m_tmEnd;

	CObjVar m_nickName;
    CRefString m_sysId; //for push from transcode server
    CRefString m_os;
	


	BOOL m_recordTs;
	BOOL m_recordFlv;
	BOOL m_recordMp4;

	BOOL m_fromCdn;
		
	UINT64 m_nBytesRead;
	UINT64 m_nBytesWrite;
    

	UINT64 m_nBytesReadPre;
	UINT64 m_nBytesWritePre;
	

    INET_ADDR_STR m_addrPeer;
	CClientList *m_clientlist;

	BOOL m_bValidRequest;
    UINT32 m_verAudio;
    UINT32 m_verVideo;


    CRefString m_devId;
    CRefString m_srcDevId;

    UINT64 m_nCid;
    UINT64 m_nSrcCid;
    
};

class  CCdnPar :
	public CObj
{

public:
	_CPP_UTIL_DYNAMIC(CCdnPar)
	_CPP_UTIL_CLASSNAME(CCdnPar);
	_CPP_UTIL_QUERYOBJ(CObj);

	CCdnPar();
	virtual ~CCdnPar();

public:
    CRefString m_url;
    CRefString m_app;
    CRefString m_stream;
    CRefString m_srcDevId;
    CRefString m_addr;
    CStrKey m_key;
	BOOL   m_bUrl;
};





#endif


























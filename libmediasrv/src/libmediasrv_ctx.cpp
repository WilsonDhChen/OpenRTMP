#include "libmediasrv_def.h"



UINT64 _cid = 0;


char *_ctxTypes[] = {
	"Unknown",
	"RTSP",
	"RTSP_HTTP",
	"RTSP_TS",
	"RTSP_FLV",
	"RTSP_AAC",
	"HTTP_TS",
	"HTTP_FLV",
	"HTTP_AAC",
	"ETS",
	"RTMP",
	"RTMPT",
	"HLS",
	"MP4",
	"FLV"

};

const char * CtxType2String(int type)
{
	if (type < 0 || type >= CtxTypeEnd)
	{
		return _ctxTypes[0];
	}

	return _ctxTypes[type];
}
CInetAddrStr::CInetAddrStr()
{
    nPort = 0;
    szAddr[0] = 0;
}
////////////////////////////////////////
CCtx::CCtx()
{
    m_nSrcCid = 0;
	m_nCid = MtxInterlockedInc64((INT64 volatile*)(&_cid));
    m_verAudio = 0 ;
    m_verVideo = 0 ;
	m_clientlist = NULL;
	m_bValidRequest = FALSE;
	m_fromCdn = FALSE;
	m_typeCtx = CtxTypeUnknown;
	m_bSrcCtx = FALSE ;
	m_bInMdSource = FALSE ;
	m_objCtx = NULL ;
    m_bCloseByPlugin = FALSE;
	m_bCtxError = FALSE;
	m_sessionManagerStatus = SessionMgrStatusUnknown ;


	m_recordTs = FALSE ;
	m_recordFlv = FALSE;
	m_recordMp4 = FALSE;


	


    m_addrPeer.nPort = 0;
    m_addrPeer.szAddr[ 0 ] = 0;
	m_bForceCloseMdsource = FALSE;
	m_bCtxClosing = FALSE;


	/*
	memset( m_szRequestURI , 0 , sizeof(m_szRequestURI) );
	memset( m_szUser , 0 , sizeof(m_szUser) );
	memset( m_szPwd , 0 , sizeof(m_szPwd) );
	memset( m_szId , 0 , sizeof(m_szId) );
	memset( m_szName , 0 , sizeof(m_szName) );
	memset( m_szSessionJoin , 0 , sizeof(m_szSessionJoin) );
	memset( m_szOs , 0 , sizeof(m_szOs) );
	memset( m_szJoinTimeBegin , 0 , sizeof(m_szJoinTimeBegin) );
	memset( m_szJoinTimeEnd , 0 , sizeof(m_szJoinTimeEnd) );
	*/




	m_nBytesRead = 0;
	m_nBytesWrite = 0;

	m_nBytesReadPre = 0;
	m_nBytesWritePre = 0;

	m_szTimeEnd[0] = 0;
	m_tmEnd = 0;

	UpdateBeginTime();


}
CCtx::~CCtx()
{
	if( m_mdsrc != NULL )
	{
		if (m_bInMdSource)
		{
			m_mdsrc->RemoveClientSession(this);
		}

        m_mdsrc->ClearSrcCtxIfMatch(this);

	}

}
void CCtx::UpdateBeginTime()
{
	DateTimeNowStr(m_szTimeBegin, FALSE, TRUE);
	m_tmBegin = OSTickCount();
}
void CCtx::UpdateEndTime()
{
	DateTimeNowStr(m_szTimeEnd, FALSE, TRUE);
	m_tmEnd = OSTickCount();
}
int CtxClose(CCtx *ctx)
{
	ctx->Close();

	return 0;
}
int CtxGetConnections(CCtx *ctx)
{
    if (ctx->m_mdsrc.p == NULL)
    {
        return -1;
    }
    return ctx->m_mdsrc->GetConnections();
}
int CtxSendCall(CCtx *ctx1, const char *szMethod, CObjVar *value)
{
    switch (ctx1->m_typeCtx)
    {
#if defined(_RTMP_SUPPORT_)
        case 	CtxTypeRTMP:
        case 	CtxTypeRTMPT:
        {
            CRTMPNetSession *ctx = (CRTMPNetSession *)(ctx1);
            CObjPtr<CRTMPConnContext> netCtx;
            netCtx.Attach(ctx->GetConnCtx());
            if (netCtx.p == NULL)
            {
                return -1;
            }
            if (netCtx->SendInvoke(szMethod, *value))
            {
                return 0;
            }
            else
            {
                return -1;
            }
        }
#endif
        default:
        {
            return -1;
        }
    }

}
int CtxAddRef(CCtx *ctx1)
{
    if (ctx1 == NULL)
    {
        return -1;
    }
	switch (ctx1->m_typeCtx)
	{

#if defined(_RTSP_SUPPORT_)
	case 	CtxTypeRTSP:
	case 	CtxTypeRTSP_HTTP:

	{
		CRTSPNetContext *ctx = (CRTSPNetContext *)(ctx1);
		return ctx->AddObjRef();
	}
#endif
#if defined(_HTTPTS_SUPPORT_) || defined(_HTTPFLV_SUPPORT_)
	case 	CtxTypeHTTP_TS:
	case 	CtxTypeHTTP_FLV:
	case 	CtxTypeHTTP_AAC:
	{
		CHttpTsNetContext *ctx = (CHttpTsNetContext *)(ctx1);
		return ctx->AddObjRef();
	}
#endif
#if defined(_ETS_SUPPORT_)
	case 	CtxTypeETS:
	{
		CETSConnContext *ctx = (CETSConnContext *)(ctx1);
		return ctx->AddObjRef();
	}
#endif
#if defined(_RTMP_SUPPORT_)
	case 	CtxTypeRTMP:
	case 	CtxTypeRTMPT:
	{
		CRTMPNetSession *ctx = (CRTMPNetSession *)(ctx1);
		return ctx->AddObjRef();
	}
#endif
#if defined(_HLS_SUPPORT_)
	case 	CtxTypeHLS:
	{
		CHLSCtx *ctx = (CHLSCtx *)(ctx1);
		return ctx->AddObjRef();
	}
#endif
#if defined(_MP4_SUPPORT_)
	case 	CtxTypeMP4:
	{
		CMP4Ctx *ctx = (CMP4Ctx *)(ctx1);
		return ctx->AddObjRef();
	}
#endif
	default:
	{
		return 0;
	}
	}
	return 0;
}
int CtxRelease(CCtx *ctx1)
{
    if (ctx1 == NULL)
    {
        return -1;
    }
	switch (ctx1->m_typeCtx)
	{

#if defined(_RTSP_SUPPORT_)
	case 	CtxTypeRTSP:
	case 	CtxTypeRTSP_HTTP:

	{
		CRTSPNetContext *ctx = (CRTSPNetContext *)(ctx1);
		return ctx->ReleaseObj();
	}
#endif
#if defined(_HTTPTS_SUPPORT_) || defined(_HTTPFLV_SUPPORT_)
	case 	CtxTypeHTTP_TS:
	case 	CtxTypeHTTP_FLV:
	case 	CtxTypeHTTP_AAC:
	{
		CHttpTsNetContext *ctx = (CHttpTsNetContext *)(ctx1);
		return ctx->ReleaseObj();
	}
#endif
#if defined(_ETS_SUPPORT_)
	case 	CtxTypeETS:
	{
		CETSConnContext *ctx = (CETSConnContext *)(ctx1);
		return ctx->ReleaseObj();
	}
#endif
#if defined(_RTMP_SUPPORT_)
	case 	CtxTypeRTMP:
	case 	CtxTypeRTMPT:
	{
		CRTMPNetSession *ctx = (CRTMPNetSession *)(ctx1);
		return ctx->ReleaseObj();
	}
#endif
#if defined(_HLS_SUPPORT_)
	case 	CtxTypeHLS:
	{
		CHLSCtx *ctx = (CHLSCtx *)(ctx1);
		return ctx->ReleaseObj();
	}
#endif
#if defined(_MP4_SUPPORT_)
	case 	CtxTypeMP4:
	{
		CMP4Ctx *ctx = (CMP4Ctx *)(ctx1);
		return ctx->ReleaseObj();
	}
#endif
	default:
	{
		return 0;
	}
	}
	return 0;
}
long CCtx::AddRef()
{
	return CtxAddRef(this);
}
long CCtx::ReleaseRef()
{
	return CtxRelease(this);
}
void CCtx::Close()
{
	m_bCtxClosing = TRUE;
	if( m_objCtx == NULL )
	{
		return ;
	}
	m_objCtx->m_net->CloseConnection( m_objCtx );
}
BOOL CCtx::IsDied()
{
	return m_bCtxError || m_bCtxClosing ;
}
BOOL CCtx::SendIoBuffer(CObjNetIOBuffer *pBuf)
{
	if( m_objCtx == NULL )
	{
		return FALSE;
	}

	if( m_objCtx->m_bClosing )
	{
		return FALSE ;
	}
	if (!m_objCtx->m_net->Send(m_objCtx, pBuf))
	{
		m_bCtxError = TRUE;
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}
///////////////////////////////////////////////////////






CCdnPar::CCdnPar()
{
	m_bUrl = FALSE;
}
CCdnPar::~CCdnPar()
{

}















#include "libmediasrv_def.h"



_MediaInitCallback _funcMediaInitCallback = NULL;
_MediaOpenCallback _funcMediaOpenCallback = NULL;
_MediaCloseCallback _funcMediaCloseCallback = NULL;



CCallbackProcessorCtx::CCallbackProcessorCtx()
{
	m_callback = NULL;
}
CCallbackProcessorCtx::~CCallbackProcessorCtx()
{

}

///////////////////////////////////////////////////////////////////

CCallbackEvent::CCallbackEvent()
{
	m_type = CallbackUnknown;
	m_ctx = NULL;
	m_callback = NULL;
}
CCallbackEvent::~CCallbackEvent()
{
	if (m_ctx != NULL)
	{
		m_ctx->ReleaseRef();
	}

}
void CCallbackEvent::SetCtx(CCtx *ctx)
{
	ctx->AddRef();
	if (m_ctx != NULL)
	{
		m_ctx->ReleaseRef();
	}
	m_ctx = ctx;
}

int CCallbackEvent::OnExecute(CObj *pThreadContext)
{
	CCallbackProcessorCtx *ctx = (CCallbackProcessorCtx *)pThreadContext;
	m_callback = ctx->m_callback;

	return 0;
}
BOOL CCallbackEvent::Post()
{
	if (_MediasrvApp->m_queueCallback == NULL)
	{
		return FALSE;
	}

	const CObj *idl = _MediasrvApp->m_queueCallback->GetIdleThreadObj();

	return _MediasrvApp->m_queueCallback->PostEvent(this, idl, TRUE);
}
/////////////////////////////////////////////////////

CMediasrvCallback::CMediasrvCallback()
{

}
CMediasrvCallback::~CMediasrvCallback()
{

}

int CMediasrvCallback::OnStreamWillPublish(CCtx *ctx)
{
	return 0;
}
int CMediasrvCallback::OnStreamPublished(CCtx *ctx)
{
	return 0;
}
int CMediasrvCallback::OnStreamPublishClosed(CCtx *ctx)
{
	return 0;
}
int CMediasrvCallback::OnStreamWillPlay(CCtx *ctx)
{
	return 0;
}
int CMediasrvCallback::OnStreamPlayStarted(CCtx *ctx)
{
    return 0;
}
int CMediasrvCallback::OnStreamPlayClosed(CCtx *ctx)
{
	return  0;
}
int CMediasrvCallback::OnInvalidRequestClosed(CCtx *ctx)
{
	return  0;
}
int CMediasrvCallback::OnStreamRecordFinished(CCtx *ctx, const char *szFileName, const char *szFilePath, double dur,
    const char *szBeginTime, const char *szEndTime, const char *szType)
{
	return  0;
}
int CMediasrvCallback::OnStreamRecordFlush(CCtx *ctx)
{
    return  0;
}
int CMediasrvCallback::IsInLocalNetwork(const char *reqDevId, CObjVar *outAddress)
{
    return  0;
}
int CMediasrvCallback::OnGetSourceStreamURL(const char *szApp, const char *szStream, CObjVar *outAddress) //  for cdn support
{
	return  -1;
}
int CMediasrvCallback::OnStreamHLSRecordFinished(CCtx *ctx, const char *szFileName, const char *szFilePath, double dur,
    const char *szBeginTime, const char *szEndTime)
{
    return  0;
}

int CMediasrvCallback::OnStreamHLSFlush(CCtx *ctx)
{
    return  0;
}
int CMediasrvCallback::OnStreamHLSReOpen(CCtx *ctx)
{
    return  0;
}

/////////////////////////////////////////////////////////////////
CCallbackProcessor::CCallbackProcessor(int nThreads , int nStackSize , int nWaitMilliSecond )
	:CObjEventQueue(nThreads, nStackSize, nWaitMilliSecond)

{

}
CCallbackProcessor::~CCallbackProcessor()
{

}

CObj * CCallbackProcessor::CreateThreadContext(const CObj *threadObj)
{
	CCallbackProcessorCtx *ctx = CCallbackProcessorCtx::CreateObj();

	if (_AppConfig.m_bCallbackEnable && _funcMediaOpenCallback != NULL)
	{
		ctx->m_callback = (CMediasrvCallback *)_funcMediaOpenCallback(_AppConfig.m_szPathIniFile, _AppConfig.m_szPathWorkDir);
	}
	
	return  ctx;
}
void CCallbackProcessor::OnQueueEmpty(CObj *pThreadContext, const CObj *threadObj)
{

}



















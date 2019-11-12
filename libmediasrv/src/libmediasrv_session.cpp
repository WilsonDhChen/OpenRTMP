


#include "libmediasrv_def.h"



	#undef max
	#undef min


#include "map"

#define _BKDRHASH_
//#define  _CStrKey_LEN_ 100

#include "map_ext.h"


static BOOL _bInited = FALSE ;
static char _szNULL[2] = {0,0};
//////////////////////////////RTMP HTTP Session//////////////////////////////////////////////////////

#if defined(_RTMP_SUPPORT_)
typedef std::pair <CStrKey, CRTMPNetSession* > RTMPHttpSessionPair;
typedef std::map <CStrKey, CRTMPNetSession* > RTMPHttpSessionList;

static RTMPHttpSessionList _SessionRtmpHttpList;
static MTX_OBJ _SessionRtmpHttpMtx;


class CRtmpHttpSessionAutoLock
{
public:
	CRtmpHttpSessionAutoLock()
	{
		MtxLock( &_SessionRtmpHttpMtx );
	}
	~CRtmpHttpSessionAutoLock()
	{
		MtxUnLock( &_SessionRtmpHttpMtx  );
	}
};

CRTMPNetSession *GetRTMPNetSession(const char *szId)
{

	CRTMPNetSession *src;
	CRtmpHttpSessionAutoLock lock;
	RTMPHttpSessionList::iterator itor  ;
	itor = _SessionRtmpHttpList.find( szId );
	if( itor ==  _SessionRtmpHttpList.end() )
	{
		return NULL ;
	}

	src = itor->second ;

	if( src != NULL )
	{
		src->AddObjRef();
	}


	return src ;
}
void RemoveRTMPNetSession(const char *szId ,  CRTMPNetSession * ss)
{
	if( !ss->m_bInSessionList )
	{
		return ;
	}
    LogPrint( _LOG_LEVEL_DEBUG, _TAGNAME, "%s %d: szId=%s ; ss=%p\n", __FUNCTION__, __LINE__, szId, ss );
	CRtmpHttpSessionAutoLock lock;
	RTMPHttpSessionList::iterator itor  ;
	itor = _SessionRtmpHttpList.find( szId );
	if( itor ==  _SessionRtmpHttpList.end() )
	{
		return  ;
	}

	if( itor->second == ss )
	{

		_SessionRtmpHttpList.erase( itor ) ;
		ss->m_bInSessionList = FALSE ;
	}


}
void AddRTMPNetSession(const char *szId ,  CRTMPNetSession * ss)
{
	if( ss->m_bInSessionList )
	{
		return ;
	}

    LogPrint( _LOG_LEVEL_DEBUG, _TAGNAME, "%s %d: szId=%s ; ss=%p\n", __FUNCTION__, __LINE__, szId, ss );
	CRtmpHttpSessionAutoLock lock;

	RTMPHttpSessionList::iterator itor  ;
	itor = _SessionRtmpHttpList.find( szId );
	if( itor !=  _SessionRtmpHttpList.end() )
	{
		if( itor->second == NULL )
		{
			_SessionRtmpHttpList.erase( itor );
		}
		else 
		{
			if( itor->second  == ss )
			{
				return ;
			}
			else
			{
				CRTMPNetSession * ssTmp = itor->second ;
				if( ssTmp != NULL )
				{
					ssTmp->Close();
				}
				_SessionRtmpHttpList.erase( itor );
			}

		}

	}
	ss->m_bInSessionList = TRUE ;
	_SessionRtmpHttpList.insert( RTMPHttpSessionPair(szId,ss ) );
}

#endif



////////////////////////////////////////////////////////////////////////////////////
typedef std::pair <CStrKey, CMdSource* > StreamSourcePair;
typedef std::map <CStrKey, CMdSource* > StreamSourceList;







class  CMdSourceApp :
	public CObj
{

public:
	_CPP_UTIL_DYNAMIC(CMdSourceApp);
	_CPP_UTIL_CLASSNAME(CMdSourceApp);
	_CPP_UTIL_QUERYOBJ(CObj);
	CMdSourceApp()
	{
		MtxInit(&m_mtx , 0 );
	}
	virtual ~CMdSourceApp()
	{
		MtxDestroy(&m_mtx);
	}

	BOOL AddMdSource(const char *urlSuffix, CMdSource * mdsrc)
	{
        LogPrint(_LOG_LEVEL_DEBUG, _TAGNAME, "CMdSourceApp::%s %s,%s\n", __FUNCTION__, (const char *)m_app,urlSuffix);
		CMtxAutoLock lock(&m_mtx);
		if (mdsrc == NULL || mdsrc->m_bClosedAll)
		{
			return FALSE;
		}
		StreamSourceList::iterator itor;
		itor = m_streams.find(CStrKey(urlSuffix));
		if (itor != m_streams.end())
		{
			if (itor->second == NULL)
			{
				m_streams.erase(itor);
			}
			else
			{
				if (itor->second == mdsrc)
				{
					return TRUE;
				}
				else
				{
					CMdSource * mdsrcTmp = itor->second;
					if (mdsrcTmp != NULL)
					{
                        mdsrcTmp->CloseSrcCtx();
						mdsrcTmp->CloseAllClients();
					}
					m_streams.erase(itor);
				}

			}

		}
		mdsrc->AddObjRef();
		m_streams.insert(StreamSourcePair(CStrKey(urlSuffix), mdsrc));
		return TRUE;
	}


	BOOL RemoveMdSource(const char *urlSuffix, CMdSource * mdsrc)
	{
        LogPrint(_LOG_LEVEL_DEBUG, _TAGNAME, "CMdSourceApp::%s %s,%s\n", __FUNCTION__, (const char *)m_app, urlSuffix);
		CMtxAutoLock lock(&m_mtx);
		StreamSourceList::iterator itor;
		itor = m_streams.find(CStrKey(urlSuffix));
		if (itor == m_streams.end())
		{
			return  FALSE;
		}

		if (itor->second == mdsrc)
		{
			m_streams.erase(itor);
			mdsrc->ReleaseObj();
			return  TRUE;
		}

		return  FALSE;
	}
	BOOL RemoveMdSourceClose(const char *urlSuffix, BOOL removeByPlugin)
	{
        LogPrint(_LOG_LEVEL_DEBUG, _TAGNAME, "CMdSourceApp::%s %s,%s\n", __FUNCTION__, (const char *)m_app, urlSuffix);
		CMtxAutoLock lock(&m_mtx);
		StreamSourceList::iterator itor;
		CMdSource * mdsrc = NULL;
		itor = m_streams.find(CStrKey(urlSuffix));
		if (itor == m_streams.end())
		{
			return  FALSE;
		}
		mdsrc = itor->second;
		m_streams.erase(itor);

		if (mdsrc != NULL)
		{
			mdsrc->m_bVirtual = FALSE;
			if (mdsrc->m_ctxSrc != NULL)
			{
				mdsrc->m_ctxSrc->m_bForceCloseMdsource = TRUE;
				mdsrc->m_ctxSrc->m_bCloseByPlugin = removeByPlugin;
                mdsrc->CloseSrcCtx();
			}
			mdsrc->CloseAllClients();
			mdsrc->ReleaseObj();
		}

		return  TRUE;
	}

	BOOL IsExistMdSource(const char *urlSuffix)
	{

		CMdSource *src;
		CMtxAutoLock lock(&m_mtx);
		StreamSourceList::iterator itor;
		itor = m_streams.find(CStrKey(urlSuffix));
		if (itor == m_streams.end())
		{
			return FALSE;
		}
		else
		{
			return TRUE;
		}
	}

	CMdSource *GetMdSource( const char *urlSuffix)
	{
		CMdSource *src = NULL;
		CMtxAutoLock lock(&m_mtx);

		StreamSourceList::iterator itor;
		itor = m_streams.find(CStrKey(urlSuffix));
		if (itor == m_streams.end())
		{

			return NULL;
		}

		src = itor->second;

		if (src != NULL)
		{
			src->AddObjRef();
		}


		return src;
	}
	int Count()
	{
		return m_streams.size();
	}
public:
	MTX_OBJ m_mtx;
	StreamSourceList m_streams;
	CStrKey m_app;
};



typedef std::pair <CStrKey, CMdSourceApp* > StreamAppPair;
typedef std::map <CStrKey, CMdSourceApp* > StreamAppList;

static StreamAppList _AppList;
static MTX_OBJ _AppMtx;


class StreamAppAutoLock
{
public:
	StreamAppAutoLock()
	{
		MtxLock(&_AppMtx);
	}
	~StreamAppAutoLock()
	{
		MtxUnLock(&_AppMtx);
	}
};
void SessionInit()
{
	if( _bInited )
	{
		return ;
	}
	_bInited = TRUE ;

	MtxInit(&_AppMtx, 0);
#if defined(_RTMP_SUPPORT_)
	MtxInit(&_SessionRtmpHttpMtx,0) ;
#endif

}


int GetStreamConnections(const char *szApp, const char *szStream)
{
    CObjPtr<CMdSource>   mdsource;

    mdsource.Attach(GetMdSource(szApp, szStream));
    if (mdsource.p == NULL)
    {
        return -1;
    }

    return  mdsource->GetConnections();
}
CMdSource *GetMdSource(const char *szApp,const char *urlSuffix)
{

	if (_AppConfig.m_mode != WorkModeCdn)
	{
		szApp = _szNULL;
	}

	CObjPtr<CMdSourceApp > app;

	{
		StreamAppAutoLock lock;
		StreamAppList::iterator itor;
		itor = _AppList.find(CStrKey(szApp));
		if (itor == _AppList.end())
		{

			return NULL;
		}
		app = itor->second;
	}

	if (app.p != NULL)
	{
		return app->GetMdSource(urlSuffix);
	}
	

	return NULL ;
}

BOOL IsExistMdSource(const char *szApp, const char *urlSuffix)
{
	if (_AppConfig.m_mode != WorkModeCdn)
	{
		szApp = _szNULL;
	}

	CObjPtr<CMdSourceApp > app;
	{
		StreamAppAutoLock lock;
		StreamAppList::iterator itor;
		itor = _AppList.find(CStrKey(szApp));
		if (itor == _AppList.end())
		{
			return FALSE;
		}
		else
		{
			app = itor->second;
		}
	}

	if (app.p != NULL)
	{
		return app->IsExistMdSource(urlSuffix);
	}
	else
	{
		return FALSE;
	}

}

BOOL RemoveMdSource(const char *szApp, const char *urlSuffix, CMdSource * mdsrc)
{


	if (_AppConfig.m_mode != WorkModeCdn)
	{
		szApp = _szNULL;
	}

	CObjPtr<CMdSourceApp > app;
	
	StreamAppAutoLock lock;
	StreamAppList::iterator itor;
	itor = _AppList.find(CStrKey(szApp));
	if (itor == _AppList.end())
	{
		return  FALSE;
	}
	else
	{
		app = itor->second;
	}

	

	if (app.p == NULL)
	{
		return FALSE;
	}

	BOOL bRemoved = app->RemoveMdSource(urlSuffix, mdsrc);

	if (bRemoved)
	{
		if (app->Count() < 1)
		{
			_AppList.erase(itor);
			app->ReleaseObj();
		}
	}


	return  bRemoved;
}
BOOL RemoveMdSourceClose(const char *szApp, const char *urlSuffix, BOOL removeByPlugin)
{


	if (_AppConfig.m_mode != WorkModeCdn)
	{
		szApp = _szNULL;

	}
	CObjPtr<CMdSourceApp > app;

	StreamAppAutoLock lock;
	StreamAppList::iterator itor;
	itor = _AppList.find(CStrKey(szApp));
	if (itor == _AppList.end())
	{
		return  FALSE;
	}
	else
	{
		app = itor->second;
	}



	if (app.p == NULL)
	{
		return FALSE;
	}

	BOOL bRemoved = app->RemoveMdSourceClose(urlSuffix, removeByPlugin);

	if (bRemoved)
	{
		if (app->Count() < 1)
		{
			_AppList.erase(itor);
			app->ReleaseObj();
		}
	}


	return  bRemoved;
}
BOOL AddMdSource(const char *szApp, const char *urlSuffix, CMdSource * mdsrc)
{

	
	if (_AppConfig.m_mode != WorkModeCdn)
	{
		szApp = _szNULL;
	}

	if (mdsrc == NULL || mdsrc->m_bClosedAll )
	{
		return FALSE;
	}
	CObjPtr<CMdSourceApp > app;

	StreamAppAutoLock lock;
	StreamAppList::iterator itor;
	itor = _AppList.find(CStrKey(szApp));
	if (itor == _AppList.end())
	{
		goto LABEL_NotFound;
	}
	else
	{
		app = itor->second;
	}
	if (app.p != NULL)
	{
		return app->AddMdSource(urlSuffix, mdsrc);
	}

LABEL_NotFound:
	app = CMdSourceApp::CreateObj();
	app->m_app = szApp;
	_AppList.insert(StreamAppPair(CStrKey(szApp), app));
	return app->AddMdSource(urlSuffix, mdsrc);
}







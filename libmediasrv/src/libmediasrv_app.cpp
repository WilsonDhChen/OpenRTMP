


#include "libmediasrv_def.h"

int _portRTSP = 554 ;


CMediaSrvApp *_MediasrvApp = NULL;
//#define _SELECT_IO
#if defined(_SELECT_IO)
CObjNetAsync::NetAsyncType _NetType = CObjNetAsync::NetAsyncTypeSelect;
#else
CObjNetAsync::NetAsyncType _NetType = CObjNetAsync::NetAsyncTypeDefault;
#endif

static char _szNULL[2] = { 0, 0 };
//////////////////////////////////////////////////////





CAppLogLock::CAppLogLock()
{
    MtxLock(&_AppConfig.m_mtxLog);
}
CAppLogLock::~CAppLogLock()
{
    MtxUnLock(&_AppConfig.m_mtxLog);
}

CMediaSrvApp::CMediaSrvApp()
{
	_MediasrvApp = this;
	m_queueDiskIo = NULL ;
    
	m_queueDataProcess = NULL;
	m_queueNetworkClose = NULL;
	m_queueSettingSrvNotify = NULL;
	
	m_httptsCount = 0;
	m_httpdCount = 0;
	m_etsCount = 0;
	m_rtmpCount = 0;
	m_rtspCount = 0;

	
#if defined(_MDSOURCE_MT)
	m_queueSendThread = NULL;
#endif


#if defined(_RTMP_SUPPORT_)
	#if defined(_USE_TCPSERVER)
		memset( m_rtmpServer , 0 , sizeof(m_rtmpServer) );
	#else
		memset( m_rtmpNetwork , 0 , sizeof(m_rtmpNetwork) );
	#endif
	m_rtmpLocal = NULL;
	m_queueNetworkEventRTMP = NULL;
#endif


//////////////////////////////////////////////////
#if defined(_ETS_SUPPORT_)
	m_queueNetworkEventETS = NULL;
	#if defined(_USE_TCPSERVER)
		memset(m_etsServer, 0, sizeof(m_etsServer));
	#else
		memset( m_etsNetwork , 0 , sizeof(m_etsNetwork) );
	#endif
#endif

/////////////////////////////////////////////
#if defined(_ENABLE_HTTPD_)
	m_queueHttpd = NULL;
	#if defined(_USE_TCPSERVER)
		memset(m_httpdServer, 0, sizeof(m_httpdServer));
	#else
		memset( m_httpdNetwork, 0, sizeof( m_httpdNetwork ) );
	#endif
#endif	

////////////////////////////////////////////////////
#if defined(_RTSP_SUPPORT_)
	m_queueNetworkEventRTSP_TS = NULL;
	#if defined(_USE_TCPSERVER)
		memset(m_rtspTsServer, 0, sizeof(m_rtspTsServer));
	#else
		memset(m_rtsp_tsNetwork, 0, sizeof(m_rtsp_tsNetwork));
	#endif
#endif

#if defined(_HTTPTS_SUPPORT_)
	m_queueHttpTs = NULL;
	#if defined(_USE_TCPSERVER)
		memset(m_httpTsServer, 0, sizeof(m_httpTsServer));
	#else
		memset(m_httptsNetwork, 0, sizeof(m_httptsNetwork));
	#endif
#endif

    m_queueCallback = NULL;




    
}
CMediaSrvApp::~CMediaSrvApp()
{
    CleanUp();

#if defined(_CDNNET_SUPPORT_)
    MtxDestroy(&m_mtxNetInterfaces);
    MtxDestroy(&m_mtxCdnCtxs);
#endif

    

}
#if defined(_CDNNET_SUPPORT_)
BOOL    CMediaSrvApp::SendRequestStreamSource(const char *szApp, const char *szStreamId)
{

    if (szStreamId == NULL || szStreamId[0] == 0)
    {
        return FALSE;
    }
    int nAppLen = 0;
    int nStreamLen = strlen(szStreamId);
    if (nStreamLen > 254)
    {

        LogPrint(_LOG_LEVEL_ERROR, _TAGNAME_CDN, "Stream id [%s] too long,MAX is 254\n", szStreamId);
        return FALSE;
    }
    if (szApp != NULL)
    {
        nAppLen = strlen(szApp);
        if (nAppLen > 254)
        {
            LogPrint(_LOG_LEVEL_ERROR, _TAGNAME_CDN, "App [%s] too long,MAX is 254\n", szApp);
            return FALSE;
        }

    }
    int nCount = 0;
    CMtxAutoLock lock(&m_mtxCdnCtxs);

    CObj * const pItemHead = &m_cdnCtxs;
    CObj *pItem = NULL;

    pItem = pItemHead->m_pNextObj;
                      
    while (pItem != NULL && pItem != pItemHead)
    {
        CCdnNetContext *ctxTmp = Convert2CCdnCtx(pItem);
        pItem = pItem->m_pNextObj;

        if (ctxTmp->SendRequestStreamSource(szApp, szStreamId))
        {
            nCount++;
        }
        
    }

    return  (nCount > 0);

}
void CMediaSrvApp::CopyNetInterfaces(CObjArray *interfaces)
{
    CMtxAutoLock lock(&m_mtxNetInterfaces);
    *interfaces = m_cdnnetInterfaces;
}
void CMediaSrvApp::AddCdnCtx(CCdnNetContext   *ctx)
{
    CMtxAutoLock lock(&m_mtxCdnCtxs);
    m_cdnCtxs.ListAddTail(&(ctx->m_item));
}
void CMediaSrvApp::RemoveCdnCtx(CCdnNetContext   *ctx)
{
    CMtxAutoLock lock(&m_mtxCdnCtxs);
    m_cdnCtxs.ListDel(&(ctx->m_item));
}
#endif
void CMediaSrvApp::CleanUp()
{
    if (m_queueDiskIo != NULL)
    {
        delete m_queueDiskIo;
        m_queueDiskIo = NULL;
    }

    int  i = 0;




    if (m_queueSettingSrvNotify != NULL)
    {
       delete m_queueSettingSrvNotify; 
       m_queueSettingSrvNotify = NULL;
    }

#if defined(_CDNNET_SUPPORT_)
    if (m_cdnNetwork != NULL)
    {
        m_cdnNetwork->ReleaseObj();
        m_cdnNetwork = NULL;
    }
#endif

#if defined(_RTSP_SUPPORT_)
#if defined(_USE_TCPSERVER)
    for (i = 0; i < (_MAX_NETWORKS - 1) && m_rtspTsServer[i] != NULL; i++)
    {
        m_rtspTsServer[i]->ReleaseObj();
        m_rtspTsServer[i] = NULL ;
    }
#else
    for (i = 0; i < (_MAX_NETWORKS - 1) && m_rtsp_tsNetwork[i] != NULL; i++)
    {
        m_rtsp_tsNetwork[i]->ReleaseObj();
        m_rtsp_tsNetwork[i] = NULL;
    }
#endif
#endif

#if defined(_ETS_SUPPORT_)
#if defined(_USE_TCPSERVER)
    for (i = 0; i < (_MAX_NETWORKS - 1) && m_etsServer[i] != NULL; i++)
    {
        m_etsServer[i]->ReleaseObj();
        m_etsServer[i] = NULL ;
    }
#else
    for (i = 0; i < (_MAX_NETWORKS - 1) && m_etsNetwork[i] != NULL; i++)
    {
        m_etsNetwork[i]->ReleaseObj();
        m_etsNetwork[i] = NULL;
    }
#endif
#endif

#if defined(_RTMP_SUPPORT_)
#if defined(_USE_TCPSERVER)
    for (i = 0; i < (_MAX_NETWORKS - 1) && m_rtmpServer[i] != NULL; i++)
    {
        m_rtmpServer[i]->ReleaseObj();
        m_rtmpServer[i] = NULL ;
    }
#else
    for (i = 0; i < (_MAX_NETWORKS - 1) && m_rtmpNetwork[i] != NULL; i++)
    {
        m_rtmpNetwork[i]->ReleaseObj();
        m_rtmpNetwork[i] = NULL;
    }
#endif
#endif

#if defined(_ENABLE_HTTPD_)
#if defined(_USE_TCPSERVER)
    for (i = 0; i < (_MAX_NETWORKS - 1) && m_httpdServer[i] != NULL; i++)
    {
        m_httpdServer[i]->ReleaseObj();
        m_httpdServer[i] = NULL;
    }
#else
    for (i = 0; i < (_MAX_NETWORKS - 1) && m_httpdNetwork[i] != NULL; i++)
    {
        m_httpdNetwork[i]->ReleaseObj();
        m_httpdNetwork[i] = NULL;
    }
#endif
    if (m_queueHttpd != NULL)
    {
        delete m_queueHttpd;
        m_queueHttpd = NULL;
    }

#endif	

#if defined(_HTTPTS_SUPPORT_)
#if defined(_USE_TCPSERVER)
    for (i = 0; i < (_MAX_NETWORKS - 1) && m_httpTsServer[i] != NULL; i++)
    {
        m_httpTsServer[i]->ReleaseObj();
        m_httpTsServer[i] = NULL;

    }
#else
    for (i = 0; i < (_MAX_NETWORKS - 1) && m_httptsNetwork[i] != NULL; i++)
    {
        m_httptsNetwork[i]->ReleaseObj();
        m_httptsNetwork[i] = NULL;
    }
#endif
    if (m_queueHttpTs != NULL)
    {
        delete   m_queueHttpTs;
        m_queueHttpTs = NULL;
    }

#endif




#if defined(_RTMP_SUPPORT_)
    if (m_rtmpLocal != NULL)
    {
        m_rtmpLocal->ReleaseObj();
        m_rtmpLocal = NULL;
    }
#endif



    if (m_queueDataProcess != NULL)
    {
        delete m_queueDataProcess;
        m_queueDataProcess = NULL;
    }

#if defined(_RTMP_SUPPORT_)
    if (m_queueNetworkEventRTMP != NULL)
    {
        delete m_queueNetworkEventRTMP;
        m_queueNetworkEventRTMP = NULL;
    }
#endif

#if defined(_ETS_SUPPORT_)
    if (m_queueNetworkEventETS != NULL)
    {
        delete m_queueNetworkEventETS;
        m_queueNetworkEventETS = NULL;
    }
#endif

#if defined(_RTSP_SUPPORT_)
    if (m_queueNetworkEventRTSP_TS != NULL)
    {
        delete m_queueNetworkEventRTSP_TS;
        m_queueNetworkEventRTSP_TS = NULL ;
    }
#endif

    if (m_queueNetworkClose != NULL)
    {
        delete m_queueNetworkClose;
        m_queueNetworkClose = NULL;
    }

#if defined(_MDSOURCE_MT)
    if (m_queueSendThread != NULL)
    {
        delete m_queueSendThread;
        m_queueSendThread = NULL;
    }
#endif

    if (m_queueCallback != NULL)
    {
        m_queueCallback->ReleaseObj();
        m_queueCallback = NULL;
    }



}
void CMediaSrvApp::ReloadSetting()
{

}
int CMediaSrvApp::GetOutTotalConnections()
{
    int total = 0;
    if (total < 0)
    {
        total = 0;
    }
    return   total;
}
int CMediaSrvApp::GetRtmpTotalConnections()
{
    int nCount = 0;
#if defined(_RTMP_SUPPORT_)
    int  i = 0;
#if defined(_USE_TCPSERVER)
    for (i = 0; i < _MAX_NETWORKS && m_rtmpServer[i] != NULL; i++)
    {

         nCount += m_rtmpServer[i]->GetConnections();
     

    }
#else
    for (i = 0; i < _MAX_NETWORKS && m_rtmpNetwork[i] != NULL; i++)
    {
         nCount += m_rtmpNetwork[i]->GetConnections();
    }
#endif
#endif

    return nCount;
}
int CMediaSrvApp::GetRtmpConnections()
{
	int nCount = 0;
#if defined(_RTMP_SUPPORT_)
	int  i = 0;
	#if defined(_USE_TCPSERVER)
		for (i = 0; i < _MAX_NETWORKS && m_rtmpServer[i] != NULL; i++)
		{
			if (m_rtmpServer[i]->m_pCfg->m_bPlayback && !m_rtmpServer[i]->m_pCfg->m_bExcludeConnections)
			{
				nCount += m_rtmpServer[i]->GetConnections();
			}

		}
	#else
		for (i = 0; i < _MAX_NETWORKS && m_rtmpNetwork[i] != NULL ; i++)
		{
			if (m_rtmpNetwork[i]->m_pCfg->m_bPlayback && !m_rtmpNetwork[i]->m_pCfg->m_bExcludeConnections)
			{
				nCount += m_rtmpNetwork[i]->GetConnections();
			}
		
		}
	#endif
#endif

	return nCount;
}
int CMediaSrvApp::GetEtsConnections()
{
	int nCount = 0;
#if defined(_ETS_SUPPORT_)
	int  i = 0;
	#if defined(_USE_TCPSERVER)
		for (i = 0; i < _MAX_NETWORKS && m_etsServer[i] != NULL; i++)
		{
			if (m_etsServer[i]->m_pCfg->m_bPlayback && !m_etsServer[i]->m_pCfg->m_bExcludeConnections)
			{
				nCount += m_etsServer[i]->GetConnections();
			}

		}
	#else
		for (i = 0; i < _MAX_NETWORKS && m_etsNetwork[i] != NULL; i++)
		{
			if (m_etsNetwork[i]->m_pCfg->m_bPlayback && !m_etsNetwork[i]->m_pCfg->m_bExcludeConnections)
			{
				nCount += m_etsNetwork[i]->GetConnections();
			}
		}
	#endif

#endif
	return nCount;
}
int CMediaSrvApp::GetHttpConnections()
{
	int nCount = 0;
#if defined(_ENABLE_HTTPD_)
	int  i = 0;
	#if defined(_USE_TCPSERVER)
		for (i = 0; i < _MAX_NETWORKS && m_httpdServer[i] != NULL; i++)
		{
			if (!m_httpdServer[i]->m_pCfg->m_bExcludeConnections)
			{
				nCount += m_httpdServer[i]->GetConnections();
			}
		}
	#else
		for (i = 0; i < _MAX_NETWORKS && m_httpdNetwork[i] != NULL; i++)
		{
			if (!m_httpdNetwork[i]->m_pCfg->m_bExcludeConnections)
			{
				nCount += m_httpdNetwork[i]->GetConnections();
			}
			
		}
	#endif
#endif
	return nCount;
}
int CMediaSrvApp::GetRtspTsConnections()
{
	int nCount = 0;
	int  i = 0;
#if defined(_RTSP_SUPPORT_)
	i = 0;
	#if defined(_USE_TCPSERVER)
		for (i = 0; i < _MAX_NETWORKS && m_rtspTsServer[i] != NULL; i++)
		{
			if (!m_rtspTsServer[i]->m_pCfg->m_bExcludeConnections)
			{
				nCount += m_rtspTsServer[i]->GetConnections();
			}

		}
	#else
		for (i = 0; i < _MAX_NETWORKS && m_rtsp_tsNetwork[i] != NULL; i++)
		{
			if (!m_rtsp_tsNetwork[i]->m_pCfg->m_bExcludeConnections)
			{
				nCount += m_rtsp_tsNetwork[i]->GetConnections();
			}
			
		}
	#endif
#endif

#if defined(_HTTPTS_SUPPORT_)
	i = 0;
	#if defined(_USE_TCPSERVER)
		for (i = 0; i < _MAX_NETWORKS && m_httpTsServer[i] != NULL; i++)
		{
			if (!m_httpTsServer[i]->m_pCfg->m_bExcludeConnections)
			{
				nCount += m_httpTsServer[i]->GetConnections();
			}

		}
	#else
		for (i = 0; i < _MAX_NETWORKS && m_httptsNetwork[i] != NULL; i++)
		{
			if (!m_httptsNetwork[i]->m_pCfg->m_bExcludeConnections)
			{
				nCount += m_httptsNetwork[i]->GetConnections();
			}
			
		}
	#endif
#endif
	return nCount;
}
void CMediaSrvApp::LoadETSNetwork()
{

}
void CMediaSrvApp::LoadETSNetworkTcpServer()
{


}
void CMediaSrvApp::LoadRTMPNetwork()
{
#if defined(_RTMP_SUPPORT_)&& !defined(_USE_TCPSERVER)
	int i = 0;
	int j = 0;
	for (i = 0; i < (_MAX_NETWORKS - 1); i++)
	{
		RTMPCFG cfg;
		memset(&cfg, 0, sizeof(cfg));
		if (!_AppConfig.LoadRTMP(&cfg, i))
		{
			break;
		}
		if (!cfg.enable || (!cfg.enable_rtmp  && !cfg.enable_rtmpt))
		{
			continue;
		}

		if (m_rtmpNetwork[j] == NULL)
		{
			m_rtmpNetwork[j] = CRTMPNetworkSrv::CreateObj();
		}


#if    defined(_PE_)
		m_rtmpNetwork[j]->SetMaxConnections(3);
#endif

		strcpyn(m_rtmpNetwork[j]->m_pCfg->m_szApp, cfg.app, sizeof(m_rtmpNetwork[j]->m_pCfg->m_szApp));
		if (cfg.chunksize < 0)
		{
			cfg.chunksize = 0;
		}
		if (cfg.chunksize > 65536)
		{
			cfg.chunksize = 65536;
		}
		m_rtmpNetwork[j]->m_Protocol = 0;
		if (cfg.enable_rtmp)
		{
			m_rtmpNetwork[j]->m_Protocol |= CRTMPNetworkSrv::RTMPProtocolRTMP;
		}
		if (cfg.enable_rtmpt)
		{
			m_rtmpNetwork[j]->m_Protocol |= CRTMPNetworkSrv::RTMPProtocolRTMPT;
		}
		m_rtmpNetwork[j]->m_pCfg->m_ChunkSize = cfg.chunksize;
		m_rtmpNetwork[j]->m_pCfg->m_bPlayback = cfg.playback;
		m_rtmpNetwork[j]->m_pCfg->m_bPublish = cfg.publish;
		m_rtmpNetwork[j]->m_pCfg->m_bRecordTs = cfg.record_ts;
		m_rtmpNetwork[j]->m_pCfg->m_bRecordMp4 = cfg.record_mp4;
		m_rtmpNetwork[j]->m_pCfg->m_bRecordFlv = cfg.record_flv;
		m_rtmpNetwork[j]->m_pCfg->m_bOutput_hls = cfg.output_hls;
		m_rtmpNetwork[j]->m_pCfg->m_bHlsOutputMemory = cfg.memory_file;
		m_rtmpNetwork[j]->m_pCfg->m_bForceHlsSetting = cfg.force_hls_setting;

		m_rtmpNetwork[j]->m_pCfg->m_hls_dir->m_var.SetString(cfg.hls_dir);
		m_rtmpNetwork[j]->m_pCfg->m_hls_ts_prefix->m_var.SetString(cfg.hls_ts_prefix);
		m_rtmpNetwork[j]->m_pCfg->m_record_ts_dir->m_var.SetString(cfg.record_ts_dir);
		m_rtmpNetwork[j]->m_pCfg->m_record_mp4_dir->m_var.SetString(cfg.record_mp4_dir);
		m_rtmpNetwork[j]->m_pCfg->m_record_flv_dir->m_var.SetString(cfg.record_flv_dir);
		m_rtmpNetwork[j]->m_pCfg->m_bUseSettingSrv = cfg.use_setting_srv;
		m_rtmpNetwork[j]->m_pCfg->m_bEnable_virtual_live = cfg.enable_virtual_live;
		m_rtmpNetwork[j]->m_pCfg->m_bForcePlayback = cfg.force_playback;
		m_rtmpNetwork[j]->m_pCfg->m_bExcludeConnections = cfg.exclude_connections;




		m_rtmpNetwork[j]->SetSessionTimeout(_AppConfig.m_nSessionTimeout);

		if (!m_rtmpNetwork[j]->BindAddr(cfg.port, cfg.bind_addr))
		{
			LogPrint(_LOG_LEVEL_ERROR, _TAGNAME, "can't bind RTMP%d port at [%s:%d] <%s>\n", i, cfg.bind_addr, cfg.port, (const char *)CObjNetError());
			m_rtmpNetwork[j]->ReleaseObj();
			m_rtmpNetwork[j] = 0;
			continue;
		}



		m_rtmpNetwork[j]->m_index = i;

		j++;
		m_rtmpCount = j;

	}
#endif
}
void CMediaSrvApp::LoadRTMPNetworkTcpServer()
{

}
void CMediaSrvApp::LoadHTTPDNetwork()
{
#if defined(_ENABLE_HTTPD_) && !defined(_USE_TCPSERVER)
	int i = 0;
	int j = 0;
	for (i = 0; i < (_MAX_NETWORKS - 1); i++)
	{
		HTTPDCFG cfg;
		memset(&cfg, 0, sizeof(cfg));
		if (!_AppConfig.LoadHTTPD(&cfg, i))
		{
			break;
		}
		if (!cfg.enable)
		{
			continue;
		}
		if (m_httpdNetwork[j] == NULL)
		{
			m_httpdNetwork[j] = CHttpdNetworkSrv::CreateObj();
		}

		m_httpdNetwork[j]->SetSessionTimeout(_AppConfig.m_nSessionTimeout);




		m_httpdNetwork[j]->m_pCfg->m_wwwRoot->m_var.SetString(cfg.www_root);
		m_httpdNetwork[j]->m_pCfg->m_bExcludeConnections = cfg.exclude_connections;




		if (!m_httpdNetwork[j]->BindAddr(cfg.port, cfg.bind_addr))
		{
			LogPrint(_LOG_LEVEL_ERROR, _TAGNAME, "can't bind httpd%d port at [%s:%d] <%s>\n", i, cfg.bind_addr, cfg.port, (const char *)CObjNetError());
			m_httpdNetwork[j]->ReleaseObj();
			m_httpdNetwork[j] = 0;
			continue;
		}



		m_httpdNetwork[j]->m_index = i;
		m_httpdNetwork[j]->m_pCfg->m_keepalive = cfg.keepalive;
        m_httpdNetwork[j]->m_pCfg->m_enable206 = cfg.enable206;

		j++;
		m_httpdCount = j;
	}

#endif
}
void CMediaSrvApp::LoadHTTPDNetworkTcpServer()
{

}
void CMediaSrvApp::LoadHTTPTSNetwork()
{

}
void CMediaSrvApp::LoadHTTPTSNetworkTcpServer()
{

}
void CMediaSrvApp::LoadRTSPNetwork()
{
#if defined(_RTSP_SUPPORT_) && !defined(_USE_TCPSERVER)
	int i = 0;
	int j = 0;
	for (i = 0; i < (_MAX_NETWORKS - 1); i++)
	{
		RTSPTSCFG cfg;
		memset(&cfg, 0, sizeof(cfg));
		if (!_AppConfig.LoadRTSP(&cfg, i))
		{
			break;
		}
		if (!cfg.enable)
		{
			continue;
		}
		if (m_rtsp_tsNetwork[j] == NULL)
		{
			m_rtsp_tsNetwork[j] = CRTSPNetworkSrv::CreateObj();
		}

		m_rtsp_tsNetwork[j]->SetSessionTimeout(_AppConfig.m_nSessionTimeout);




		m_rtsp_tsNetwork[j]->m_pCfg->m_bForceHttpTsPlay = cfg.force_http_ts_play;
		m_rtsp_tsNetwork[j]->m_pCfg->m_bForceRtspPlay = cfg.force_rtsp_play;
		m_rtsp_tsNetwork[j]->m_pCfg->m_bExcludeConnections = cfg.exclude_connections;




		if (!m_rtsp_tsNetwork[j]->BindAddr(cfg.port, cfg.bind_addr))
		{
			LogPrint(_LOG_LEVEL_ERROR, _TAGNAME, "can't bind rtsp-ts%d port at [%s:%d] <%s>\n", i, cfg.bind_addr, cfg.port, (const char *)CObjNetError());
			m_rtsp_tsNetwork[j]->ReleaseObj();
			m_rtsp_tsNetwork[j] = 0;
			continue;
		}


		m_rtsp_tsNetwork[j]->m_index = i;

		j++;
		m_rtspCount = j;
	}
#endif
}
void CMediaSrvApp::LoadRTSPNetworkTcpServer()
{

}
CObjNetAsync *CMediaSrvApp::GetCdnNet()
{
#if defined(_ETS_SUPPORT_)

	#if defined(_USE_TCPSERVER)
	if (m_etsServer[0] == NULL)
	{
		return NULL ;
	}
		return m_etsServer[0]->GetIdleNet();
	#else
		return m_etsNetwork[0];
	#endif
#else
	return NULL;
#endif
}
BOOL CMediaSrvApp::StartNetworkAndQueue()
{
	int i = 0 ;

	m_httptsCount = 0;
	m_httpdCount = 0;
	m_etsCount = 0;
	m_rtmpCount = 0;
	m_rtspCount = 0;
	






	if (m_queueSettingSrvNotify == NULL)
	{
        m_queueSettingSrvNotify = new CCallbackProcessor(1, 0, 5000);
		m_queueSettingSrvNotify->SetName("m_queueSettingSrvNotify");
	}

	m_queueSettingSrvNotify->Start();






	if( m_queueDiskIo == NULL )
	{
		m_queueDiskIo = new CCallbackProcessor(_AppConfig.m_nDiskIoThreads, 0, _DEFAULT_QUEUE_TIMEOUT);
		m_queueDiskIo->SetName("m_queueDiskIo");
	}
	m_queueDiskIo->Start();

	if (m_queueNetworkClose == NULL)
	{
		m_queueNetworkClose = new CCallbackProcessor(_AppConfig.m_nNetworkevent_threadsClose, 0, _DEFAULT_QUEUE_TIMEOUT);
		m_queueNetworkClose->SetName("m_queueNetworkClose");
	}
	m_queueNetworkClose->Start();

	

	if (m_queueDataProcess == NULL)
	{
		m_queueDataProcess = new CObjEventQueue(_AppConfig.m_nDataProcessThread, 0, _DEFAULT_QUEUE_TIMEOUT);
		m_queueDataProcess->SetName("m_queueDataProcess");
	}
	m_queueDataProcess->Start();



	


#if defined(_RTMP_SUPPORT_)
    if( _AppConfig.m_funcStreamPushStart != NULL )
    {
        if( m_rtmpLocal == NULL )
        {
            LOCALSTREAM *cfg = &(_AppConfig.m_localStream);
            m_rtmpLocal = CRTMPNetworkSrv::CreateObj( );
			m_rtmpLocal->m_pCfg->m_bPlayback = FALSE;
			m_rtmpLocal->m_pCfg->m_bPublish = TRUE;

			m_rtmpLocal->m_pCfg->m_bRecordTs = cfg->record;
			m_rtmpLocal->m_pCfg->m_record_ts_dir->m_var.SetString(cfg->record_dir);

			m_rtmpLocal->m_pCfg->m_bRecordMp4 = cfg->record_mp4;
			m_rtmpLocal->m_pCfg->m_record_mp4_dir->m_var.SetString(cfg->record_mp4_dir);


			m_rtmpLocal->m_pCfg->m_bOutput_hls = cfg->output_hls;
			m_rtmpLocal->m_pCfg->m_bHlsOutputMemory = cfg->memory_file;
			m_rtmpLocal->m_pCfg->m_hls_dir->m_var.SetString(cfg->hls_dir);
			m_rtmpLocal->m_pCfg->m_hls_ts_prefix->m_var.SetString(cfg->hls_ts_prefix);
            
            m_rtmpLocal->SetSessionTimeout( _AppConfig.m_nSessionTimeout );
        }
        if( ! m_rtmpLocal->BindAddr( 0, "127.0.0.1") )
        {
            LogPrint(_LOG_LEVEL_ERROR ,_TAGNAME,"can't bind Local stream port\n" , (const char *)CObjNetError() );
            return FALSE;
        }
        m_rtmpLocal->Start( );

    }
#endif


#if defined(_ETS_SUPPORT_)
	#if defined(_USE_TCPSERVER)
	LoadETSNetworkTcpServer();
	#else
	LoadETSNetwork();
	#endif
#endif


#if defined(_RTMP_SUPPORT_)
	#if defined(_USE_TCPSERVER)
	LoadRTMPNetworkTcpServer();
	#else
	LoadRTMPNetwork();
	#endif
#endif
	
	
#if defined(_ENABLE_HTTPD_)
	#if defined(_USE_TCPSERVER)
		LoadHTTPDNetworkTcpServer();
	#else
		LoadHTTPDNetwork();
	#endif
#endif


#if defined(_HTTPTS_SUPPORT_) 
	#if defined(_USE_TCPSERVER)
		LoadHTTPTSNetworkTcpServer();
	#else
		LoadHTTPTSNetwork();
	#endif
#endif

#if defined(_RTSP_SUPPORT_) 
	#if defined(_USE_TCPSERVER)
		LoadRTSPNetworkTcpServer();
	#else
		LoadRTSPNetwork();
	#endif
#endif

////////////////////////////////////////////////////////////////////////////////////////////////


#if defined(_RTMP_SUPPORT_)
		if (_AppConfig.m_funcStreamPushStart != NULL || m_rtmpCount > 0 )
		{
			if (m_queueNetworkEventRTMP == NULL)
			{
				m_queueNetworkEventRTMP = new CCallbackProcessor(_AppConfig.m_nNetworkevent_threadsRTMP, 0, _DEFAULT_QUEUE_TIMEOUT);
				m_queueNetworkEventRTMP->SetName("m_queueNetworkEventRTMP");
			}
			m_queueNetworkEventRTMP->Start();
		}
#endif


#if defined(_ETS_SUPPORT_)
		if (m_etsCount > 0)
		{
			if (m_queueNetworkEventETS == NULL)
			{
				m_queueNetworkEventETS = new CCallbackProcessor(_AppConfig.m_nNetworkevent_threadsETS, 0, _DEFAULT_QUEUE_TIMEOUT);
				m_queueNetworkEventETS->SetName("m_queueNetworkEventETS");
			}
			m_queueNetworkEventETS->Start();
		}

#endif

#if defined(_RTSP_SUPPORT_)
		if (m_rtspCount > 0)
		{
			if (m_queueNetworkEventRTSP_TS == NULL)
			{
				m_queueNetworkEventRTSP_TS = new CCallbackProcessor(_AppConfig.m_nNetworkevent_threadsRTSP_TS, 0, _DEFAULT_QUEUE_TIMEOUT);
				m_queueNetworkEventRTSP_TS->SetName("m_queueNetworkEventRTSP_TS");
			}
			m_queueNetworkEventRTSP_TS->Start();
		}

#endif




#if defined(_ENABLE_HTTPD_)
		if (m_httpdCount > 0)
		{
			if (m_queueHttpd == NULL)
			{
				m_queueHttpd = new CCallbackProcessor(_AppConfig.m_nNetworkevent_threadsHTTPD, 0, _DEFAULT_QUEUE_TIMEOUT);
				m_queueHttpd->SetName("m_queueHttpd");
			}
			m_queueHttpd->Start();
		}

#endif


#if defined(_HTTPTS_SUPPORT_)
		if (m_httptsCount > 0)
		{
			if (m_queueHttpTs == NULL)
			{
				m_queueHttpTs = new CCallbackProcessor(_AppConfig.m_nNetworkevent_threadsHTTP_TS, 0, _DEFAULT_QUEUE_TIMEOUT);
				m_queueHttpTs->SetName("m_queueHttpTs");
			}
			m_queueHttpTs->Start();
		}

#endif






//////////////////////////////////////////////////////////////////////////////////////////////

#if defined(_RTSP_SUPPORT_)
	#if defined(_USE_TCPSERVER)
		for (i = 0; i < (_MAX_NETWORKS - 1) && m_rtspTsServer[i] != NULL; i++)
		{
			if (!m_rtspTsServer[i]->Start())
			{
				LogPrint(_LOG_LEVEL_ERROR, _TAGNAME, "can't start rtsp-ts%d service\n", m_rtspTsServer[i]->m_index);
			}
		}
	#else
		for (i = 0; i < (_MAX_NETWORKS - 1) && m_rtsp_tsNetwork[i] != NULL; i++)
		{
			if (!m_rtsp_tsNetwork[i]->Start())
			{
				LogPrint(_LOG_LEVEL_ERROR, _TAGNAME, "can't start rtsp-ts%d service\n", m_rtsp_tsNetwork[i]->m_index);
			}
		}
	#endif
#endif

#if defined(_ETS_SUPPORT_)
	#if defined(_USE_TCPSERVER)
		for (i = 0; i < (_MAX_NETWORKS - 1) && m_etsServer[i] != NULL; i++)
		{
			if( !m_etsServer[i]->Start() )
			{
				LogPrint(_LOG_LEVEL_ERROR ,_TAGNAME,"can't start ETS%d service\n", m_etsServer[i]->m_index);
			}
		}
	#else
		for( i = 0 ; i < ( _MAX_NETWORKS -1 )  && m_etsNetwork[i] != NULL ; i ++ )
		{
			if( !m_etsNetwork[i]->Start() )
			{
				LogPrint(_LOG_LEVEL_ERROR ,_TAGNAME,"can't start ETS%d service\n", m_etsNetwork[i]->m_index);
			}
		}
	#endif
#endif

#if defined(_RTMP_SUPPORT_)
	#if defined(_USE_TCPSERVER)
		for (i = 0; i < (_MAX_NETWORKS - 1) && m_rtmpServer[i] != NULL; i++)
		{
			if( !m_rtmpServer[i]->Start() )
			{
				LogPrint(_LOG_LEVEL_ERROR ,_TAGNAME,"can't start RTMP%d service\n", m_rtmpServer[i]->m_index);
			}
		}
	#else
		for( i = 0 ; i < ( _MAX_NETWORKS -1 )  && m_rtmpNetwork[i] != NULL ; i ++ )
		{
			if( !m_rtmpNetwork[i]->Start() )
			{
				LogPrint(_LOG_LEVEL_ERROR ,_TAGNAME,"can't start RTMP%d service\n", m_rtmpNetwork[i]->m_index);
			}
		}
	#endif
#endif

#if defined(_ENABLE_HTTPD_)
	#if defined(_USE_TCPSERVER)
		for (i = 0; i < (_MAX_NETWORKS - 1) && m_httpdServer[i] != NULL; i++)
		{
			if( !m_httpdServer[ i ]->Start( ) )
			{
				LogPrint(_LOG_LEVEL_ERROR ,_TAGNAME, "can't start httpd%d service\n", m_httpdServer[ i ]->m_index );
			}
		}
	#else
		for( i = 0; i < ( _MAX_NETWORKS - 1 ) && m_httpdNetwork[ i ] != NULL; i++ )
		{
			if( !m_httpdNetwork[ i ]->Start( ) )
			{
				LogPrint(_LOG_LEVEL_ERROR ,_TAGNAME, "can't start httpd%d service\n", m_httpdNetwork[ i ]->m_index );
			}
		}
	#endif

#endif

#if defined(_HTTPTS_SUPPORT_)
	#if defined(_USE_TCPSERVER)
		for (i = 0; i < (_MAX_NETWORKS - 1) && m_httpTsServer[i] != NULL; i++)
		{
			if (!m_httpTsServer[i]->Start())
			{
				LogPrint(_LOG_LEVEL_ERROR, _TAGNAME, "can't start http-ts%d service\n", m_httpTsServer[i]->m_index);
			}
		}
	#else
		for (i = 0; i < (_MAX_NETWORKS - 1) && m_httptsNetwork[i] != NULL; i++)
		{
			if (!m_httptsNetwork[i]->Start())
			{
				LogPrint(_LOG_LEVEL_ERROR, _TAGNAME, "can't start http-ts%d service\n", m_httptsNetwork[i]->m_index);
			}
		}
	#endif
#endif

#if defined(_RTMP_SUPPORT_)
    if( _AppConfig.m_funcStreamPushStart != NULL && m_rtmpLocal != NULL )
    {
        char szRtmpUrl[400 ];
        snprintf( szRtmpUrl, sizeof( szRtmpUrl )-4  , "rtmp://127.0.0.1:%d/live/",m_rtmpLocal->GetBindPort() );

        _AppConfig.m_funcStreamPushStart( szRtmpUrl, _AppConfig.m_szStreamPushIniPath );
    }
#endif
	

#if defined(_CDNNET_SUPPORT_)
    if (_AppConfig.m_cfgCdnSrv.enable)
    {
        if (m_cdnNetwork == NULL)
        {
            m_cdnNetwork = CCdnNetwork::CreateObj();
        }
        m_cdnNetwork->Start();

        CCdnSrvDelayConnect *item = CCdnSrvDelayConnect::CreateObj();
        item->m_bOneShot = TRUE;
        item->m_nInterval = 3000;
        if (!_MediasrvApp->m_namedTimer.AddItem("cdn_boot", item))
        {
            item->ReleaseObj();

        }
    } 
#endif

	return TRUE ;

}

void CMediaSrvApp::StopNetworkAndQueue()
{
	int i = 0 ;


#if defined(_GPS_SUPPORT_)
    StopGps();
#endif
    if( _AppConfig.m_funcStreamPushStop != NULL )
    {
        _AppConfig.m_funcStreamPushStop();
    }



 
    if (m_queueSettingSrvNotify != NULL)
    {
        m_queueSettingSrvNotify->Shutdown();
    }

#if defined(_CDNNET_SUPPORT_)
    if (m_cdnNetwork != NULL)
    {
        m_cdnNetwork->Shutdown();
    }
    PerfStop();
#endif

#if defined(_RTSP_SUPPORT_)
	#if defined(_USE_TCPSERVER)
		for (i = 0; i < (_MAX_NETWORKS - 1) && m_rtspTsServer[i] != NULL; i++)
		{
			m_rtspTsServer[i]->Shutdown();
		}
	#else
		for (i = 0; i < (_MAX_NETWORKS - 1) && m_rtsp_tsNetwork[i] != NULL; i++)
		{
			m_rtsp_tsNetwork[i]->Shutdown();
		}
	#endif
#endif

#if defined(_ETS_SUPPORT_)
	#if defined(_USE_TCPSERVER)
		for( i = 0 ; i < ( _MAX_NETWORKS -1 )  && m_etsServer[i] != NULL ; i ++ )
		{
			m_etsServer[i]->Shutdown() ;
		}
	#else
		for( i = 0 ; i < ( _MAX_NETWORKS -1 )  && m_etsNetwork[i] != NULL ; i ++ )
		{
			m_etsNetwork[i]->Shutdown() ;
		}
	#endif
#endif

#if defined(_RTMP_SUPPORT_)
	#if defined(_USE_TCPSERVER)
		for( i = 0 ; i < ( _MAX_NETWORKS -1 )  && m_rtmpServer[i] != NULL ; i ++ )
		{
			m_rtmpServer[i]->Shutdown() ;
		}
	#else
		for( i = 0 ; i < ( _MAX_NETWORKS -1 )  && m_rtmpNetwork[i] != NULL ; i ++ )
		{
			m_rtmpNetwork[i]->Shutdown() ;
		}
	#endif
#endif

#if defined(_ENABLE_HTTPD_)
	#if defined(_USE_TCPSERVER)
		for( i = 0; i < ( _MAX_NETWORKS - 1 ) && m_httpdServer[ i ] != NULL; i++ )
		{
			m_httpdServer[ i ]->Shutdown( );
		}
	#else
		for( i = 0; i < ( _MAX_NETWORKS - 1 ) && m_httpdNetwork[ i ] != NULL; i++ )
		{
			m_httpdNetwork[ i ]->Shutdown( );
		}
	#endif
	if (m_queueHttpd != NULL)
	{
		m_queueHttpd->Shutdown();
	}

#endif	

#if defined(_HTTPTS_SUPPORT_)
	#if defined(_USE_TCPSERVER)
		for (i = 0; i < (_MAX_NETWORKS - 1) && m_httpTsServer[i] != NULL; i++)
		{
			m_httpTsServer[i]->Shutdown();
		}
	#else
		for (i = 0; i < (_MAX_NETWORKS - 1) && m_httptsNetwork[i] != NULL; i++)
		{
			m_httptsNetwork[i]->Shutdown();
		}
	#endif
	if (m_queueHttpTs != NULL)
	{
		m_queueHttpTs->Shutdown();
	}
	
#endif




#if defined(_RTMP_SUPPORT_)
	if (m_rtmpLocal != NULL)
	{
		m_rtmpLocal->Shutdown();
	}
#endif

	if( m_queueDiskIo != NULL )
	{
		m_queueDiskIo->Shutdown();
	}


	if (m_queueDataProcess != NULL)
	{
		m_queueDataProcess->Shutdown();
	}

#if defined(_RTMP_SUPPORT_)
	if (m_queueNetworkEventRTMP != NULL)
	{
		m_queueNetworkEventRTMP->Shutdown();
	}
#endif

#if defined(_ETS_SUPPORT_)
	if (m_queueNetworkEventETS != NULL)
	{
		m_queueNetworkEventETS->Shutdown();
	}
#endif

#if defined(_RTSP_SUPPORT_)
	if (m_queueNetworkEventRTSP_TS != NULL)
	{
		m_queueNetworkEventRTSP_TS->Shutdown();
	}
#endif
	
	if (m_queueNetworkClose != NULL)
	{
		m_queueNetworkClose->Shutdown();
	}

#if defined(_MDSOURCE_MT)
	if (m_queueSendThread != NULL)
	{
		m_queueSendThread->Shutdown();
	}
#endif

	if (m_queueCallback != NULL)
	{
		m_queueCallback->Shutdown();
	}


}
void CMediaSrvApp::ParseCmdline(const char *szCmdline)
{

}
int  CMediaSrvApp::Run()
{
	TIME_T tmPre = 0;
	char szBuf[4096];
	memset(szBuf,0,sizeof(szBuf));
	if( !StartNetworkAndQueue() )
	{
		return -1;
	}

	while( TRUE )
	{
		printf(">>");
		memset(szBuf,0,sizeof(szBuf));
		tmPre = OSTickCount() ;
		fgets(szBuf, sizeof(szBuf)-1, stdin);
		if(  ( OSTickCount()  - tmPre ) < 10 )
		{
			SleepMilli( 10 );
		}
		if( szBuf[0] == 0 )
		{
			continue ;
		}
		else if( strcmpix(szBuf,"exit") == 0 )
		{
			break;
		}
        else if( strcmpix( szBuf, "stoppub" ) == 0 )
        {
            if( _AppConfig.m_funcStreamPushStop != NULL )
            {
                _AppConfig.m_funcStreamPushStop( );
            }
        }
#if defined(_RTMP_SUPPORT_)
        else if( strcmpix( szBuf, "startpub" ) == 0 )
        {
            if( _AppConfig.m_funcStreamPushStart != NULL && m_rtmpLocal != NULL )
            {
                char szRtmpUrl[ 400 ];
                snprintf( szRtmpUrl, sizeof( szRtmpUrl )-4, "rtmp://127.0.0.1:%d/live/", m_rtmpLocal->GetBindPort( ) );

                _AppConfig.m_funcStreamPushStart( szRtmpUrl, _AppConfig.m_szStreamPushIniPath );
            }
        }
#endif




	}
	StopNetworkAndQueue();
	return 0;
}
void CMediaSrvApp::Stop()
{

}




























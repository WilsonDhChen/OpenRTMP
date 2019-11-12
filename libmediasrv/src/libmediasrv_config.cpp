 

#include "libmediasrv_def.h"
#if defined(_AUDIO_CONVERT_)
#include "libxencoder.h"
#endif

_UdpTsSendFrame _udpTsSendFrame = NULL;
CStreamSetting::CStreamSetting()
{
	m_bHls = FALSE;
	m_bHlsOutputMemory = FALSE;
	m_bRecord = FALSE;
	m_bOutRTSP = FALSE;
	m_bOutRTMP = FALSE;
	m_bOutHTTP_TS = FALSE;
	m_nRecordDuration = 0;
	m_recordFormat = RecordFormatTS;


	m_hlsDir.Attach(CObjVarRef::CreateObj());
	m_hlsTsPrefix.Attach(CObjVarRef::CreateObj());
	m_recordDir.Attach(CObjVarRef::CreateObj());
#if defined(_HLS_SUPPORT_)
	memset(&m_hlscfg, 0, sizeof(m_hlscfg));
#endif
	

}
CStreamSetting::~CStreamSetting()
{

}
CStreamSetting::RecordFormat CStreamSetting::String2RecordFormat(const char *str)
{
	if (strcmpix(str, "mp4") == 0)
	{
		return RecordFormatMP4;
	}
	else if (strcmpix(str, "flv") == 0)
	{
		return RecordFormatFLV;
	}
	else if (strcmpix(str, "ts") == 0)
	{
		return RecordFormatTS;
	}
	else
	{
		return RecordFormatUnknown;
	}
}

WorkMode Str2WorkMode(const char *str)
{
	if (strcmpix(str, "cdn") == 0)
	{
		return WorkModeCdn;
	}
	else if (strcmpix(str, "slave") == 0)
	{
		return WorkModeSlave;
	}
	else if (strcmpix(str, "mixed") == 0)
	{
		return WorkModeMixed;
	}
	else
	{
		return WorkModeMaster;
	}
}
const char *WorkMode2Str(WorkMode mode)
{
    switch (mode)
    {
    case  WorkModeCdn:
    {
        static char cdn[] = "cdn";
        return cdn;
    }
    case  WorkModeMaster:
    {
        static char master[] = "master";
        return master;
    }
    case WorkModeSlave:
    {
        static char slave[] = "slave";
        return slave;
    }
    case WorkModeMixed:
    {
        static char mixed[] = "mixed";
        return mixed;
    }
    default:
    {
        static char unknown[] = "unknown";
        return unknown;
    }
        break;
    }
}
////////////////////////////////////////////////////////////////////////////////
static int __LogPrintFuncMediasrv( int prio, const char *tag,const char *fmt, va_list ap );
CConfigIni _AppConfig;
OBJECTS_COUNT _ObjCount;




int ConfigSetRecordType(const char *type)
{
    _AppConfig.m_luaRecordType = type;

    return 0;

}
int ConfigSetRecordDuration(int seconds)
{
    _AppConfig.m_luaRecordDuration = seconds;
    return 0;
}
int ConfigSetRecordFormat(const char *format)
{
    _AppConfig.m_luaRecordFormat = format;
    return 0;
}

int ConfigSetRecordTsPath(const char *path)
{
    _AppConfig.m_luaRecordTsPath = path;
    return 0;
}
int ConfigSetRecordMp4Path(const char *path)
{
    _AppConfig.m_luaRecordMp4Path = path;
    return 0;
}
int ConfigSetRecordFlvPath(const char *path)
{
    _AppConfig.m_luaRecordFlvPath = path;
    return 0;
}

int ConfigSetRecordTsEnable(BOOL enable)
{
    _AppConfig.m_luaRecordTsEnable = enable;
    return 0;
}
int ConfigSetRecordMp4Enable(BOOL enable)
{
    _AppConfig.m_luaRecordMp4Enable = enable;
    return 0;
}
int ConfigSetRecordFlvEnable(BOOL enable)
{
    _AppConfig.m_luaRecordFlvEnable = enable;
    return 0;
}
CConfigIni::CConfigIni()
{

    m_bTs_mpegts = FALSE;
    m_nFlash_interactive_time = 60;
    m_nSocketRecvBuf = SIZE_1M;
    m_nSocketSendBuf = SIZE_1M;
    m_bTsForceVideoAudio = FALSE;
    m_bHttpTsEnableText = FALSE;
    m_bVideoUseAudioTime = FALSE;
	m_bCallbackEnable = FALSE;
	m_nHlsAudioWaitVideoSeconds = 2;
	m_nNetworkevent_threadsRTMP = 0 ;
	m_nNetworkevent_threadsETS = 0 ;
	m_nNetworkevent_threadsRTSP_TS = 0 ;
	m_nNetworkevent_threadsClose = 0;
	m_nCallback_threads = 0;

	m_nMaxCachedMem = 0;
	m_nMaxSends = 2000;
	m_nMaxClientsPerList = 500;
	m_nSendThreads = 10;
	m_nMaxWrittingBytes = SIZE_1M;
	m_bDemoVersion = FALSE;
	m_bOverdue = FALSE;

	m_recordFormat = CStreamSetting::RecordFormatTS;
	m_nMaxVideoCacheTime = 3000;
	m_mode = WorkModeMaster;
    m_nDelayclose_virtual_live_seconds = 10;
	

    m_bTs_record_pcr_video = FALSE;
    m_bTs_hls_pcr_video = FALSE;
    m_bTs_http_pcr_video = FALSE;

	memset(m_szMachineId, 0, sizeof(m_szMachineId));
	memset( m_szPluginPath, 0 , sizeof(m_szPluginPath)  );
    memset(m_szRtspRecordPath, 0, sizeof(m_szRtspRecordPath));
	memset(m_szCallbackPluginPath, 0, sizeof(m_szCallbackPluginPath));
	

    memset(&m_cfgCdnSrv, 0, sizeof(m_cfgCdnSrv));
    m_cfgCdnSrv.search_stream_time = 10;
    
#if defined(_HLS_SUPPORT_)
	memset( &m_hlsCfg , 0 , sizeof(m_hlsCfg)  );
#endif
	memset(&m_cfgSrv, 0, sizeof(m_cfgSrv));

	
	memset(&m_key, 0, sizeof(m_key));
	
	
    memset( m_szStreamPushPluginPath, 0, sizeof( m_szStreamPushPluginPath ) );
    memset( m_szStreamPushIniPath, 0, sizeof( m_szStreamPushIniPath ) );

    memset( &m_localStream, 0, sizeof( m_localStream ) );
    memset(&m_szGpsBandrate, 0, sizeof(m_szGpsBandrate));

    
    
    m_tmStarted = OSTickCount();


    m_bTs_enable_sdt = FALSE;
	m_bEnablePCR = TRUE ;
	m_nRtspPort = 0 ;
	m_nRecordDuration = 1800 ;
    m_bLogDebug = FALSE ;

	m_nSessionTimeout = 0 ;
	m_bUseClientID = TRUE ;
    m_bRtspRecord = FALSE;

    m_funcStreamPushStart = NULL ;

	m_nRecordType = _RECORD_TYPE_NVR;

	m_nDiskIoThreads = 1 ;
	m_nNetworkevent_threadsHTTPD = 1;
	m_nNetworkevent_threadsHTTP_TS = 1;
	m_nDataProcessThread = 1;
	m_bNeedNoop = FALSE ;
	m_bBreaked = FALSE;
	SetAppName( _APP_NAME );
	SetCompanyName( _COMPANY_NAME );

	

	SystemTime(&m_systmBoot, FALSE);

	m_nRecordReserveSpace = 512;
	
	m_tmLogNoSpace = 0;

	m_bCacheKeyFrame = TRUE;
    m_bDebug_speex = FALSE;

    m_szGpsPluginPath[0] = 0 ;
    m_szGpsDev[0] = 0;
    m_bEnableGps = FALSE ;
}
CConfigIni::~CConfigIni()
{
#if defined(_ENABLE_MEMORY_FILE_)
    CMemoryFile::CleanUp();
#endif
}
void CConfigIni::LoadBaseIni()
{
    char szTmp[200];
    IniReadStr(m_szAppName, "plugin", m_szPluginPath, sizeof(m_szPluginPath), m_szPathIniFile);
    IniReadStr(m_szAppName, "callback_plugin", m_szCallbackPluginPath, sizeof(m_szCallbackPluginPath), m_szPathIniFile);
    m_bCallbackEnable = IniReadBool(m_szAppName, "callback_enable", m_szPathIniFile, FALSE);



    IniReadStr(m_szAppName, "gps_plugin", m_szGpsPluginPath, sizeof(m_szGpsPluginPath), m_szPathIniFile);
    IniReadStr(m_szAppName, "gps_dev", m_szGpsDev, sizeof(m_szGpsDev), m_szPathIniFile);
    IniReadStr(m_szAppName, "gps_bandrate", m_szGpsBandrate, sizeof(m_szGpsBandrate), m_szPathIniFile);
    
    m_bEnableGps = IniReadBool(m_szAppName, "gps_enable", m_szPathIniFile, FALSE);



    IniReadStr(m_szAppName, "mode", szTmp, sizeof(szTmp), m_szPathIniFile);
    m_mode = Str2WorkMode(szTmp);
    LogPrint(_LOG_LEVEL_INFO, _TAGNAME, "work mode is %s\n", WorkMode2Str(m_mode));

    IniReadStr(m_szAppName, "master_addr", szTmp, sizeof(szTmp), m_szPathIniFile);
    RDE_IP_PORT(szTmp, m_masterAddr.szAddr, sizeof(m_masterAddr.szAddr), &(m_masterAddr.nPort));
    if (m_masterAddr.nPort == 0)
    {
        m_masterAddr.nPort = _ETS_PORT;
    }
    

    m_nMaxCachedMem = IniReadInt(m_szAppName, "cached_memory", m_szPathIniFile, 0);
    if (m_nMaxCachedMem < 0)
    {
        m_nMaxCachedMem = 0;
    }
    m_nMaxCachedMem *= SIZE_1M;

    m_bLogDebug = IniReadBool(m_szAppName, "log_debug", m_szPathIniFile, FALSE);

}
void CConfigIni::LoadIni()
{
	char szTmp[200];
	
    IniReadStr("rtsp", "record_dir", m_szRtspRecordPath, sizeof(m_szRtspRecordPath), m_szPathIniFile);
    m_bRtspRecord = IniReadBool("rtsp", "record", m_szPathIniFile, FALSE);





	m_nRtspPort = IniReadInt(m_szAppName,"port",m_szPathIniFile,554);

	m_nMaxClientsPerList = IniReadInt(m_szAppName, "max_clients_per_thread", m_szPathIniFile, 1);

	if (m_nMaxClientsPerList < 1)
	{
		m_nMaxClientsPerList = 500;
	}

    m_nDelayclose_virtual_live_seconds = IniReadInt(m_szAppName, "delayclose_virtual_live_seconds", m_szPathIniFile, 10);

	m_nMaxSends = IniReadInt(m_szAppName, "max_sends", m_szPathIniFile, 1000);

	if (m_nMaxSends < 1)
	{
		m_nMaxSends = 1000;
	}

	m_nMaxWrittingBytes = IniReadInt(m_szAppName, "max_writting_bytes", m_szPathIniFile, 1024);

	if (m_nMaxWrittingBytes != -1)
	{
		if (m_nMaxWrittingBytes < 1)
		{
			m_nMaxWrittingBytes = 1024;
		}
		m_nMaxWrittingBytes *= 1024;
		
	}

	


	m_nMaxVideoCacheTime = IniReadInt(m_szAppName, "max_video_cache_time", m_szPathIniFile, 2000);
	if (m_nMaxVideoCacheTime < 0)
	{
		m_nMaxVideoCacheTime = 0;
	}
	
		 
	m_nSendThreads = IniReadInt(m_szAppName, "send_threads", m_szPathIniFile, 1);
	if (m_nSendThreads < 1)
	{
		m_nSendThreads = MinInt( CpuCoreNumber(),8);
	}

	m_nDiskIoThreads = IniReadInt(m_szAppName,"disk_io_threads",m_szPathIniFile,1);
	
	if( m_nDiskIoThreads < 1 )
	{
		m_nDiskIoThreads = 1 ;
	}

	m_nCallback_threads = IniReadInt(m_szAppName, "callback_threads", m_szPathIniFile, 1);
	if (m_nCallback_threads < 1)
	{
		m_nCallback_threads = 1;
	}


	m_nNetworkevent_threadsHTTPD = IniReadInt(m_szAppName, "networkevent_threads_httpd", m_szPathIniFile, 1);
	if (m_nNetworkevent_threadsHTTPD < 1)
    {
		m_nNetworkevent_threadsHTTPD = 1;
    }

	m_nNetworkevent_threadsHTTP_TS = IniReadInt(m_szAppName, "networkevent_threads_httpts", m_szPathIniFile, 1);
	if (m_nNetworkevent_threadsHTTP_TS < 1)
	{
		m_nNetworkevent_threadsHTTP_TS = 1;
	}

	

	m_nDataProcessThread = IniReadInt(m_szAppName, "dataprocess_threads", m_szPathIniFile, 1);
	if (m_nDataProcessThread < 1)
	{
		m_nDataProcessThread = 1;
	}

	m_nNetworkevent_threadsClose = IniReadInt(m_szAppName, "networkevent_threads_close", m_szPathIniFile, 1);
	if (m_nNetworkevent_threadsClose < 1)
	{
		m_nNetworkevent_threadsClose = CpuCoreNumber();
	}

	


	m_nNetworkevent_threadsRTMP = IniReadInt(m_szAppName, "networkevent_threads_rtmp", m_szPathIniFile, 1);
	if (m_nNetworkevent_threadsRTMP < 1)
	{
		m_nNetworkevent_threadsRTMP = 1;
	}

	m_nNetworkevent_threadsETS = IniReadInt(m_szAppName, "networkevent_threads_ets", m_szPathIniFile, 1);
	if (m_nNetworkevent_threadsETS < 1)
	{
		m_nNetworkevent_threadsETS = 1;
	}

	m_nNetworkevent_threadsRTSP_TS = IniReadInt(m_szAppName, "networkevent_threads_rtsp", m_szPathIniFile, 1);
	if (m_nNetworkevent_threadsRTSP_TS < 1)
	{
		m_nNetworkevent_threadsRTSP_TS = 1;
	}	
	
	
	m_nSessionTimeout = IniReadInt(m_szAppName,"timeout",m_szPathIniFile,60);
	if( m_nSessionTimeout < 1 )
	{
		m_nSessionTimeout = 20 ;
	}

    m_bHttpTsEnableText = IniReadBool(m_szAppName, "ts_enable_text", m_szPathIniFile, FALSE);
    m_bTs_enable_sdt = IniReadBool(m_szAppName, "ts_enable_sdt", m_szPathIniFile, FALSE);
    m_bTsForceVideoAudio =  IniReadBool(m_szAppName, "ts_force_video_audio", m_szPathIniFile, TRUE);

    m_bTs_record_pcr_video = IniReadBool(m_szAppName, "ts_record_pcr_video", m_szPathIniFile, TRUE);
    m_bTs_hls_pcr_video = IniReadBool(m_szAppName, "ts_hls_pcr_video", m_szPathIniFile, TRUE);
    m_bTs_http_pcr_video = IniReadBool(m_szAppName, "ts_http_pcr_video", m_szPathIniFile, TRUE);

	m_bCacheKeyFrame = IniReadBool(m_szAppName, "cached_gop", m_szPathIniFile, TRUE);

    m_bTs_mpegts = IniReadBool(m_szAppName, "ts_mpegts", m_szPathIniFile, FALSE);
                                            

    m_nSocketSendBuf = IniReadInt(m_szAppName, "socket_send_buf", m_szPathIniFile, SIZE_1M);
    if (m_nSocketSendBuf < 0)
    {
        m_nSocketSendBuf = SIZE_1M;
    }

    m_nSocketRecvBuf = IniReadInt(m_szAppName, "socket_recv_buf", m_szPathIniFile, SIZE_1M);
    if (m_nSocketRecvBuf < 0)
    {
        m_nSocketRecvBuf = SIZE_1M;
    }

	



    m_bDebug_speex = IniReadBool(m_szAppName, "debug_speex", m_szPathIniFile, FALSE);


	m_nRecordReserveSpace = IniReadInt("record", "reserve_space", m_szPathIniFile, 1800);
	if (m_nRecordReserveSpace < 1)
	{
		m_nRecordReserveSpace = 512;
	}


    ////////////////////////////////////////////////////////////////////////////////
    if (m_luaRecordType.IsEmpty() || m_luaRecordType.IsNull())
    {
        IniReadStr("record", "record_type", szTmp, sizeof(szTmp), m_szPathIniFile);
    }
    else
    {
        strcpyn(szTmp, m_luaRecordType, sizeof(szTmp));
    }
	if (strcmpix(szTmp, "ses") == 0 || strcmpix(szTmp, "session") == 0 || strcmpix(szTmp, "s") == 0)
	{
		m_nRecordType = _RECORD_TYPE_SESSION;
	}
	else if (strcmpix(szTmp, "nvr") == 0 )
	{
		m_nRecordType = _RECORD_TYPE_NVR;
	}
	else
	{
		m_nRecordType = atoi(szTmp);
	}



    ///////////////////////////////////////////////////////////////////////////////////////
    if (m_luaRecordFormat.IsEmpty() || m_luaRecordFormat.IsNull())
    {
        IniReadStr("record", "default_format", szTmp, sizeof(szTmp), m_szPathIniFile);
    }
    else
    {
        strcpyn(szTmp, m_luaRecordFormat, sizeof(szTmp));
    }
	m_recordFormat = CStreamSetting::String2RecordFormat(szTmp);

	if (m_recordFormat == CStreamSetting::RecordFormatUnknown)
	{
		m_recordFormat = CStreamSetting::RecordFormatTS;
	}


    ///////////////////////////////////////////////////////////////////////////////////////
    if (m_luaRecordDuration.IsEmpty() || m_luaRecordDuration.IsNull())
    {
        m_nRecordDuration = IniReadInt("record", "record_duration", m_szPathIniFile, 1800);
    }
    else
    {
        m_nRecordDuration = m_luaRecordDuration;
    }
    
    
    if (m_nRecordDuration < 5)
    {
        m_nRecordDuration = 5;
    }


    m_bVideoUseAudioTime = IniReadBool(m_szAppName, "use_audio_time", m_szPathIniFile, FALSE);
	m_bNeedNoop = IniReadBool(m_szAppName,"neednoop",m_szPathIniFile , FALSE);
    


    m_nFlash_interactive_time = IniReadInt(m_szAppName, "flash_interactive_time", m_szPathIniFile, 60);



	/************************** HLS ********************************************/
#if defined(_HLS_SUPPORT_)
	LoadHLS( &m_hlsCfg );
#endif
#if defined(_RTMP_SUPPORT_)
    LoadLocalStream();
#endif
	LoadSettingSrv(&m_cfgSrv);



//	LogPrint(_LOG_LEVEL_DEBUG ,_TAGNAME, "m_bRecord = %d\n" , m_bRecord );
//	LogPrint(_LOG_LEVEL_DEBUG ,_TAGNAME, "m_szRecordPath = %s\n" , m_szRecordPath );
}
BOOL CConfigIni::LoadCdnNetInterface()
{
    char szSection[] = "cdn_net_interface";
    int nRet = 0;
    int  i = 1;
    char szTmp[1000];
#if defined(_CDNNET_SUPPORT_)


#endif
    return TRUE;
}
BOOL CConfigIni::LoadCdnServer(CDN_SERVER *cfg)
{
 

    return TRUE;
}
BOOL CConfigIni::LoadSettingSrv(CFGSRV *cfg)
{
	char szSection[] = "streamsrv_setting_port";
	int nRet = 0;

	cfg->enable = IniReadBool(szSection, "enable", m_szPathIniFile, TRUE);
	nRet = IniReadStr(szSection, "addr", cfg->addr, sizeof(cfg->addr), m_szPathIniFile);
	if (nRet < INI_ERROR_VARNOTFIND)
	{
		return FALSE;
	}

	cfg->port = IniReadInt(szSection, "port", m_szPathIniFile, _CFG_PORT);
	if (cfg->port == 0)
	{
		cfg->port = _CFG_PORT;
	}

	return  TRUE;
}

#if defined(_HLS_SUPPORT_)
BOOL  CConfigIni::LoadHLS(HLSCFG *cfg )
{

	char szSection[] = "hls" ;
	char szTmp[20];
	cfg->ts_duration = IniReadInt(szSection,"ts_duration",m_szPathIniFile,RTMP_PORT);
	if( cfg->ts_duration < 1 )
	{
		cfg->ts_duration = 5 ;
	}
	cfg->m3u8_files = IniReadInt(szSection,"m3u8_files",m_szPathIniFile,RTMP_PORT);
	if( cfg->m3u8_files < 1 )
	{
		cfg->m3u8_files = 1 ;
	}

	cfg->keep_old_files = IniReadInt(szSection,"keep_old_files",m_szPathIniFile,4);
	if( cfg->keep_old_files < (cfg->m3u8_files ) )
	{
		cfg->keep_old_files = cfg->m3u8_files ;
	}
    
    cfg->pcr_in_every_frame = IniReadBool(szSection, "pcr_in_every_frame", m_szPathIniFile, FALSE);
    cfg->split_inkeyframe = IniReadBool(szSection,"split_inkeyframe",m_szPathIniFile,TRUE);
    cfg->hls_as_record = IniReadBool(szSection, "hls_as_record", m_szPathIniFile, FALSE);

	IniReadStr(szSection, "name_type", szTmp, sizeof(szTmp), m_szPathIniFile);

	cfg->nameType = String2HlsNameType(szTmp);

	IniReadStr(szSection, "split_time", szTmp, sizeof(szTmp), m_szPathIniFile);
	cfg->split_time = String2HlsSplitTime(szTmp);
	

	m_nHlsAudioWaitVideoSeconds = IniReadInt(szSection, "audio_wait_video_seconds", m_szPathIniFile, 2);
	if (m_nHlsAudioWaitVideoSeconds < 1)
	{
		m_nHlsAudioWaitVideoSeconds = 2;
	}
	return TRUE ;
}
#endif

#if defined(_HTTPTS_SUPPORT_)
BOOL CConfigIni::LoadHTTP_TS(HTTPTSCFG *cfg, int i)
{
	char szSection[40];
	int nRet = 0;
	int nRet2 = 0;
	sprintf(szSection, "http%d", i);


	nRet = IniReadStr(szSection, "bind_addr", cfg->bind_addr, sizeof(cfg->bind_addr), m_szPathIniFile);
	nRet2 = IniReadStr(szSection, "bind_addr6", cfg->bind_addr6, sizeof(cfg->bind_addr6), m_szPathIniFile);
	if (nRet < INI_ERROR_VARNOTFIND && nRet2 < INI_ERROR_VARNOTFIND)
	{
		return FALSE;
	}

	cfg->network_threads = IniReadInt(szSection, "network_threads", m_szPathIniFile, 0);

	cfg->port = IniReadInt(szSection, "port", m_szPathIniFile, RTMP_PORT);
	if (cfg->port == 0)
	{
		cfg->port = 181;
	}


	cfg->enable = IniReadBool(szSection, "enable", m_szPathIniFile, TRUE);

	cfg->enable_flv = IniReadBool(szSection, "enable_flv", m_szPathIniFile, TRUE);
	cfg->enable_ts = IniReadBool(szSection, "enable_ts", m_szPathIniFile, TRUE);
	cfg->enable_aac = IniReadBool(szSection, "enable_aac", m_szPathIniFile, TRUE); 
	cfg->exclude_connections = IniReadBool(szSection, "exclude_connections", m_szPathIniFile, FALSE);

	return TRUE;
}
#endif


#if defined(_ENABLE_HTTPD_)
BOOL CConfigIni::LoadHTTPD( HTTPDCFG *cfg, int i )
{
    char szSection[ 40 ];
    int nRet = 0;
	int nRet2 = 0;
    sprintf( szSection, "httpd-hls%d", i );


	nRet = IniReadStr(szSection, "bind_addr", cfg->bind_addr, sizeof(cfg->bind_addr), m_szPathIniFile);
	nRet2 = IniReadStr(szSection, "bind_addr6", cfg->bind_addr6, sizeof(cfg->bind_addr6), m_szPathIniFile);
	if (nRet < INI_ERROR_VARNOTFIND && nRet2 < INI_ERROR_VARNOTFIND)
	{
		return FALSE;
	}

	cfg->network_threads = IniReadInt(szSection, "network_threads", m_szPathIniFile, 0);

    cfg->port = IniReadInt( szSection, "port", m_szPathIniFile, RTMP_PORT );
    if( cfg->port == 0 )
    {
        cfg->port = RTMP_PORT;
    }

    cfg->enable = IniReadBool( szSection, "enable", m_szPathIniFile, TRUE );
    cfg->enable206 = IniReadBool(szSection, "enable206", m_szPathIniFile, FALSE);
	cfg->keepalive = IniReadBool(szSection, "keepalive", m_szPathIniFile, TRUE);
	cfg->exclude_connections = IniReadBool(szSection, "exclude_connections", m_szPathIniFile, FALSE);
    IniReadStr( szSection, "www_root", cfg->www_root, sizeof( cfg->www_root ), m_szPathIniFile );
    return TRUE;
}
#endif

#if defined(_RTSP_SUPPORT_)
BOOL CConfigIni::LoadRTSP(RTSPTSCFG *cfg, int i)
{
	char szSection[40];
	int nRet = 0;
	int nRet2 = 0;
	sprintf(szSection, "rtsp%d", i);

	nRet = IniReadStr(szSection, "bind_addr", cfg->bind_addr, sizeof(cfg->bind_addr), m_szPathIniFile);
	nRet2 = IniReadStr(szSection, "bind_addr6", cfg->bind_addr6, sizeof(cfg->bind_addr6), m_szPathIniFile);
	if (nRet < INI_ERROR_VARNOTFIND && nRet2 < INI_ERROR_VARNOTFIND)
	{
		return FALSE;
	}

	cfg->port = IniReadInt(szSection, "port", m_szPathIniFile, 554);
	if (cfg->port == 0)
	{
		cfg->port = 554;
	}

	cfg->network_threads = IniReadInt(szSection, "network_threads", m_szPathIniFile, 0);

	cfg->enable = IniReadBool(szSection, "enable", m_szPathIniFile, TRUE);
	cfg->force_rtsp_play = IniReadBool(szSection, "force_rtsp_play", m_szPathIniFile, TRUE);
	cfg->force_http_ts_play = IniReadBool(szSection, "force_http_ts_play", m_szPathIniFile, TRUE);


	cfg->enable_flv = IniReadBool(szSection, "enable_flv", m_szPathIniFile, TRUE);
	cfg->enable_ts = IniReadBool(szSection, "enable_ts", m_szPathIniFile, TRUE);
	cfg->enable_aac = IniReadBool(szSection, "enable_aac", m_szPathIniFile, TRUE); 
	cfg->exclude_connections = IniReadBool(szSection, "exclude_connections", m_szPathIniFile, FALSE);
	return TRUE;
}
#endif



#if defined(_RTMP_SUPPORT_)
BOOL CConfigIni::LoadLocalStream( )
{
	char szSection[ ] = "local-stream";
	LOCALSTREAM *cfg = &m_localStream;

	cfg->enable = IniReadBool( szSection, "enable", m_szPathIniFile, TRUE );

	if( !cfg->enable )
	{
		return TRUE ;
	}

	IniReadStr( szSection, "streampush_plugin", m_szStreamPushPluginPath, sizeof( m_szStreamPushPluginPath ), m_szPathIniFile );
	IniReadStr( szSection, "streampush_ini", m_szStreamPushIniPath, sizeof( m_szStreamPushIniPath ), m_szPathIniFile );


	cfg->output_hls = IniReadBool( szSection, "output_hls", m_szPathIniFile, TRUE );
	cfg->memory_file = IniReadBool( szSection, "memory_file", m_szPathIniFile, FALSE );
	IniReadStr( szSection, "hls_dir", cfg->hls_dir, sizeof( cfg->hls_dir ), m_szPathIniFile );
	IniReadStr( szSection, "hls_ts_prefix", cfg->hls_ts_prefix, sizeof( cfg->hls_ts_prefix ), m_szPathIniFile );


	IniReadStr( szSection, "record_dir", cfg->record_dir, sizeof( cfg->record_dir ), m_szPathIniFile );
	cfg->record = IniReadBool( szSection, "record", m_szPathIniFile, FALSE );

	IniReadStr( szSection, "record_mp4_dir", cfg->record_mp4_dir, sizeof( cfg->record_mp4_dir ), m_szPathIniFile );
	cfg->record_mp4 = IniReadBool( szSection, "record_mp4", m_szPathIniFile, FALSE );

#if !defined(_ENABLE_HTTPD_) || !defined(_ENABLE_MEMORY_FILE_)
	cfg->memory_file = FALSE;
#endif
	return TRUE;

}

BOOL CConfigIni::LoadRTMP(RTMPCFG *cfg , int i)
{
	char szSection[40];
	int nRet =  0 ;
	int nRet2 = 0;
	sprintf( szSection , "rtmp%d" , i );


	nRet = IniReadStr(szSection, "bind_addr", cfg->bind_addr, sizeof(cfg->bind_addr), m_szPathIniFile);
	nRet2 = IniReadStr(szSection, "bind_addr6", cfg->bind_addr6, sizeof(cfg->bind_addr6), m_szPathIniFile);
	if (nRet < INI_ERROR_VARNOTFIND && nRet2 < INI_ERROR_VARNOTFIND)
	{
		return FALSE;
	}

	cfg->network_threads = IniReadInt(szSection, "network_threads", m_szPathIniFile, 0);
	cfg->enable = IniReadBool( szSection, "enable", m_szPathIniFile, TRUE );
	
	IniReadStr(szSection,"app",cfg->app ,sizeof(cfg->app),m_szPathIniFile);

	cfg->enable_rtmp = IniReadBool( szSection ,"enable_rtmp",m_szPathIniFile , TRUE);
	cfg->enable_rtmpt = IniReadBool( szSection ,"enable_rtmpt",m_szPathIniFile , TRUE);


	cfg->port = IniReadInt(szSection,"port",m_szPathIniFile,RTMP_PORT);
	if( cfg->port == 0 )
	{
		cfg->port = RTMP_PORT ;
	}

	cfg->chunksize = IniReadInt(szSection,"chunksize",m_szPathIniFile,65535);

	cfg->publish = IniReadBool( szSection ,"publish",m_szPathIniFile , TRUE);
	cfg->playback = IniReadBool( szSection ,"playback",m_szPathIniFile , TRUE);
	cfg->output_hls = IniReadBool( szSection ,"output_hls",m_szPathIniFile , TRUE);
    cfg->memory_file = TRUE;
	cfg->force_playback = IniReadBool(szSection, "force_playback", m_szPathIniFile, FALSE);
	cfg->force_hls_setting = IniReadBool(szSection, "force_hls_setting", m_szPathIniFile, FALSE);
	cfg->exclude_connections = IniReadBool(szSection, "exclude_connections", m_szPathIniFile, FALSE);

	IniReadStr(szSection,"hls_dir",cfg->hls_dir ,sizeof(cfg->hls_dir),m_szPathIniFile);
	IniReadStr(szSection,"hls_ts_prefix",cfg->hls_ts_prefix ,sizeof(cfg->hls_ts_prefix),m_szPathIniFile);


    if (m_luaRecordTsPath.IsEmpty() || m_luaRecordTsPath.IsNull())
    {
        IniReadStr(szSection, "record_ts_dir", cfg->record_ts_dir, sizeof(cfg->record_ts_dir), m_szPathIniFile);
    }
    else
    {
        strcpyn(cfg->record_ts_dir, m_luaRecordTsPath, sizeof(cfg->record_ts_dir));
    }

    if (m_luaRecordMp4Path.IsEmpty() || m_luaRecordMp4Path.IsNull())
    {
        IniReadStr(szSection, "record_mp4_dir", cfg->record_mp4_dir, sizeof(cfg->record_mp4_dir), m_szPathIniFile);
    }
    else
    {
        strcpyn(cfg->record_mp4_dir, m_luaRecordMp4Path, sizeof(cfg->record_mp4_dir));
    }

    if (m_luaRecordFlvPath.IsEmpty() || m_luaRecordFlvPath.IsNull())
    {
        IniReadStr(szSection, "record_flv_dir", cfg->record_flv_dir, sizeof(cfg->record_flv_dir), m_szPathIniFile);
    }
    else
    {
        strcpyn(cfg->record_flv_dir, m_luaRecordFlvPath, sizeof(cfg->record_flv_dir));
    }


    if (m_luaRecordTsEnable.IsEmpty() || m_luaRecordTsEnable.IsNull())
    {
        cfg->record_ts = IniReadBool(szSection, "record_ts", m_szPathIniFile, FALSE);
    }
    else
    {
        cfg->record_ts = (int)m_luaRecordTsEnable;
    }


    if (m_luaRecordMp4Enable.IsEmpty() || m_luaRecordMp4Enable.IsNull())
    {
        cfg->record_mp4 = IniReadBool(szSection, "record_mp4", m_szPathIniFile, FALSE);
    }
    else
    {
        cfg->record_mp4 = (int)m_luaRecordMp4Enable;
    }


    if (m_luaRecordFlvEnable.IsEmpty() || m_luaRecordFlvEnable.IsNull())
    {
        cfg->record_flv = IniReadBool(szSection, "record_flv", m_szPathIniFile, FALSE);
    }
    else
    {
        cfg->record_flv = (int)m_luaRecordFlvEnable;
    }

#if !defined(_ENABLE_HTTPD_) || !defined(_ENABLE_MEMORY_FILE_)
    cfg->memory_file = FALSE ;
#endif


	cfg->use_setting_srv = IniReadBool(szSection, "use_setting_srv", m_szPathIniFile, FALSE);
	cfg->enable_virtual_live = IniReadBool(szSection, "enable_virtual_live", m_szPathIniFile, FALSE);

	if (m_mode == WorkModeSlave)
	{
		cfg->publish = FALSE;
	}
	return TRUE ;

}

BOOL CConfigIni::LoadStreamPushPlugin( )
{
 
    return TRUE;
}
#endif
void CConfigIni::LoadDevId()
{

}
BOOL CConfigIni::Init(int argc, char *argv[])
{
	BOOL bRet = CObjConfig::Init(argc, argv);


	memset(&_ObjCount, 0, sizeof(_ObjCount));
	CalCRC32(NULL,0);
	SetLogPrint(&__LogPrintFuncMediasrv);
    LoadDevId();
    LoadBaseIni();
    LoadCdnServer(&m_cfgCdnSrv);
    LoadCdnNetInterface();


    m_bDemoVersion = FALSE;
    m_bOverdue = FALSE;
	


    MemoryRelease(MemoryEmptyString());

	LibCppUtilInit(_AppConfig.m_nMaxCachedMem);
	if (argc > 1 && strcmpix(argv[1], "start") == 0)
	{
		m_IsLogConsole = FALSE;
	}


	SessionInit();


    LoadIni();



#if defined(_RTMP_SUPPORT_)
    LoadStreamPushPlugin();
#endif


	return bRet ;
}

int GetDevId(CRefString *value)
{
    *value = _AppConfig.m_devId;
    return 0;
}
int GetNodeName(CRefString *value)
{
    *value = _AppConfig.m_cfgCdnSrv.node_id;
    return 0;
}

static int __LogPrintFuncMediasrv( int prio, const char *tag,const char *fmt, va_list ap )
{
    if( !_AppConfig.m_bLogDebug &&  prio == _LOG_LEVEL_DEBUG )
    {
        return 0 ;
    }
	_AppConfig.VLog( prio, fmt , ap );
	return 0;
}


















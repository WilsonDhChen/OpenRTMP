 
#ifndef _libmediasrv_config_h_ 
#define _libmediasrv_config_h_

#if !defined _APP_NAME
#define _APP_NAME "mediasrv"
#endif

#if !defined _COMPANY_NAME
#define _COMPANY_NAME ""
#endif                                 

#define _CFG_PORT 8500
#define _CONFIGURE_PORT_PORT 8501
#define _CONFIGURE_PORT_HTTP 8580
typedef int * ( *_StreamPushStart )( const char *szPushAddr, const char *szIni );
typedef int * ( *_StreamPushStop )();
//////////////////////////////////////

typedef enum tagWorkMode
{
    WorkModeCdn ,
	WorkModeMaster ,
	WorkModeSlave,
	WorkModeMixed,

}WorkMode;

typedef struct tagOBJECTS_COUNT
{

	INT32 hls_files;


}OBJECTS_COUNT;


typedef struct tagLOCALSTREAM
{
    BOOL enable;
    BOOL output_hls;
    BOOL memory_file;
    char hls_dir[ 400 ];
    char hls_ts_prefix[ 400 ];

    BOOL record;
    char record_dir[ 400 ];

	BOOL record_mp4;
	char record_mp4_dir[400];
}LOCALSTREAM;

typedef struct tagCFGSRV
{

	char addr[80];
	int  port;
	BOOL enable;
}CFGSRV;

typedef struct tagCDN_SERVER
{
    BOOL enable;
    int  search_stream_time;
    char node_id[200];
    char cdn_addr1[100];
    char cdn_addr2[100];
    char cdn_addr3[100];

}CDN_SERVER;

class CStreamSetting
{
public:
	typedef enum tagRecordFormat
	{
		RecordFormatUnknown,
		RecordFormatTS,
		RecordFormatMP4,
		RecordFormatFLV,

	}RecordFormat;
public:
	CStreamSetting();
	~CStreamSetting();
	static CStreamSetting::RecordFormat String2RecordFormat(const char *str);
	BOOL m_bHls;
	BOOL m_bHlsOutputMemory;
	
	BOOL m_bOutRTSP ;
	BOOL m_bOutRTMP ;
	BOOL m_bOutHTTP_TS ;

	BOOL m_bRecord;
	int  m_nRecordDuration;
	CObjPtr<CObjVarRef> m_recordDir;

	CObjPtr<CObjVarRef> m_hlsDir;
	CObjPtr<CObjVarRef> m_hlsTsPrefix;
	
#if defined(_HLS_SUPPORT_)
	HLSCFG m_hlscfg;
#endif
	RecordFormat m_recordFormat;
};

class CConfigIni :
		public CObjConfig
{
public:
	CConfigIni();
	~CConfigIni();
	void LoadBaseIni();
    void LoadIni();
	virtual BOOL Init(int argc = 0,char *argv[] = NULL );
#if defined(_RTMP_SUPPORT_)
	BOOL LoadRTMP(RTMPCFG *cfg , int i);
	BOOL LoadLocalStream();
	BOOL LoadStreamPushPlugin();
#endif

#if defined(_ETS_SUPPORT_)
	BOOL LoadETS(ETSCFG *cfg , int i);
#endif

#if defined(_RTSP_SUPPORT_)
	BOOL LoadRTSP(RTSPTSCFG *cfg, int i);
#endif	

#if defined(_ENABLE_HTTPD_)
    BOOL LoadHTTPD( HTTPDCFG *cfg, int i );
#endif

#if defined(_HTTPTS_SUPPORT_)
	BOOL LoadHTTP_TS(HTTPTSCFG *cfg, int i);
#endif


#if defined(_HLS_SUPPORT_)
	BOOL LoadHLS(HLSCFG *cfg );
#endif

    BOOL LoadCdnServer(CDN_SERVER *cfg);
    BOOL LoadCdnNetInterface();
	BOOL LoadSettingSrv(CFGSRV *cfg);
    void LoadDevId();

public:
	char m_szPluginPath[LEN_MAX_PATH];
	char m_szCallbackPluginPath[LEN_MAX_PATH];
	char m_szRtspRecordPath[LEN_MAX_PATH];

    char m_szStreamPushPluginPath[ LEN_MAX_PATH ];
    char m_szStreamPushIniPath[ LEN_MAX_PATH ];

    char m_szGpsPluginPath[LEN_MAX_PATH];
    char m_szGpsDev[LEN_MAX_PATH];
    char m_szGpsBandrate[40];
    BOOL m_bEnableGps;

	int  m_nRtspPort;
#if defined(_HLS_SUPPORT_)
	HLSCFG m_hlsCfg;
#endif
	BOOL m_bOverdue;
	CFGSRV m_cfgSrv;
    CDN_SERVER m_cfgCdnSrv;
	int  m_nMaxSends;
	int  m_nMaxWrittingBytes;

	
	int  m_nSessionTimeout;
	BOOL m_bUseClientID;
	BOOL m_bRtspRecord;

	BOOL m_bCacheKeyFrame;
	BOOL m_bCallbackEnable;
    BOOL m_bDebug_speex;
    BOOL m_bTs_enable_sdt;
    BOOL m_bHttpTsEnableText;
    BOOL m_bTsForceVideoAudio;

    BOOL m_bTs_record_pcr_video;
    BOOL m_bTs_hls_pcr_video;
    BOOL m_bTs_http_pcr_video;
    BOOL m_bTs_mpegts;


	int m_nDiskIoThreads;
	int m_nDataProcessThread;

	int m_nNetworkevent_threadsHTTPD;
	int m_nNetworkevent_threadsHTTP_TS;
	
	int m_nNetworkevent_threadsRTMP;
	int m_nNetworkevent_threadsETS;
	int m_nNetworkevent_threadsRTSP_TS;
	int m_nNetworkevent_threadsClose;
	int m_nCallback_threads;

    int m_nFlash_interactive_time;
	int m_nRecordDuration;
	int m_nRecordType;
	int m_nRecordReserveSpace;
	CStreamSetting::RecordFormat m_recordFormat;
	BOOL m_bNeedNoop ;
	BOOL m_bDemoVersion;
	BOOL m_bEnablePCR;
	int  m_nMaxClientsPerList;
	int  m_nSendThreads;
	BOOL m_bBreaked;
    BOOL m_bVideoUseAudioTime;
	int  m_nHlsAudioWaitVideoSeconds;
    int  m_nDelayclose_virtual_live_seconds;

    int  m_nSocketSendBuf;
    int  m_nSocketRecvBuf;
	int   m_nMaxVideoCacheTime;
    TIME_T m_tmStarted;
	SYSTEMTIME m_systmBoot;
    BOOL m_bLogDebug;
	INT64 m_nMaxCachedMem;
	char m_szMachineId[80];
	APP_KEY m_key;
	TIME_T m_tmLogNoSpace;
	WorkMode m_mode;
    CInetAddrStr m_masterAddr;
    CObjVar m_devId;
    
public:
    LOCALSTREAM m_localStream;
    _StreamPushStart m_funcStreamPushStart;
    _StreamPushStop m_funcStreamPushStop;

    CObjVar m_luaRecordType;
    CObjVar m_luaRecordDuration;
    CObjVar m_luaRecordFormat;
    CObjVar m_luaRecordTsPath;
    CObjVar m_luaRecordMp4Path;
    CObjVar m_luaRecordFlvPath;
    CObjVar m_luaRecordTsEnable;
    CObjVar m_luaRecordMp4Enable;
    CObjVar m_luaRecordFlvEnable;

};
BOOL UpdateHlsNameType2StreamSetting(const char *szIp, int hlsNameType);
BOOL UpdateStreamSettingStatus(const char *szId, const char *szIp, BOOL bSysId, int status, int hlsNameType);
BOOL GetStreamSetting(CStreamSetting &setting, const char *szId, const char *szIp, BOOL bSysId, const char *szUsr, const char *szPwd);

#endif







































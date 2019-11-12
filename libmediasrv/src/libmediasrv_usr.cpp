

#include "libmediasrv_def.h"

static UINT8 _Aud[6] = { 0, 0, 0, 1, 0x09, 0x30 };
#if !defined(_USE_X264)


BOOL X264_GetSpsPps(int width, int height, int framerate, int avclevel, int avcprofile,int videokeyframe_frequency ,  CObjBuffer &sps, CObjBuffer &pps, CObjBuffer &sei)
{

	return FALSE;
}


#endif

BOOL IsH264KeyFrame(UINT8 *data, int nDatalen, BOOL *findAud)
{
	NALU_T nalu;
	CH264NaluParse naluParse(data, nDatalen);

	while (naluParse.GetNextNalu(&nalu))
	{
        if (nalu.nal_unit_type == NALU_TYPE_AUD)
        {
            if (findAud != NULL)
            {
                *findAud = TRUE;
            }
        }
		if (nalu.nal_unit_type == NALU_TYPE_IDR)
		{
			return TRUE;
		}
	}


	return FALSE;
}

CETS2SharedMemVar::CETS2SharedMemVar(CMdSource *mdsrc, int  cts, INT64 tick, UINT8 *buf, int nLen, XCodec codec)
{

	m_sharedMem = NULL;
	m_buf = buf;
	m_bufLen = nLen;

	m_cts = cts;
	m_tick = tick;
	m_codec = codec;

}
CETS2SharedMemVar::~CETS2SharedMemVar()
{
	if (m_sharedMem != NULL)
	{
		SharedMemRelease( m_sharedMem );
	}
}
BOOL CETS2SharedMemVar::Init()
{
	if (m_sharedMem != NULL)
	{
		return TRUE;
	}

	int  nDataLen = AlignLength(m_bufLen, SIZE_1KB);
	m_sharedMem = SharedMemCreate(nDataLen);
	if (m_sharedMem == NULL)
	{
		return FALSE;
	}
	memcpy(m_sharedMem, m_buf, m_bufLen);


	return TRUE;
}
////////////////////////////////////////////////
CObjNetIOBufferSharedMemoryTSSource * CObjNetIOBufferSharedMemoryTSSource::CreateObj(void *memShared, int memSize, BOOL bMalloc2)
{
	if (memSize < 1 && memShared == NULL)
	{
		return NULL;
	}
	CObjNetIOBufferSharedMemoryTSSource *pObj = new CObjNetIOBufferSharedMemoryTSSource(memShared, memSize, bMalloc2);
	if (pObj == NULL)
	{
		return NULL;
	}

	pObj->m_bAlloced = TRUE;
	pObj->m_nObjSize = sizeof(CObjNetIOBufferSharedMemoryTSSource);
	pObj->AddObjRef();
	return pObj;
}

CObjNetIOBufferSharedMemoryTSSource::CObjNetIOBufferSharedMemoryTSSource(void *memShared, int memSize, BOOL bMalloc2)
	:CObjNetIOBufferSharedMemory(memShared, memSize, bMalloc2)
{

}
CObjNetIOBufferSharedMemoryTSSource::~CObjNetIOBufferSharedMemoryTSSource()
{

}
////////////////////////////////////////////////////////////////////

CObjNetIOBufferSharedMemoryVideoFrame * CObjNetIOBufferSharedMemoryVideoFrame::CreateObj(void *memShared, int memSize, BOOL bMalloc2)
{
	if (memSize < 1 && memShared == NULL)
	{
		return NULL;
	}
	CObjNetIOBufferSharedMemoryVideoFrame *pObj = new CObjNetIOBufferSharedMemoryVideoFrame(memShared, memSize, bMalloc2);
	if (pObj == NULL)
	{
		return NULL;
	}

	pObj->m_bAlloced = TRUE;
	pObj->m_nObjSize = sizeof(CObjNetIOBufferSharedMemoryVideoFrame);
	pObj->AddObjRef();
	return pObj;
}

CObjNetIOBufferSharedMemoryVideoFrame::CObjNetIOBufferSharedMemoryVideoFrame(void *memShared, int memSize, BOOL bMalloc2)
	:CObjNetIOBufferSharedMemory(memShared, memSize, bMalloc2)
{
	m_iVideoFrameQueue = 0 ;
	m_iAudioFrameQueue = 0 ;
	m_tick = 0 ;
	m_bKeyFrame = FALSE ;
	m_cts = 0;
	m_bAudio = FALSE;
}
CObjNetIOBufferSharedMemoryVideoFrame::~CObjNetIOBufferSharedMemoryVideoFrame()
{
}
CObjNetIOBufferSharedMemoryVideoFrame *CObjNetIOBufferSharedMemoryVideoFrame::Copy()
{
	CObjNetIOBufferSharedMemoryVideoFrame *ioBuf = CObjNetIOBufferSharedMemoryVideoFrame::CreateObj(m_BufShared, m_nDataLen, m_bMalloc2);
	if (ioBuf != NULL)
	{
		ioBuf->m_iVideoFrameQueue = m_iVideoFrameQueue;
		ioBuf->m_tick = m_tick;
		ioBuf->m_bKeyFrame = m_bKeyFrame;
		return  ioBuf;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////
CRTMPAudioConfig::CRTMPAudioConfig()
{

    samplesRate = 0;
    channels = 0;
    bitsPerSample = 0;

    bytesPerFrame = 0;
    samplesPerFrame = 0;

    fps = 0;
    aac_frame_dur = 0;



    inited = 0;
}
BOOL CRTMPAudioConfig::Parse(UINT8 type)
{
    type &= 0X0F;

    UINT8 sr = (type >> 2);
    UINT8 s16 = (type & 0X02) >> 1;
    UINT8 stero = (type & 0X01) ;

    static int samplingrates[4] = {5512,11025,22050,44100};
    samplesRate = samplingrates[sr];
    

    if (s16)
    {
        bitsPerSample = 16;
    }
    else
    {
        bitsPerSample = 8;
    }

    if (stero)
    {
        channels = 2;
    }
    else
    {
        channels = 1;
    }

    samplesRate = 16000;
    channels = 1;
    bitsPerSample = 16;

    aac_frame_dur = 1024.0*1000.0 / samplesRate / channels;
    inited = 1;
    return TRUE;

}

////////////////////////////////////////////////////////////////////
_CPP_UTIL_CLASSNAME_IMP(CMdSource)
_CPP_UTIL_QUERYOBJ_IMP(CMdSource,CObj)


CMdSource * CMdSource::CreateObj(const char *szApp, const char *szId)
{
	CMdSource *pObj = NULL ; 
	if( szId == NULL || szId[0] == 0 )
	{
		return NULL ;
	}
    pObj = new CMdSource(szApp,szId);
	if( pObj == NULL ) 
	{ 
		return NULL ; 
	} 
	pObj->m_bAlloced = TRUE ; 
	pObj->m_nObjSize = sizeof(CMdSource) ; 
	pObj->AddObjRef(); 
	return pObj ; 
}
void * CMdSource::operator new( size_t cb )
{


	return malloc2(cb);


}
void CMdSource::operator delete( void* p )
{


	free2(p);

}


CMdSource::CMdSource(const char *szApp, const char *szId)
{

    m_tmSendGps = 0 ;
	m_audiosamplerate = 0 ;
	m_audiosamplesize = 0 ;
	m_audiocodecid = 0 ;
	m_audiochannels = 0 ;
	m_bVirtual = FALSE;

	m_tmETSVideo = 0 ;
	m_tmETSAudio = 0 ;
	m_tmETSText = 0 ;

	memset(&m_tmChannel, 0, sizeof(m_tmChannel));

	m_audioAbsTime = 0;
	m_rtmpChunkSize = 0 ;
	m_MaxPacketSize = 0 ;
	m_StreamCount = 0 ;
	m_interleavedNext = 0 ;
	m_nSdpLength = 0 ;
	m_rtmpMetaDataBuf = NULL ;
	m_rtmpSpsPpsBuf = NULL ;
	m_rtmpAACHeaderBuf = NULL ;
	m_flvHeader = NULL ;
	m_flvHeaderVideo = NULL;
	m_flvHeaderAudio = NULL;

	m_flvSpsPpsTag = NULL ;
	m_flvAACHeaderTag = NULL ;
	m_bRtmpSrcInited = FALSE ;
	m_tmRTMPPublish = 0 ;
	m_bClosedAll = FALSE ;
    m_rtmpTimeVideo = 0 ;
  //  m_rtmpTimeAudio = 0 ;
	m_bOutRTSP = TRUE ;
	m_bOutRTMP = TRUE;
	m_bOutHTTP_TS = TRUE;
	m_bOutHTTP_FLV = TRUE; 
	m_avclevel = 0 ;
	m_avcprofile = 0 ;
	m_bCreatedSpsPps = FALSE;
	m_videokeyframe_frequency = 0;

	m_bHaveVideo = FALSE ;
	m_bHaveAudio = FALSE ;

	memset( m_szSdp, 0 , sizeof(m_szSdp) );
#if defined(_RTSP_SUPPORT_)
	memset( m_streams, 0 , sizeof(m_streams) );
#endif
	memset( m_szContentType, 0 , sizeof(m_szContentType) );
	memset(m_szGlobalTimestamp,0,sizeof(m_szGlobalTimestamp));
	memset(m_szAACConfig,0,sizeof(m_szAACConfig));
	memset(m_szAACConfigStr,0,sizeof(m_szAACConfigStr));

	memset(&m_amrProp,0,sizeof(m_amrProp));
	
	
	
	

    m_stream = szId;
    m_app = szApp;

    char szTmp[2048];
    snprintf(szTmp, sizeof(szTmp) - 1, "mdsource-%s-%s", (const char *)m_app, (const char *)m_stream);
    szTmp[sizeof(szTmp) - 1] = 0;
    m_remveKey = szTmp;
	
	m_ctxSrc = NULL ; 

	m_srcType = CtxTypeUnknown ;

	m_sps.SafeTail = 16 ;
	m_pps.SafeTail = 16 ;

	m_rtmpTimePreVideo = 0 ;
	m_rtmpTimePreAudio = 0 ;
	m_firstRTMPFrameTime = -1 ;

	m_flvTimePreVideo = 0 ;
	m_flvTimePreAudio  = 0 ;
	m_firstFlvFrameTime = -1 ;

	m_audioType = XCodecUnknown ;
	m_videoType = XCodecUnknown ;

	m_width = 0 ;
	m_height = 0  ;
	m_framerate = 0 ;
#if defined(_HLS_SUPPORT_)
	m_hls = NULL ;
#endif

#if defined(_MP4_SUPPORT_)
	m_mp4 = NULL;
#endif	

#if defined(_FLVREC_SUPPORT_)
	m_flv = NULL;
#endif


#if defined(_MDSOURCE_MT)
	memset(&m_nConnections, 0, sizeof(m_nConnections));
#else
	MtxInit(&m_mtxList, 0);
#endif

	m_iVideoFrameQueue = 0;
	m_iAudioFrameQueue = 0;
	m_bCacheStarted = FALSE;
	m_tmFirstCachedFrame = 0;
    m_fromCdn = FALSE;
    m_bAudioNotSupport = FALSE;
#if defined(_AUDIO_CONVERT_)
    m_bAudioUseRTMPTime = FALSE;
#endif
    m_bVideoUseAudioTime = _AppConfig.m_bVideoUseAudioTime;
    m_tmAudioAbsTimeUpdate = OSTickCount();

	m_tmCacheBegin = OSTickCount();
    m_bConvertAudio = FALSE;
    m_bNewSrcCtx = FALSE;

    m_timestampDelta = 0 ;
    m_timestampMax = 0 ;
 







	m_queue = _MediasrvApp->m_queueDataProcess;
	m_queueThread = m_queue->GetNextThreadObj();

	LogPrint(_LOG_LEVEL_INFO, _TAGNAME, "%s %p \n",__FUNCTION__, this);

	MtxInit(&m_mtxVideoFrames, 0);
	MtxInit(&m_mtxMetaData, 0);
	MtxInit(&m_mtxFlvHeader, 0);
    MtxInit(&m_mtxSrcCtx,0);
	
	



	

}
CMdSource::~CMdSource()
{


    RelaseMediaKeyInfoAudio();
    RelaseMediaKeyInfoVideo();
	if( m_rtmpMetaDataBuf != NULL )
	{
		m_rtmpMetaDataBuf->ReleaseObj();
		m_rtmpMetaDataBuf = NULL ;
	}



	if( m_flvHeader != NULL )
	{
		m_flvHeader->ReleaseObj();
		m_flvHeader = NULL ;
	}

	if (m_flvHeaderVideo != NULL)
	{
		m_flvHeaderVideo->ReleaseObj();
		m_flvHeaderVideo = NULL;
	}


	if (m_flvHeaderAudio != NULL)
	{
		m_flvHeaderAudio->ReleaseObj();
		m_flvHeaderAudio = NULL;
	}

	



	
	


#if !defined(_MDSOURCE_MT)
	MtxDestroy(  &m_mtxList  );
#endif
	LogPrint(_LOG_LEVEL_INFO, _TAGNAME, "%s %p \n", __FUNCTION__, this);

	MtxDestroy(&m_mtxVideoFrames);
	MtxDestroy(&m_mtxMetaData);
	MtxDestroy(&m_mtxFlvHeader);
    MtxDestroy(&m_mtxSrcCtx);


}
void CMdSource::RelaseMediaKeyInfoVideo()
{
    if (m_rtmpSpsPpsBuf != NULL)
    {
        m_rtmpSpsPpsBuf->ReleaseObj();
        m_rtmpSpsPpsBuf = NULL;
    }

    if (m_flvSpsPpsTag != NULL)
    {
        m_flvSpsPpsTag->ReleaseObj();
        m_flvSpsPpsTag = NULL;
    }


}
void CMdSource::RelaseMediaKeyInfoAudio()
{

    if (m_flvAACHeaderTag != NULL)
    {
        m_flvAACHeaderTag->ReleaseObj();
        m_flvAACHeaderTag = NULL;
    }

    if (m_rtmpAACHeaderBuf != NULL)
    {
        m_rtmpAACHeaderBuf->ReleaseObj();
        m_rtmpAACHeaderBuf = NULL;
    }
}

void CMdSource::ReOpen()
{
#if defined(_HLS_SUPPORT_)
	if (m_hls != NULL)
	{
	//	m_hls->ReOpen();
	}
#endif

#if defined(_MP4_SUPPORT_)
	if (m_mp4 != NULL)
	{
		m_mp4->ReOpen();
	}
#endif	

#if defined(_FLVREC_SUPPORT_)
    if (m_flv != NULL)
    {
     //   m_flv->ReOpen();
    }
#endif	
}

void CMdSource::ClearSrcCtxIfMatch(CCtx *ctx)
{
    CMtxAutoLock lock(&m_mtxSrcCtx);
    if (ctx != m_ctxSrc)
    {
        return;
    }
    m_ctxSrc = NULL;
}
void CMdSource::CopySrcCid2Ctx(CCtx *ctx)
{
    CMtxAutoLock lock(&m_mtxSrcCtx);
    if (m_ctxSrc != NULL)
    {
        ctx->m_srcDevId = m_ctxSrc->m_devId;
        ctx->m_nSrcCid = m_ctxSrc->m_nCid;
    }
}
void CMdSource::CloseSrcCtx()
{
    CMtxAutoLock lock(&m_mtxSrcCtx);
    if (m_ctxSrc != NULL)
    {
        m_ctxSrc->Close();
    }
}
void CMdSource::DelayRemoveMdSourceIfMatchSrcCtx(CCtx *ctx, BOOL bSettingSrv)
{
    CMtxAutoLock lock(&m_mtxSrcCtx);

    if (ctx != m_ctxSrc)
    {
        return;
    }

    m_ctxSrc = NULL;

    if (_AppConfig.m_nDelayclose_virtual_live_seconds == 0)
    {
        return;
    }

}
void CMdSource::AttachNewSrcCtx(CCtx *ctx)
{
    CMtxAutoLock lock(&m_mtxSrcCtx);

    if (ctx == m_ctxSrc)
    {
        return;
    }

    

    CCtx *ctxOld = m_ctxSrc;
    m_ctxSrc = ctx;



    if (ctxOld != NULL)
    {
        ctxOld->AddRef();
        ctxOld->Close();
        ctxOld->ReleaseRef();


    }

    m_bNewSrcCtx = TRUE;
    m_timestampDelta = m_timestampMax;
    RelaseMediaKeyInfoAudio();
    RelaseMediaKeyInfoVideo();

 

}
BOOL CMdSource::IsOpened()
{
	if( CtxTypeRTSP == m_srcType )
	{
		return (m_nSdpLength > 0) ;
	}
	else if( CtxTypeETS == m_srcType )
	{
		return TRUE ;
	}
	else if( CtxTypeRTMP == m_srcType || CtxTypeRTMPT == m_srcType  )
	{
		return m_bRtmpSrcInited  ;
	}
	else 
	{
		return FALSE ;
	}
	
}
#if defined(_RTSP_SUPPORT_)
int CMdSource::GetRtspSDP(XCodec codec,int trackId,char *buffer,int len )
{
	buffer[ len - 1 ] = 0 ;
	buffer[0] = 0 ;
	int nLen = 0;


	switch ( codec )
	{
	case XCodecMP4V_ES:
		{
			nLen = snprintf(buffer, len -1 ,
				"m=video 0 RTP/AVP %d\r\n"
				"a=rtpmap:%d MP4V-ES/%d\r\n"
				"a=fmtp:%d profile-level-id=1;config=000001B001000001B50900000100000001200086C4002B18582120A31F00\r\n"
				"a=control:trackID=%d\r\n",
				PT(codec),PT(codec),
				_SAMPLE_RATE_ ,
				PT(codec),
				trackId
				) ;

			return nLen ;
		}
	case XCodecH263:
		{
			nLen = snprintf(buffer, len -1 ,
				"m=video 0 RTP/AVP %d\r\n"
				"a=rtpmap:%d H263-1998/%d\r\n"
				"a=control:trackID=%d\r\n",
				PT(codec),PT(codec),
				_SAMPLE_RATE_ ,
				trackId
				) ;

			return nLen  ;
		}
	case XCodecH264:
		{
			nLen = snprintf(buffer, len -1 ,	
				"m=video 0 RTP/AVP %d\r\n"
				"a=rtpmap:%d H264/%d\r\n"
				"a=fmtp:%d packetization-mode=1\r\n"
				"a=control:trackID=%d\r\n",
				PT(codec),PT(codec),
				_SAMPLE_RATE_ ,
				PT(codec),
				trackId
				) ;

			return nLen  ;
		}
	case XCodecAudioAMRNB:
		{
			int channels = m_amrProp.channels ;
			if( channels == 0 )
			{
				channels = 1 ;
			}
			int samplesRate = m_amrProp.samplerate ;
			if( samplesRate == 0 )
			{
				samplesRate = _SAMPLE_RATE_AUDIO_ ;
			}

			nLen = snprintf(buffer, len -1 ,	
				"m=audio 0 RTP/AVP %d\r\n"
				"a=rtpmap:%d AMR/%d/%d\r\n"
				"a=fmtp:%d octet-align=1\r\n"
				"a=control:trackID=%d\r\n",
				PT(codec),PT(codec),
				samplesRate ,
				channels,
				PT(codec),
				trackId
				) ;

			return nLen  ;
		}
	case XCodecAudioAAC:
		{
			int channels = m_adtsHeader.channels ;


			if( channels == 0 )
			{
				channels = 1 ;
			}
			nLen = snprintf(buffer, len -1 ,	
				"m=audio 0 RTP/AVP %d\r\n"
				"a=rtpmap:%d mpeg4-generic/%d/%d\r\n"
				"a=fmtp:%d streamtype=5;  mode=AAC-hbr; config=%s; profile-level-id=15; SizeLength=13; IndexLength=3; IndexDeltaLength=3; Profile=%d;\r\n"
				"a=control:trackID=%d\r\n",
				PT(codec),PT(codec),
				m_adtsHeader.SampleRate() ,channels,
				PT(codec),
                m_szAACConfigStr, CADTSHeader::ADTSProfile2AACConfig(m_adtsHeader.profile)-1 ,
				trackId
				) ;

			return nLen  ;
		}
	default:
		return 0 ;
	}
}
CRTSPNetIOBuffer  * CMdSource::GetSDP(CRTSPNetContext *ctx ,char const* cseq)
{
	int  i = 0 ;
	CRTSPNetIOBuffer  *ioBuf = NULL ;
	char *pBuf = NULL ;
	int nBufLen = 0 ;
	char bufDate[200];

	
	if( m_nSdpLength < 1 )
	{
		if( m_videoType == XCodecUnknown && m_audioType ==  XCodecUnknown )
		{
			return NULL ;
		}
		char szSdp[2048];
		int nSdpLen = 0 ;
		memset( szSdp + sizeof(szSdp) -8 , 0, 8 );

		if( m_videoType != XCodecUnknown )
		{
			nSdpLen = GetRtspSDP( m_videoType ,_VIDEO_TRACKID , szSdp ,  sizeof(szSdp)-8 );
		}

		
		if( m_audioType != XCodecUnknown )
		{
			nSdpLen += GetRtspSDP( m_audioType ,_AUDIO_TRACKID , szSdp + nSdpLen ,  sizeof(szSdp) - nSdpLen-8  );
		}
		

		ParseSdp( szSdp );

	}


	ioBuf = CRTSPNetIOBuffer::CreateObj();
	if( ioBuf == NULL )
	{	
		return NULL ;
	}
	pBuf = ioBuf->m_pBuf ;
	nBufLen = ioBuf->m_nBufLen ;	

	i = 0 ;

	i +=  snprintf( pBuf + i , nBufLen - i ,
		"RTSP/1.0 200 OK\r\n"
		"Server: %s\r\n"
		"CSeq: %s\r\n"
		"%s"
		"x-Accept-Retransmit: our-retransmit\r\n"
		"x-Accept-Dynamic-Rate: 1\r\n"
		"Content-Type: application/sdp\r\n"
		"Content-length: %d\r\n"
		"Content-Base: %s/\r\n"
		"\r\n"
		"%s"
		,
		GetServerInfo() ,
		cseq ,
		CRtspEvent::dateHeader(bufDate),
		m_nSdpLength ,
		(const char *)(ctx->m_uri),
		m_szSdp
		);


	ioBuf->m_nDataLen = i ;

	return ioBuf ;
}
RTSPSTREAM*  CMdSource::GetSetupStream()
{
	int i = 0;
	for (i = 0; i < m_StreamCount && i < _RTSP_MAX_STREAM_; i++)
	{
		if (!m_streams[i].bSetup)
		{
			return (m_streams + i);
		}
	}

	return NULL;

}
BOOL CMdSource::ParseSdp( char *szBuf )
{
	int iStream = 0;
	int i = 0;
	char *pTagM = NULL;
	char *pTagMNext = NULL;
	char *pTmp = NULL;

	char szTmp[SIZE_2KB];
	int nReaden = 0;
	int nSdpLength = strlen(szBuf);
	m_nSdpLength = 0;
	CObjMemIO ioSdpRead(szBuf, nSdpLength);
	CObjMemIO ioSdpWrite(m_szSdp, sizeof(m_szSdp));

	while (!ioSdpRead.IsEof())
	{
		nReaden = ioSdpRead.ReadLine(szTmp, sizeof(szTmp) - 1);
		if (nReaden < 1)
		{
			continue;
		}


		if (strcmpipre(szTmp, "o=-") == 0)
		{
			continue;
		}

		static char szAControl[] = "a=control:";
		if (strcmpipre(szTmp, szAControl) == 0)
		{
			i = strcmpifind_from_end(szTmp + nReaden - 1, "/", szTmp);

			if (i > 0)
			{
				pTmp = szTmp + i + 1;
				if (pTmp[0] == 0 || pTmp[0] == '?')
				{
					pTmp[0] = '*';
					pTmp[1] = 0;
					continue;

				}
				m_nSdpLength += ioSdpWrite.Write(szAControl, -1);
				m_nSdpLength += ioSdpWrite.Write(pTmp, -1);
				m_nSdpLength += ioSdpWrite.Write("\r\n", -1);
				continue;
			}

		}

		m_nSdpLength += ioSdpWrite.Write(szTmp, nReaden);
		m_nSdpLength += ioSdpWrite.Write("\r\n", -1);


	}






	GetValueFromString(m_szSdp, "a=MaxPacketSize:integer", szTmp, sizeof(szTmp), ";", "\"\r\n ;");
	m_MaxPacketSize = atoi(szTmp);
	GetValueFromString(m_szSdp, "a=StreamCount:integer", szTmp, sizeof(szTmp), ";", "\r\n ;");
	m_StreamCount = atoi(szTmp);
	if (m_StreamCount > _RTSP_MAX_STREAM_)
	{
		return FALSE;
	}

	if (m_StreamCount == 0)
	{
		m_StreamCount = 2;
	}

	GetValueFromString(m_szSdp, "a=pixel", szTmp, sizeof(szTmp), ":", "\"\r\n ;");
	for (i = 0; i < m_StreamCount; i++)
	{
		strcpyn(m_streams[iStream].pixel, szTmp, sizeof(m_streams[iStream].pixel));
	}

	pTagM = FindNextMtag(szBuf);
	if (pTagM != NULL)
	{
		pTagMNext = FindNextMtag(pTagM + 1);
	}

	while (pTagM != NULL && (iStream < m_StreamCount))
	{
		GetValueFromString(pTagM, "m", m_streams[iStream].m, sizeof(m_streams[iStream].m), "=", "\r\n");
		GetValueFromString(pTagM, "a=mimetype:string;", m_streams[iStream].mimetype, sizeof(m_streams[iStream].mimetype), "\"", "\"\r\n ;");

		nReaden = GetValueFromString(pTagM, "a=control", szTmp, sizeof(szTmp), ":", "\"\r\n ;");

		if (nReaden > 0)
		{
			pTmp = szTmp;
			i = strcmpifind_from_end(szTmp + nReaden - 1, "/", szTmp);
			if (i > 0)
			{
				pTmp = szTmp + i + 1;
			}



			i = strcmpifind(pTmp, "?");
			if (i >= 0)
			{
				pTmp[i] = 0;
				strcpyn(m_streams[iStream].control_query, pTmp + i + 1, sizeof(m_streams[iStream].control_query));
			}

			strcpyn(m_streams[iStream].control, pTmp, sizeof(m_streams[iStream].control));




		}




		GetValueFromString(pTagM, "a=AvgBitRate:integer", szTmp, sizeof(szTmp), ";", "\"\r\n ;");
		m_streams[iStream].AvgBitRate = atoi(szTmp);
		GetValueFromString(pTagM, "a=MaxBitRate:integer", szTmp, sizeof(szTmp), ";", "\"\r\n ;");
		m_streams[iStream].MaxBitRate = atoi(szTmp);
		GetValueFromString(pTagM, "a=length:npt", szTmp, sizeof(szTmp), "=", "\"\r\n ;");
		m_streams[iStream].length = atof(szTmp);

		GetValueFromString(pTagM, "a=fmtp", m_streams[iStream].rtpmap.fmtp, sizeof(m_streams[iStream].rtpmap.fmtp), ":", "\r\n;");
		GetValueFromString(pTagM, "a=rtpmap", szTmp, sizeof(szTmp), ":", "\r\n;");

		if (szTmp[0] != 0)
		{
			for (i = 0; szTmp[i] != 0; i++)
			{
				if (szTmp[i] == ' ')
				{
					szTmp[i] = 0;
					m_streams[iStream].rtpmap.pt = atoi(szTmp);
					i++;
					for (; szTmp[i] == ' '; i++);

					{
						char *pszType = szTmp + i;
						for (; szTmp[i] != 0; i++)
						{
							if (szTmp[i] == '/')
							{
								szTmp[i] = 0;
								strcpyn(m_streams[iStream].rtpmap.method, pszType, sizeof(m_streams[iStream].rtpmap.method));
								i++;
								m_streams[iStream].rtpmap.rate = atoi(szTmp + i);

								break;

							}

						}
						for (; szTmp[i] != 0; i++)
						{
							if (szTmp[i] == '/')
							{
								m_streams[iStream].rtpmap.channels = atoi(szTmp + i + 1);
								break;
							}
						}

					}
					break;
				}

			}
		}


		iStream++;
		pTagM = pTagMNext;
		if (pTagM != NULL)
		{
			pTagMNext = FindNextMtag(pTagM + 1);
		}
	}
	m_StreamCount = iStream;



	return  (iStream > 0);
}
int  CMdSource::GetTrackIndex(const char *track)
{
	int i = 0;
	for (i = 0; i < m_StreamCount && i < _RTSP_MAX_STREAM_; i++)
	{
		if (strcmpix(track, m_streams[i].control) == 0)
		{
			return i;
		}
	}

	return -1;
}
unsigned int CMdSource::SendRtspData( CRTSPNetIOBuffer *ioBuf1)
{

	unsigned int nBytes = 0;

#if defined(_MDSOURCE_MT)
	int i = 0;
	for( i = 0 ; i < _MDSOURCE_THREADS ; i ++ )
	{
		nBytes += m_clientLists[i].SendRtspData(ioBuf1 , this );
	}
#else
	CMtxAutoLock lock( &m_mtxList );

	CObj * const pItemHead = &m_clientList;

	CObj *pItem = NULL;

	pItem = pItemHead->m_pNextObj ; 

	while( pItem != NULL && pItem != pItemHead )
	{
		CCtx *ctxTmp = Convert2CCtx( pItem );
		pItem = pItem->m_pNextObj ;

		if(   ctxTmp->m_typeCtx != CtxTypeRTSP )
		{
			continue ;
		}
		CRTSPNetContext *ctx = (CRTSPNetContext *)(ctxTmp);
	
		

		if( ctx->m_status != RtspStatusPlaying )
		{
			continue ;
		}

		CRTSPNetIOBuffer *ioBuf = CRTSPNetIOBuffer::CreateObj( ioBuf1 );
		if( ioBuf == NULL )
		{
			ctx->Close();
		}
		else
		{
			if( !ctx->Send( ioBuf ) )
			{
				ioBuf->ReleaseObj();
				ctx->Close();
			}
			else
			{
				nBytes += ioBuf1->m_nDataLen ;
			}
		}


	}
#endif

	return nBytes ;
}

#endif

char *CMdSource::FindNextMtag(char *szBuf)
{
	int i = 0 ;
	if( szBuf == NULL )
	{
		return NULL ;
	}
	for( i = 0 ; szBuf[i] != 0 ; i ++ )
	{
		if( strcmpipre(szBuf + i ,"m=") == 0 ) 
		{
			if( i == 0 )
			{
				return ( szBuf + i );
			}
			else if( szBuf[i-1] == '\r' || szBuf[i-1] == '\n' )
			{
				szBuf[i-1] = 0 ;
				return ( szBuf + i );
			}
		}

	}
	return NULL ;

}

CObjNetIOBuffer *CMdSource::AllocTSIOBuffer()
{
	return CObjNetIOBufferSharedMemoryTSSource::CreateObj(NULL, 18800, TRUE);
}
BOOL CMdSource::CreateFlvHeader()
{
	CMtxAutoLock lock(&m_mtxFlvHeader);
	if (m_flvHeader == NULL)
	{
		

		CAMFBuffer buf;
		const int  headsize = 9 + 4 + _FLV_TAGHEADER_LEN;



		buf.m_channelId = RTMP_METADATA_CHANNEL;
		buf.m_chunksize = SIZE_1M;

		buf.SetReadPos(headsize);

		buf.Data[0] = 'F';
		buf.Data[1] = 'L';
		buf.Data[2] = 'V';

		buf.Data[3] = 0x01;

		buf.Data[4] = 0x00;

		if (m_bHaveVideo)
		{
			buf.Data[4] |= 0x01;
		}

		if (m_bHaveAudio)
		{
			buf.Data[4] |= 0x04;
		}


		buf.Data[5] = 0x00;
		buf.Data[6] = 0x00;
		buf.Data[7] = 0x00;
		buf.Data[8] = 0x09;


		buf.Data[9] = 0x00;
		buf.Data[10] = 0x00;
		buf.Data[11] = 0x00;
		buf.Data[12] = 0x00;



		CAMFObject::EncodeString(buf, "onMetaData");

		CAMFObject::EncodeObjectBegin(buf);



		if (m_width != 0 && m_height != 0)
		{
			CAMFObject::EncodeNamedNumber(buf, "width", m_width);
			CAMFObject::EncodeNamedNumber(buf, "height", m_height);
		}

		if (m_avclevel != 0)
		{
			CAMFObject::EncodeNamedNumber(buf, "avclevel", m_avclevel);
		}

		if (m_avcprofile != 0)
		{
			CAMFObject::EncodeNamedNumber(buf, "avcprofile", m_avcprofile);
		}

		if (m_framerate != 0)
		{
			CAMFObject::EncodeNamedNumber(buf, "framerate", m_framerate);
		}



		CAMFObject::EncodeNamedNumber(buf, "videocodecid", _FLV_CODECID_H264);
		CAMFObject::EncodeObjectEnd(buf);






		UINT8 *Data = buf.Data + 13;

		Data[0] = RTMPPacketType_Script;
		CAMFObject::EncodeInt24((UINT8 *)(Data + 1), buf.m_amfsize);		// packet size
		CAMFObject::EncodeInt24((UINT8 *)(Data + 1 + 3), 0);	// timmer
		CAMFObject::EncodeInt8((UINT8 *)(Data + 1 + 3 + 3), 0);	// ext timmer
		CAMFObject::EncodeInt24((UINT8 *)(Data + 1 + 3 + 3 + 1), 0); // stream id

		UINT8 szTmp[4];
		CAMFObject::EncodeInt32(szTmp, buf.m_amfsize + _FLV_TAGHEADER_LEN); // pre tag size
		buf.Write(szTmp, 4, 1);


		buf.SetReadPos(0);

		m_flvHeader = CObjNetIOBufferSharedMemory::CreateObj(NULL, AlignLength(buf.DataLength(), SIZE_1KB), TRUE);
		if (m_flvHeader == NULL)
		{
			return FALSE;
		}
		memcpy(m_flvHeader->m_pBuf, buf.Data, buf.DataLength());
		m_flvHeader->m_nDataLen = buf.DataLength();


	}


	if (m_flvHeaderVideo == NULL)
	{
		CAMFBuffer buf;
		const int  headsize = 9 + 4 + _FLV_TAGHEADER_LEN;



		buf.m_channelId = RTMP_METADATA_CHANNEL;
		buf.m_chunksize = SIZE_1M;

		buf.SetReadPos(headsize);

		buf.Data[0] = 'F';
		buf.Data[1] = 'L';
		buf.Data[2] = 'V';

		buf.Data[3] = 0x01;

		buf.Data[4] = 0x00;
		buf.Data[4] |= 0x01;


		buf.Data[5] = 0x00;
		buf.Data[6] = 0x00;
		buf.Data[7] = 0x00;
		buf.Data[8] = 0x09;


		buf.Data[9] = 0x00;
		buf.Data[10] = 0x00;
		buf.Data[11] = 0x00;
		buf.Data[12] = 0x00;



		CAMFObject::EncodeString(buf, "onMetaData");

		CAMFObject::EncodeObjectBegin(buf);



		if (m_width != 0 && m_height != 0)
		{
			CAMFObject::EncodeNamedNumber(buf, "width", m_width);
			CAMFObject::EncodeNamedNumber(buf, "height", m_height);
		}

		if (m_avclevel != 0)
		{
			CAMFObject::EncodeNamedNumber(buf, "avclevel", m_avclevel);
		}

		if (m_avcprofile != 0)
		{
			CAMFObject::EncodeNamedNumber(buf, "avcprofile", m_avcprofile);
		}

		if (m_framerate != 0)
		{
			CAMFObject::EncodeNamedNumber(buf, "framerate", m_framerate);
		}



		CAMFObject::EncodeNamedNumber(buf, "videocodecid", _FLV_CODECID_H264);
		CAMFObject::EncodeObjectEnd(buf);






		UINT8 *Data = buf.Data + 13;

		Data[0] = RTMPPacketType_Script;
		CAMFObject::EncodeInt24((UINT8 *)(Data + 1), buf.m_amfsize);		// packet size
		CAMFObject::EncodeInt24((UINT8 *)(Data + 1 + 3), 0);	// timmer
		CAMFObject::EncodeInt8((UINT8 *)(Data + 1 + 3 + 3), 0);	// ext timmer
		CAMFObject::EncodeInt24((UINT8 *)(Data + 1 + 3 + 3 + 1), 0); // stream id

		UINT8 szTmp[4];
		CAMFObject::EncodeInt32(szTmp, buf.m_amfsize + _FLV_TAGHEADER_LEN); // pre tag size
		buf.Write(szTmp, 4, 1);


		buf.SetReadPos(0);

		m_flvHeaderVideo = CObjNetIOBufferSharedMemory::CreateObj(NULL, AlignLength(buf.DataLength(), SIZE_1KB), TRUE);
		if (m_flvHeaderVideo == NULL)
		{
			return FALSE;
		}
		memcpy(m_flvHeaderVideo->m_pBuf, buf.Data, buf.DataLength());
		m_flvHeaderVideo->m_nDataLen = buf.DataLength();


	}


	if (m_flvHeaderAudio == NULL)
	{
		CAMFBuffer buf;
		const int  headsize = 9 + 4 + _FLV_TAGHEADER_LEN;



		buf.m_channelId = RTMP_METADATA_CHANNEL;
		buf.m_chunksize = SIZE_1M;

		buf.SetReadPos(headsize);

		buf.Data[0] = 'F';
		buf.Data[1] = 'L';
		buf.Data[2] = 'V';

		buf.Data[3] = 0x01;

		buf.Data[4] = 0x00;


		buf.Data[4] |= 0x04;





		buf.Data[5] = 0x00;
		buf.Data[6] = 0x00;
		buf.Data[7] = 0x00;
		buf.Data[8] = 0x09;


		buf.Data[9] = 0x00;
		buf.Data[10] = 0x00;
		buf.Data[11] = 0x00;
		buf.Data[12] = 0x00;



		CAMFObject::EncodeString(buf, "onMetaData");

		CAMFObject::EncodeObjectBegin(buf);




		CAMFObject::EncodeNamedString(buf, "audiocodecid", "mp4a");
		CAMFObject::EncodeObjectEnd(buf);






		UINT8 *Data = buf.Data + 13;

		Data[0] = RTMPPacketType_Script;
		CAMFObject::EncodeInt24((UINT8 *)(Data + 1), buf.m_amfsize);		// packet size
		CAMFObject::EncodeInt24((UINT8 *)(Data + 1 + 3), 0);	// timmer
		CAMFObject::EncodeInt8((UINT8 *)(Data + 1 + 3 + 3), 0);	// ext timmer
		CAMFObject::EncodeInt24((UINT8 *)(Data + 1 + 3 + 3 + 1), 0); // stream id

		UINT8 szTmp[4];
		CAMFObject::EncodeInt32(szTmp, buf.m_amfsize + _FLV_TAGHEADER_LEN); // pre tag size
		buf.Write(szTmp, 4, 1);


		buf.SetReadPos(0);

		m_flvHeaderAudio = CObjNetIOBufferSharedMemory::CreateObj(NULL, AlignLength(buf.DataLength(), SIZE_1KB), TRUE);
		if (m_flvHeaderAudio == NULL)
		{
			return FALSE;
		}
		memcpy(m_flvHeaderAudio->m_pBuf, buf.Data, buf.DataLength());
		m_flvHeaderAudio->m_nDataLen = buf.DataLength();


	}






	return TRUE;
}
BOOL CMdSource::CreateRtmpH264MetaData()
{
	if (m_rtmpMetaDataBuf != NULL)
	{
		return TRUE;
	}



	CMtxAutoLock lock(&m_mtxMetaData);

	

	CAMFBuffer buf;
	const int  headsize = 12;



	buf.m_channelId = RTMP_METADATA_CHANNEL;
	buf.m_chunksize = GetRTMPChunkSize();

	buf.SetReadPos(headsize);
	CAMFObject::EncodeString(buf, "onMetaData");

	CAMFObject::EncodeObjectBegin(buf);



	if (m_width != 0 && m_height != 0)
	{
		CAMFObject::EncodeNamedNumber(buf, "width", m_width);
		CAMFObject::EncodeNamedNumber(buf, "height", m_height);
	}

	if (m_avclevel != 0)
	{
		CAMFObject::EncodeNamedNumber(buf, "avclevel", m_avclevel);
	}

	if (m_avcprofile != 0)
	{
		CAMFObject::EncodeNamedNumber(buf, "avcprofile", m_avcprofile);
	}

	if (m_framerate != 0)
	{
		CAMFObject::EncodeNamedNumber(buf, "framerate", m_framerate);
	}

 



	CAMFObject::EncodeNamedNumber(buf, "videocodecid", _FLV_CODECID_H264);
	CAMFObject::EncodeObjectEnd(buf);

	CAMFObject *obj = CAMFObject::CreateFromAMFBuffer(buf.Data + buf.ReadPos, buf.DataLength());
	obj->Dump();
	obj->ReleaseObj();



	buf.SetReadPos(0);


	memset(buf.Data, 0, headsize);

	buf.Data[0] = buf.m_channelId;
	CAMFObject::EncodeInt24(buf.Data + 4, buf.m_amfsize);
	buf.Data[7] = RTMPPacketType_Script;
	CAMFObject::EncodeInt32LE(buf.Data + 8, GetRTMPStreamId());

	m_rtmpMetaDataBuf = CObjNetIOBufferSharedMemory::CreateObj(NULL, AlignLength(buf.DataLength(), SIZE_1KB), TRUE);
	if (m_rtmpMetaDataBuf == NULL)
	{

		return FALSE;
	}
	memcpy(m_rtmpMetaDataBuf->m_pBuf, buf.Data, buf.DataLength());
	m_rtmpMetaDataBuf->m_nDataLen = buf.DataLength();


	return TRUE;
}
int  CMdSource::GetRTMPChunkSize()
{

	if(  m_rtmpChunkSize != 0 )
	{
		return m_rtmpChunkSize ;
	}
	else
	{
		return 65535;
	}

}
int  CMdSource::GetRTMPStreamId()
{
	return 1 ;
}
void CMdSource::GenAACConfigStr()
{
	m_szAACConfigStr[ 0 ] = 0 ;
	for( int i  = 2 ; i < 4 ; i ++ )
	{
		char szTmp[20];
		sprintf( szTmp, "%02X" , m_szAACConfig[i] );
		strcatn( m_szAACConfigStr, szTmp, sizeof(m_szAACConfigStr)  );

	}
}
//#define _TIME_CORRECT
#define _TIME_CORRECT_DELTA  100

static char * _rtmpAudioFormat[16] =
{
    "Linear PCM,platform endian",
    "ADPCM",
    "MP3",
    "Linear PCM, little endian",
    "Nellymoser 16 kHz mono",
    "Nellymoser 8 kHz mono",
    "Nellymoser",
    "G.711 A-law logarithmic PCM",
    "G.711 mu-law logarithmic PCM",
    "9 reserved",
    "AAC",
    "Speex",
    "12 unknown",
    "13 unknown",
    "MP3 8 kHz" ,
    "Device-specific sound"
};
unsigned int CMdSource::SendRTMP_AUDIO(CRTMPHeader *header, CObjNetIOBuffer *pBuffer)
{

    return SendRTMP_AAC(header, pBuffer);
 
  
}
unsigned int CMdSource::SendRTMP_AAC(CRTMPHeader *header,CObjNetIOBuffer *pBuffer)
{
	m_audioType = XCodecAudioAAC ;



	UINT8 *buf = (UINT8 *) (pBuffer->m_pBuf) ;
	int    nDataLen = pBuffer->m_nDataLen ;
	CObjBuffer objBuf;
	objBuf.bMalloc2 = TRUE ;
	int ChannelId = header->ChannelId;
    TIME_T tmTimeFrame = 0;
#define _ALIGN_AAC SIZE_1KB

    if (header->Timmer_Type == CRTMPHeader::TimmerTypeABS)
    {
		m_tmChannel[ChannelId] = header->Timmer;
		//m_rtmpTimeAudio = header->Timmer;
        
    }
    else if (header->Timmer_Type == CRTMPHeader::TimmerTypeExt)
    {
        m_tmChannel[ChannelId] = header->Timmer;
        //m_rtmpTimeVideo = header->Timmer ;
    }
    else if( header->Timmer_Type == CRTMPHeader::TimmerTypeRelative )
    {
		m_tmChannel[ChannelId] += header->Timmer;
        //m_rtmpTimeVideo += (header->Timmer);

    }
    tmTimeFrame = m_tmChannel[ChannelId] + m_timestampDelta;
	//printf("%d\n", header->Timmer);
	//printf("%d\n", m_rtmpTimeVideo);
	//m_rtmpTimeVideo = OSTickCount();

    if (tmTimeFrame > m_timestampMax)
    {
        m_timestampMax = tmTimeFrame;
    }

 

	if( buf[1] == 0x00  )
	{
		buf += 2 ;
		nDataLen -= 2 ;
		if (nDataLen >= 2 )
		{
    
            RelaseMediaKeyInfoAudio();
			m_adtsHeader.DecodeAACConfigData(buf);
		}
		

		if( !m_adtsHeader.IsValid() )
		{
			UINT8 bufConfig[4];
			CADTSHeader::GetAACConfigData(2, m_audiosamplerate, m_audiochannels, bufConfig);

			m_adtsHeader.DecodeAACConfigData(bufConfig+2);

			if (!m_adtsHeader.IsValid())
			{
				return -1;
			}

			
		}

		

		m_adtsHeader.GetAACConfigData( m_szAACConfig );
		GenAACConfigStr();


		if( m_rtmpAACHeaderBuf != NULL )
		{
			m_rtmpAACHeaderBuf->ReleaseObj() ;
			m_rtmpAACHeaderBuf = NULL ;
		}

	}
	else if( buf[1] == 0x01  )
	{
		buf += 2 ;
		nDataLen -= 2 ;

		if( !m_adtsHeader.IsValid() )
		{
			return 0 ;
		}


        m_tmAudioAbsTimeUpdate = OSTickCount();
		m_audioAbsTime += m_adtsHeader.frame_dur;
		objBuf.Alloc( nDataLen + 8*2, _ALIGN_AAC );
		UINT8 adts[7];
		m_adtsHeader.frame_length = 7 + nDataLen ;
		m_adtsHeader.EncodeADTS( adts , 7 );

		objBuf.Write( adts , 7,  _ALIGN_AAC ) ;
		objBuf.Write( buf , nDataLen,  _ALIGN_AAC ) ;
        
        if (m_bVideoUseAudioTime)
        {
            tmTimeFrame = m_audioAbsTime;
        }


		if (m_firstRTMPFrameTime == -1)
		{
            m_firstRTMPFrameTime = tmTimeFrame;
		}
		if (m_rtmpTimePreAudio == 0)
		{
            m_rtmpTimePreAudio = tmTimeFrame;
		}


		m_iAudioFrameQueue++;




        if( header->Timmer_Type == CRTMPHeader::TimmerTypeABS )
        {
            
            //SendETS_AAC( objBuf.Data, objBuf.DataLength( ), m_rtmpTimeAudio );
#if defined(_MDSOURCE_MT)
            SendETS_AAC_MT(objBuf.Data, objBuf.DataLength(), tmTimeFrame, m_iAudioFrameQueue);
#else
            SendETS_AAC(objBuf.Data, objBuf.DataLength(), tmTimeFrame,m_iAudioFrameQueue);
#endif
        }
        else if( header->Timmer_Type == CRTMPHeader::TimmerTypeRelative )
        {
			
#if defined(_TIME_CORRECT)

			if ( (m_rtmpTimeVideo - m_audioAbsTime) > 1000) //rtmp 发送端长时间没有发送音频，修正实际音频时间
			{
				m_audioAbsTime = m_rtmpTimeVideo;
			}

			if (fabs(m_audioAbsTime - m_rtmpTimeVideo) > _TIME_CORRECT_DELTA )
			{
				m_rtmpTimeVideo = m_audioAbsTime ;
			}
#endif
			//SendETS_AAC(objBuf.Data, objBuf.DataLength(), m_rtmpTimeVideo);
#if defined(_MDSOURCE_MT)
            SendETS_AAC_MT(objBuf.Data, objBuf.DataLength(), tmTimeFrame, m_iAudioFrameQueue);
#else
            SendETS_AAC(objBuf.Data, objBuf.DataLength(), tmTimeFrame,m_iAudioFrameQueue);
#endif
        }

        
        
	}
	else
	{
		return -1;
	}





	return 0 ;
}
unsigned int CMdSource::SendRTMP_H264(CRTMPHeader *header,CObjNetIOBuffer *pBuffer)
{
#define _ALIGN_H264 SIZE_2KB
	if (m_srcType != CtxTypeRTMP && m_srcType != CtxTypeRTMPT)
	{
		return 0;
	}


	static UINT8 _naluSep[4] = { 0, 0, 0, 1 };
	static UINT8 _Aud[2] = { 0x09 , 0x30 };
	m_videoType = XCodecH264 ;

	UINT8 *buf = (UINT8 *) (pBuffer->m_pBuf) ;
	int    nDataLen = pBuffer->m_nDataLen ;
	BOOL bIDRFrame = FALSE ;
	int nCpy = 0 ;
	UINT32 nNaluLen = 0 ;
	int cts = 0;
    TIME_T tmTimeFrame = 0 ;
	
	CObjBuffer objBuf;
	NaluType naluType = ( NaluType ) 0 ;
	objBuf.bMalloc2 = TRUE ;

	int ChannelId = header->ChannelId;
    
    if( header->Timmer_Type == CRTMPHeader::TimmerTypeABS )
    {
		m_tmChannel[ChannelId] = header->Timmer;
        //m_rtmpTimeVideo = header->Timmer ;
    }
    else if (header->Timmer_Type == CRTMPHeader::TimmerTypeExt)
    {
        m_tmChannel[ChannelId] = header->Timmer;
        //m_rtmpTimeVideo = header->Timmer ;
    }
    else if( header->Timmer_Type == CRTMPHeader::TimmerTypeRelative  )
	{
		m_tmChannel[ChannelId] += header->Timmer;

		//m_rtmpTimeVideo += header->Timmer;
		//printf("%d\n", header->Timmer);


    }
    m_rtmpTimeVideo = tmTimeFrame = (m_tmChannel[ChannelId] + m_timestampDelta);

    if (tmTimeFrame > m_timestampMax)
    {
        m_timestampMax = tmTimeFrame;
    }
    //tmTimeFrame += 5;

    //printf("video:%d\n", (int)tmTimeFrame);

	//LogPrint(_LOG_LEVEL_ERROR, _TAGNAME, "%d\n", (int)m_tmChannel[ChannelId]);
	//printf("%d\n", m_rtmpTimeVideo);
	//m_rtmpTimeVideo = OSTickCount();
	if (nDataLen > 4)
	{
        cts =  (buf[2] << 16) | (buf[3] << 8) | buf[4];
	}




	if( buf[0] == 0x17  && buf[1] == 0 )  // SPS PPS
	{
		buf += 10 ;
		nDataLen -= 10 ;


        
        RelaseMediaKeyInfoVideo();

		
		//if( buf[0] == 0xE1 )
		if ( nDataLen > 4 )
		{
			UINT16 nSpsLen = 0 ;
			nSpsLen = buf[ 1 ] << 8 ;
			nSpsLen |= buf[ 2 ] ;

			nCpy = MinInt( nDataLen - 3 ,nSpsLen )  ;
			if( nCpy < 0 )
			{
				return -1 ;
			}
			m_sps.Drop();
			m_sps.Write( buf + 3 , nCpy  ,  256 );

			buf += ( 3 +  nSpsLen ) ;
			nDataLen -= ( 3 +  nSpsLen ) ;
		}

		//if( buf[0] == 0x01 )
		if (nDataLen > 4)
		{
			UINT16 nPpsLen = 0 ;
			nPpsLen = buf[ 1 ] << 8 ;
			nPpsLen |= buf[ 2 ] ;

			nCpy = MinInt( nDataLen - 3 ,nPpsLen )  ;
			if( nCpy < 0 )
			{
				return -1 ;
			}

			m_pps.Drop();
			m_pps.Write( buf + 3 , nCpy ,  256 );

			buf += ( 3 +  nPpsLen ) ;
			nDataLen -= ( 3 +  nPpsLen ) ;
		}


		if( m_rtmpSpsPpsBuf != NULL )
		{
			m_rtmpSpsPpsBuf->ReleaseObj() ;
			m_rtmpSpsPpsBuf = NULL ;
		}


	}
	else if( buf[1] == 0x01 )  // AVC NALU  
	{
		if( m_sps.DataLength() < 1 || m_pps.DataLength() < 1 )
		{
			if (!m_bCreatedSpsPps)
			{
				m_bCreatedSpsPps = TRUE;
				//X264_GetSpsPps(m_width, m_height, m_framerate, m_avclevel, m_avcprofile, m_videokeyframe_frequency, m_sps, m_pps, m_sei);
			}
			
			return 0 ;
		}
		if( buf[0] == 0x17 )
		{
			bIDRFrame = TRUE ;
            

		}
		else if( buf[0] == 0x27 )
		{
			bIDRFrame = FALSE ;
		}
		else
		{
			bIDRFrame = FALSE ;
		}
		buf += 5 ;
		nDataLen -= 5 ;

		
		 

	
		objBuf.Write( _naluSep , sizeof(_naluSep) ,  _ALIGN_H264 ) ;
		objBuf.Write( _Aud ,  sizeof(_Aud),  _ALIGN_H264 ) ;

		objBuf.Alloc( nDataLen + 10 * 5  + m_sps.DataLength() + m_pps.DataLength() , _ALIGN_H264 );
		if( bIDRFrame && m_sps.DataLength() > 0 &&  m_pps.DataLength() > 0 )
		{
			objBuf.Write( _naluSep , sizeof(_naluSep) ,  _ALIGN_H264 ) ;
			objBuf.Write( m_sps.Data , m_sps.WritePos,  _ALIGN_H264 ) ;

			objBuf.Write( _naluSep , sizeof(_naluSep) ,  _ALIGN_H264 ) ;
			objBuf.Write( m_pps.Data , m_pps.WritePos,  _ALIGN_H264 ) ;

			/*
			if (m_sei.DataLength() > 0)
			{
				objBuf.Write(_naluSep, sizeof(_naluSep), _ALIGN_H264);
				objBuf.Write(m_sei.Data, m_sei.WritePos, _ALIGN_H264);
			}
			*/
		}

		while ( nDataLen > 4 )
		{
			nNaluLen = 0 ;
			nNaluLen |= ( buf[0] << 24  );
			nNaluLen |= ( buf[1] << 16  );
			nNaluLen |= ( buf[2] << 8  );
			nNaluLen |= ( buf[3]  );
			buf += 4 ;
			nDataLen -= 4 ;
			if( nNaluLen > nDataLen  )
			{
				return -1 ;
			}

			
			naluType = (NaluType)((buf[0]) & 0x1f) ;
			if( naluType == NALU_TYPE_AUD
               || naluType == NALU_TYPE_SPS
               || naluType == NALU_TYPE_PPS
             //  || naluType == NALU_TYPE_SEI
               )
			{ 
                buf += nNaluLen ;
                nDataLen -= nNaluLen ;
                continue ;
			}
			
			

			objBuf.Write( _naluSep , sizeof(_naluSep) ,  _ALIGN_H264 ) ;;
			objBuf.Write( buf , nNaluLen,  _ALIGN_H264 ) ;

			buf += nNaluLen ;
			nDataLen -= nNaluLen ;
            
            /*
            if( nDataLen >= 5 )
            {
                nDataLen -= 5 ;
                buf += 5 ;
                
            }
             */

		}
       // printf("%d\n",(int)(header->Timmer/1000));

#if defined(_TIME_CORRECT_VIDEO)        
		if (m_adtsHeader.IsValid() && fabs(m_audioAbsTime - m_rtmpTimeVideo) > _TIME_CORRECT_DELTA)
		{
			m_rtmpTimeVideo = m_audioAbsTime;
		}
#endif
        if (m_adtsHeader.IsValid() && m_bVideoUseAudioTime)
        {
            tmTimeFrame = m_audioAbsTime;
        }

		//SendETS_H264( objBuf.Data , objBuf.DataLength() , m_rtmpTimeVideo ,cts );
		//SendETS_H264(objBuf.Data, objBuf.DataLength(), m_audioAbsTime, cts);
		m_iVideoFrameQueue ++ ;

		if (m_firstRTMPFrameTime == -1)
		{
            m_firstRTMPFrameTime = tmTimeFrame;
		}
		if (m_rtmpTimePreVideo == 0)
		{
            m_rtmpTimePreVideo = tmTimeFrame;
		}



#if defined(_MDSOURCE_MT)
        SendETS_H264_MT(objBuf.Data, objBuf.DataLength(), tmTimeFrame, cts, bIDRFrame ? _H64_KEY_FRAME : _H64_NONE_KEY_FRAME, m_iVideoFrameQueue);
#else
        SendETS_H264(objBuf.Data, objBuf.DataLength(), tmTimeFrame, cts, bIDRFrame ? _H64_KEY_FRAME : _H64_NONE_KEY_FRAME, m_iVideoFrameQueue);
#endif

		
#if defined(_GPS_SUPPORT_)
        if(_gpsOpened && OSTickCount() -  m_tmSendGps >= 1000 )
        {
            int nLen = 0;
            char gpsBuf[SIZE_1KB];
            m_tmSendGps = OSTickCount();
            nLen = ReadGpsData(gpsBuf,sizeof(gpsBuf));
            
            if( nLen > 0 )
            {

            }
        }
#endif
        
        
        
        
	}




	return 0 ;
}



unsigned int CMdSource::SendETSData(CObjNetIOBuffer *ioBuf, BOOL bDeltaTime, CETSConnContext *ctx)
{

	INT64 tmOffset = 0 ;
    int keyFlag = 0;
	UINT8 type = (UINT8)ioBuf->m_pBuf[0];
	int cts = 0;
	unsigned int nRet = 0;

	if (m_srcType != CtxTypeETS)
	{
		return 0;
	}




	if(   ( ETS_VIDEO_BEGIN <= type  &&  type <= ETS_VIDEO_END )  )
	{



		memcpy(&tmOffset, ioBuf->m_pBuf + 1, sizeof(INT64));
		tmOffset = letoh_64(tmOffset);
		cts = ( (tmOffset >> 40) & 0xFFFF );
		tmOffset = (tmOffset & 0X000000FFFFFFFFFF);
		if (bDeltaTime)
		{
			m_tmETSVideo += tmOffset;
		}
		else
		{
			m_tmETSVideo = tmOffset;
		}

		BOOL bIDRFrame = FALSE;
		int  nDataLen = ioBuf->m_nDataLen - 1 - sizeof(INT64);
		UINT8 *pData = (UINT8 *)(ioBuf->m_pBuf + 1 + sizeof(INT64));
		m_iVideoFrameQueue++;
		
		if( type == ETS_H264 )
		{
            keyFlag = tmOffset >> 56;
            CObjBuffer h264Buf;
            h264Buf.bMalloc2 = TRUE;
            if (keyFlag == 0)
            {
                BOOL findAud = FALSE;
                bIDRFrame = IsH264KeyFrame(pData, nDataLen, &findAud);
                if (!findAud)
                {
                    h264Buf.Write(_Aud, sizeof(_Aud), _ALIGN_H264 );
                    h264Buf.Write(pData, nDataLen, _ALIGN_H264);
                    pData = h264Buf.Data;
                    nDataLen = h264Buf.DataLength();
                }

                if (bIDRFrame)
                {
                    keyFlag = _H64_KEY_FRAME;
                }
                else
                {
                    keyFlag = _H64_NONE_KEY_FRAME;
                }

            }
            else
            {
                bIDRFrame = (_H64_KEY_FRAME == keyFlag) || (_H64_KEY_FRAME_NO_PATCH == keyFlag);
            }

			




#if defined(_MDSOURCE_MT)
            nRet = SendETS_H264_MT(pData, nDataLen, m_tmETSVideo, cts, 
                keyFlag, m_iVideoFrameQueue);
#else
            nRet = SendETS_H264(pData, nDataLen, m_tmETSVideo, cts, 
                keyFlag, m_iVideoFrameQueue);
#endif
            
           ////////////////////////////////////////////////////////
            

            //////////////////////////////////////////////////////////////////////
            
			return nRet;
		}
		else if (type == ETS_H265)
		{

            
            ////////////////////////////////////////////////////////
            

            //////////////////////////////////////////////////////////////////////

			return nRet;
		}

	}
	else if(   ( ETS_AUDIO_BEGIN <= type  &&  type <= ETS_AUDIO_END )  )
	{
		memcpy(&tmOffset, ioBuf->m_pBuf + 1, sizeof(INT64));
		tmOffset = letoh_64(tmOffset);
		tmOffset = (tmOffset & 0X000000FFFFFFFFFF);
		if (bDeltaTime)
		{
			m_tmETSAudio += tmOffset;
		}
		else
		{
			m_tmETSAudio = tmOffset;
		}

		int  nDataLen = ioBuf->m_nDataLen - 1 - sizeof(INT64);
		UINT8 *pData = (UINT8 *)(ioBuf->m_pBuf + 1 + sizeof(INT64));
		m_iAudioFrameQueue++;
		
		if( type == ETS_AAC )
		{

#if defined(_MDSOURCE_MT)
			nRet = SendETS_AAC_MT(pData, nDataLen, m_tmETSAudio, m_iAudioFrameQueue);
#else
			nRet = SendETS_AAC(pData, nDataLen, m_tmETSAudio, m_iAudioFrameQueue);
#endif
			return nRet;
		}
	}
	else if(   ( ETS_TEXT_BEGIN <= type  &&  type <= ETS_TEXT_END )  )
	{
		memcpy(&tmOffset, ioBuf->m_pBuf + 1, sizeof(INT64));
		tmOffset = letoh_64(tmOffset);
		tmOffset = (tmOffset & 0X000000FFFFFFFFFF);

		if (bDeltaTime)
		{
			m_tmETSText += tmOffset;
		}
		else
		{
			m_tmETSText = tmOffset;
		}
		
		if( type == ETS_TEXT )
		{

		}

	}
	else
	{

	}
	return 0 ;
}
void CMdSource::CreateFlvSession(const char *szName)
{
#if defined(_FLVREC_SUPPORT_)
	m_flv.Attach(CFlvCtx::CreateObj(szName));
	if (m_flv != NULL)
	{
        m_flv->m_app = m_app;

        {
            CMtxAutoLock lock(&m_mtxSrcCtx);
            if (m_ctxSrc != NULL)
            {
                m_flv->m_query = m_ctxSrc->m_query;
            }
        }


		AddClientSession(m_flv);
	}
#endif

}
CHLSCtx *CMdSource::RemoveHLSSession()
{
#if defined(_HLS_SUPPORT_)
    if( m_hls.p != NULL)
    {
        RemoveClientSession(m_hls);
        return  m_hls.p;
    }
    else
    {
        return NULL;
    }
#else
    return NULL;
#endif
}
void CMdSource::CreateHLSSession(const char *szName, CHLSCtx *oldCtx)
{
#if defined(_HLS_SUPPORT_)
    if (oldCtx != NULL)
    {
        m_hls = oldCtx;
        oldCtx->NewStream();
    }
    else
    {
        m_hls.Attach(CHLSCtx::CreateObj(szName));
    }
	
	if( m_hls != NULL )
	{
		m_hls->m_app = m_app;

        {
            CMtxAutoLock lock(&m_mtxSrcCtx);
            if (m_ctxSrc != NULL)
            {
                m_hls->m_query = m_ctxSrc->m_query;
            }
        }

		AddClientSession( m_hls );
	}
#endif
}
void CMdSource::CreateMP4Session(const char *szName)
{

}
int CMdSource::GetConnections()
{

	int nCount = 0;
#if defined(_MDSOURCE_MT)
    for (int i = 0; i < CtxTypeHLS; i++)
    {
        nCount += m_nConnections[i];
	}
#else
	nCount = m_clientList.ListItemsCount();

#if defined(_MP4_SUPPORT_)
    if (m_mp4 != NULL)
    {
        nCount--;
    }
#endif

#if defined(_HLS_SUPPORT_)
    if (m_hls != NULL)
    {
        nCount--;
    }
#endif

#if defined(_FLVREC_SUPPORT_)
    if (m_flv != NULL)
    {
        nCount--;
    }
#endif

#endif

	return nCount;
}
BOOL CMdSource::AddClientSession(CCtx *ctx)
{
	if( ctx->m_bInMdSource || m_bClosedAll )
	{
		return TRUE;
	}
	if (ctx->m_bCtxClosing || ctx->m_bCtxError)
	{
		return FALSE ;
	}

	CMtxAutoLock lock( &m_mtxList );
	m_clientList.ListAddTail( &(ctx->m_mdItem) );
	ctx->m_bInMdSource = TRUE ;
	ctx->AddRef();
	return TRUE;

}
void CMdSource::RemoveClientSession( CCtx *ctx )
{
	if( !ctx->m_bInMdSource )
	{
		return ;
	}

    BOOL bDel = FALSE;
	CMtxAutoLock lock( &m_mtxList );
    bDel = m_clientList.ListDel(&(ctx->m_mdItem));
	ctx->m_bInMdSource = FALSE ;

	if( bDel)
	{
		ctx->ReleaseRef();
	}
	

}
void CMdSource::CloseAllClients()
{
	if( m_bClosedAll )
	{
		return ;
	}
    m_bClosedAll = TRUE ;

	CMtxAutoLock lock( &m_mtxList );
	CObj * item = NULL ;

	while( ( item = m_clientList.ListRemoveHead()) != NULL )
	{
		CCtx *ctx = Convert2CCtx( item );
		ctx->m_bInMdSource = FALSE ;
		ctx->Close();
		ctx->ReleaseRef();

	}

	m_srcType = CtxTypeUnknown ;
	
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
_CPP_UTIL_CLASSNAME_IMP(CMdUser)
_CPP_UTIL_QUERYOBJ_IMP(CMdUser,CObj)


CMdUser * CMdUser::CreateObj(const char *szUserName)
{
		CMdUser *pObj = NULL ; 
		if( szUserName == NULL || szUserName[0] == 0 )
		{
			return NULL ;
		}
		pObj = new CMdUser(szUserName) ; 
		if( pObj == NULL ) 
		{ 
			return NULL ; 
		} 
		pObj->m_bAlloced = TRUE ; 
		pObj->m_nObjSize = sizeof(CMdUser) ; 
		pObj->AddObjRef(); 
		return pObj ; 
}
void * CMdUser::operator new( size_t cb )
{


	return malloc2(cb);


}
void CMdUser::operator delete( void* p )
{


	free2(p);

}


CMdUser::CMdUser(const char *szUserName)
{

}
CMdUser::~CMdUser()
{

}
















#include "libmediasrv_def.h"

#if defined(_RTSPTS_SUPPORT_) || defined(_HLS_SUPPORT_) || defined(_HTTPTS_SUPPORT_) 



///////////////////////////////////////////////////////

CTSIOCtx::CTSIOCtx()
{
    m_ts_patpmt_interval = TS_SEND_PATPMT_INTERVAL;
    m_bVideoValid = TRUE;
    if (_AppConfig.m_bTsForceVideoAudio)
    {
        m_bAudioValid = TRUE;  
    }
    else
    {
        m_bAudioValid = FALSE;
    }



	m_bHaveInitPatPmt = FALSE ;
	m_tmLastSendPATPMT = 0 ;
	m_tmLastSendPCR = 0 ;
	m_firstTSFrameTime = 0 ;
	m_enablePCR = _AppConfig.m_bEnablePCR;
	
}
CTSIOCtx::~CTSIOCtx()
{

}
CObjNetIOBuffer *CTSIOCtx::AllocTSIOBuffer()
{
	return CObjNetIOBufferSharedMemoryTS::CreateObj(NULL, 18800, TRUE);
}

BOOL CTSIOCtx::CheckPATPMT(TIME_T tmNow,CObj &ioList )
{



    /*
	if( ! m_bHaveInitPatPmt )
	{
		InitTSConnection(ioList);
	}

    */
    if ((tmNow - m_tmLastSendPATPMT) > m_ts_patpmt_interval )
	{
		if( SendPATandPMT(1,ioList) < 1 )
		{
			return FALSE;
		}

	}







	return TRUE ;
}
BOOL CTSIOCtx::CheckPCR(BOOL &bSendPCR, TIME_T tmNow)
{
    bSendPCR = FALSE;

    if ((tmNow - m_tmLastSendPCR) > TS_SEND_PCR_INTERVAL)
    {
        bSendPCR = TRUE;
        m_tmLastSendPCR = tmNow;
    }

    return TRUE;
}
int  CTSIOCtx::SendPATandPMT( int count,CObj &ioList)
{
    int nPackets = 0;
	CObjNetIOBuffer *ioBuf = AllocTSIOBuffer();
	if( ioBuf == NULL )
	{
		return -1;
	}
	m_tmLastSendPATPMT = OSTickCount();

    if (_AppConfig.m_bTs_enable_sdt)
    {
        nPackets = MinInt((ioBuf->m_nBufLen / TS_PACKET_SIZE) / 3, count);
    }
    else
    {
        nPackets = MinInt((ioBuf->m_nBufLen / TS_PACKET_SIZE) / 2, count);
    }
	

	BYTE * buf = (BYTE *)(ioBuf->m_pBuf) ;
	for( int i = 0 ; i < nPackets ; i ++ )
	{

        if (_AppConfig.m_bTs_enable_sdt)
        {
            TS_WriteSDT(buf, NextContinuityCount(TsCCTypeSDT));
            buf += TS_PACKET_SIZE;
        }

		TS_WritePAT( buf ,NextContinuityCount(TsCCTypePAT) );
		buf += TS_PACKET_SIZE ;
        if (m_bAudioValid && m_bVideoValid)
        {
            TS_WritePMT4_H264_AAC(buf, NextContinuityCount(TsCCTypePMT));
            buf += TS_PACKET_SIZE;
        }
        else if (m_bAudioValid)
        {
            TS_WritePMT4_AAC(buf, NextContinuityCount(TsCCTypePMT));
            buf += TS_PACKET_SIZE;
        }
        else if (m_bVideoValid)
        {
            TS_WritePMT4_H264(buf, NextContinuityCount(TsCCTypePMT));
            buf += TS_PACKET_SIZE;
        }

	}
	ioBuf->m_nDataLen = ( (char *)buf - (ioBuf->m_pBuf) );

	ioList.ListAddTail( ioBuf );
	return nPackets ;
}
void CTSIOCtx::InitTSConnection(CObj &ioList)
{
	int count = 5 ;
	int send = 0 ;

	while ( count > 0 )
	{
		send = SendPATandPMT( count,ioList );
		if( send < 0 )
		{
			break ;
		}
		count -= send ;
	}

	m_bHaveInitPatPmt = TRUE ;
}
//////////////////////////////////////

CETS2TSVar::CETS2TSVar()
{
	m_bSendPCR = FALSE;
	m_bInitedBuffer = FALSE;
	m_timestamp = 0;
	m_cts = 0;
	m_dts = 0;
	m_pts = 0;
	m_got_dts = 0;
	m_pcr = 0;
    m_pcr_ext = 0;
}
CETS2TSVar::CETS2TSVar(CMdSource *mdsrc, int  cts, INT64 tick)
{
	m_bSendPCR = FALSE;
	m_bInitedBuffer = FALSE;
	m_timestamp = 0;
    m_pcr_ext = 0;
	SetPar(mdsrc, cts, tick);
}
void CETS2TSVar::SetPar(CMdSource *mdsrc, int  cts, INT64 tick)
{

	m_cts = cts;
	if (mdsrc->m_firstTSFrameTime == 0)
	{
		mdsrc->m_firstTSFrameTime = tick;
		m_timestamp = 0;
	}
	else
	{
		m_timestamp = (UINT32)(tick - mdsrc->m_firstTSFrameTime);
	}

	m_dts = (m_timestamp)* 90;
	m_pts = (m_timestamp + m_cts) * 90;
	m_got_dts = (m_dts != m_pts);
	m_pcr = 0;
    m_pcr_ext = 0;

    GetPcr(m_timestamp, m_pcr, m_pcr_ext);


}
CETS2TSVar::~CETS2TSVar()
{
	CObjNetIOBuffer *buf = NULL;
	while ((buf = (CObjNetIOBuffer *)m_ioList.ListRemoveHead()) != NULL)
	{
		buf->ReleaseObj();
	}
}


#endif




































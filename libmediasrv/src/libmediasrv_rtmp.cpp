 

#include "libmediasrv_def.h"


#if defined(_RTMP_SUPPORT_)

#define RECEIVED_BYTES_COUNT_REPORT_CHUNK 131072
#define _DEF_CHUNKSIZE 4096

#define _RTMP_PACKET_SIZE SIZE_16KB
#define _SINGLE_BUFFER
static char _szNULL[1] = {0};


char*     StrTrimAppLeft(char *str)
{
    int i = 0, j = 0, n = 0;
    if (str == NULL || str[0] == 0)
    {
        return str;
    }
    for (; *(str + i) == ' ' || 
        *(str + i) == '\t' || 
        *(str + i) == '\r' || 
        *(str + i) == '\n'  ||
        *(str + i) == '\\'  ||
        *(str + i) == '/'
        ; i++, n++);

    while (*(str + j + n))
    {
        *(str + j) = *(str + j + n);
        j++;
    }
    *(str + j) = 0;
    return str;
}
char*     StrTrimAppRight(char *szBuf)
{
    int i, n;
    if (szBuf == NULL || szBuf[0] == 0)
    {
        return szBuf;
    }
    n = strlenx(szBuf);
    //    printf("n=%d\n",n);
    for (i = n - 1; i>-1 && (*(szBuf + i) == ' ' ||
        *(szBuf + i) == '\t' ||
        *(szBuf + i) == '\r' || 
        *(szBuf + i) == '\n' ||
        *(szBuf + i) == '\\' ||
        *(szBuf + i) == '/'
        ); i--);
    *(szBuf + i + 1) = 0;
    return szBuf;

}
static char*  StrTrimApp(const char *str)
{
    return StrTrimAppLeft(StrTrimAppRight((char *)str));
}


///////////////////////////////////////////////////////////////////
CETS2RTMPVar::CETS2RTMPVar()
{
	m_nDataLength = 0;
	m_amfsize = 0;
	m_cur_ioBuf = NULL;
	m_bInitedBuffer = FALSE;
	m_timestamp = 0;
	m_packetType = RTMPPacketTypeUnkown;
	m_mdsrc = NULL;
	m_timestampDelta = 0;
	m_header_ioBuf = NULL;
	m_bDeltaTimestamp = FALSE;
	m_cts = 0;
	m_bKeyFrame = FALSE;
}
CETS2RTMPVar::CETS2RTMPVar(CMdSource *mdsrc, INT64 tick, int  cts, RTMPPacketType pktType)
{
	m_nDataLength = 0;
	m_amfsize = 0;
	m_cur_ioBuf = NULL;
	m_bInitedBuffer = FALSE;
	m_timestamp = 0;
	m_timestampDelta = 0;
	m_header_ioBuf = NULL;
	m_bDeltaTimestamp = FALSE;
	m_bKeyFrame = FALSE;
	SetPar(mdsrc, tick, cts, pktType);
}
void CETS2RTMPVar::SetPar(CMdSource *mdsrc, INT64 tick, int  cts, RTMPPacketType pktType)
{

	m_packetType = pktType;
	m_mdsrc = mdsrc;

	m_cts = cts;
	

	if (mdsrc->m_firstRTMPFrameTime == -1)
	{
		mdsrc->m_firstRTMPFrameTime = tick;
		m_timestamp = 0;
	}
	else
	{
		m_timestamp = (UINT32)(tick - mdsrc->m_firstRTMPFrameTime);
	}

	//m_timestamp = 100 ;


	if (m_packetType == RTMPPacketType_Video)
	{
		if (mdsrc->m_rtmpTimePreVideo == 0)
		{
			m_timestampDelta = 0;
			mdsrc->m_rtmpTimePreVideo = tick;
		}
		else
		{
			m_timestampDelta = (UINT32)(tick - mdsrc->m_rtmpTimePreVideo);
		}
		mdsrc->m_rtmpTimePreVideo = tick;
	}
	else if (m_packetType == RTMPPacketType_Audio)
	{
		if (mdsrc->m_rtmpTimePreAudio == 0)
		{
			m_timestampDelta = 0;
			mdsrc->m_rtmpTimePreAudio = tick;
		}
		else
		{
			m_timestampDelta = (UINT32)(tick - mdsrc->m_rtmpTimePreAudio);
		}
		mdsrc->m_rtmpTimePreAudio = tick;
	}

	if (m_bDeltaTimestamp)
	{
		m_timestamp = m_timestampDelta;
	}


}
CETS2RTMPVar::~CETS2RTMPVar()
{
	CObjNetIOBuffer *buf = NULL ;
	while( ( buf = (CObjNetIOBuffer *) m_ioList.ListRemoveHead() ) != NULL )
	{
		buf->ReleaseObj();
	}

	if( m_cur_ioBuf != NULL )
	{
		m_cur_ioBuf->ReleaseObj();
	}

	if( m_header_ioBuf != NULL )
	{
		m_header_ioBuf->ReleaseObj();
	}

}
void CETS2RTMPVar::Write( UINT8 *buf , int Length )
{
	if( m_cur_ioBuf == NULL )
	{
		m_cur_ioBuf = CObjNetIOBufferSharedMemory::CreateObj( NULL , _RTMP_PACKET_SIZE , TRUE );
	}
	int Totol = 0 ;
	int nCpyed = 0 ;
	while( Totol < Length )
	{
		if( m_cur_ioBuf->m_nDataLen >= m_cur_ioBuf->m_nBufLen )
		{
			m_ioList.ListAddTail( m_cur_ioBuf );
			m_cur_ioBuf = CObjNetIOBufferSharedMemory::CreateObj( NULL , _RTMP_PACKET_SIZE , TRUE );

		}
		nCpyed = MinInt( Length-Totol ,  m_cur_ioBuf->m_nBufLen - m_cur_ioBuf->m_nDataLen  );
		memcpy( m_cur_ioBuf->m_pBuf + m_cur_ioBuf->m_nDataLen , buf + Totol ,  nCpyed );
		Totol += nCpyed ;
		m_cur_ioBuf->m_nDataLen += nCpyed ;


	}

}

int  CETS2RTMPVar::Write2IoBuffer( UINT8 *buf,int Length , UINT8 channelId  )
{
	int Totol = 0 ;
	int nCpyed = 0 ;
	int chunksize = m_mdsrc->GetRTMPChunkSize() ;







	while( Totol < Length )
	{
		if( m_amfsize > 0 )
		{
			if( m_amfsize % chunksize == 0 )
			{
				UINT8 tmp = 0xC0 | channelId ;
				Write( &tmp , 1  );

			}
		}
		nCpyed = MinInt( Length-Totol ,  chunksize - (m_amfsize % chunksize)  );
		Write( buf+Totol , nCpyed  );

		Totol += nCpyed ;
		m_amfsize += nCpyed ;

	}
	return 0 ;
}
CObjNetIOBufferSharedMemory *CETS2RTMPVar::CopySpsPps()
{
	if( m_mdsrc->m_rtmpSpsPpsBuf == NULL )
	{
		return NULL ;
	}
	CObjNetIOBufferSharedMemory *iobuf = m_mdsrc->m_rtmpSpsPpsBuf->Copy();

	if( iobuf != NULL )
	{
		CAMFObject::EncodeInt24( (UINT8 *)(iobuf->m_pBuf + 1) , m_timestamp );	
	}

	return iobuf ;

}
CObjNetIOBufferSharedMemory *CETS2RTMPVar::CopyAACHeader()
{
	if( m_mdsrc->m_rtmpAACHeaderBuf == NULL )
	{
		return NULL ;
	}
	CObjNetIOBufferSharedMemory *iobuf = m_mdsrc->m_rtmpAACHeaderBuf->Copy();

	if( iobuf != NULL )
	{
		CAMFObject::EncodeInt24( (UINT8 *)(iobuf->m_pBuf + 1) , m_timestamp );	
	}

	return iobuf ;
}
BOOL CETS2RTMPVar::CreateRtmpH264MetaData()
{
	return m_mdsrc->CreateRtmpH264MetaData();
}
BOOL CETS2RTMPVar::ETSCreateAACIoBuffers(UINT8 *buf ,int Length )
{
	const UINT8 channelId = RTMP_AUDIO_CHANNEL ;
	int headsize = 8;
	m_amfsize = 0 ;
	if( Length < 7 )
	{
		return FALSE ;
	}

	m_nDataLength = Length ;



	if( m_mdsrc->m_rtmpAACHeaderBuf == NULL )
	{
		if( ! m_mdsrc->m_adtsHeader.IsValid() )
		{

			if( ! m_mdsrc->m_adtsHeader.DecodeADTS( buf , Length ) )
			{
				return FALSE ;
			}



		}

		m_mdsrc->m_adtsHeader.GetAACConfigData( m_mdsrc->m_szAACConfig );


		CObjNetIOBufferSharedMemory *ioBuf = CObjNetIOBufferSharedMemory::CreateObj( NULL , SIZE_1KB , TRUE );
		if( ioBuf == NULL )
		{
			return FALSE ;
		}


		headsize = 12;
		memset( ioBuf->m_pBuf , 0 , headsize );
		ioBuf->m_pBuf[0] = channelId  | ( CAMFObject::HeadSize2HeadType( headsize ) << 6 ) ;
		CAMFObject::EncodeInt24( (UINT8 *)(ioBuf->m_pBuf + 1) , m_timestamp );	// timmer
		CAMFObject::EncodeInt24( (UINT8 *)(ioBuf->m_pBuf + 4) , 4 );		// packet size
		ioBuf->m_pBuf[7] = m_packetType ;
		if( headsize == 12 )
		{
			CAMFObject::EncodeInt32LE ( (UINT8 *)(ioBuf->m_pBuf + 8)  , m_mdsrc->GetRTMPStreamId() );		// stream id

		}
		ioBuf->m_nDataLen = headsize;


		memcpy( ioBuf->m_pBuf + ioBuf->m_nDataLen ,  m_mdsrc->m_szAACConfig , 4 );
		ioBuf->m_nDataLen += 4 ;

		m_mdsrc->m_rtmpAACHeaderBuf = ioBuf ;
	}
	static UINT8 aachead[2] = {0xAF ,0x01};

#if defined(_SINGLE_BUFFER)
	if (m_cur_ioBuf == NULL)
	{
		m_cur_ioBuf = CObjNetIOBufferSharedMemory::CreateObj( NULL , AlignLength(m_nDataLength + (m_nDataLength/m_mdsrc->GetRTMPChunkSize()+1) + 24 + 128 , SIZE_1KB) , TRUE );
		if( m_bDeltaTimestamp )
		{
			m_cur_ioBuf->m_nDataLen = 8 ;
		}
		else
		{
			m_cur_ioBuf->m_nDataLen = 12 ;
		}
	}
#endif

	Write2IoBuffer( aachead ,2   , channelId );
	Write2IoBuffer( buf + 7 ,Length - 7   , channelId );
	if( m_cur_ioBuf != NULL ) 
	{
		m_ioList.ListAddTail( m_cur_ioBuf );
		m_cur_ioBuf = NULL ;
	}

#if defined(_SINGLE_BUFFER)
	if (m_ioList.ListItemsCount() > 0)
	{
		if( m_bDeltaTimestamp )
		{
			headsize = 8;
		}
		else
		{
			headsize = 12;
		}

		CObjNetIOBufferSharedMemory *ioBuf = (CObjNetIOBufferSharedMemory *)( m_ioList.m_pNextObj ) ;
		if( ioBuf == NULL )
		{
			return FALSE ;
		}

		if( m_bDeltaTimestamp )
		{
			headsize = 8;
		}
		else
		{
			headsize = 12;
		}

		memset( ioBuf->m_pBuf , 0 , headsize );
		ioBuf->m_pBuf[0] = channelId | ( CAMFObject::HeadSize2HeadType( headsize ) << 6 )    ;
		CAMFObject::EncodeInt24( (UINT8 *)(ioBuf->m_pBuf + 1) , m_timestamp );	// timmer
		CAMFObject::EncodeInt24( (UINT8 *)(ioBuf->m_pBuf + 4) , m_amfsize );		// packet size
		ioBuf->m_pBuf[7] = m_packetType ;
		if( headsize == 12 )
		{
			CAMFObject::EncodeInt32LE ( (UINT8 *)(ioBuf->m_pBuf + 8)  , m_mdsrc->GetRTMPStreamId() );		// stream id
		}

	}


#else
	if( m_amfsize > 0 )
	{


		CObjNetIOBufferSharedMemory *ioBuf = CObjNetIOBufferSharedMemory::CreateObj( NULL , SIZE_1KB , TRUE );
		if( ioBuf == NULL )
		{
			return FALSE ;
		}

		if( m_bDeltaTimestamp )
		{
			headsize = 8;
		}
		else
		{
			headsize = 12;
		}

		memset( ioBuf->m_pBuf , 0 , headsize );
		ioBuf->m_pBuf[0] = channelId | ( CAMFObject::HeadSize2HeadType( headsize ) << 6 )    ;
		CAMFObject::EncodeInt24( (UINT8 *)(ioBuf->m_pBuf + 1) , m_timestamp );	// timmer
		CAMFObject::EncodeInt24( (UINT8 *)(ioBuf->m_pBuf + 4) , m_amfsize );		// packet size
		ioBuf->m_pBuf[7] = m_packetType ;
		if( headsize == 12 )
		{
			CAMFObject::EncodeInt32LE ( (UINT8 *)(ioBuf->m_pBuf + 8)  , m_mdsrc->GetRTMPStreamId() );		// stream id
		}

		ioBuf->m_nDataLen = headsize ;

		m_ioList.ListAddHead( ioBuf );
	}
#endif
	return TRUE ;
}

BOOL CETS2RTMPVar::ETSCreateH264IoBuffers(UINT8 *buf ,int Length )
{

	const UINT8 channelId = RTMP_VIDEO_CHANNEL ;
	int headsize = 8;

	int i = 0 ;

	UINT8 naluHead[20];
	m_amfsize = 0 ;
	BOOL bCheckSpsPps = FALSE ;
	BOOL bWriteHead = FALSE ;
	BOOL bFirst = TRUE ;
	unsigned char cts[3];


	NALU_T nalu;
	CH264NaluParse naluParse( buf ,Length);
	m_nDataLength = Length ;
#if defined(_SINGLE_BUFFER)
	if (m_cur_ioBuf == NULL)
	{
		m_cur_ioBuf = CObjNetIOBufferSharedMemory::CreateObj( NULL , AlignLength(m_nDataLength + (m_nDataLength/m_mdsrc->GetRTMPChunkSize()+1) + 24 + 128 , SIZE_2KB) , TRUE );
		if( m_bDeltaTimestamp )
		{
			m_cur_ioBuf->m_nDataLen = 8 ;
		}
		else
		{
			m_cur_ioBuf->m_nDataLen = 12 ;
		}
	}
#endif



	cts[0] = (m_cts & 0XFF0000) >> 16;
	cts[1] = (m_cts & 0XFF00) >> 8;
	cts[2] = m_cts;


	while(  naluParse.GetNextNalu(&nalu) )
	{
		i = 0 ;

		bCheckSpsPps = FALSE ;
		if( m_mdsrc->m_rtmpSpsPpsBuf == NULL )
		{
			if( nalu.nal_unit_type == NALU_TYPE_SPS )
			{
				if( m_mdsrc->m_sps.DataLength() < 1 )
				{
					m_mdsrc->m_sps.Write( nalu.buf , nalu.len , 256 );
				}

			}
			else if( nalu.nal_unit_type == NALU_TYPE_PPS )
			{
				if( m_mdsrc->m_pps.DataLength() < 1 )
				{
					m_mdsrc->m_pps.Write( nalu.buf , nalu.len , 256 );
				}
			}
			else if (nalu.nal_unit_type == NALU_TYPE_SEI)
			{
				if (m_mdsrc->m_sei.DataLength() < 1)
				{
					m_mdsrc->m_sei.Write(nalu.buf, nalu.len, 256);
				}
			}

			bCheckSpsPps = TRUE ;
		}

		if( bCheckSpsPps )
		{
			if( m_mdsrc->m_rtmpSpsPpsBuf == NULL  && 
				m_mdsrc->m_sps.DataLength() > 0  && 
				m_mdsrc->m_pps.DataLength() > 0  
				)
			{
				CObjNetIOBufferSharedMemoryRTMP *ioBuf = CObjNetIOBufferSharedMemoryRTMP::CreateObj( NULL , AlignLength( MaxInt( SIZE_2KB , (m_mdsrc->m_sps.DataLength() + m_mdsrc->m_pps.DataLength() + 64 ) ) , SIZE_1KB ) , TRUE );
				if( ioBuf == NULL )
				{
					continue ;
				}
				if( ioBuf->m_pBuf == NULL )
				{
					ioBuf->ReleaseObj() ;
					continue ;
				}
				ioBuf->m_chunksize = m_mdsrc->GetRTMPChunkSize() ;
				ioBuf->m_channelId = channelId ;

				headsize = 12;
				ioBuf->m_nDataLen = headsize ;

				UINT32 nSpsLen = m_mdsrc->m_sps.DataLength();
				UINT32 nPpsLen = m_mdsrc->m_pps.DataLength();




				ioBuf->WriteUINT8( 0x17 ); // 1:keyframe  7:AVC   
				ioBuf->WriteUINT8( 0x00 ); // AVC sequence header   

				ioBuf->WriteUINT8(cts[0]);
				ioBuf->WriteUINT8(cts[1]);
				ioBuf->WriteUINT8(cts[2]); // fill in 0;   

				// AVCDecoderConfigurationRecord.   
				ioBuf->WriteUINT8( 0x01 ); // configurationVersion   
				ioBuf->WriteUINT8( m_mdsrc->m_sps.Data[1] ); // AVCProfileIndication   
				ioBuf->WriteUINT8( m_mdsrc->m_sps.Data[2] ); // profile_compatibility   
				ioBuf->WriteUINT8( m_mdsrc->m_sps.Data[3] ); // AVCLevelIndication    
				ioBuf->WriteUINT8( 0xff ); // lengthSizeMinusOne 


				// sps nums   
				ioBuf->WriteUINT8( 0xE1 ); //&0x1f   
				// sps data length   


				ioBuf->WriteUINT8( nSpsLen >> 8 );  // 11
				ioBuf->WriteUINT8( nSpsLen & 0xff );  
				// sps data   
				ioBuf->Write( m_mdsrc->m_sps.Data ,nSpsLen);  


				// pps nums   
				ioBuf->WriteUINT8( 0x01 ); //&0x1f   
				// pps data length    

				ioBuf->WriteUINT8( nPpsLen >> 8 );  
				ioBuf->WriteUINT8( nPpsLen & 0xff );  
				// sps data   
				ioBuf->Write( m_mdsrc->m_pps.Data ,nPpsLen );  


				ioBuf->m_pBuf[0] = channelId  | ( CAMFObject::HeadSize2HeadType( headsize ) << 6 ) ;
				CAMFObject::EncodeInt24( (UINT8 *)(ioBuf->m_pBuf + 1) , m_timestamp );	// timmer
				CAMFObject::EncodeInt24( (UINT8 *)(ioBuf->m_pBuf + 4) , ioBuf->m_amfsize );		// packet size
				ioBuf->m_pBuf[7] = m_packetType ;

				if( headsize == 12 )
				{
					CAMFObject::EncodeInt32LE ( (UINT8 *)(ioBuf->m_pBuf + 8)  , m_mdsrc->GetRTMPStreamId() );		// stream id
				}





				m_mdsrc->m_rtmpSpsPpsBuf  = ioBuf ;

			}
			continue ;
		}

		if( m_mdsrc->m_rtmpSpsPpsBuf  == NULL )
		{
			continue ;
		}

		if( nalu.nal_unit_type == NALU_TYPE_SPS
			|| nalu.nal_unit_type == NALU_TYPE_PPS
			|| nalu.nal_unit_type == NALU_TYPE_SEI
			)
		{
			continue ;
		}




		if( nalu.nal_unit_type == NALU_TYPE_AUD )
		{
			continue ;
		}

		i = 0 ;


		if( bFirst )
		{
			if( nalu.nal_unit_type == NALU_TYPE_IDR )
			{
				naluHead[i++] = 0x17 ;
				m_bKeyFrame = TRUE ;
			}
			else 
			{
				naluHead[i++] = 0x27 ;
			}

			naluHead[i++] = 0x01;// AVC NALU   

			naluHead[i++] = cts[0];
			naluHead[i++] = cts[1];
			naluHead[i++] = cts[2];

			bFirst = FALSE ;
		}


		// NALU size   
		naluHead[i++] = nalu.len>>24;  
		naluHead[i++] = nalu.len>>16;  
		naluHead[i++] = nalu.len>>8;  
		naluHead[i++] = nalu.len&0xff;




		Write2IoBuffer( naluHead ,i  , channelId );
		Write2IoBuffer( nalu.buf ,nalu.len  , channelId );




	}

	if( m_cur_ioBuf != NULL ) 
	{
		m_ioList.ListAddTail( m_cur_ioBuf );
		m_cur_ioBuf = NULL ;
	}
#if defined(_SINGLE_BUFFER)
	if (m_ioList.ListItemsCount() > 0)
	{
		if (m_bDeltaTimestamp)
		{
			headsize = 8;
		}
		else
		{
			headsize = 12;
		}

		CObjNetIOBufferSharedMemory *ioBuf = (CObjNetIOBufferSharedMemory *)(m_ioList.m_pNextObj);
		if( m_bDeltaTimestamp )
		{
			headsize = 8;
		}
		else
		{
			headsize = 12;
		}
		memset( ioBuf->m_pBuf , 0 , headsize );
		ioBuf->m_pBuf[0] = channelId  | ( CAMFObject::HeadSize2HeadType( headsize ) << 6 ) ;
		CAMFObject::EncodeInt24( (UINT8 *)(ioBuf->m_pBuf + 1) , m_timestamp );	// timmer
		CAMFObject::EncodeInt24( (UINT8 *)(ioBuf->m_pBuf + 4) , m_amfsize );		// packet size
		ioBuf->m_pBuf[7] = m_packetType ;
		if( headsize == 12 )
		{
			CAMFObject::EncodeInt32LE ( (UINT8 *)(ioBuf->m_pBuf + 8)  , m_mdsrc->GetRTMPStreamId() );		// stream id
		}
	}



#else
	if( m_amfsize > 0 )
	{

		CObjNetIOBufferSharedMemory *ioBuf = CObjNetIOBufferSharedMemory::CreateObj( NULL , 64 , TRUE );
		if( ioBuf == NULL )
		{
			return FALSE ;
		}
		if( m_bDeltaTimestamp )
		{
			headsize = 8;
		}
		else
		{
			headsize = 12;
		}
		memset( ioBuf->m_pBuf , 0 , headsize );
		ioBuf->m_pBuf[0] = channelId  | ( CAMFObject::HeadSize2HeadType( headsize ) << 6 ) ;
		CAMFObject::EncodeInt24( (UINT8 *)(ioBuf->m_pBuf + 1) , m_timestamp );	// timmer
		CAMFObject::EncodeInt24( (UINT8 *)(ioBuf->m_pBuf + 4) , m_amfsize );		// packet size
		ioBuf->m_pBuf[7] = m_packetType ;
		if( headsize == 12 )
		{
			CAMFObject::EncodeInt32LE ( (UINT8 *)(ioBuf->m_pBuf + 8)  , m_mdsrc->GetRTMPStreamId() );		// stream id
		}

		ioBuf->m_nDataLen = headsize ;


		m_header_ioBuf = ioBuf ;
		//m_ioList.ListAddHead( ioBuf );


	}

#endif


	return TRUE ;
}


///////////////////////////////////////////////////////////////////
CRTMPNetSession::CRTMPNetSession()
{
	m_bSendCodecInfo = FALSE ;

	m_nChannelOfSend = RTMP_AUDIO_CHANNEL ;
	m_typeCtx = CtxTypeRTMP ;
	m_bInSessionList = FALSE ;
	m_bOnCloseCalled = FALSE;
	m_bUseSettingSrv = FALSE;
	m_iVideoFrameQueue = 0;
	m_iAudioFrameQueue = 0;
	m_bNoCache = FALSE;
	m_bVideoOnly = FALSE;
	m_bAudioOnly = FALSE;
    m_indexPing = 1;
    m_resultNumber = 0;

    m_nBytesReceived = 0;
    m_windowAck = RECEIVED_BYTES_COUNT_REPORT_CHUNK;
    m_nextReceivedBytesCountReport = m_windowAck;

    m_tmCreateStream = 0;
    m_bPublishCalled = FALSE;
    m_bFlashPlayer = FALSE;
    


}
CRTMPNetSession::~CRTMPNetSession()
{
	OnClose(NULL);

}
void CRTMPNetSession::Close()
{
	if (m_bCtxClosing)
	{
		return;
	}
	m_bCtxClosing = TRUE;
	RTMPConnCtxCloseAll();
	//CloseByRTMPT();
}

BOOL CRTMPNetSession::Send(CObjNetIOBuffer *pBuf)
{
	if (!CRTMPCtx::Send(pBuf))
	{
		m_bCtxError = TRUE;
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}
BOOL  CRTMPNetSession::NeedSendPing(int &indexPing)
{

     return FALSE;
  
}
void CRTMPNetSession::CloseByRTMPT()
{

	
	UpdateEndTime();
	// RTMPT连接在某些时候，flash客户端会重用socket而不断开
	// 导致无法收到 OnClose事件，所以在这里要调用
	CRTMPEvent *ev = CRTMPEvent::CreateObj(NULL);

	ev->m_event = CRTMPEvent::RTMPEventCloseBySession;
	ev->m_ss = this;

	if (!ev->Post2Close())
	{
		ev->ReleaseObj();
	}
}
void CRTMPNetSession::CloseMdSource()
{
	CloseByRTMPT();
}
void CRTMPNetSession::OnClose(CMediasrvCallback *callback)
{
	if (m_bOnCloseCalled)
	{
		return;
	}
	m_bOnCloseCalled = TRUE;
	
	RTMPConnCtxCloseAll();
	SessionMgrStatus mgrStatus = m_sessionManagerStatus ;
	BOOL bNotify = FALSE ;

	LogPrint(_LOG_LEVEL_DETAIL, _TAGNAME_RTMP, "CRTMPNetSession::OnClose %d, %p, m_bSrcCtx=%d;m_szSessionJoin=%s;m_mdsrc=%p, m_bInMdSource=%d\n", __LINE__, this ,
		m_bSrcCtx, (const char *)m_stream, m_mdsrc.p, m_bInMdSource);



	if (m_type == CRTMPCtx::ConnTypeRTMPT)
	{
		RemoveRTMPNetSession(m_szClientId, this);
	}

	if (!m_bValidRequest)
	{

		return;
	}
	

	if( m_bSrcCtx )
	{
		

		if( m_mdsrc != NULL )
		{

			
			if (m_mdsrc->m_bVirtual && !m_bForceCloseMdsource )
			{
				m_sessionManagerStatus = SessionMgrStatusUnknown;
                m_mdsrc->DelayRemoveMdSourceIfMatchSrcCtx(this, m_bUseSettingSrv);
			}
			else
			{
                m_mdsrc->ClearSrcCtxIfMatch(this);
				if (m_sessionManagerStatus != SessionMgrStatusUnknown)
				{
					RemoveMdSource(m_app, m_stream, m_mdsrc);
					m_sessionManagerStatus = SessionMgrStatusUnknown;
					bNotify = TRUE;
				}

				m_mdsrc->CloseAllClients();
				if (bNotify)
				{
					m_sessionManagerStatus = SessionMgrStatusUnknown;

				}


				if (m_bSrcCtx && m_bUseSettingSrv && mgrStatus == SessionMgrStatusAttached)
				{
                    if (m_sysId.Length() < 1 && m_stream.DataLength()>0)
					{

						CSettingSrvEvent *ev = CSettingSrvEvent::CreateObj();
						ev->m_status = StreamStatusStoped;
						strcpyn(ev->m_szId, m_stream, sizeof(ev->m_szId));

						if (!ev->Post())
						{
							ev->ReleaseObj();
						}
					}
				}
			}
			


		}

		

	}
	else
	{
		if( m_mdsrc != NULL )
		{
			if( m_bInMdSource )
			{
				m_mdsrc->RemoveClientSession( this );
			}

		}



	}




	//RTMPConnCtxRemoveAll();


}

/////////////////////////////////////////////////
CRTMPConnNetCtx::CRTMPConnNetCtx()
{
	m_typeCtx = CtxTypeRTMP;
	m_bOnReallyCloseCalled = FALSE ;
	m_bOnCloseCalled = FALSE ;
	m_bCloseAfterSend = FALSE;
//	LogPrint(_LOG_LEVEL_DETAIL ,_TAGNAME_RTMP ,"%s %d,%p\n", __FUNCTION__ , __LINE__  , (CObjConnContext*)this );

	m_queue = _MediasrvApp->m_queueNetworkEventRTMP;
	m_queueThread = m_queue->GetNextThreadObj();
	m_tmBeginSession = OSTickCount();

}
CRTMPConnNetCtx::~CRTMPConnNetCtx()
{

	OnReallyClose(NULL);


}
BOOL CRTMPConnNetCtx::InitLater()
{
	return CRTMPConnContext::InitLater();
}
CRTMPCtx *CRTMPConnNetCtx::CreateRTMPCtx()
{
    CRTMPNetSession * ss = CRTMPNetSession::CreateObj( );
    if( ss != NULL )
    {
		ss->m_nBytesRead += m_nBytesRead;
        ss->m_nBytesReceived += m_nBytesRead;
		m_nBytesRead = 0;
		ss->m_nBytesWrite += m_nBytesWrite;
		m_nBytesWrite = 0;
        ss->m_addrPeer = m_AddrRemote;
		ss->m_bUseSettingSrv = ((CRTMPNetworkSrv *)(m_net))->m_pCfg->m_bUseSettingSrv;
    }
    return ss;
}
void CRTMPConnNetCtx::Close()
{
	m_net->CloseConnection( this );
}
void CRTMPConnNetCtx::OnClose()
{
	if (m_bOnCloseCalled)
	{
		return ;
	}
	m_bOnCloseCalled = TRUE ;

	UpdateEndTime();

	if (m_ctxRtmp != NULL)
	{
		((CRTMPNetSession *)m_ctxRtmp)->UpdateEndTime();
	}
	CRTMPEvent *ev = CRTMPEvent::CreateObj(this);

	ev->m_event = CRTMPEvent::RTMPEventCloseByCtx;

	if (!ev->Post2Close())
	{
		ev->ReleaseObj();
	}
}
void CRTMPConnNetCtx::OnReallyClose(CMediasrvCallback *callback)
{
	if (m_bOnReallyCloseCalled)
	{
		return ;
	}
	m_bOnReallyCloseCalled = TRUE ;

	if (m_ctxRtmp == NULL)
	{

		return;
	}



	LogPrint(_LOG_LEVEL_DETAIL, _TAGNAME_RTMP, "CRTMPConnNetCtx::OnReallyClose %d, %p,%p\n", __LINE__, this, m_ctxRtmp);
	if( m_type == CRTMPCtx::ConnTypeRTMPT )
	{

	}
	else
	{
		if( m_ctxRtmp != NULL )
		{
			SessionMgrStatus status = ((CRTMPNetSession*)m_ctxRtmp)->m_sessionManagerStatus;

			((CRTMPNetSession*)m_ctxRtmp)->OnClose(callback);



			m_ctxRtmp->RTMPConnCtxRemove(this);

		}
	}

}
BOOL CRTMPConnNetCtx::SendOnMetaData(CMdSource *mdsrc)
{
	CRTMPNetSession *ss = (CRTMPNetSession *)m_ctxRtmp;
	if (mdsrc == NULL)
	{
		mdsrc = ss->m_mdsrc;
	}
	int width = 0 ;
	int height = 0 ;


	CAMFBuffer buf;
	const int  headsize = 8 ;



	buf.m_channelId = 0x14 ;
	buf.m_chunksize = mdsrc->GetRTMPChunkSize();

	buf.SetReadPos(headsize);
	CAMFObject::EncodeString( buf , "onMetaData");

	CAMFObject::EncodeObjectBegin( buf );



	if( width != 0 && height != 0 )
	{
		CAMFObject::EncodeNamedNumber( buf , "width" ,width  );
		CAMFObject::EncodeNamedNumber( buf , "height" ,height  );
	}
	CAMFObject::EncodeNamedString( buf , "server", CRTMPCtx::RTMPServerName() );
	CAMFObject::EncodeObjectEnd( buf );

	CAMFObject *obj = CAMFObject::CreateFromAMFBuffer( buf.Data + buf.ReadPos ,  buf.DataLength() ) ;
	obj->Dump();
	obj->ReleaseObj();



	buf.SetReadPos( 0 );


	memset( buf.Data , 0 , headsize );

	buf.Data[0] = buf.m_channelId  | ( CAMFObject::HeadSize2HeadType( headsize ) << 6 ) ;
	CAMFObject::EncodeInt24( buf.Data + 4 , buf.m_amfsize );
	buf.Data[7] = RTMPPacketType_Script ;
	if( headsize == 12 )
	{
		CAMFObject::EncodeInt32LE(buf.Data + 8, mdsrc->GetRTMPStreamId());
	}


	CObjNetIOBufferSharedMemory *ioBuf = CObjNetIOBufferSharedMemory::CreateObj( NULL , AlignLength( buf.DataLength() , SIZE_1KB) , TRUE );
	if( ioBuf == NULL )
	{
		return FALSE ;
	}
	memcpy( ioBuf->m_pBuf ,  buf.Data  , buf.DataLength() );
	ioBuf->m_nDataLen = buf.DataLength();


	if( !Send( ioBuf ) )
	{
		ioBuf->ReleaseObj();
		return FALSE ;
	}
	return TRUE ;
}


//////////////////////////////////////////////////////////////////////////////////////////

CRTMPEvent::CRTMPEvent(CRTMPConnNetCtx *ctx)
{
	m_callback = NULL;
	m_ctx = ctx ;
	m_event = RTMPEventUnknown ;

	if (m_ctx != NULL && m_ctx->m_ctxRtmp != NULL)
	{
		m_mdsrc = ((CRTMPNetSession *)(m_ctx->m_ctxRtmp))->m_mdsrc;
	}

}
CRTMPEvent::~CRTMPEvent()
{

}
BOOL CRTMPEvent::Post(BOOL bNotify )
{
	if (m_ctx == NULL)
	{
		return FALSE;
	}
	//return m_ctx->m_queue->PostEvent(this, bNotify);
	return m_ctx->m_queue->PostEvent(this, m_ctx->m_queueThread, bNotify);
}
BOOL CRTMPEvent::Post2MdSource(BOOL bNotify)
{
	CRTMPNetSession *ss = (CRTMPNetSession *)m_ctx->m_ctxRtmp;
	if (m_mdsrc == NULL)
	{
		return FALSE;
	}

	return m_mdsrc->m_queue->PostEvent(this, m_mdsrc->m_queueThread, bNotify);
}
BOOL CRTMPEvent::Post2Close(BOOL bNotify )
{
#if defined(_POST2CLOSE )
	const CObj *idle = _MediasrvApp->m_queueNetworkClose->GetIdleThreadObj();

	return _MediasrvApp->m_queueNetworkClose->PostEvent(this, idle, bNotify);
#else
	return Post(bNotify);
#endif
}
int CRTMPEvent::OnExecute(CObj *pThreadContext  )
{
	CCallbackProcessorCtx *ctx = (CCallbackProcessorCtx *)pThreadContext;
	if (pThreadContext != NULL)
	{
		m_callback = ctx->m_callback;
	}
	

    CRTMPNetSession *ss = NULL;
    if (m_ctx != NULL)
    {
        ss = (CRTMPNetSession *)m_ctx->m_ctxRtmp;
    }
	


	switch ( m_event )
	{
	case RTMPEventCloseByCtx:
	{
	
		m_ctx->OnReallyClose(m_callback);

		break;
	}
	case RTMPEventCloseBySession:
	{
		m_ss->OnClose(m_callback);
		break;
	}
	case RTMPEventVideo:
	{
		if (m_mdsrc->m_ctxSrc != ss)
		{
			break;
		}
		int bytes = m_mdsrc->SendRTMP_H264(&m_rtmpHeader, m_buf);
		
		if (bytes == -1)
		{

			m_ctx->Close();

		}
		break;
	}
	case RTMPEventAudio:
	{

		if (m_mdsrc->m_ctxSrc != ss)
		{
			break;
		}
        m_mdsrc->SendRTMP_AUDIO(&m_rtmpHeader, m_buf);
		break;
	}
	
	case RTMPEventCreateStream:
		{


			break ;
		}
	case RTMPEventConnect:
		{



			break;
		}
	case RTMPEventPlay:
		{
			char szTmp[600];
			if( ss->m_bSrcCtx )
			{
				m_ctx->Close();
				return 0 ;
			}
			
			CAMFObjectProperty *file = m_objAmf->GetProp( 3 );
			if( file == NULL )
			{
				m_ctx->Close();
				return 0 ;
			}
			const char *pszQuery = _szNULL;
			int nLenSession = strcpyn_stop(szTmp, file->m_value, sizeof(szTmp), "?");
			ss->m_stream = szTmp;
			if( ss->m_mdsrc == NULL )
			{
				m_mdsrc.Attach(GetMdSource(ss->m_app, ss->m_stream));
				ss->m_mdsrc = m_mdsrc;
			}

			
			
            ss->m_bValidRequest = TRUE;

			if (m_mdsrc == NULL || !m_mdsrc->IsOpened() || (!((CRTMPNetworkSrv *)(m_ctx->m_net))->m_pCfg->m_bForcePlayback && !m_mdsrc->m_bOutRTMP))
			{
				m_ctx->m_bCloseAfterSend = TRUE;
				m_ctx->SendPlayNotFound();
				
				break ;
			}
			else
			{
                
                m_mdsrc->CopySrcCid2Ctx(ss);
   

				m_mdsrc->m_rtmpChunkSize = ((CRTMPNetworkSrv *)(m_ctx->m_net))->m_pCfg->m_ChunkSize;
				m_ctx->SendChunkSize(m_mdsrc->GetRTMPChunkSize());
				m_ctx->SendCtrl( RTMPCtrl_StreamBegin , 1 );
				m_ctx->SendPlayReset(ss->m_stream, ss->m_stream);
				m_ctx->SendPlayStart(ss->m_stream, ss->m_stream);
				m_ctx->SendRtmpSampleAccess();
				//m_ctx->SendOnMetaData(m_mdsrc);

				if(  !ss->m_bInMdSource && !ss->m_bSrcCtx && ss->m_mdsrc != NULL )
				{
					// rtmp may not support quick start


					if(!m_mdsrc->AddClientSession(ss))
					{
						if (ss->m_type == CRTMPCtx::ConnTypeRTMP)
						{
							LogPrint(_LOG_LEVEL_ERROR, _TAGNAME_RTMP, "AddClientSession(RTMP) failed,%s, ADDR=%s:%d\n",
								(const char*)ss->m_stream, m_ctx->m_AddrRemote.szAddr, m_ctx->m_AddrRemote.nPort);
						}
						else
						{
							LogPrint(_LOG_LEVEL_ERROR, _TAGNAME_RTMP, "AddClientSession(RTMPT) failed,%s\n",
								(const char*)ss->m_stream);
						}

						ss->Close();
					}
                    else
                    {
                        if (m_callback != NULL)
                        {
                            if (m_callback->OnStreamPlayStarted(ss) != 0)
                            {
                                m_mdsrc->RemoveClientSession(ss);
                                ss->Close();
                                break;
                            }
                        }
                    }
				}
			}

			ss->m_bPlayCalled = TRUE ;
			ss->m_bPlaying = TRUE ;


			break ;
		}
	case RTMPEventPublish:
		{
			BOOL bVirtual = FALSE;
			char szTmp[800];
#if defined(_HLS_SUPPORT_)   
            CObjPtr<CHLSCtx> oldHls;
#endif
			if( ss->m_bPlayCalled )
			{
				 
				m_ctx->Close();
				return 0 ;
			}


			ss->m_mdsrc = NULL;

			if( ss->m_mdsrc != NULL )
			{
				break ;
			}
			ss->m_bSrcCtx = TRUE ;

			 
			CAMFObjectProperty *file = m_objAmf->GetProp( 3 );
			if( file == NULL )
			{
				m_ctx->Close();
				return 0 ;
			}
		
			 
			const char *pszQuery = _szNULL;
			int nLenSession = strcpyn_stop(szTmp, file->m_value, sizeof(szTmp), "?");
            ss->m_stream = StrTrimApp(szTmp);



			 
			GetValueFromString(pszQuery, "sysid", szTmp, sizeof(szTmp), "=", "&?\r\n");
			ss->m_sysId = szTmp;

			ss->m_bValidRequest = TRUE;

			


	

			BOOL bGenClientID = FALSE  ;

			 

			{

				if (ss->m_stream.DataLength() > 0 )
				{
					if (_AppConfig.m_bUseClientID)
					{

						RemoveMdSourceClose(ss->m_app, ss->m_stream);
					}
					else
					{

						if (IsExistMdSource(ss->m_app, ss->m_stream))
						{
							bGenClientID = TRUE;
						}
					}

				}
				else
				{
					bGenClientID = TRUE;
				}

				if (bGenClientID)
				{
					char buf[200];



					sprintf(buf, "%p-%s:%d", ss, m_ctx->m_AddrRemote.szAddr, m_ctx->m_AddrRemote.nPort);
					Base64Encode(buf, -1, szTmp, sizeof(szTmp));
					ss->m_stream = szTmp;


				}
			}
			
			if( ss->m_mdsrc == NULL )
			{


				CStreamSetting setting;
				BOOL bRet = FALSE;
				if (((CRTMPNetworkSrv *)(m_ctx->m_net))->m_pCfg->m_bUseSettingSrv)
				{
                    if (ss->m_sysId.Length()> 0)
					{
						 
						bRet = GetStreamSetting(setting, ss->m_sysId, m_ctx->m_AddrRemote.szAddr, TRUE, NULL, NULL);
						 
					}
					else
					{
						 
						bRet = GetStreamSetting(setting, ss->m_stream, NULL, FALSE, NULL, NULL);
						 
					}
					

					if (!bRet)
					{
						 
						char szDesc[400];

						snprintf(szDesc, sizeof(szDesc), "Stream '%s' get setting error", (const char*)ss->m_stream);
						CAMFObjectProperty *txn = m_objAmf->GetProp(1);

						m_ctx->m_bCloseAfterSend = TRUE;
						m_ctx->SendInvokeCallFailed(txn->m_value, szDesc);

						LogPrint(_LOG_LEVEL_ERROR, _TAGNAME_RTMP, "%s %d,Stream '%s' get setting error\n", __FUNCTION__, __LINE__, 
							(const char*)ss->m_stream);
						break;
					}
					else
					{
						 
                        m_mdsrc.Attach(CMdSource::CreateObj(ss->m_app,ss->m_stream));
						m_mdsrc->m_bVirtual = bVirtual;
	

						ss->m_mdsrc = m_mdsrc;
						m_mdsrc->m_ctxSrc = ss;
						m_mdsrc->m_srcType = CtxTypeRTMP;
						strcpyn(m_mdsrc->m_szContentType, "video/MP2T", sizeof(m_mdsrc->m_szContentType));





#if defined(_FLVREC_SUPPORT_)
						if (
							setting.m_recordFormat == CStreamSetting::RecordFormatFLV &&
							setting.m_bRecord
							)
						{

							do
							{
								if (
									((CRTMPNetworkSrv *)(m_ctx->m_net))->m_pCfg->m_record_flv_dir->m_var.DataLength() < 1 &&
									setting.m_recordDir->m_var.DataLength() < 1
									)
								{
									LogPrint(_LOG_LEVEL_ERROR, _TAGNAME_RTMP, "%s %d,(%s) FLV record dir is empty, FLV record not started\n", __FUNCTION__, __LINE__, 
										(const char*)ss->m_stream);
									break;
								}
								m_mdsrc->CreateFlvSession(ss->m_stream);
								if (m_mdsrc->m_flv == NULL)
								{
									LogPrint(_LOG_LEVEL_ERROR, _TAGNAME_RTMP, "%s %d,CreateFlvSession failed\n", __FUNCTION__, __LINE__);
									break;
								}
								else
								{


									m_mdsrc->m_flv->m_sysId = ss->m_sysId;
									m_mdsrc->m_flv->m_bRecord = setting.m_bRecord;
									if (setting.m_recordDir->m_var.DataLength() < 1)
									{
										m_mdsrc->m_flv->m_recordDir = ((CRTMPNetworkSrv *)(m_ctx->m_net))->m_pCfg->m_record_flv_dir;
									}
									else
									{
										m_mdsrc->m_flv->m_recordDir = setting.m_recordDir;
									}
									m_mdsrc->m_flv->m_nRecordDuration = setting.m_nRecordDuration;
									m_mdsrc->m_flv->OnFinishInit();


								}

							} while (0);

						}
#endif



#if defined(_HLS_SUPPORT_)    
						if (
							((CRTMPNetworkSrv *)(m_ctx->m_net))->m_pCfg->m_bForceHlsSetting &&
							(((CRTMPNetworkSrv *)(m_ctx->m_net))->m_pCfg->m_bOutput_hls || ((CRTMPNetworkSrv *)(m_ctx->m_net))->m_pCfg->m_bRecordTs)
							)
						{

                            m_mdsrc->CreateHLSSession(ss->m_stream, oldHls);
							if (m_mdsrc->m_hls == NULL)
							{
								LogPrint(_LOG_LEVEL_ERROR, _TAGNAME_RTMP, "%s %d,CreateHLSSession failed\n", __FUNCTION__, __LINE__);
							}
							else
							{


								m_mdsrc->m_hls->m_sysId = ss->m_sysId;
								m_mdsrc->m_hls->m_bHls = ((CRTMPNetworkSrv *)(m_ctx->m_net))->m_pCfg->m_bOutput_hls;
								m_mdsrc->m_hls->m_bHlsOutputMemory = ((CRTMPNetworkSrv *)(m_ctx->m_net))->m_pCfg->m_bHlsOutputMemory;


								m_mdsrc->m_hls->m_bRecord = ((CRTMPNetworkSrv *)(m_ctx->m_net))->m_pCfg->m_bRecordTs;

								m_mdsrc->m_hls->m_hlsDir = ((CRTMPNetworkSrv *)(m_ctx->m_net))->m_pCfg->m_hls_dir;
								m_mdsrc->m_hls->m_hlsTsPrefix = ((CRTMPNetworkSrv *)(m_ctx->m_net))->m_pCfg->m_hls_ts_prefix;
								m_mdsrc->m_hls->m_recordDir = ((CRTMPNetworkSrv *)(m_ctx->m_net))->m_pCfg->m_record_ts_dir;

								m_mdsrc->m_hls->OnFinishInit();


							} 
						}
						else if ( 
							!(((CRTMPNetworkSrv *)(m_ctx->m_net))->m_pCfg->m_bForceHlsSetting) &&
							(setting.m_bHls || (setting.m_bRecord && setting.m_recordFormat == CStreamSetting::RecordFormatTS))
							)
						{
                            m_mdsrc->CreateHLSSession(ss->m_stream, oldHls);
							if (m_mdsrc->m_hls == NULL)
							{
								LogPrint(_LOG_LEVEL_ERROR, _TAGNAME_RTMP, "%s %d,CreateHLSSession failed\n", __FUNCTION__, __LINE__);
							}
							else
							{
								 
								m_mdsrc->m_hls->m_sysId = ss->m_sysId;
								m_mdsrc->m_hls->m_bHls = setting.m_bHls;
								m_mdsrc->m_hls->m_bHlsOutputMemory = setting.m_bHlsOutputMemory;

								if (setting.m_recordFormat == CStreamSetting::RecordFormatTS)
								{
									m_mdsrc->m_hls->m_bRecord = setting.m_bRecord;
								}
								


								m_mdsrc->m_hls->m_hlsTsPrefix = setting.m_hlsTsPrefix;

								if (setting.m_hlsDir->m_var.DataLength() < 1)
								{
									m_mdsrc->m_hls->m_hlsDir = ((CRTMPNetworkSrv *)(m_ctx->m_net))->m_pCfg->m_hls_dir;
								}
								else
								{
									m_mdsrc->m_hls->m_hlsDir = setting.m_hlsDir;
								}

								if (setting.m_recordDir->m_var.DataLength() < 1)
								{
									m_mdsrc->m_hls->m_recordDir = ((CRTMPNetworkSrv *)(m_ctx->m_net))->m_pCfg->m_record_ts_dir;
								}
								else
								{
									m_mdsrc->m_hls->m_recordDir = setting.m_recordDir;
								}

								m_mdsrc->m_hls->m_hlscfg = setting.m_hlscfg;
								m_mdsrc->m_hls->m_cfg = &(m_mdsrc->m_hls->m_hlscfg);


								if (m_mdsrc->m_hls->m_hlscfg.nameType == HlsNameTypeUnknown)
								{
									m_mdsrc->m_hls->m_hlscfg.nameType = _AppConfig.m_hlsCfg.nameType;
								}

								m_mdsrc->m_hls->m_nRecordDuration = setting.m_nRecordDuration;

								m_mdsrc->m_bOutHTTP_TS = setting.m_bOutHTTP_TS;
								m_mdsrc->m_bOutRTMP = setting.m_bOutRTMP;
								m_mdsrc->m_bOutRTSP = setting.m_bOutRTSP;

								m_mdsrc->m_hls->OnFinishInit();

							}






							
			


							



						}
#endif

					}

				}
				else // else if (((CRTMPNetworkSrv *)(m_ctx->m_net))->m_bUseSettingSrv )
				{
                    m_mdsrc.Attach(CMdSource::CreateObj(ss->m_app, ss->m_stream));
					m_mdsrc->m_bVirtual = bVirtual;

					ss->m_mdsrc = m_mdsrc;
					m_mdsrc->m_ctxSrc = ss;
					m_mdsrc->m_srcType = CtxTypeRTMP;
					strcpyn(m_mdsrc->m_szContentType, "video/MP2T", sizeof(m_mdsrc->m_szContentType));





#if defined(_FLVREC_SUPPORT_)
					if (((CRTMPNetworkSrv *)(m_ctx->m_net))->m_pCfg->m_bRecordFlv || ss->m_recordFlv )
					{

						do
						{
							if (((CRTMPNetworkSrv *)(m_ctx->m_net))->m_pCfg->m_record_flv_dir->m_var.DataLength() < 1)
							{
								LogPrint(_LOG_LEVEL_ERROR, _TAGNAME_RTMP, "%s %d,(%s) FLV record dir is empty, FLV record not started\n", __FUNCTION__, __LINE__,
									(const char*)ss->m_stream);
								break;
							}
							m_mdsrc->CreateFlvSession(ss->m_stream);
							if (m_mdsrc->m_flv == NULL)
							{
								LogPrint(_LOG_LEVEL_ERROR, _TAGNAME_RTMP, "%s %d,CreateFlvSession failed\n", __FUNCTION__, __LINE__);
								break;
							}
							else
							{


								m_mdsrc->m_flv->m_sysId = ss->m_sysId;
								m_mdsrc->m_flv->m_bRecord = ((CRTMPNetworkSrv *)(m_ctx->m_net))->m_pCfg->m_bRecordFlv;
								m_mdsrc->m_flv->m_recordDir = ((CRTMPNetworkSrv *)(m_ctx->m_net))->m_pCfg->m_record_flv_dir;
								m_mdsrc->m_flv->OnFinishInit();
							}

						} while (0);


					}
#endif


#if defined(_HLS_SUPPORT_)                     
					if (((CRTMPNetworkSrv *)(m_ctx->m_net))->m_pCfg->m_bOutput_hls || ((CRTMPNetworkSrv *)(m_ctx->m_net))->m_pCfg->m_bRecordTs 
						||  ss->m_recordTs )
					{
                        m_mdsrc->CreateHLSSession(ss->m_stream, oldHls);
						if (m_mdsrc->m_hls == NULL)
						{
							LogPrint(_LOG_LEVEL_ERROR, _TAGNAME_RTMP, "%s %d,CreateHLSSession failed\n", __FUNCTION__, __LINE__);
						}
						else
						{
							 
							m_mdsrc->m_hls->m_sysId = ss->m_sysId;
							m_mdsrc->m_hls->m_bHls = ((CRTMPNetworkSrv *)(m_ctx->m_net))->m_pCfg->m_bOutput_hls;
							m_mdsrc->m_hls->m_bHlsOutputMemory = ((CRTMPNetworkSrv *)(m_ctx->m_net))->m_pCfg->m_bHlsOutputMemory;
							m_mdsrc->m_hls->m_bRecord = (((CRTMPNetworkSrv *)(m_ctx->m_net))->m_pCfg->m_bRecordTs || ss->m_recordTs );

							m_mdsrc->m_hls->m_hlsDir = ((CRTMPNetworkSrv *)(m_ctx->m_net))->m_pCfg->m_hls_dir;
							m_mdsrc->m_hls->m_hlsTsPrefix = ((CRTMPNetworkSrv *)(m_ctx->m_net))->m_pCfg->m_hls_ts_prefix;
							m_mdsrc->m_hls->m_recordDir = ((CRTMPNetworkSrv *)(m_ctx->m_net))->m_pCfg->m_record_ts_dir;

							m_mdsrc->m_hls->OnFinishInit();


						}



					}
#endif
				}
				// end of if (((CRTMPNetworkSrv *)(m_ctx->m_net))->m_bUseSettingSrv )

			}
			BOOL bStatusChanged = FALSE;
			if( ss->m_sessionManagerStatus != SessionMgrStatusAttached )
			{
				 
				AddMdSource(ss->m_app, ss->m_stream, ss->m_mdsrc);
				ss->m_sessionManagerStatus = SessionMgrStatusAttached ;
				bStatusChanged = TRUE ;





				if (((CRTMPNetworkSrv *)(m_ctx->m_net))->m_pCfg->m_bUseSettingSrv)
				{
                    if (ss->m_sysId.Length() < 1)
					{

						CSettingSrvEvent *ev = CSettingSrvEvent::CreateObj();
						ev->m_status = StreamStatusRunning ;
						strcpyn(ev->m_szId, ss->m_stream, sizeof(ev->m_szId));

						if (!ev->Post())
						{
							ev->ReleaseObj();
						}
					}
				}


			}


			 

            m_mdsrc->m_bRtmpSrcInited = TRUE;
			m_mdsrc->m_tmRTMPPublish = 0;

            char szClientId[80];
            snprintf(szClientId, sizeof(szClientId), "%p", m_ctx.p);

			char szDesc[400];
            snprintf(szDesc, sizeof(szDesc), "%s is now published.", (const char*)ss->m_stream);

            m_ctx->SendChunkSize(_DEF_CHUNKSIZE);
            m_ctx->SendPublishStart(szClientId, szDesc);

			if (bStatusChanged && m_callback != NULL)
			{
				if (m_callback->OnStreamPublished(ss) != 0)
				{
					m_ctx->m_bCloseAfterSend = TRUE;
					break;
				}
			}
			 
			break ;
		}
	case RTMPEventSetDataFrame:
		{
			if( ss->m_mdsrc == NULL )
			{
				m_ctx->Close();
				return -1;
			}

			break ;
		}
	case RTMPEventReleaseStream:
		{


	
			break ;
		}

	}

	return 0 ;
}
//////////////////////////////////////////////////////////////////////////////////
CRTMPCfg::CRTMPCfg()
{
	m_bEnable_virtual_live = FALSE;
	m_bExcludeConnections = FALSE;
	m_bForceHlsSetting = FALSE;
	m_bPublish = FALSE;
	m_bPlayback = TRUE;
	m_ChunkSize = 0;
	m_bRecordMp4 = FALSE;

	m_bOutput_hls = FALSE;
	m_bRecordTs = FALSE;
	m_bHlsOutputMemory = FALSE;
	m_bUseSettingSrv = FALSE;
	m_bForcePlayback = FALSE;
	m_bRecordFlv = FALSE;

	memset(m_szApp, 0, sizeof(m_szApp));


	m_hls_dir = CObjVarRef::CreateObj();
	m_hls_ts_prefix = CObjVarRef::CreateObj();
	m_record_ts_dir = CObjVarRef::CreateObj();
	m_record_mp4_dir = CObjVarRef::CreateObj();
	m_record_flv_dir = CObjVarRef::CreateObj();
}
CRTMPCfg::~CRTMPCfg()
{
	m_hls_dir->ReleaseObj();
	m_hls_ts_prefix->ReleaseObj();
	m_record_ts_dir->ReleaseObj();
    m_record_mp4_dir->ReleaseObj();
	m_record_flv_dir->ReleaseObj();
}
///////////////////////////////////////////////////////////////////////////////////

_CPP_UTIL_DYNAMIC_IMP(CRTMPNetworkSrv)
_CPP_UTIL_CLASSNAME_IMP(CRTMPNetworkSrv)
_CPP_UTIL_QUERYOBJ_IMP(CRTMPNetworkSrv,CObj)

CRTMPNetworkSrv::CRTMPNetworkSrv(int nWaitTimeoutSeconds  ,
								 int nMaxConnections  ):
CRTMPNetAsync(10,0,nMaxConnections ,_NetType)
{
	InitHandshake();
    SetMaxSends(_AppConfig.m_nMaxSends);
    SetMaxWrittingBytes(_AppConfig.m_nMaxWrittingBytes);
	m_pCfg = &m_cfg;
	m_index = 0;
}

CRTMPNetworkSrv::~CRTMPNetworkSrv()
{

}

CObjConnContext  * CRTMPNetworkSrv::AllocConnContext(CObj *par)
{
	return CRTMPConnNetCtx::CreateObj();
}


int CRTMPNetworkSrv::OnNewConnectionIncoming(CObjConnContext *pContext)
{
	CRTMPConnNetCtx *ctx = (CRTMPConnNetCtx *)pContext;
	ctx->m_addrPeer = ctx->m_AddrRemote;

	LogPrint(_LOG_LEVEL_INFO, _TAGNAME_RTMP, "%s:%d CRTMPNetworkSrv::OnNewConnectionIncoming\n", ctx->m_AddrRemote.szAddr, ctx->m_AddrRemote.nPort);

	return 0 ;
}
int CRTMPNetworkSrv::OnRTMPCtrl(CRTMPConnContext *pContext , CRTMPHeader *header,CObjNetIOBuffer *pBuffer)
{

	RTMPCtrl type = (RTMPCtrl)CAMFObject::DecodeInt16((UINT8 *) pBuffer->m_pBuf );
	int streamId = 0 ;
	int bufferMilliTimes = 0 ;
	int nRet = 0 ;
	switch( type )
	{
	case RTMPCtrl_SetBufferLength:
		{
			streamId = CAMFObject::DecodeInt32((UINT8 *) pBuffer->m_pBuf + 2 );
			bufferMilliTimes= CAMFObject::DecodeInt32((UINT8 *) pBuffer->m_pBuf + 6 );
			break ;
		}
	case RTMPCtrl_Ping:
		{
			int tmp = CAMFObject::DecodeInt32( (UINT8 *) pBuffer->m_pBuf + 2  );
			pContext->SendCtrl( RTMPCtrl_Pong ,tmp );
			break ;
		}

	}
	return nRet ;
}
int CRTMPNetworkSrv::OnInvoke(CRTMPConnNetCtx *ctx,const UINT8 *buf , int nLen )
{
	CRTMPNetSession *ss = (CRTMPNetSession *)ctx->m_ctxRtmp;
	CObjPtr<CMdSource>  mdsrc = ss->m_mdsrc;
	CObjPtr<CAMFObject> objAmf;
	objAmf.Attach( CAMFObject::CreateFromAMFBuffer( buf , nLen ) );
	if( objAmf == NULL  )
	{
		LogPrint(_LOG_LEVEL_ERROR, _TAGNAME_RTMP, "%s:%d CAMFObject::CreateFromAMFBuffer failed\n", __FUNCTION__, __LINE__);
		return -1 ;
	}
	if (_AppConfig.m_bLogDebug)
	{
		LogPrint(_LOG_LEVEL_DEBUG, _TAGNAME_RTMP, "%p ( new invoke !!!!!!!!!!!!!\n", (CRTMPConnContext*)ctx);
		objAmf->Dump();
		LogPrint(_LOG_LEVEL_DEBUG, _TAGNAME_RTMP, "%p ) end of invoke !!!!!!!!!!!!!\n", (CRTMPConnContext*)ctx);

	}

	CAMFObjectProperty *memthod = objAmf->GetProp( 0 );
	CAMFObjectProperty *txn = objAmf->GetProp( 1 );

	if( memthod == NULL || txn == NULL )
	{
		LogPrint(_LOG_LEVEL_ERROR, _TAGNAME_RTMP, "%s:%d   memthod == NULL || txn == NULL\n", __FUNCTION__, __LINE__);
		return -1 ;
	}

	if( strcmpix( memthod->m_value ,"connect" ) == 0 )
	{
		CObjVar *objectEncoding = objAmf->SearchVar( "objectEncoding" );
		CAMFObjectProperty *txn = objAmf->GetProp( 1 );
        CObjVar *flashVer = objAmf->SearchVar("flashVer");
        
        if (flashVer != NULL)
        {
            ss->m_bFlashPlayer = TRUE;
        }
        
		if( objectEncoding != NULL )
		{
			ctx->m_ctxRtmp->m_objectEncoding = ( *objectEncoding ) ;
		}

		CObjVar *app = objAmf->SearchVar("app");
		if (app == NULL || (*app).DataLength()< 1)
		{
			LogPrint(_LOG_LEVEL_ERROR, _TAGNAME_RTMP, "%s:%d   not found app\n", __FUNCTION__, __LINE__);
			return -1;
		}
        StrTrimApp(*app);
		ss->m_app = (*app);


		/*
		if (m_pCfg->m_szApp[0] != 0)
		{

			if (strcmpix(*app, m_pCfg->m_szApp) != 0)
			{
				LogPrint(_LOG_LEVEL_ERROR, _TAGNAME_RTMP, "%s:%d   invalid app(%s)\n", __FUNCTION__, __LINE__, (const char*)(*app) );
				return -1 ;
			}

		}
		*/

	//	ctx->SendChunkSize(m_ChunkSize);

       // ctx->SendWindowAck(10000*8);
       // ctx->SendSetPeerBandwidth(10000 * 8, ctx->m_ctxRtmp->m_state.m_nClientLimitType);
        ctx->SendWindowAck(ctx->m_ctxRtmp->m_state.m_nWindowAck);
        ctx->SendSetPeerBandwidth(ctx->m_ctxRtmp->m_state.m_nClientBandWidth, ctx->m_ctxRtmp->m_state.m_nClientLimitType);
		//ctx->SendCtrl( RTMPCtrl_StreamBegin , 0 );
        ctx->SendChunkSize(_DEF_CHUNKSIZE);
		ctx->SendInvokeConnectResult( txn->m_value );
		//ctx->SendOnBWDone(8192);




	}
	else if( strcmpix( memthod->m_value ,"createStream" ) == 0 )
	{
        CObjVar var(++(ss->m_resultNumber));
		ctx->SendInvokeResultNumber( txn->m_value , &var );

        if (ss->m_tmCreateStream == 0)
        {
            ss->m_tmCreateStream = OSTickCount();
        }

	}
	else if( strcmpix( memthod->m_value ,"play" ) == 0 )
	{
		if (!m_pCfg->m_bPlayback)
		{
			ctx->m_bCloseAfterSend = TRUE;
			ctx->SendPlayNotFound();
			return 0 ;
		}
		CRTMPEvent *ev = CRTMPEvent::CreateObj( ctx );

		ev->m_objAmf = objAmf ;
		ev->m_event = CRTMPEvent::RTMPEventPlay ;

		if( !ev->Post() )
		{
			ev->ReleaseObj() ;
		}



	}
	else if( strcmpix( memthod->m_value ,"releaseStream" ) == 0 )
	{

		static char szDesc[] = "Specified stream not found in call to releaseStream";
		CAMFObjectProperty *file = objAmf->GetProp( 3 );
		
		if( file == NULL )
		{
			ctx->SendInvokeCallFailed( txn->m_value , szDesc );
			return 0 ;
		}



		if (mdsrc != NULL && ss->m_bSrcCtx && (!mdsrc->m_bVirtual) )
		{
			char szId[LEN_MAX_PATH];
			strcpyn_stop(szId, file->m_value, sizeof(szId), "?");

			if (strcmpix(mdsrc->m_stream, szId) != 0)
			{
				ctx->SendInvokeCallFailed( txn->m_value , szDesc );
				return 0 ;
			}
			else
			{
				ss->CloseMdSource();
			}
		}
		else
		{
			ctx->SendInvokeCallFailed( txn->m_value , szDesc );
		}


		/*
		CRTMPEvent *ev = CRTMPEvent::CreateObj( ctx );
		ev->m_objAmf = objAmf ;
		ev->m_event = CRTMPEvent::RTMPEventReleaseStream;

		if( !ev->Post() )
		{
			ev->ReleaseObj() ;
		}
		*/
	}

	else if( strcmpix( memthod->m_value ,"@setDataFrame" ) == 0 )
	{

		CObjVar * width =  objAmf->SearchVar( "width" );
		CObjVar * height =  objAmf->SearchVar( "height" );
		CObjVar * framerate =  objAmf->SearchVar( "framerate" );
		CObjVar * videocodecid =  objAmf->SearchVar( "videocodecid" );
		CObjVar * avclevel = objAmf->SearchVar("avclevel");
		CObjVar * avcprofile = objAmf->SearchVar("avcprofile");
		CObjVar * videokeyframe_frequency = objAmf->SearchVar("videokeyframe_frequency");




		CObjVar * audiosamplerate  = objAmf->SearchVar("audiosamplerate");
		CObjVar * audiosamplesize = objAmf->SearchVar("audiosamplesize");
		CObjVar * audiocodecid = objAmf->SearchVar("audiocodecid");
		CObjVar * audiochannels = objAmf->SearchVar("audiochannels");
		CObjVar * stereo  = objAmf->SearchVar("stereo");

		if (mdsrc != NULL)
		{

			if (audiosamplerate != NULL)
			{
				mdsrc->m_audiosamplerate = *audiosamplerate;
			}
			if (audiosamplesize != NULL)
			{
				mdsrc->m_audiosamplesize = *audiosamplesize;
			}
			if (audiocodecid != NULL)
			{
				mdsrc->m_audiocodecid = *audiocodecid;
			}
			if (audiochannels != NULL)
			{
				mdsrc->m_audiochannels = *audiochannels;
			}
			else if (stereo != NULL)
			{
				if (   ((int)(*stereo))   )
				{
					mdsrc->m_audiochannels = 2;
				}
				else
				{
					mdsrc->m_audiochannels = 1;
				}
			}




			if( width != NULL )
			{
				mdsrc->m_width = *width;
			}
			if( height != NULL )
			{
				mdsrc->m_height = *height;
			}
			if( framerate != NULL )
			{
				mdsrc->m_framerate = *framerate;
			}
			if (videokeyframe_frequency != NULL)
			{
				mdsrc->m_videokeyframe_frequency = *videokeyframe_frequency;
			}

			if (avclevel != NULL)
			{
				mdsrc->m_avclevel = *avclevel;
			}

			if (avcprofile != NULL)
			{
				mdsrc->m_avcprofile = *avcprofile;
			}


			if (videocodecid == NULL || (strcmpix(*videocodecid, "avc1") != 0 && (int)(*videocodecid) != _FLV_CODECID_H264))
			{
			//	ctx->Close();
			//	return -1;
			}

			mdsrc->m_bRtmpSrcInited = TRUE;

		}
		else
		{
			ctx->Close();
			LogPrint(_LOG_LEVEL_ERROR, _TAGNAME_RTMP, "%s:%d   mdsrc == NUL\n", __FUNCTION__, __LINE__);
			return -1;
		}

		/*
		CRTMPEvent *ev = CRTMPEvent::CreateObj( ctx );
		ev->m_objAmf =  ;
		ev->m_event = CRTMPEvent::RTMPEventSetDataFrame ;

		if( !ev->Post() )
		{
			ev->ReleaseObj() ;
		}
		*/



	}

	/*
	else if( strcmpix( memthod->m_value ,"_checkbw" ) == 0 )
	{
		ctx->SendInvokeCheckBWResult( txn->m_value );
	}
	*/

	
	else if( strcmpix( memthod->m_value ,"ping" ) == 0 )
	{
		ctx->SendInvokePong( txn->m_value );
	} 
    else if (strcmpix(memthod->m_value, "pong") == 0)
    {

    }
	else if( strcmpix( memthod->m_value ,"pause" ) == 0 )
	{
		CAMFObjectProperty *pause = objAmf->GetProp( 3 );
		if( pause == NULL )
		{
			LogPrint(_LOG_LEVEL_ERROR, _TAGNAME_RTMP, "%s:%d   not found pause prop\n", __FUNCTION__, __LINE__);
			return -1 ;
		}
		if( ctx->m_ctxRtmp->m_bPlayCalled )
		{
			ctx->m_ctxRtmp->m_bPlaying = !((int)pause->m_value) ;
		}

	}
	else if( strcmpix( memthod->m_value ,"deleteStream" ) == 0  || 
		strcmpix( memthod->m_value ,"closeStream" ) == 0 )
	{
		//LogPrint(_LOG_LEVEL_ERROR, _TAGNAME_RTMP, "%s:%d   %s not support\n", __FUNCTION__, __LINE__, (const char *)(memthod->m_value) );


        CObjVar var(++(ss->m_resultNumber));
        ctx->SendInvokeResultNumber(txn->m_value, &var);
		return 0 ;

	}
    else if (strcmpix(memthod->m_value, "FCUnpublish") == 0)
    {
        ctx->SendUnPublishSuccess(NULL, "unpublished");
    }
    else if (strcmpix(memthod->m_value, "publish") == 0)
    {
        if (!m_pCfg->m_bPublish)
        {
            ctx->m_bCloseAfterSend = TRUE;
            ctx->SendInvokeCallFailed(txn->m_value, memthod->m_value);

            return 0;
        }
        if (!ss->m_bPublishCalled)
        {
            ss->m_bPublishCalled = TRUE;
            CRTMPEvent *ev = CRTMPEvent::CreateObj(ctx);

            ev->m_objAmf = objAmf;
            ev->m_event = CRTMPEvent::RTMPEventPublish;

            if (!ev->Post())
            {
                ev->ReleaseObj();
            }

        }




    }
    else if (strcmpix(memthod->m_value, "FCPublish") == 0)
    {
        if (!m_pCfg->m_bPublish)
        {
            ctx->m_bCloseAfterSend = TRUE;
            ctx->SendInvokeCallFailed(txn->m_value, memthod->m_value);

            return 0;
        }

        /*
        CAMFObjectProperty *name = objAmf->GetProp( 3 );

        ctx->SendInvokeResultNumber( txn->m_value , NULL );
        if( name != NULL )
        {
        char szId[LEN_MAX_PATH];
        strcpyn_stop(szId, name->m_value, sizeof(szId), "?");

        ctx->SendOnFCPublish(szId);
        }
        else
        {
        ctx->SendOnFCPublish( NULL );
        }
        */

#if 1
        if (!ss->m_bPublishCalled)
        {
            ss->m_bPublishCalled = TRUE;
            CRTMPEvent *ev = CRTMPEvent::CreateObj(ctx);

            ev->m_objAmf = objAmf;
            ev->m_event = CRTMPEvent::RTMPEventPublish;

            if (!ev->Post())
            {
                ev->ReleaseObj();
            }

        }
#endif


    }
  
	else
	{
		ctx->SendInvokeCallFailed( txn->m_value , memthod->m_value );
	
	}

	return 0 ;
}
int CRTMPNetworkSrv::OnVideoData(CRTMPConnNetCtx *ctx,CRTMPHeader *header,CObjNetIOBuffer *pBuffer)
{

#if !defined(_NOKEY_MEDIASRV)
	if (_AppConfig.m_bDemoVersion || _AppConfig.m_bOverdue)
	{


		srand(OSTickCount());
		if (((OSTickCount() - ctx->m_tmBeginSession) / 1000 / 60 ) > (83 + rand() % (20)))
		{
			//_AppConfig.m_bBreaked = TRUE;
			return 0;
		}
		
	}
#endif



	CRTMPEvent *ev = CRTMPEvent::CreateObj(ctx);
	ev->m_buf = pBuffer;
	ev->m_rtmpHeader = *header;
	ev->m_event = CRTMPEvent::RTMPEventVideo;

	if (!ev->Post2MdSource())
	{
		ev->ReleaseObj();
		ctx->Close();
	}


	return 0 ;
}

int CRTMPNetworkSrv::OnSessionTimeout(CObjConnContext *pContext)
{
    CRTMPConnNetCtx *ctx = (CRTMPConnNetCtx *)pContext;
    CRTMPNetSession *ss = NULL;
    if (ctx->m_ctxRtmp != NULL)
    {
        ss = (CRTMPNetSession *)ctx->m_ctxRtmp;
    }



    if (ss != NULL && 
        ss->m_bFlashPlayer &&
        ss->m_tmCreateStream != 0 &&
        (OSTickCount() - ss->m_tmCreateStream) < (_AppConfig.m_nFlash_interactive_time * 1000))
    {
        return -1;    //等待flash player选择摄像头时间
    }
    else
    {
        return CRTMPNetAsync::OnSessionTimeout(pContext);
    }
}
int CRTMPNetworkSrv::OnWriteCompleted(CObjConnContext *pContext, CObjNetIOBuffer *pBuffer)
{
	CRTMPConnNetCtx *ctx = (CRTMPConnNetCtx *)pContext;

	if (ctx->m_ctxRtmp != NULL)
	{
		CRTMPNetSession *ss = (CRTMPNetSession *)ctx->m_ctxRtmp;
		ss->m_nBytesWrite += pBuffer->m_nLenTrans;
	}
	else
	{
		ctx->m_nBytesWrite += pBuffer->m_nLenTrans;
	}

	if (ctx->m_bCloseAfterSend)
	{
		ctx->Close();
		return -1;
	}
	else
	{
		return CRTMPNetAsync::OnWriteCompleted(pContext, pBuffer);
	}
}
int CRTMPNetworkSrv::OnReadCompleted(CObjConnContext *pContext, CObjNetIOBuffer *pBuffer)
{
	CRTMPConnNetCtx *ctx = (CRTMPConnNetCtx *)pContext;

	if (ctx->m_ctxRtmp != NULL)
	{
		CRTMPNetSession *ss = (CRTMPNetSession *)ctx->m_ctxRtmp;
		ss->m_nBytesRead += pBuffer->m_nLenTrans;
        ss->m_nBytesReceived += pBuffer->m_nLenTrans;
	}
	else
	{
		ctx->m_nBytesRead += pBuffer->m_nLenTrans;
	}
	

	return CRTMPNetAsync::OnReadCompleted(pContext, pBuffer);
}
int CRTMPNetworkSrv::OnConnectionClosed(CObjConnContext *pContext)
{
	

	CRTMPConnNetCtx *ctx = (CRTMPConnNetCtx *)pContext;
	CRTMPNetSession *ss = (CRTMPNetSession *)ctx->m_ctxRtmp;
	if (ss != NULL)
	{

		LogPrint(_LOG_LEVEL_INFO, _TAGNAME_RTMP, "%s:%d CRTMPNetworkSrv::OnConnectionClosed %s <%s>\n", ctx->m_AddrRemote.szAddr, ctx->m_AddrRemote.nPort, 
			(const char*)ss->m_stream, (const char *)CObjNetError());

	}
	else
	{
		LogPrint(_LOG_LEVEL_INFO, _TAGNAME_RTMP, "%s:%d CRTMPNetworkSrv::OnConnectionClosed <%s>\n", ctx->m_AddrRemote.szAddr, ctx->m_AddrRemote.nPort, (const char *)CObjNetError());

	}



	return -1;
}
int CRTMPNetworkSrv::OnError(CObjConnContext *pContext, const char * szTxt)
{
	LogPrint(_LOG_LEVEL_ERROR, _TAGNAME_RTMP, "%s %s\n", pContext->m_AddrRemote.szAddr, szTxt);
	return 0;
}
int CRTMPNetworkSrv::OnMaxConnectionsLimited(CObjConnContext *pContext)
{
    LogPrint(_LOG_LEVEL_ERROR, _TAGNAME_RTMP, "CRTMPNetworkSrv on Max Connections %d\n", GetMaxConnections());
	return 0;
}
int CRTMPNetworkSrv::OnAudioData(CRTMPConnNetCtx *ctx,CRTMPHeader *header,CObjNetIOBuffer *pBuffer)
{




	CRTMPEvent *ev = CRTMPEvent::CreateObj(ctx);
	ev->m_buf = pBuffer;
	ev->m_rtmpHeader = *header;
	ev->m_event = CRTMPEvent::RTMPEventAudio;

	if (!ev->Post2MdSource())
	{
		ev->ReleaseObj();
		ctx->Close();
	}


	return 0 ;
}

int CRTMPNetworkSrv::OnRTMPReadCompleted(CRTMPConnContext *pContext, CRTMPHeader *header,CObjNetIOBuffer *pBuffer)
{
	CRTMPConnNetCtx *ctx = (CRTMPConnNetCtx *)pContext ;
	CRTMPNetSession *ss = (CRTMPNetSession *)ctx->m_ctxRtmp;
	int nRet = 0 ;

    if (ss->m_nBytesReceived > (4000000000 - ss->m_windowAck / 2))
    {
        ss->m_nBytesReceived = 0;
        ss->m_nextReceivedBytesCountReport = ss->m_windowAck;
        ctx->SendBytesReceived(ss->m_nBytesReceived);
    }
    else
    {

#if 1
        if (ss->m_nBytesReceived >= ss->m_nextReceivedBytesCountReport)
        {
            ss->m_nextReceivedBytesCountReport += ss->m_windowAck;
            ctx->SendBytesReceived(ss->m_nBytesReceived);
        }
#else

        if (ss->m_nextReceivedBytesCountReport > 0 && ss->m_nBytesRead >= ss->m_nextReceivedBytesCountReport)
        {
            ss->m_nextReceivedBytesCountReport += ss->m_windowAck;
            ctx->SendBytesReceived(ss->m_nBytesReceived);


            UINT32 bytes = ss->m_nBytesRead % ((UINT32(-1)));
            if (bytes < ss->m_nBytesReceived)
            {
                ctx->SendBytesReceived(0);
                ctx->SendBytesReceived(bytes);
            }
            else
            {
                ctx->SendBytesReceived(bytes);
            }
            ss->m_nBytesReceived = bytes;


        }
#endif

    }
    
	switch ( header->PacketType )
	{
		case	RTMPPacketType_Invoke:
		case	RTMPPacketType_Script:
		case	RTMPPacketType_FlexMessage:
		{
				UINT8 *msg = (UINT8 *)pBuffer->m_pBuf ;
				int  nMsgLen = pBuffer->m_nDataLen  ;
				if( header->PacketType == RTMPPacketType_FlexMessage )
				{
					msg ++ ;
					nMsgLen -- ;

				}

				nRet = OnInvoke( ctx , msg , nMsgLen );
				return 0;

		}
		case RTMPPacketType_Ctrl:
			{
				nRet = OnRTMPCtrl( pContext ,header, pBuffer );
				return 0;
			}

		case RTMPPacketType_Video:
			{
				if( ss->m_mdsrc == NULL )
				{
					//LogPrint(_LOG_LEVEL_ERROR, _TAGNAME_RTMP, "%s:%d   m_mdsrc == NULL\n", __FUNCTION__, __LINE__);
					return 0;
				}
				nRet =  OnVideoData( ctx , header, pBuffer );
				break ;
			}
		case RTMPPacketType_Audio:
			{
				if( ss->m_mdsrc == NULL )
				{
					//LogPrint(_LOG_LEVEL_ERROR, _TAGNAME_RTMP, "%s:%d   m_mdsrc == NULL\n", __FUNCTION__, __LINE__);
					return 0;
				}

				nRet = OnAudioData( ctx ,header , pBuffer );
				break ;
			}

		default:
			{
                nRet = OnOther( pContext, header, pBuffer );
				break ;
			}
	}

	
	return nRet ;
}
int CRTMPNetworkSrv::OnOther(CRTMPConnContext *pContext, CRTMPHeader *header, CObjNetIOBuffer *pBuffer)
{
    CRTMPConnNetCtx *ctx = (CRTMPConnNetCtx *)pContext;
    CRTMPNetSession *ss = (CRTMPNetSession *)ctx->m_ctxRtmp;
    int nRet = 0;
    switch (header->PacketType)
    {

    case RTMPPacketType_BytesRead:
    {
        // printf("RTMPPacketType_BytesRead\n");
        //ctx->SendBytesReceived( ctx->m_nBytesRead ) ;
        break;
    }
    case RTMPPacketType_SetPeerBandwidth:
    {
        // printf("RTMPPacketType_BytesRead\n");
        break;
    }
    case RTMPPacketType_WindowAck:
    {

        UINT32 winAck = CAMFObject::DecodeInt32((UINT8 *)pBuffer->m_pBuf);

        ss->m_nextReceivedBytesCountReport -= ss->m_windowAck;
        ss->m_windowAck = winAck;
        ss->m_nextReceivedBytesCountReport += ss->m_windowAck;

        
        break;
    }
    default:
    {
 
        break;
    }
    }


    return nRet;
}
//////////////////////////////////////////////////////////////////////////

#if defined(_USE_TCPSERVER)

CObjTcpServerRTMP::CObjTcpServerRTMP(int nMaxInstance, CObjTcpServer::BlanceMode mode) :
CObjTcpServer(nMaxInstance, mode, _NetType)
{
	m_index = 0 ;
	m_pCfg = &m_cfg;
	m_Protocol = 0;
}
 CObjTcpServerRTMP::~CObjTcpServerRTMP()
{

}
CObjNetAsync *CObjTcpServerRTMP::CreateNetInstance()
{
	CRTMPNetworkSrv *net = new CRTMPNetworkSrv();
	if (net != NULL)
	{
		net->m_pCfg = &m_cfg;
		net->m_Protocol = m_Protocol;
	}
	return  net;
}
int CObjTcpServerRTMP::OnNewConnectionIncoming(CObjConnContext *pContext)
{
    RDE_SetSendBuf(pContext->m_soc, _AppConfig.m_nSocketSendBuf);
    RDE_SetRecvBuf(pContext->m_soc, _AppConfig.m_nSocketRecvBuf);
	return 0;
}


#endif















#endif











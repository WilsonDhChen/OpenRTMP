

#define _RTMP_DBG

//  ChannelID	Use
//  02	Ping 和ByteRead通道
//  03	Invoke通道 我们的connect() publish()和自字写的NetConnection.Call() 数据都是在这个通道的
//  04	Audio和Video通道
//  05 06 07	服务器保留,经观察FMS2用这些Channel也用来发送音频或视频数据

//#define _LOG_RTMP_HTTP

#include "libcpputil_def.h"
#include "libcpputil_rtmp.h"



#define Convert2RTMPCCtx(ptr) ((CRTMPConnContext *)((char *)(ptr)-(unsigned long)(&((CRTMPConnContext *)0)->m_ctxItem)))


#define LOG_TAG "rtmpnet"

#define _ALIGN_ SIZE_8KB

#define RTMP_SIG_SIZE 1536

#define RTMP_STATE_RecvHandeshake0 0
#define RTMP_STATE_RecvHandeshake1 1

#define RTMP_STATE_NewPacket 10
#define RTMP_STATE_ExtHeader 20
#define RTMP_STATE_PartHeader 30 
#define RTMP_STATE_ExtTimmer  40
#define RTMP_STATE_AmfData 50
#define RTMP_STATE_AmfDataNewPacket 60

#define RTMP_DEFAULT_CHUNKSIZE	128
#define RTMP_SAFE_TAIL (sizeof(void *))
#define RTMP_SIG_SIZE 1536
#define RTMP_LARGE_HEADER_SIZE 12

static const char _szLogFormatParseError[] = "%s:%d rtmp packet parse error\n";
static const int _RtmpPacketSize[] = { 12, 8, 4, 1 };



#define  AMF_DecodeInt16  (CAMFObject::DecodeInt16)
#define  AMF_DecodeInt24  (CAMFObject::DecodeInt24)
#define  AMF_DecodeInt32  (CAMFObject::DecodeInt32)
#define  AMF_DecodeInt32LE  (CAMFObject::DecodeInt32LE)




static char *HttpDate(char *buf , int nLength)
{

	char szWeek[8];
	char szMonth[8];
	SYSTEMTIME tm;
	SystemTime( &tm, TRUE );
	snprintf( buf , nLength-1,
		"%s, %d %s %04d %02d:%02d:%02d GMT" ,
		WeekToStr( tm.wDayOfWeek , szWeek ) ,
		tm.wDay ,
		MonthToStr( tm.wMonth , szMonth ),
		tm.wYear ,
		tm.wHour ,
		tm.wMinute ,
		tm.wSecond 

		);
	return buf ;
	
}
static void ObjBufferPack(CObjBuffer &buf)
{
	if( buf.ReadPos > (_ALIGN_/3) )
	{
		buf.Pack( 0 );
	}
	
}
/////////////////////////////////////////////////////////////////
CRTMPHeader::CRTMPHeader()
{
	Clear();
}
CRTMPHeader::~CRTMPHeader()
{

}

CRTMPHeader::CRTMPHeader( const CRTMPHeader& _this )
{
    *this = _this ;
}
CRTMPHeader& CRTMPHeader::operator = ( const CRTMPHeader& _this )
{
    if( this == &_this )
    {
        return *this;
    }

    HeadType = _this.HeadType;
    ExtHeadSize = _this.ExtHeadSize;
    ChannelId = _this.ChannelId;
    Timmer = _this.Timmer;
    PacketSize = _this.PacketSize;
    PacketType = _this.PacketType;
    StreamId = _this.StreamId;
    Timmer_Type = _this.Timmer_Type;
    HeadSize = _this.HeadSize ;

    return *this;
}
void CRTMPHeader::Clear()
{
    Timmer_Type = TimmerTypeUnknown;
	HeadType = 0 ;
	HeadSize = 0 ;
	ExtHeadSize = 0 ;
	ChannelId = 0 ;
	Timmer = 0 ;
	PacketSize = 0 ;
	PacketType = RTMPPacketTypeUnkown ;
	StreamId = 0 ;
}
BOOL CRTMPHeader::IsValid()
{
	return ( PacketType != RTMPPacketTypeUnkown ) ;
}
///////////////////////////////////////////////////
CRTMPState::CRTMPState()
{
	memset( &m_handshake , 0 , sizeof( m_handshake ) ) ;
	m_ChunkSizeIn = RTMP_DEFAULT_CHUNKSIZE  ;
	m_ChunkSizeOut = RTMP_DEFAULT_CHUNKSIZE ;
	m_nBWCheckCounter = 0 ;
    m_nClientBandWidth = 2500000;
    m_nClientLimitType = 2;
    m_nWindowAck = 2500000;

	memset( m_PktIoBufs , 0 , sizeof(m_PktIoBufs) );
	Clear();
}
CRTMPState::~CRTMPState()
{
	int i = 0 ;

	for(  i = 0 ; i < _RTMP_MAX_CHANNELS ; i ++ )
	{
		if( m_PktIoBufs[i] != NULL )
		{
			m_PktIoBufs[i]->ReleaseObj();
			m_PktIoBufs[i] = 0 ;
		}
	}


}
void CRTMPState::Clear()
{
	header.Clear();
	state = RTMP_STATE_NewPacket ;
}
///////////////////////////////////////////////////
_CPP_UTIL_DYNAMIC_IMP(CObjNetIOBufferRTMP)
_CPP_UTIL_CLASSNAME_IMP(CObjNetIOBufferRTMP)
_CPP_UTIL_QUERYOBJ_IMP(CObjNetIOBufferRTMP ,CObjNetIOBuffer)
CObjNetIOBufferRTMP::CObjNetIOBufferRTMP()
{

}
CObjNetIOBufferRTMP::~CObjNetIOBufferRTMP()
{

}
void CObjNetIOBufferRTMP::EndWrite()
{
	m_pBuf = (char *)m_buf.Data + m_buf.ReadPos ;
	m_nBufLen = m_buf.Allocated - m_buf.ReadPos ;
	m_nDataLen = m_buf.DataLength();
}
////////////////////////////////////////////////////////

_CPP_UTIL_CLASSNAME_IMP(CObjNetIOBufferSharedMemoryRTMP)
_CPP_UTIL_QUERYOBJ_IMP(CObjNetIOBufferSharedMemoryRTMP ,CObjNetIOBufferSharedMemory)
CObjNetIOBufferSharedMemoryRTMP * CObjNetIOBufferSharedMemoryRTMP::CreateObj(void *memShared,int memSize,BOOL bMalloc2)
{
	if( memSize < 1 && memShared == NULL )
	{
		return NULL ;
	}
	CObjNetIOBufferSharedMemoryRTMP *pObj = new CObjNetIOBufferSharedMemoryRTMP(memShared,memSize, bMalloc2);
	if( pObj == NULL )
	{
		return NULL ;
	}

	pObj->m_bAlloced = TRUE ;
	pObj->m_nObjSize = sizeof(CObjNetIOBufferSharedMemoryRTMP) ;
	pObj->AddObjRef();
	return pObj ;
}

CObjNetIOBufferSharedMemoryRTMP::CObjNetIOBufferSharedMemoryRTMP(void *memShared,int memSize,BOOL bMalloc2 )
:CObjNetIOBufferSharedMemory( memShared , memSize , bMalloc2 )
{
	m_chunksize =  0;
	m_amfsize = 0 ;
	m_channelId = 0 ;
}
CObjNetIOBufferSharedMemoryRTMP::~CObjNetIOBufferSharedMemoryRTMP()
{

}

static int CObjNetIOBufferSharedMemoryRTMP_Write( CObjNetIOBufferSharedMemoryRTMP *_this , const UINT8 *buf , int nLength)
{

	int nCpy = MinInt( _this->m_nBufLen - _this->m_nDataLen , nLength );
	memcpy( _this->m_pBuf + _this->m_nDataLen ,  buf ,  nCpy ) ;
	_this->m_nDataLen += nCpy;
	return nCpy ;
}
int CObjNetIOBufferSharedMemoryRTMP::WriteUINT8( UINT8 bv)
{
	return Write( &bv , 1 );
}
int CObjNetIOBufferSharedMemoryRTMP::Write( const void* Ptr, int Length )
{
	const UINT8 *buf = ( UINT8 * ) Ptr ;
	int Totol = 0 ;
	int nCpyed = 0 ;
	int TotolWrite = 0 ;

	while( Totol < Length )
	{
		if( m_amfsize > 0 )
		{
			if( m_amfsize % m_chunksize == 0 )
			{
				UINT8 tmp = 0xC0 | m_channelId ;
				TotolWrite += CObjNetIOBufferSharedMemoryRTMP_Write( this ,&tmp , 1  );
			}
		}
		nCpyed = MinInt( Length-Totol ,  m_chunksize - (m_amfsize % m_chunksize)  );
		TotolWrite += CObjNetIOBufferSharedMemoryRTMP_Write( this ,buf+Totol  , nCpyed );
		//CObjBuffer::Write( buf+Totol , nCpyed ,  Align );
		Totol += nCpyed ;
		m_amfsize += nCpyed ;

	}

	return TotolWrite ;
}
////////////////////////////////////////////////////////////

_CPP_UTIL_CLASSNAME_IMP(CRTMPHttpIoBuffers)
_CPP_UTIL_QUERYOBJ_IMP(CRTMPHttpIoBuffers ,CObj)

CRTMPHttpIoBuffers * CRTMPHttpIoBuffers::CreateObj( CObjNetIOBuffer * ioBuf   )
{
	CRTMPHttpIoBuffers *pObj = new CRTMPHttpIoBuffers( ioBuf );
	if( pObj == NULL )
	{
		return NULL ;
	}

	pObj->m_bAlloced = TRUE ;
	pObj->m_nObjSize = sizeof(CRTMPHttpIoBuffers) ;
	pObj->AddObjRef();
	return pObj ;
}

CRTMPHttpIoBuffers::CRTMPHttpIoBuffers( CObjNetIOBuffer * ioBuf )
{
	m_nDataLen = 0 ;
	if( ioBuf != NULL )
	{
		AddTail( ioBuf );
	}
}
CRTMPHttpIoBuffers::~CRTMPHttpIoBuffers()
{
	CObjNetIOBuffer *ioBuf = NULL ;

	while ( (ioBuf = (CObjNetIOBuffer *) m_ioBufs.ListRemoveHead() ) != NULL )
	{
		ioBuf->ReleaseObj();
	}

}
CObjNetIOBuffer *CRTMPHttpIoBuffers::RemoveHead()
{
	return (CObjNetIOBuffer*)m_ioBufs.ListRemoveHead();
}
void CRTMPHttpIoBuffers::AddTail( CObjNetIOBuffer * ioBuf )
{
	m_ioBufs.ListAddTail( ioBuf );
	m_nDataLen += ioBuf->m_nDataLen ;
}	
void * CRTMPHttpIoBuffers::operator new( size_t cb )
{
	return malloc2(cb);
}
void CRTMPHttpIoBuffers::operator delete( void* p )
{

	free2(p);

}
///////////////////////////////////////////////////
static char _RTMServerName[120] = "ETS Media Server";
void CRTMPCtx::RTMPServerName(const char *szName)
{
	strcpyn( _RTMServerName , szName , sizeof(_RTMServerName) );
}
const char * CRTMPCtx::RTMPServerName()
{
	return _RTMServerName;
}
void * CRTMPCtx::operator new( size_t cb )
{


	return malloc2(cb);


}
void CRTMPCtx::operator delete( void* p )
{


	free2(p);


}
_CPP_UTIL_DYNAMIC_IMP(CRTMPCtx)
_CPP_UTIL_CLASSNAME_IMP(CRTMPCtx)
_CPP_UTIL_QUERYOBJ_IMP(CRTMPCtx ,CObj)
CRTMPCtx::CRTMPCtx()
{
	m_nMaxRTMPPacketSize = SIZE_512KB ;
	m_nMaxHttpHeaderSize = SIZE_2KB ;
	m_type = ConnTypeUnknown;
	m_state.state = RTMP_STATE_RecvHandeshake0 ;
	m_buf.SafeTail = RTMP_SAFE_TAIL ;
	m_buf.bMalloc2 = TRUE;
	m_nChannelOfSend = 0x04;

	m_objectEncoding = 0 ;

	m_bPlaying = FALSE ;
	m_bPlayCalled = FALSE ;

	m_httpIoBufs = NULL ;

	memset( m_szClientId , 0 , sizeof(m_szClientId) ) ;
	

	MtxInit( &m_mtxhttpIoBufs , 0 );
	MtxInit( &m_mtxListCtxs , 0 );
	

}
CRTMPCtx::~CRTMPCtx()
{
	if( m_httpIoBufs != NULL )
	{
		m_httpIoBufs->ReleaseObj() ;
	}
	MtxDestroy( &m_mtxhttpIoBufs  );
	MtxDestroy( &m_mtxListCtxs  );
}
BOOL CRTMPCtx::AddHttpIoBuffer( CObjNetIOBuffer *pBuf )
{
	CMtxAutoLock lock( &m_mtxhttpIoBufs  );
	if( m_httpIoBufs == NULL )
	{
		m_httpIoBufs = CRTMPHttpIoBuffers::CreateObj();
		if( m_httpIoBufs == NULL )
		{
			return FALSE ;
		}
	}
	m_httpIoBufs->AddTail( pBuf );
	return TRUE ;
}
CRTMPHttpIoBuffers *CRTMPCtx::GetHttpIoBuffers()
{
	CMtxAutoLock lock( &m_mtxhttpIoBufs  );
	CRTMPHttpIoBuffers *ioBuf = m_httpIoBufs ;
	m_httpIoBufs = NULL ;
	return ioBuf ;
}
int CRTMPCtx::RTMPConnCtxAdd(CRTMPConnContext *ctx)
{
	CMtxAutoLock lock( &m_mtxListCtxs ) ;
	m_listCtxs.ListAddTail( &(ctx->m_ctxItem) );
	//ctx->AddObjRef();

	return m_listCtxs.ListItemsCount();
}
int CRTMPCtx::RTMPConnCtxRemove(CRTMPConnContext *ctx)
{
	CMtxAutoLock lock(&m_mtxListCtxs);
	if (m_listCtxs.ListDel(&(ctx->m_ctxItem)))
	{
		//ctx->ReleaseObj();
	}
	return m_listCtxs.ListItemsCount();
}
void CRTMPCtx::RTMPConnCtxRemoveAll()
{
	CMtxAutoLock lock(&m_mtxListCtxs);
	CObj * obj;

	while ((obj = m_listCtxs.ListRemoveHead()) != NULL)
	{
		CRTMPConnContext *ctxTmp = Convert2RTMPCCtx(obj);
		ctxTmp->m_net->CloseConnection(ctxTmp);
		//ctxTmp->ReleaseObj();
	}

}
void CRTMPCtx::RTMPConnCtxCloseAll()
{
	CMtxAutoLock lock( &m_mtxListCtxs ) ;

	CObj * const pItemHead = &m_listCtxs;
	CObj *pItem = NULL;
	pItem = pItemHead->m_pNextObj ; 

	while( pItem != NULL && pItem != pItemHead )
	{
		CRTMPConnContext *ctxTmp = Convert2RTMPCCtx( pItem );
		pItem = pItem->m_pNextObj ;
		ctxTmp->m_net->CloseConnection( ctxTmp );
	}
}

int CRTMPCtx::RTMPConnCtxCount()
{
	CMtxAutoLock lock( &m_mtxListCtxs ) ;
	return m_listCtxs.ListItemsCount();
}

void CRTMPCtx::CloseByRTMPT()
{
}
CRTMPConnContext *CRTMPCtx::GetConnCtx()
{
    CMtxAutoLock lock(&m_mtxListCtxs);
    if (m_listCtxs.ListIsEmpty())
    {
        return NULL;
    }


    CRTMPConnContext *ctx = Convert2RTMPCCtx(m_listCtxs.m_pNextObj);
    ctx->AddObjRef();
    return ctx;
}
BOOL CRTMPCtx::Send(CObjNetIOBuffer *pBuf)
{
	if(m_type == CRTMPCtx::ConnTypeRTMPT )
	{

		return AddHttpIoBuffer( pBuf ) ;
	}
	else
	{
		CMtxAutoLock lock( &m_mtxListCtxs ) ;
		if( m_listCtxs.ListIsEmpty() )
		{
			return FALSE ;
		}
		

		CRTMPConnContext *ctx = Convert2RTMPCCtx( m_listCtxs.m_pNextObj ) ;

		return ctx->SendIoBuffer( pBuf );
	}
}
BOOL CRTMPCtx::Sync()
{
	if( m_type != ConnTypeRTMPT )
	{
		return FALSE ;
	}
	BOOL bSync = FALSE ;
	CMtxAutoLock lock( &m_mtxListCtxs ) ;
	CObj * const pItemHead = &m_listCtxs;
	CObj *pItem = NULL;
	pItem = pItemHead->m_pNextObj ; 

	while( pItem != NULL && pItem != pItemHead )
	{
		CRTMPConnContext *ctxTmp = Convert2RTMPCCtx( pItem );
		pItem = pItem->m_pNextObj ;

		bSync = ctxTmp->SyncIoBuffer() ;
		if( bSync )
		{
			break ;
		}
	}

	return bSync ;
}
////////////////////////////////////////////////////

_CPP_UTIL_DYNAMIC_IMP(CRTMPConnContext)
_CPP_UTIL_CLASSNAME_IMP(CRTMPConnContext)
_CPP_UTIL_QUERYOBJ_IMP(CRTMPConnContext ,CObjConnContext)
CRTMPConnContext::CRTMPConnContext()
{
	m_ctxRtmp = NULL ;

	m_objEncrypt = NULL ;



	m_nHttpRequests = 0 ;

	m_tmHttpRequest = 0 ;
	m_nTimeoutHttpRequest = 10000 ;
	m_bHttpOpened = FALSE ;
	m_type = CRTMPCtx::ConnTypeUnknown ;
	m_nHttpIdleCount = 0 ;
	m_HttpDelayIndex = 0 ;
	
	MtxInit( &m_mtxhttpRequests , 0 );
	


}
CRTMPConnContext::~CRTMPConnContext()
{
	if( m_objEncrypt != NULL )
	{
		m_objEncrypt->ReleaseObj() ;
		m_objEncrypt = NULL;
	}


	if( m_ctxRtmp != NULL )
	{
		m_ctxRtmp->ReleaseObj();
		m_ctxRtmp = NULL;
	}

	
	MtxDestroy( &m_mtxhttpRequests  );
}
BOOL CRTMPConnContext::InitLater()
{
	return CObjConnContext::InitLater();
}
CRTMPCtx *CRTMPConnContext::CreateRTMPCtx()
{
	return CRTMPCtx::CreateObj();
}
static unsigned char _HttpDelayFlags[6] = { 0x01 , 0x03 , 0x05 , 0x09 , 0x11 ,0x21 };

BOOL CRTMPConnContext::SendHttpIoBuffers(CRTMPHttpIoBuffers *ioBufs)
{

	CObjNetIOBuffer *ioHeader =  SendHttpContentLength( ioBufs->m_nDataLen + 1 ) ;
	if( ioHeader == NULL )
	{
		return FALSE ;
	}
	if( (unsigned int)m_HttpDelayIndex > 5 )
	{
		m_HttpDelayIndex = 5 ;
	}
	ioHeader->m_pBuf[ ioHeader->m_nDataLen ] = _HttpDelayFlags[ m_HttpDelayIndex ] ;
	ioHeader->m_nDataLen ++ ;

	if( !SendIoBuffer ( ioHeader )  )
	{
		ioHeader->ReleaseObj();
		return FALSE ;
	}

	CObjNetIOBuffer *ioBuf = NULL ;

	while( ( ioBuf = ioBufs->RemoveHead() ) != NULL )
	{
		if( !SendIoBuffer ( ioBuf )  )
		{
			ioBuf->ReleaseObj();
			return FALSE ;
		}
	}

	ioBufs->ReleaseObj();
	return TRUE ;
}
BOOL CRTMPConnContext::SyncIoBuffer()
{
	if( m_bClosing )
	{
		return FALSE ;
	}
	if( m_type != CRTMPCtx::ConnTypeRTMPT )
	{
		return FALSE ;
	}


	MtxLock( &m_mtxhttpRequests );

	if( m_nHttpRequests < 1 )
	{
		
		MtxUnLock( &m_mtxhttpRequests );
		if( m_nTimeoutHttpRequest != -1 )
		{
			UINT32 nTmp = OSTickCount() - m_tmHttpRequest ;
			if( nTmp > m_nTimeoutHttpRequest )
			{
				m_net->CloseConnection( this );

			}
		}

		return FALSE;
		


	}
	else
	{
		m_nHttpRequests -- ;

	}

	MtxUnLock( &m_mtxhttpRequests );
	CRTMPHttpIoBuffers *ioBufs  = NULL ;

	if( m_ctxRtmp != NULL )
	{
		ioBufs = m_ctxRtmp->GetHttpIoBuffers() ;
	}
	
	if( ioBufs == NULL  )
	{
		
#ifdef _LOG_RTMP_HTTP
        LogPrint( _LOG_LEVEL_DEBUG , _TAGNAME ,"HTTP %p: Send Default\n"  , this );
#endif
		CObjNetIOBufferDefault *ioBuf = CObjNetIOBufferDefault::CreateObj();
		if( ioBuf == NULL )
		{
			m_net->CloseConnection( this );
			return FALSE ;
		}
		if( (unsigned int)m_HttpDelayIndex > 5 )
		{
			m_HttpDelayIndex = 5 ;
		}

		char szDate[80];
		ioBuf->m_nDataLen = snprintf(ioBuf->m_pBuf ,ioBuf->m_nBufLen-4, 
			"HTTP/1.1 200 OK\r\n"
			"Date: %s\r\n"
			"Connection: Keep-Alive\r\n"
			"Content-Type: application/x-fcs\r\n"
			"Content-Length: 1\r\n"
			"Server: %s\r\n"
			"\r\n"
			, HttpDate( szDate , sizeof(szDate) )
			, CRTMPCtx::RTMPServerName()
			);

		ioBuf->m_pBuf[ ioBuf->m_nDataLen ] = _HttpDelayFlags[ m_HttpDelayIndex ]  ;
		ioBuf->m_nDataLen ++ ;

		m_nHttpIdleCount ++ ;

	
		m_HttpDelayIndex = m_nHttpIdleCount / 5 ;
		
		
		if( !SendIoBuffer( ioBuf ) )
		{
			ioBuf->ReleaseObj() ;
			m_net->CloseConnection( this );
			return FALSE ;
		}

		return FALSE ;
	}


	if( ioBufs != NULL )
	{
#ifdef _LOG_RTMP_HTTP
        LogPrint( _LOG_LEVEL_DEBUG , _TAGNAME ,"HTTP %p: Send %d\n" , this , ioBufs->m_nDataLen  );
#endif
		m_nHttpIdleCount = 0 ;
		m_HttpDelayIndex = 0 ;
		if( ! SendHttpIoBuffers( ioBufs ) )
		{
			ioBufs->ReleaseObj() ;
			m_net->CloseConnection( this );
			return FALSE; 
		}
	}


	
	return TRUE ;
}

void CRTMPConnContext::IncHttpRequests()
{
	CMtxAutoLock lock( &m_mtxhttpRequests  );
	m_nHttpRequests ++ ;
}
void CRTMPConnContext::DecHttpRequests()
{
	CMtxAutoLock lock( &m_mtxhttpRequests  );
	m_nHttpRequests -- ;
}
BOOL CRTMPConnContext::Send(CObjNetIOBuffer *pBuf)
{
	if( m_bClosing || m_net == NULL )
	{
		return FALSE ;
	}

	if( m_ctxRtmp == NULL )
	{
		return SendIoBuffer( pBuf );
	}
	else
	{
		if( m_ctxRtmp->m_type == CRTMPCtx::ConnTypeRTMPT )
		{

			return m_ctxRtmp->AddHttpIoBuffer( pBuf ) ;
		}
		else
		{
			return SendIoBuffer( pBuf );
		}

	}


	
}
BOOL CRTMPConnContext::SendIoBuffer(CObjNetIOBuffer *pBuf)
{
	if( m_bClosing || m_net == NULL )
	{
		return FALSE ;
	}
	return m_net->Send(this ,pBuf );
}

BOOL CRTMPConnContext::SendBytesReceived(UINT32 bytes)
{
	if( m_bClosing )
	{
		return FALSE ;
	}

	const int headsize = 12;

	CObjNetIOBufferDefault *ioBuf = CObjNetIOBufferDefault::CreateObj();
	if( ioBuf == NULL )
	{
		return FALSE ;
	}
	BOOL bOk = FALSE ;


	int nSize = 4 ;


	memset( ioBuf->m_pBuf , 0 , headsize );
	ioBuf->m_pBuf[0] = 0x02| (  CAMFObject::HeadSize2HeadType( headsize ) << 6 )  ;
	CAMFObject::EncodeInt24( (UINT8 *)(ioBuf->m_pBuf + 4) , nSize );
	ioBuf->m_pBuf[7] = RTMPPacketType_BytesRead ;

	if( headsize == RTMP_LARGE_HEADER_SIZE )
	{
		CAMFObject::EncodeInt32LE ( (UINT8 *)(ioBuf->m_pBuf + 8)  , 0 );	
	}


	CAMFObject::EncodeInt32( (UINT8 *)(ioBuf->m_pBuf + headsize ) , bytes );

	ioBuf->m_nDataLen = headsize + nSize ;

	bOk = Send( ioBuf );

	if( !bOk )
	{
		ioBuf->ReleaseObj() ;
	}

	return bOk ;
}
BOOL CRTMPConnContext::SendWindowAck(int size) // Window Acknowledgement Size 
{
	if( m_bClosing )
	{
		return FALSE ;
	}

	CObjNetIOBufferDefault *ioBuf = CObjNetIOBufferDefault::CreateObj();
	if( ioBuf == NULL )
	{
		return FALSE ;
	}
	BOOL bOk = FALSE ;

	const int headsize = 12;

	int nSize = 4 ;


	memset( ioBuf->m_pBuf , 0 , headsize );
	ioBuf->m_pBuf[0] = 0x02| (  CAMFObject::HeadSize2HeadType( headsize ) << 6 )   ;
	CAMFObject::EncodeInt24( (UINT8 *)(ioBuf->m_pBuf + 4) , nSize );
    ioBuf->m_pBuf[7] = RTMPPacketType_WindowAck;

	if( headsize == RTMP_LARGE_HEADER_SIZE )
	{
		CAMFObject::EncodeInt32LE ( (UINT8 *)(ioBuf->m_pBuf + 8)  , 0 );	
	}

	CAMFObject::EncodeInt32( (UINT8 *)(ioBuf->m_pBuf + headsize) , size );

	ioBuf->m_nDataLen = headsize + nSize ;



	bOk = Send( ioBuf );

	if( !bOk )
	{
		ioBuf->ReleaseObj() ;
	}

	return bOk ;
}
BOOL CRTMPConnContext::SendSetPeerBandwidth(int bw1, UINT8 limitType)
{
	if( m_bClosing )
	{
		return FALSE ;
	}


	CObjNetIOBufferDefault *ioBuf = CObjNetIOBufferDefault::CreateObj();
	if( ioBuf == NULL )
	{
		return FALSE ;
	}
	BOOL bOk = FALSE ;

	const int headsize = 12;

	int nSize = 4 ;
    if (limitType != (UINT8)-1)
	{
		nSize = 5 ;
	}


	memset( ioBuf->m_pBuf , 0 , headsize );
	ioBuf->m_pBuf[0] = 0x02| (  CAMFObject::HeadSize2HeadType( headsize ) << 6 )     ;
	CAMFObject::EncodeInt24( (UINT8 *)(ioBuf->m_pBuf + 4) , nSize );
    ioBuf->m_pBuf[7] = RTMPPacketType_SetPeerBandwidth;

	if( headsize == RTMP_LARGE_HEADER_SIZE )
	{
		CAMFObject::EncodeInt32LE ( (UINT8 *)(ioBuf->m_pBuf + 8)  , 0 );	
	}


	CAMFObject::EncodeInt32( (UINT8 *)(ioBuf->m_pBuf + headsize ) , bw1 );

	if( nSize > 4 )
	{
        *((UINT8 *)(ioBuf->m_pBuf + headsize + 4)) = limitType;
	}

	ioBuf->m_nDataLen = headsize + nSize ;

	bOk = Send( ioBuf );

	if( !bOk )
	{
		ioBuf->ReleaseObj() ;
	}

	return bOk ;
}
BOOL CRTMPConnContext::SendCtrl(INT16 nType,UINT32 nObject,UINT32 nTime  )
{
	if( m_bClosing )
	{
		return FALSE ;
	}


	CObjNetIOBufferDefault *ioBuf = CObjNetIOBufferDefault::CreateObj();
	if( ioBuf == NULL )
	{
		return FALSE ;
	}
	BOOL bOk = FALSE ;

	const int headsize = 12;
	int nSize = 0 ;

	switch(nType)
	{
	case 0x03:
		nSize=10; break;	/* buffer time */
	case 0x1A:
		nSize=3; break;	/* SWF verify request */
	case 0x1B:
		nSize=44; break;	/* SWF verify response */
	default:
		nSize=6; break;
	}

	memset( ioBuf->m_pBuf , 0 , headsize );
	ioBuf->m_pBuf[0] = 0x02| (  CAMFObject::HeadSize2HeadType( headsize ) << 6 )   ;
	CAMFObject::EncodeInt24( (UINT8 *)(ioBuf->m_pBuf + 4) , nSize );
	ioBuf->m_pBuf[7] = RTMPPacketType_Ctrl ;

	if( headsize == RTMP_LARGE_HEADER_SIZE )
	{
		CAMFObject::EncodeInt32LE ( (UINT8 *)(ioBuf->m_pBuf + 8)  , 0 );	
	}


	CAMFObject::EncodeInt16( (UINT8 *)(ioBuf->m_pBuf + headsize ) , nType );

	if(  nType == 0x1B )
	{

	}
	else if(  nType == 0x1A )
	{

	}
	else
	{
		if ( nSize > 2 )
		{
			CAMFObject::EncodeInt32( (UINT8 *)(ioBuf->m_pBuf + headsize + 2  ) , nObject );
		}

		if ( nSize > 6 )
		{
			CAMFObject::EncodeInt32( (UINT8 *)(ioBuf->m_pBuf + headsize + 2 + 4 ) , nObject );
		}
	}


	
	ioBuf->m_nDataLen = headsize + nSize ;

	bOk = Send( ioBuf );

	if( !bOk )
	{
		ioBuf->ReleaseObj() ;
	}

	return bOk ;
}
BOOL CRTMPConnContext::SendChunkSize(int chunksize)
{
	if( m_bClosing || m_ctxRtmp == NULL )
	{
		return FALSE ;
	}
	CObjNetIOBufferDefault *ioBuf = CObjNetIOBufferDefault::CreateObj();
	if( ioBuf == NULL )
	{
		return FALSE ;
	}
	BOOL bOk = FALSE ;

	const int headsize = 12;

	if( chunksize == 0 )
	{
		chunksize = RTMP_DEFAULT_CHUNKSIZE ;
	}
	m_ctxRtmp->m_state.m_ChunkSizeOut = chunksize ;
	memset( ioBuf->m_pBuf , 0 , headsize );
	ioBuf->m_pBuf[0] = 0x02| (  CAMFObject::HeadSize2HeadType( headsize ) << 6 )    ;
	CAMFObject::EncodeInt24( (UINT8 *)(ioBuf->m_pBuf + 4) , 4 );
	ioBuf->m_pBuf[7] = RTMPPacketType_ChunkSize ;
	if( headsize == RTMP_LARGE_HEADER_SIZE )
	{
		CAMFObject::EncodeInt32LE ( (UINT8 *)(ioBuf->m_pBuf + 8)  , 0 );	
	}

	CAMFObject::EncodeInt32( (UINT8 *)(ioBuf->m_pBuf + headsize ) , chunksize );
	ioBuf->m_nDataLen = headsize + 4 ;

	bOk = Send( ioBuf );

	if( !bOk )
	{
		ioBuf->ReleaseObj() ;
	}

	return bOk ;
}

BOOL CRTMPConnContext::SendInvokePing(double txn)
{
    if (m_bClosing || m_ctxRtmp == NULL)
    {
        return FALSE;
    }

    CObjNetIOBufferRTMP *ioBuf = CObjNetIOBufferRTMP::CreateObj();
    if (ioBuf == NULL)
    {
        return FALSE;
    }
    BOOL bOk = FALSE;
    const int headsize = 12;
    ioBuf->m_buf.m_chunksize = m_ctxRtmp->m_state.m_ChunkSizeOut;
    ioBuf->m_buf.m_channelId = 0x03;

    ioBuf->m_buf.SetReadPos(headsize);
    CAMFObject::EncodeString(ioBuf->m_buf, "ping");
    CAMFObject::EncodeNumber(ioBuf->m_buf, txn);


    ioBuf->m_buf.SetReadPos(0);

    memset(ioBuf->m_buf.Data, 0, headsize);
    ioBuf->m_buf.Data[0] = ioBuf->m_buf.m_channelId | (CAMFObject::HeadSize2HeadType(headsize) << 6);
    CAMFObject::EncodeInt24(ioBuf->m_buf.Data + 4, ioBuf->m_buf.m_amfsize);
    ioBuf->m_buf.Data[7] = RTMPPacketType_Invoke;
    if (headsize == RTMP_LARGE_HEADER_SIZE)
    {
        CAMFObject::EncodeInt32LE((UINT8 *)(ioBuf->m_buf.Data + 8), 0);
    }


    ioBuf->EndWrite();
    bOk = Send(ioBuf);

    if (!bOk)
    {
        ioBuf->ReleaseObj();
    }

    return bOk;
}

BOOL CRTMPConnContext::SendInvokePong(double txn)
{
	if( m_bClosing || m_ctxRtmp == NULL )
	{
		return FALSE ;
	}

	CObjNetIOBufferRTMP *ioBuf = CObjNetIOBufferRTMP::CreateObj();
	if( ioBuf == NULL )
	{
		return FALSE ;
	}
	BOOL bOk = FALSE ;
	const int headsize = 12;
	ioBuf->m_buf.m_chunksize = m_ctxRtmp->m_state.m_ChunkSizeOut ;
	ioBuf->m_buf.m_channelId = 0x03 ;

	ioBuf->m_buf.SetReadPos(headsize);
	CAMFObject::EncodeString( ioBuf->m_buf , "pong");
	CAMFObject::EncodeNumber( ioBuf->m_buf , txn );


	ioBuf->m_buf.SetReadPos(0);

	memset( ioBuf->m_buf.Data , 0 , headsize );
	ioBuf->m_buf.Data[0] = ioBuf->m_buf.m_channelId | (  CAMFObject::HeadSize2HeadType( headsize ) << 6 )    ;
	CAMFObject::EncodeInt24( ioBuf->m_buf.Data + 4 , ioBuf->m_buf.m_amfsize );
	ioBuf->m_buf.Data[7] = RTMPPacketType_Invoke ;
	if( headsize == RTMP_LARGE_HEADER_SIZE )
	{
		CAMFObject::EncodeInt32LE ( (UINT8 *)(ioBuf->m_buf.Data + 8)  , 0 );	
	}


	ioBuf->EndWrite();
	bOk = Send( ioBuf );

	if( !bOk )
	{
		ioBuf->ReleaseObj() ;
	}

	return bOk ;
}
BOOL CRTMPConnContext::SendInvokeCheckBWResult(double txn)
{
	if( m_bClosing || m_ctxRtmp == NULL )
	{
		return FALSE ;
	}

	CObjNetIOBufferRTMP *ioBuf = CObjNetIOBufferRTMP::CreateObj();
	if( ioBuf == NULL )
	{
		return FALSE ;
	}
	BOOL bOk = FALSE ;
	const int headsize = 12;
	ioBuf->m_buf.m_chunksize = m_ctxRtmp->m_state.m_ChunkSizeOut ;
	ioBuf->m_buf.m_channelId = 0x03 ;

	ioBuf->m_buf.SetReadPos(headsize);
	CAMFObject::EncodeString( ioBuf->m_buf , "_result");
	CAMFObject::EncodeNumber( ioBuf->m_buf , txn );
	CAMFObject::EncodeInt8( ioBuf->m_buf , AMFTYPE_NULL );
	CAMFObject::EncodeNumber( ioBuf->m_buf , m_ctxRtmp->m_state.m_nBWCheckCounter++ );

	ioBuf->m_buf.SetReadPos(0);

	memset( ioBuf->m_buf.Data , 0 , headsize );
	ioBuf->m_buf.Data[0] = ioBuf->m_buf.m_channelId | (  CAMFObject::HeadSize2HeadType( headsize ) << 6 )  ;
	CAMFObject::EncodeInt24( ioBuf->m_buf.Data + 4 , ioBuf->m_buf.m_amfsize );
	ioBuf->m_buf.Data[7] = RTMPPacketType_Invoke ;
	if( headsize == RTMP_LARGE_HEADER_SIZE )
	{
		CAMFObject::EncodeInt32LE ( (UINT8 *)(ioBuf->m_buf.Data + 8)  , 0 );	
	}


	ioBuf->EndWrite();
	bOk = Send( ioBuf );

	if( !bOk )
	{
		ioBuf->ReleaseObj() ;
	}

	return bOk ;
}
BOOL CRTMPConnContext::SendOnFCPublish(const char *szDesc)
{
	if( m_bClosing || m_ctxRtmp == NULL )
	{
		return FALSE ;
	}

	CObjNetIOBufferRTMP *ioBuf = CObjNetIOBufferRTMP::CreateObj();
	if( ioBuf == NULL )
	{
		return FALSE ;
	}
	BOOL bOk = FALSE ;
	const int  headsize = 12 ;
	ioBuf->m_buf.m_chunksize = m_ctxRtmp->m_state.m_ChunkSizeOut ;
	ioBuf->m_buf.m_channelId = 0x03 ;

	ioBuf->m_buf.SetReadPos(headsize);
	CAMFObject::EncodeString( ioBuf->m_buf , "onFCPublish");
	CAMFObject::EncodeNumber( ioBuf->m_buf , 0.0 );
	CAMFObject::EncodeInt8( ioBuf->m_buf , AMFTYPE_NULL );


	CAMFObject::EncodeObjectBegin( ioBuf->m_buf );
	CAMFObject::EncodeNamedString( ioBuf->m_buf ,"code" , "NetStream.Publish.Start" );
	if( szDesc != NULL && szDesc[0] != 0 )
	{
		CAMFObject::EncodeNamedString( ioBuf->m_buf , "description" , szDesc );
	}
	
	CAMFObject::EncodeObjectEnd( ioBuf->m_buf  );


	ioBuf->m_buf.SetReadPos(0);

	memset( ioBuf->m_buf.Data , 0 , headsize );

	ioBuf->m_buf.Data[0] = ioBuf->m_buf.m_channelId | (  CAMFObject::HeadSize2HeadType( headsize ) << 6 )     ;
	CAMFObject::EncodeInt24( ioBuf->m_buf.Data + 4 , ioBuf->m_buf.m_amfsize );
	ioBuf->m_buf.Data[7] = RTMPPacketType_Invoke ;
	if( headsize == RTMP_LARGE_HEADER_SIZE )
	{
		CAMFObject::EncodeInt32LE ( (UINT8 *)(ioBuf->m_buf.Data + 8)  , 0 );	
	}

	ioBuf->EndWrite();

	bOk = Send( ioBuf );

	if( !bOk )
	{
		ioBuf->ReleaseObj() ;
	}

	return bOk ;
}
BOOL CRTMPConnContext::SendInvoke(const char *method, const CObjVar &value, double txn)
{
    if (m_bClosing || m_ctxRtmp == NULL || method == NULL || method[0] == 0 )
    {
        return FALSE;
    }

    CObjNetIOBufferRTMP *ioBuf = CObjNetIOBufferRTMP::CreateObj();
    if (ioBuf == NULL)
    {
        return FALSE;
    }
    BOOL bOk = FALSE;
    const int headsize = 12;
    ioBuf->m_buf.m_chunksize = m_ctxRtmp->m_state.m_ChunkSizeOut;
    ioBuf->m_buf.m_channelId = 0x03;

    ioBuf->m_buf.SetReadPos(headsize);
    CAMFObject::EncodeString(ioBuf->m_buf, method);
    CAMFObject::EncodeNumber(ioBuf->m_buf, txn);
    CAMFObject::EncodeInt8(ioBuf->m_buf, AMFTYPE_NULL);

    if (value.IsNull() || value.IsEmpty())
    {
        CAMFObject::EncodeInt8(ioBuf->m_buf, AMFTYPE_NULL);
    }
    else
    {

        if (value.m_type == CObjVar::ObjVarTypeString)
        {
            CAMFObject::EncodeString(ioBuf->m_buf, value);
        }
        else
        {
            CAMFObject::EncodeNumber(ioBuf->m_buf, value);
        }

    }


    ioBuf->m_buf.SetReadPos(0);

    memset(ioBuf->m_buf.Data, 0, headsize);
    ioBuf->m_buf.Data[0] = ioBuf->m_buf.m_channelId | (CAMFObject::HeadSize2HeadType(headsize) << 6);
    CAMFObject::EncodeInt24(ioBuf->m_buf.Data + 4, ioBuf->m_buf.m_amfsize);
    ioBuf->m_buf.Data[7] = RTMPPacketType_Invoke;
    if (headsize == RTMP_LARGE_HEADER_SIZE)
    {
        CAMFObject::EncodeInt32LE((UINT8 *)(ioBuf->m_buf.Data + 8), 0);
    }


    ioBuf->EndWrite();
    bOk = Send(ioBuf);

    if (!bOk)
    {
        ioBuf->ReleaseObj();
    }

    return bOk;
}
BOOL CRTMPConnContext::SendInvokeResultNumber(double txn , CObjVar *var)
{
	if( m_bClosing || m_ctxRtmp == NULL )
	{
		return FALSE ;
	}

	CObjNetIOBufferRTMP *ioBuf = CObjNetIOBufferRTMP::CreateObj();
	if( ioBuf == NULL )
	{
		return FALSE ;
	}
	BOOL bOk = FALSE ;
	const int headsize = 12;
	ioBuf->m_buf.m_chunksize = m_ctxRtmp->m_state.m_ChunkSizeOut ;
	ioBuf->m_buf.m_channelId = 0x03 ;

	ioBuf->m_buf.SetReadPos(headsize);
	CAMFObject::EncodeString( ioBuf->m_buf , "_result");
	CAMFObject::EncodeNumber( ioBuf->m_buf , txn );
	CAMFObject::EncodeInt8( ioBuf->m_buf , AMFTYPE_NULL );

	if( var == NULL )
	{
		CAMFObject::EncodeInt8( ioBuf->m_buf , AMFTYPE_NULL );
	}
	else
	{

		if( var->m_type == CObjVar::ObjVarTypeString )
		{
			CAMFObject::EncodeString( ioBuf->m_buf , *var );
		}
		else
		{
			CAMFObject::EncodeNumber( ioBuf->m_buf , *var );
		}
		
	}
	

	ioBuf->m_buf.SetReadPos(0);

	memset( ioBuf->m_buf.Data , 0 , headsize );
	ioBuf->m_buf.Data[0] = ioBuf->m_buf.m_channelId | (  CAMFObject::HeadSize2HeadType( headsize ) << 6 )  ;
	CAMFObject::EncodeInt24( ioBuf->m_buf.Data + 4 , ioBuf->m_buf.m_amfsize );
	ioBuf->m_buf.Data[7] = RTMPPacketType_Invoke ;
	if( headsize == RTMP_LARGE_HEADER_SIZE )
	{
		CAMFObject::EncodeInt32LE ( (UINT8 *)(ioBuf->m_buf.Data + 8)  , 0 );	
	}


	ioBuf->EndWrite();
	bOk = Send( ioBuf );

	if( !bOk )
	{
		ioBuf->ReleaseObj() ;
	}

	return bOk ;
}
BOOL CRTMPConnContext::SendUnPublishSuccess(const char * clientId, const char *szDesc)
{
    if (m_bClosing || m_ctxRtmp == NULL)
    {
        return FALSE;
    }

    CObjNetIOBufferRTMP *ioBuf = CObjNetIOBufferRTMP::CreateObj();
    if (ioBuf == NULL)
    {
        return FALSE;
    }
    BOOL bOk = FALSE;
    const int  headsize = 12;
    ioBuf->m_buf.m_chunksize = m_ctxRtmp->m_state.m_ChunkSizeOut;
    ioBuf->m_buf.m_channelId = 0x04;

    ioBuf->m_buf.SetReadPos(headsize);
    CAMFObject::EncodeString(ioBuf->m_buf, "onStatus");
    CAMFObject::EncodeNumber(ioBuf->m_buf, 0.0);
    CAMFObject::EncodeInt8(ioBuf->m_buf, AMFTYPE_NULL);


    CAMFObject::EncodeObjectBegin(ioBuf->m_buf);
    CAMFObject::EncodeNamedString(ioBuf->m_buf, "level", "status");
    CAMFObject::EncodeNamedString(ioBuf->m_buf, "code", "NetStream.Unpublish.Success");

    if (szDesc != NULL && szDesc[0] != 0)
    {
        CAMFObject::EncodeNamedString(ioBuf->m_buf, "description", szDesc);
    }

    if (clientId != NULL && clientId[0] != 0)
    {
        CAMFObject::EncodeNamedString(ioBuf->m_buf, "clientid", clientId);
    }

    

    CAMFObject::EncodeObjectEnd(ioBuf->m_buf);


    ioBuf->m_buf.SetReadPos(0);

    memset(ioBuf->m_buf.Data, 0, headsize);

    ioBuf->m_buf.Data[0] = ioBuf->m_buf.m_channelId | (CAMFObject::HeadSize2HeadType(headsize) << 6);
    CAMFObject::EncodeInt24(ioBuf->m_buf.Data + 4, ioBuf->m_buf.m_amfsize);
    ioBuf->m_buf.Data[7] = RTMPPacketType_Invoke;
    if (headsize == RTMP_LARGE_HEADER_SIZE)
    {
        CAMFObject::EncodeInt32LE((UINT8 *)(ioBuf->m_buf.Data + 8), 1);
    }

    ioBuf->EndWrite();

    bOk = Send(ioBuf);

    if (!bOk)
    {
        ioBuf->ReleaseObj();
    }

    return bOk;
}
BOOL CRTMPConnContext::SendPublishStart(const char * clientId , const char *szDesc)
{
	if( m_bClosing || m_ctxRtmp == NULL )
	{
		return FALSE ;
	}

	CObjNetIOBufferRTMP *ioBuf = CObjNetIOBufferRTMP::CreateObj();
	if( ioBuf == NULL )
	{
		return FALSE ;
	}
	BOOL bOk = FALSE ;
	const int  headsize = 12 ;
	ioBuf->m_buf.m_chunksize = m_ctxRtmp->m_state.m_ChunkSizeOut ;
	ioBuf->m_buf.m_channelId = 0x04 ;

	ioBuf->m_buf.SetReadPos(headsize);
	CAMFObject::EncodeString( ioBuf->m_buf , "onStatus");
	CAMFObject::EncodeNumber( ioBuf->m_buf , 0.0 );
	CAMFObject::EncodeInt8( ioBuf->m_buf , AMFTYPE_NULL );


	CAMFObject::EncodeObjectBegin( ioBuf->m_buf );
	CAMFObject::EncodeNamedString( ioBuf->m_buf , "level" , "status" );
	CAMFObject::EncodeNamedString( ioBuf->m_buf ,"code" , "NetStream.Publish.Start" );

	if( szDesc != NULL && szDesc[0] != 0 )
	{
		CAMFObject::EncodeNamedString( ioBuf->m_buf , "description" , szDesc );
	}
	
	CAMFObject::EncodeNamedString( ioBuf->m_buf , "clientid",clientId );
	CAMFObject::EncodeObjectEnd( ioBuf->m_buf  );


	ioBuf->m_buf.SetReadPos(0);

	memset( ioBuf->m_buf.Data , 0 , headsize );

	ioBuf->m_buf.Data[0] = ioBuf->m_buf.m_channelId | (  CAMFObject::HeadSize2HeadType( headsize ) << 6 )     ;
	CAMFObject::EncodeInt24( ioBuf->m_buf.Data + 4 , ioBuf->m_buf.m_amfsize );
	ioBuf->m_buf.Data[7] = RTMPPacketType_Invoke ;
	if( headsize == RTMP_LARGE_HEADER_SIZE )
	{
		CAMFObject::EncodeInt32LE ( (UINT8 *)(ioBuf->m_buf.Data + 8)  , 1 );	
	}

	ioBuf->EndWrite();

	bOk = Send( ioBuf );

	if( !bOk )
	{
		ioBuf->ReleaseObj() ;
	}

	return bOk ;
}
BOOL CRTMPConnContext::SendPlayNotFound()
{
	if( m_bClosing || m_ctxRtmp == NULL )
	{
		return FALSE ;
	}

	CObjNetIOBufferRTMP *ioBuf = CObjNetIOBufferRTMP::CreateObj();
	if( ioBuf == NULL )
	{
		return FALSE ;
	}
	BOOL bOk = FALSE ;
	const int  headsize = 12 ;
	ioBuf->m_buf.m_chunksize = m_ctxRtmp->m_state.m_ChunkSizeOut ;
	ioBuf->m_buf.m_channelId = 0x03 ;

	ioBuf->m_buf.SetReadPos(headsize);
	CAMFObject::EncodeString( ioBuf->m_buf , "onStatus");
	CAMFObject::EncodeNumber( ioBuf->m_buf , 0.0 );
	CAMFObject::EncodeInt8( ioBuf->m_buf , AMFTYPE_NULL );


	CAMFObject::EncodeObjectBegin( ioBuf->m_buf );
	CAMFObject::EncodeNamedString( ioBuf->m_buf ,"code" , "NetStream.Play.StreamNotFound" );
	CAMFObject::EncodeNamedString( ioBuf->m_buf , "level" , "status" );
	CAMFObject::EncodeNamedString( ioBuf->m_buf , "description" , "-" );
	CAMFObject::EncodeObjectEnd( ioBuf->m_buf  );


	ioBuf->m_buf.SetReadPos(0);

	memset( ioBuf->m_buf.Data , 0 , headsize );

	ioBuf->m_buf.Data[0] = ioBuf->m_buf.m_channelId | (  CAMFObject::HeadSize2HeadType( headsize ) << 6 )    ;
	CAMFObject::EncodeInt24( ioBuf->m_buf.Data + 4 , ioBuf->m_buf.m_amfsize );
	ioBuf->m_buf.Data[7] = RTMPPacketType_Invoke ;
	if( headsize == RTMP_LARGE_HEADER_SIZE )
	{
		CAMFObject::EncodeInt32LE ( (UINT8 *)(ioBuf->m_buf.Data + 8)  , 0 );	
	}

	ioBuf->EndWrite();

	bOk = Send( ioBuf );

	if( !bOk )
	{
		ioBuf->ReleaseObj() ;
	}

	return bOk ;
}
BOOL CRTMPConnContext::SendPlayReset(const char *details, const char *clientid)
{
	if( m_bClosing || m_ctxRtmp == NULL )
	{
		return FALSE ;
	}

	CObjNetIOBufferRTMP *ioBuf = CObjNetIOBufferRTMP::CreateObj();
	if( ioBuf == NULL )
	{
		return FALSE ;
	}
	BOOL bOk = FALSE ;
	const int  headsize = 12 ;
	ioBuf->m_buf.m_chunksize = m_ctxRtmp->m_state.m_ChunkSizeOut ;
	ioBuf->m_buf.m_channelId = m_ctxRtmp->m_nChannelOfSend;

	ioBuf->m_buf.SetReadPos(headsize);
	CAMFObject::EncodeString( ioBuf->m_buf , "onStatus");
	CAMFObject::EncodeNumber( ioBuf->m_buf , 0.0 );
	CAMFObject::EncodeInt8( ioBuf->m_buf , AMFTYPE_NULL );


	CAMFObject::EncodeObjectBegin( ioBuf->m_buf  );
	CAMFObject::EncodeNamedString( ioBuf->m_buf , "level" , "status" );
	CAMFObject::EncodeNamedString( ioBuf->m_buf ,"code" , "NetStream.Play.Reset" );
	CAMFObject::EncodeNamedString( ioBuf->m_buf , "description" , "reset..." );

	if( details != NULL && details[0] != 0 )
	{
		CAMFObject::EncodeNamedString( ioBuf->m_buf , "details" , details );
	}
	if( clientid != NULL && clientid[0] != 0 )
	{
		CAMFObject::EncodeNamedString( ioBuf->m_buf , "clientid" , clientid );
	}

	CAMFObject::EncodeObjectEnd( ioBuf->m_buf  );


	ioBuf->m_buf.SetReadPos(0);

	memset( ioBuf->m_buf.Data , 0 , headsize );

	ioBuf->m_buf.Data[0] = ioBuf->m_buf.m_channelId | (  CAMFObject::HeadSize2HeadType( headsize ) << 6 )    ;
	CAMFObject::EncodeInt24( ioBuf->m_buf.Data + 4 , ioBuf->m_buf.m_amfsize );
	ioBuf->m_buf.Data[7] = RTMPPacketType_Invoke ;
	if( headsize == RTMP_LARGE_HEADER_SIZE )
	{
		CAMFObject::EncodeInt32LE ( (UINT8 *)(ioBuf->m_buf.Data + 8)  , 1 );	
	}




	ioBuf->EndWrite();

	bOk = Send( ioBuf );

	if( !bOk )
	{
		ioBuf->ReleaseObj() ;
	}

	return bOk ;
}
BOOL CRTMPConnContext::SendPlayStart(const char *details, const char *clientid)
{
	if( m_bClosing || m_ctxRtmp == NULL )
	{
		return FALSE ;
	}

	CObjNetIOBufferRTMP *ioBuf = CObjNetIOBufferRTMP::CreateObj();
	if( ioBuf == NULL )
	{
		return FALSE ;
	}
	BOOL bOk = FALSE ;
	const int  headsize = 12 ;
	ioBuf->m_buf.m_chunksize = m_ctxRtmp->m_state.m_ChunkSizeOut ;
	ioBuf->m_buf.m_channelId = m_ctxRtmp->m_nChannelOfSend ;

	ioBuf->m_buf.SetReadPos(headsize);
	CAMFObject::EncodeString( ioBuf->m_buf , "onStatus");
	CAMFObject::EncodeNumber( ioBuf->m_buf , 0.0 );
	CAMFObject::EncodeInt8( ioBuf->m_buf , AMFTYPE_NULL );


	CAMFObject::EncodeObjectBegin( ioBuf->m_buf  );
	CAMFObject::EncodeNamedString( ioBuf->m_buf ,"code" , "NetStream.Play.Start" );
	CAMFObject::EncodeNamedString( ioBuf->m_buf , "level" , "status" );
	CAMFObject::EncodeNamedString( ioBuf->m_buf , "description" , "start..." );
	if( details != NULL && details[0] != 0 )
	{
		CAMFObject::EncodeNamedString( ioBuf->m_buf , "details" , details );
	}
	if( clientid != NULL && clientid[0] != 0 )
	{
		CAMFObject::EncodeNamedString( ioBuf->m_buf , "clientid" , clientid );
	}

	CAMFObject::EncodeObjectEnd( ioBuf->m_buf  );


	ioBuf->m_buf.SetReadPos(0);

	memset( ioBuf->m_buf.Data , 0 , headsize );

	ioBuf->m_buf.Data[0] = ioBuf->m_buf.m_channelId | (  CAMFObject::HeadSize2HeadType( headsize ) << 6 )    ;
	CAMFObject::EncodeInt24( ioBuf->m_buf.Data + 4 , ioBuf->m_buf.m_amfsize );
	ioBuf->m_buf.Data[7] = RTMPPacketType_Invoke ;
	if( headsize == RTMP_LARGE_HEADER_SIZE )
	{
		CAMFObject::EncodeInt32LE ( (UINT8 *)(ioBuf->m_buf.Data + 8)  , 1 );	
	}




	ioBuf->EndWrite();

	bOk = Send( ioBuf );

	if( !bOk )
	{
		ioBuf->ReleaseObj() ;
	}

	return bOk ;
}
BOOL CRTMPConnContext::SendHttpNotFound()
{
	BOOL bOk = FALSE ;
	CObjNetIOBufferDefault *ioBuf = CObjNetIOBufferDefault::CreateObj();
	if( ioBuf == NULL )
	{
		return FALSE ;
	}
	char szDate[80];
	ioBuf->m_nDataLen = snprintf(ioBuf->m_pBuf ,ioBuf->m_nBufLen-4, 
		"HTTP/1.1 404 Not Found\r\n"
		"Date: %s\r\n"
		"Connection: Keep-Alive\r\n"
		"Content-Type: application/x-fcs\r\n"
		"Content-Length: 1\r\n"
		"Server: %s\r\n"
		"\r\n"
		, HttpDate( szDate , sizeof(szDate) )
		, CRTMPCtx::RTMPServerName()
		);

	ioBuf->m_pBuf[ ioBuf->m_nDataLen ] = 0 ;
	ioBuf->m_nDataLen ++ ;
	bOk = Send( ioBuf );

	if( !bOk )
	{
		ioBuf->ReleaseObj() ;
	}

	return bOk ;

}
CObjNetIOBuffer * CRTMPConnContext::SendHttpContentLength(int ContentLength)
{

	CObjNetIOBufferDefault *ioBuf = CObjNetIOBufferDefault::CreateObj();
	if( ioBuf == NULL )
	{
		return NULL ;
	}
	char szDate[80];
	ioBuf->m_nDataLen = snprintf(ioBuf->m_pBuf ,ioBuf->m_nBufLen-4, 
		"HTTP/1.1 200 OK\r\n"
		"Connection: Keep-Alive\r\n"
		"Content-Type: application/x-fcs\r\n"
		"Content-Length: %d\r\n"
		"Server: %s\r\n"
		"\r\n"
		, ContentLength
		, CRTMPCtx::RTMPServerName()
		);

	ioBuf->m_pBuf[ ioBuf->m_nDataLen ] = 0 ;
	return ioBuf ;

}
BOOL CRTMPConnContext::SendRtmpSampleAccess()
{
	if( m_bClosing || m_ctxRtmp == NULL )
	{
		return FALSE ;
	}

	CObjNetIOBufferRTMP *ioBuf = CObjNetIOBufferRTMP::CreateObj();
	if( ioBuf == NULL  )
	{
		return FALSE ;
	}
	BOOL bOk = FALSE ;
	const int  headsize = 12 ;
	ioBuf->m_buf.m_chunksize = m_ctxRtmp->m_state.m_ChunkSizeOut ;
	ioBuf->m_buf.m_channelId = m_ctxRtmp->m_nChannelOfSend ;

	ioBuf->m_buf.SetReadPos(headsize);
	CAMFObject::EncodeString( ioBuf->m_buf , "|RtmpSampleAccess");
	CAMFObject::EncodeBoolean( ioBuf->m_buf , 0 );
	CAMFObject::EncodeBoolean( ioBuf->m_buf , 0 );
	//CAMFObject::EncodeNumber( ioBuf->m_buf , 0.0 );
	//CAMFObject::EncodeInt8( ioBuf->m_buf , AMFTYPE_NULL );


	ioBuf->m_buf.SetReadPos(0);

	memset( ioBuf->m_buf.Data , 0 , headsize );

	ioBuf->m_buf.Data[0] = ioBuf->m_buf.m_channelId | (  CAMFObject::HeadSize2HeadType( headsize ) << 6 )    ;
	CAMFObject::EncodeInt24( ioBuf->m_buf.Data + 4 , ioBuf->m_buf.m_amfsize );
	ioBuf->m_buf.Data[7] = RTMPPacketType_Script ;
	if( headsize == RTMP_LARGE_HEADER_SIZE )
	{
		CAMFObject::EncodeInt32LE ( (UINT8 *)(ioBuf->m_buf.Data + 8)  , 1 );	
	}




	ioBuf->EndWrite();

	bOk = Send( ioBuf );

	if( !bOk )
	{
		ioBuf->ReleaseObj() ;
	}

	return bOk ;
}
BOOL CRTMPConnContext::SendInvokeCallFailed(double txn , const char * szDescription)
{
	if( m_bClosing || m_ctxRtmp == NULL )
	{
		return FALSE ;
	}

	CObjNetIOBufferRTMP *ioBuf = CObjNetIOBufferRTMP::CreateObj();
	if( ioBuf == NULL )
	{
		return FALSE ;
	}
	BOOL bOk = FALSE ;

	const int  headsize = 12 ;



	ioBuf->m_buf.m_chunksize = m_ctxRtmp->m_state.m_ChunkSizeOut ;
	ioBuf->m_buf.m_channelId = 0x03 ;

	ioBuf->m_buf.SetReadPos(headsize);
	CAMFObject::EncodeString( ioBuf->m_buf , "_error");
	CAMFObject::EncodeNumber( ioBuf->m_buf , txn );
	CAMFObject::EncodeInt8( ioBuf->m_buf , AMFTYPE_NULL  );



	CAMFObject::EncodeObjectBegin( ioBuf->m_buf  );
	CAMFObject::EncodeNamedString( ioBuf->m_buf ,"level" , "error" );
	CAMFObject::EncodeNamedString( ioBuf->m_buf ,"code" , "NetConnection.Call.Failed" );

	if( szDescription != NULL && szDescription[0] != 0 )
	{
		CAMFObject::EncodeNamedString( ioBuf->m_buf ,"description" , szDescription );
	}
	
	CAMFObject::EncodeObjectEnd( ioBuf->m_buf  );

	ioBuf->m_buf.SetReadPos(0);

	memset( ioBuf->m_buf.Data , 0 , headsize );

	ioBuf->m_buf.Data[0] = ioBuf->m_buf.m_channelId | (  CAMFObject::HeadSize2HeadType( headsize ) << 6 )    ;
	CAMFObject::EncodeInt24( ioBuf->m_buf.Data + 4 , ioBuf->m_buf.m_amfsize );
	ioBuf->m_buf.Data[7] = RTMPPacketType_Invoke ;
	if( headsize == RTMP_LARGE_HEADER_SIZE )
	{
		CAMFObject::EncodeInt32LE ( (UINT8 *)(ioBuf->m_buf.Data + 8)  , 0 );	
	}


	ioBuf->EndWrite();

	bOk = Send( ioBuf );

	if( !bOk )
	{
		ioBuf->ReleaseObj() ;
	}

	return bOk ;
}
BOOL CRTMPConnContext::SendOnBWDone(double txn)
{
	if( m_bClosing  || m_ctxRtmp == NULL )
	{
		return FALSE ;
	}

	CObjNetIOBufferRTMP *ioBuf = CObjNetIOBufferRTMP::CreateObj();
	if( ioBuf == NULL )
	{
		return FALSE ;
	}
	BOOL bOk = FALSE ;
	const int  headsize = 8 ;
	ioBuf->m_buf.m_chunksize = m_ctxRtmp->m_state.m_ChunkSizeOut ;
	ioBuf->m_buf.m_channelId = 0x03 ;

	ioBuf->m_buf.SetReadPos(headsize);
	CAMFObject::EncodeString( ioBuf->m_buf , "onBWDone");
	CAMFObject::EncodeNumber( ioBuf->m_buf , 0.0 );
	CAMFObject::EncodeInt8( ioBuf->m_buf , AMFTYPE_NULL );
	CAMFObject::EncodeNumber( ioBuf->m_buf , txn );




	ioBuf->m_buf.SetReadPos(0);

	memset( ioBuf->m_buf.Data , 0 , headsize );

	ioBuf->m_buf.Data[0] = ioBuf->m_buf.m_channelId | (  CAMFObject::HeadSize2HeadType( headsize ) << 6 )   ;
	CAMFObject::EncodeInt24( ioBuf->m_buf.Data + 4 , ioBuf->m_buf.m_amfsize );
	ioBuf->m_buf.Data[7] = RTMPPacketType_Invoke ;


	ioBuf->EndWrite();

	bOk = Send( ioBuf );

	if( !bOk )
	{
		ioBuf->ReleaseObj() ;
	}

	return bOk ;
}
CObjNetIOBufferRTMP *CRTMPConnContext::CreateRTMPIoBuffer()
{
    CObjNetIOBufferRTMP *ioBuf = CObjNetIOBufferRTMP::CreateObj();
    if( ioBuf == NULL )
    {
        return NULL ;
    }
    ioBuf->m_buf.m_chunksize = m_ctxRtmp->m_state.m_ChunkSizeOut ;
    ioBuf->m_buf.m_channelId = 0x03 ;
    return ioBuf ;
}

BOOL CRTMPConnContext::SendInvokeConnectResult(double txn)
{
	if( m_bClosing || m_ctxRtmp == NULL )
	{
		return FALSE ;
	}

	CObjNetIOBufferRTMP *ioBuf = CObjNetIOBufferRTMP::CreateObj();
	if( ioBuf == NULL )
	{
		return FALSE ;
	}
	BOOL bOk = FALSE ;
	const int  headsize = 12 ;
	ioBuf->m_buf.m_chunksize = m_ctxRtmp->m_state.m_ChunkSizeOut ;
	ioBuf->m_buf.m_channelId = 0x03 ;

	ioBuf->m_buf.SetReadPos(headsize);
	CAMFObject::EncodeString( ioBuf->m_buf , "_result");
	CAMFObject::EncodeNumber( ioBuf->m_buf , txn );


	CAMFObject::EncodeObjectBegin( ioBuf->m_buf  );
	CAMFObject::EncodeNamedString( ioBuf->m_buf ,"fmsVer" , "FMS/4,5,0,297" );
	CAMFObject::EncodeNamedNumber( ioBuf->m_buf , "capabilities" , 255 );
    CAMFObject::EncodeNamedNumber(ioBuf->m_buf, "mode", 1.0);
	//CAMFObject::EncodeNamedString( ioBuf->m_buf ,"server" , CRTMPCtx::RTMPServerName());
	

	CAMFObject::EncodeObjectEnd( ioBuf->m_buf  );
	

	CAMFObject::EncodeObjectBegin( ioBuf->m_buf  );
	CAMFObject::EncodeNamedString( ioBuf->m_buf ,"level" , "status" );
	CAMFObject::EncodeNamedString( ioBuf->m_buf ,"code" , "NetConnection.Connect.Success" );
	CAMFObject::EncodeNamedString( ioBuf->m_buf ,"description" , "Connection succeeded." );
	CAMFObject::EncodeNamedNumber( ioBuf->m_buf ,"objectEncoding" , m_ctxRtmp->m_objectEncoding );

	
	CAMFObjectProperty propVer;
	propVer.m_name = "version";
	propVer.m_value = "4,5,0,297";
	CAMFObject obj;
	obj.AddProp( &propVer );
	CAMFObjectProperty prop;
	prop.m_name = "data" ;
	prop.m_value = & obj ;
	prop.Encode( ioBuf->m_buf );

	
	CAMFObject::EncodeObjectEnd( ioBuf->m_buf  );

	ioBuf->m_buf.SetReadPos(0);

    memset(ioBuf->m_buf.Data, 0, headsize);
	ioBuf->m_buf.Data[0] = ioBuf->m_buf.m_channelId | (  CAMFObject::HeadSize2HeadType( headsize ) << 6 )   ;
	CAMFObject::EncodeInt24( ioBuf->m_buf.Data + 4 , ioBuf->m_buf.m_amfsize );
	ioBuf->m_buf.Data[7] = RTMPPacketType_Invoke ;
	

	if (headsize == RTMP_LARGE_HEADER_SIZE)
	{
		CAMFObject::EncodeInt32LE((UINT8 *)(ioBuf->m_buf.Data + 8), 0);
	}

	ioBuf->EndWrite();

	bOk = Send( ioBuf );
	
	if( !bOk )
	{
		ioBuf->ReleaseObj() ;
	}
	
	return bOk ;
}
////////////////////////////////////////////////////////////////////////////////////
//CRTMPHeader _header;
static BOOL processInitialHeader(CRTMPConnContext *pClient  , BOOL &bError)
{
	if( pClient->m_ctxRtmp == NULL )
	{
		return FALSE ;
	}

	CObjBuffer *buf = &( pClient->m_ctxRtmp->m_buf );
	CRTMPState *state = & ( pClient->m_ctxRtmp->m_state );
	if ( buf->DataLength() < 1) 
	{
		return FALSE;
	}
	state->header.Clear();
	UINT8 tmp = buf->Data[buf->ReadPos];
	state->header.HeadType = ( tmp & 0xc0 ) >> 6;
	state->header.ChannelId = ( tmp & 0x3f );
	state->header.HeadSize = _RtmpPacketSize[ state->header.HeadType ] ;
	

	buf->ReadPos ++ ;

	ObjBufferPack( *buf );




	if( state->header.ChannelId == 0 )
	{
		state->header.ExtHeadSize = 1 ;
		state->state = RTMP_STATE_ExtHeader ;

	}
	else if( state->header.ChannelId == 1 )
	{
		state->header.ExtHeadSize  = 2 ;
		state->state = RTMP_STATE_ExtHeader ;

	}
	else
	{
		state->header.ExtHeadSize  = 0 ;
		state->state = RTMP_STATE_PartHeader ;

		if( state->header.HeadSize == 1 )
		{


			if( state->header.ChannelId  < _RTMP_MAX_CHANNELS )
			{
				if( state->prevHeaders[state->header.ChannelId].IsValid() )
				{
					state->header = state->prevHeaders[state->header.ChannelId] ;;
					state->state = RTMP_STATE_AmfData ;


				}
				else
				{
					LogPrint(_LOG_LEVEL_ERROR,LOG_TAG ,_szLogFormatParseError ,  __FUNCTION__ , __LINE__ );
				}

			}

			/*
			if( pClient->m_state.m_PktIoBuf != NULL )
			{
				
				if( pClient->m_state.header.ChannelId  == pClient->m_state.m_PktHeader.ChannelId )
				{
					pClient->m_state.state = RTMP_STATE_AmfData ;
				}
				else
				{
					LogPrint(_LOG_LEVEL_ERROR,LOG_TAG ,_szLogFormatParseError ,  __FUNCTION__ , __LINE__ );
				}
				
			}
			else
			{
				if( pClient->m_state.header.ChannelId  < _RTMP_MAX_CHANNELS )
				{
					if( pClient->m_state.prevHeaders[pClient->m_state.header.ChannelId].IsValid() )
					{
						pClient->m_state.header = pClient->m_state.prevHeaders[pClient->m_state.header.ChannelId] ;
						pClient->m_state.m_PktHeader = pClient->m_state.header ;
						pClient->m_state.state = RTMP_STATE_AmfData ;
					}
					else
					{
						LogPrint(_LOG_LEVEL_ERROR,LOG_TAG ,_szLogFormatParseError ,  __FUNCTION__ , __LINE__ );
					}

				}

			}
			*/
		}

	}


	return TRUE ;
}
static BOOL processExtHeader(CRTMPConnContext *pClient  , BOOL &bError)
{
	if( pClient->m_ctxRtmp == NULL )
	{
		return FALSE ;
	}

	CObjBuffer *buf = &( pClient->m_ctxRtmp->m_buf );
	CRTMPState *state = & ( pClient->m_ctxRtmp->m_state );

	if ( buf->DataLength() <  state->header.ExtHeadSize ) 
	{
		return FALSE;
	}
	UINT8 *header = buf->Data + buf->ReadPos ;
	if( state->header.ExtHeadSize == 1 )
	{
		state->header.ChannelId = header[0] ;
		state->header.ChannelId += 64 ;
	}
	else if( state->header.ExtHeadSize == 2 )
	{
		int tmp = 0 ;
		tmp = ( header[1] << 8) + header[0];
		state->header.ChannelId = tmp + 64 ;
	}
	buf->ReadPos += state->header.ExtHeadSize ;

	state->state = RTMP_STATE_PartHeader ;

	ObjBufferPack( *buf );


	return TRUE ;
}
static BOOL processRemainingHeader(CRTMPConnContext *pClient  , BOOL &bError)
{

	if( pClient->m_ctxRtmp == NULL )
	{
		return FALSE ;
	}

	CObjBuffer *buf = &( pClient->m_ctxRtmp->m_buf );
	CRTMPState *state = & ( pClient->m_ctxRtmp->m_state );

	int nHeadRemainSize = state->header.HeadSize - 1 ;

	if ( buf->DataLength() <  nHeadRemainSize  ) 
	{
		return FALSE;
	}
	//UINT32  PacketSize = 0 ;
	UINT8 *header = buf->Data + buf->ReadPos ;
	int nSize = nHeadRemainSize ;
	
	

	if( state->header.HeadSize < RTMP_LARGE_HEADER_SIZE )
	{
		if( state->header.ChannelId  < _RTMP_MAX_CHANNELS )
		{
			if( state->prevHeaders[state->header.ChannelId].IsValid() )
			{
				state->header = state->prevHeaders[state->header.ChannelId] ;
				//PacketSize = pClient->m_state.header.PacketSize ;
			}
		}
	}
	

	if (nSize >= 3)
	{
		state->header.Timmer = AMF_DecodeInt24(header);
        

		/*RTMP_Log(RTMP_LOGDEBUG, "%s, reading RTMP packet chunk on channel %x, headersz %i, timestamp %i, abs timestamp %i", __FUNCTION__, packet.m_nChannel, nSize, packet.m_nTimeStamp, packet.m_hasAbsTimestamp); */

		if (nSize >= 6)
		{

			state->header.PacketSize = AMF_DecodeInt24(header + 3);
			//packet->m_nBytesRead = 0;
			//RTMPPacket_Free(packet);

			if (nSize > 6)
			{
				state->header.PacketType = (RTMPPacketType)header[6];

				if (nSize == 11)
				{
					state->header.StreamId = AMF_DecodeInt32LE(header + 7);
					//packet->m_nInfoField2 = DecodeInt32LE(header + 7);
				}
			}
		}
		if ( state->header.Timmer == 0xffffff)
		{
			state->state = RTMP_STATE_ExtTimmer ;
			buf->ReadPos +=  nHeadRemainSize  ;
			return TRUE ;
			/*
			if (ReadN(r, header + nSize, 4) != 4)
			{
				RTMP_Log(RTMP_LOGERROR, "%s, failed to read extended timestamp",
					__FUNCTION__);
				return FALSE;
			}
			packet->m_nTimeStamp = AMF_DecodeInt32(header + nSize);
			hSize += 4;
			*/
		}
        else
        {
            if( nSize == 11 )
            {
                state->header.Timmer_Type = CRTMPHeader::TimmerTypeABS;
            }
            else
            {
                state->header.Timmer_Type = CRTMPHeader::TimmerTypeRelative;
            }
            
        }
	}

	buf->ReadPos +=  nHeadRemainSize  ;
	ObjBufferPack( *buf );

	state->state = RTMP_STATE_AmfData ;

	/*
	if( PacketSize > 0  )
	{
		if( pClient->m_state.m_PktIoBuf != NULL )
		{
			LogPrint(_LOG_LEVEL_ERROR, LOG_TAG ,_szLogFormatParseError ,  __FUNCTION__ , __LINE__ );
		}
		pClient->m_state.m_PktHeader = pClient->m_state.header ;
	}
	else
	{

		pClient->m_state.state = RTMP_STATE_NewPacket ;
	}

	*/

	if( state->header.ChannelId  < _RTMP_MAX_CHANNELS )
	{

		state->prevHeaders[state->header.ChannelId] =  state->header ;
		
	}


	return TRUE ;
}
static BOOL processExtTimmer(CRTMPConnContext *pClient  , BOOL &bError)
{
	if( pClient->m_ctxRtmp == NULL )
	{
		return FALSE ;
	}

	CObjBuffer *buf = &( pClient->m_ctxRtmp->m_buf );
	CRTMPState *state = & ( pClient->m_ctxRtmp->m_state );

	if ( buf->DataLength() < 4 ) 
	{
		return FALSE;
	}
	state->header.Timmer = AMF_DecodeInt32( buf->Data + buf->ReadPos );
    state->header.Timmer_Type = CRTMPHeader::TimmerTypeExt;
	buf->ReadPos += 4 ;
	ObjBufferPack( *buf );
	state->state = RTMP_STATE_AmfData ;

	if( state->header.ChannelId  < _RTMP_MAX_CHANNELS )
	{

		state->prevHeaders[state->header.ChannelId] =  state->header ;

	}

	return TRUE ;
}
static BOOL processData(CRTMPConnContext *pClient  , BOOL &bError)
{
	if( pClient->m_ctxRtmp == NULL )
	{
		return FALSE ;
	}

	CObjBuffer *buf = &( pClient->m_ctxRtmp->m_buf );
	CRTMPState *state = & ( pClient->m_ctxRtmp->m_state );

	int nRet = 0 ;
	int nRequiredLen = 0 ;

	if( state->header.PacketSize == 0 )
	{
		ObjBufferPack( *buf );
		state->state = RTMP_STATE_NewPacket ;
		return TRUE ;
	}

	if( state->header.ChannelId  >= _RTMP_MAX_CHANNELS )
	{
		LogPrint(_LOG_LEVEL_ERROR, LOG_TAG ,"%s:%d,ChannelId(%d) >= %d\n" ,  __FUNCTION__ , __LINE__  ,state->header.ChannelId , _RTMP_MAX_CHANNELS );
		bError = TRUE ;
		return FALSE ;
	}

	if( state->m_PktIoBufs[state->header.ChannelId ] == NULL )
	{
		CObjNetIOBuffer *ioBufTmp = ((CRTMPNetAsync  *)pClient->m_net)->CreateRTMPIoBuffer(state->header.PacketSize + 4);
		if (ioBufTmp == NULL)
		{
			bError = TRUE;
			LogPrint(_LOG_LEVEL_ERROR, LOG_TAG, "%s:%d,CreateRTMPIoBuffer failed\n", __FUNCTION__, __LINE__);
			return FALSE;
		}
		ioBufTmp->m_nLenTrans = state->header.PacketSize;

		state->m_PktIoBufs[state->header.ChannelId] = ioBufTmp;
	}
	CObjNetIOBuffer *ioBuf = state->m_PktIoBufs[ state->header.ChannelId ] ;

	if( ioBuf == NULL )
	{
		bError = TRUE ;
		LogPrint(_LOG_LEVEL_ERROR, LOG_TAG, "%s:%d,state->m_PktIoBufs[state->header.ChannelId]==NULL\n", __FUNCTION__, __LINE__);
		return FALSE ;
	}

	nRequiredLen = MinInt(state->m_ChunkSizeIn, ioBuf->m_nLenTrans - ioBuf->m_nDataLen);

	if ( nRequiredLen < 0)
	{
		LogPrint(_LOG_LEVEL_ERROR, LOG_TAG, "%s:%d,data error,m_ChunkSizeIn=%d,state->header.PacketSize=%d,ioBuf->m_nDataLen=%d\n", __FUNCTION__, __LINE__,
			state->m_ChunkSizeIn, ioBuf->m_nLenTrans, ioBuf->m_nDataLen);
		bError = TRUE;
		return FALSE;
	}

	if ( buf->DataLength() < nRequiredLen ) 
	{
		return FALSE;
	}
	UINT8 *body = buf->Data + buf->ReadPos ;

	memcpy( ioBuf->m_pBuf + ioBuf->m_nDataLen , body , nRequiredLen );
	ioBuf->m_nDataLen += nRequiredLen ;
	buf->ReadPos += nRequiredLen ;

	state->state = RTMP_STATE_NewPacket ;

	if (ioBuf->m_nDataLen >= ioBuf->m_nLenTrans )
	{
		state->m_PktIoBufs[ state->header.ChannelId ] = NULL ;
		if( RTMPPacketType_ChunkSize == state->header.PacketType )
		{
			state->m_ChunkSizeIn = AMF_DecodeInt32( (UINT8*) (ioBuf->m_pBuf) );
			if( state->m_ChunkSizeIn <= 0 )
			{
				state->m_ChunkSizeIn  = RTMP_DEFAULT_CHUNKSIZE ;
			}
		}

		ioBuf->m_nLenTrans = 0;
		nRet = (( CRTMPNetAsync  * ) pClient->m_net )->OnRTMPReadCompleted( pClient , &(state->header) ,  ioBuf ) ;
		if( nRet == -1 )
		{
			LogPrint(_LOG_LEVEL_ERROR, LOG_TAG, "%s:%d,OnRTMPReadCompleted failed\n", __FUNCTION__, __LINE__);
			bError = TRUE ;
		}


		
		


		state->header.Clear();
		ioBuf->ReleaseObj();

	}


	/*
	int nRequiredLen = pClient->m_state.header.PacketSize + ((pClient->m_state.header.PacketSize - 1)/pClient->m_state.m_ChunkSizeIn );
	if ( pClient->m_buf.DataLength() < nRequiredLen ) 
	{
		return FALSE;
	}
	UINT8 *body = pClient->m_buf.Data + pClient->m_buf.ReadPos ;

	CObjNetIOBuffer *ioBuf = (( CRTMPNetAsync  * ) pClient->m_net )->CreateRTMPIoBuffer( nRequiredLen + 4 );
	if( ioBuf == NULL )
	{
		bError = TRUE ;
		return FALSE ;
	}

	if( RTMPPacketType_ChunkSize == pClient->m_state.header.PacketType )
	{
		pClient->m_state.m_ChunkSizeIn = AMF_DecodeInt32( (UINT8*) (body) );
		if( pClient->m_state.m_ChunkSizeIn <=  0 )
		{
			pClient->m_state.m_ChunkSizeIn  = RTMP_DEFAULT_CHUNKSIZE ;
		}
	}


	if( nRequiredLen <= pClient->m_state.m_ChunkSizeIn )
	{
		memcpy( ioBuf->m_pBuf , body , nRequiredLen );
		ioBuf->m_nDataLen = nRequiredLen ;

	}
	else
	{
		CObjBuffer buf;
		int nCopyed = 0 ;
		int nTotalCopyed  = 0 ;
		int nTotalCopyedBuf = 0 ;
		buf.SafeTail = RTMP_SAFE_TAIL ;
		while ( nTotalCopyed < nRequiredLen )
		{
			nCopyed  = MinInt( (nRequiredLen-nTotalCopyed) , pClient->m_state.m_ChunkSizeIn );
			memcpy( ioBuf->m_pBuf + nTotalCopyedBuf , body+ nTotalCopyed , nCopyed );
			nTotalCopyed += nCopyed ;
			nTotalCopyedBuf += nCopyed ;

			if(  nCopyed == pClient->m_state.m_ChunkSizeIn  && (nRequiredLen-nTotalCopyed) > 0 )
			{
				nTotalCopyed ++ ;
			}
		}
		ioBuf->m_nDataLen = pClient->m_state.header.PacketSize ;


	}


	nRet = (( CRTMPNetAsync  * ) pClient->m_net )->OnRTMPReadCompleted( pClient , &(pClient->m_state.header) ,  ioBuf ) ;
	if( nRet == -1 )
	{
		bError = TRUE ;
	}


	ioBuf->ReleaseObj();
	*/

	/*
	if( pClient->m_state.header.PacketType == RTMPPacketType_Invoke )
	{
		if( AMFTYPE_STRING != body[ 0 ] )
		{
			pClient->m_buf.ReadPos += nRequiredLen ;
			pClient->m_buf.Pack( 0 );
			pClient->m_state.state = RTMP_STATE_NewPacket ;
			return TRUE ;
		}
		CAMFObject *obj = CAMFObject::CreateObj();
		if( obj == NULL )
		{
			bError = TRUE ;
			return FALSE ;
		}
		if( nRequiredLen <= pClient->m_state.m_ChunkSizeIn )
		{
			AMF_Decode( obj , body , nRequiredLen , 0 );

		}
		else
		{
			CObjBuffer buf;
			int nCopyed = 0 ;
			int nTotalCopyed  = 0 ;
			while ( nTotalCopyed < nRequiredLen )
			{
				nCopyed  = MinInt( (nRequiredLen-nTotalCopyed) , pClient->m_state.m_ChunkSizeIn );
				buf.Write( body + nTotalCopyed , nCopyed , _ALIGN_ );
				nTotalCopyed += nCopyed ;

				if(  nCopyed == pClient->m_state.m_ChunkSizeIn  && (nRequiredLen-nTotalCopyed) > 0 )
				{
					nTotalCopyed ++ ;
				}
			}
			//FileOverWriteX("d:/rtmp1.dat", buf.Data , buf.DataLength());

			AMF_Decode( obj , buf.Data , pClient->m_state.header.PacketSize , 0 );



		}

		obj->ReleaseObj();
	}
	*/


	if( bError )
	{
		return FALSE ;
	}

	
	ObjBufferPack( *buf );
	

	return TRUE ;
}

static BOOL processRecvHandshake0(CRTMPConnContext *pClient  , BOOL &bError)
{
	if( pClient->m_ctxRtmp == NULL )
	{
		return FALSE ;
	}

	CObjBuffer *buf = &( pClient->m_ctxRtmp->m_buf );
	CRTMPState *state = & ( pClient->m_ctxRtmp->m_state );

	if ( buf->DataLength() < (RTMP_SIG_SIZE + 1) ) 
	{
		return FALSE;
	}
	int nRet = 0 ;
	UINT8 *data = buf->Data + buf->ReadPos ;
	buf->ReadPos += RTMP_SIG_SIZE + 1 ;

	nRet = (( CRTMPNetAsync  * ) pClient->m_net )->OnReceivedHandshake1( pClient , data  , (RTMP_SIG_SIZE + 1)   );

	if( nRet == -1 )
	{
		bError = TRUE ;
		LogPrint(_LOG_LEVEL_ERROR, LOG_TAG, "%s:%d,OnReceivedHandshake1 failed\n", __FUNCTION__, __LINE__);
		return FALSE ;
	}

	
	ObjBufferPack( *buf );
	state->state = RTMP_STATE_RecvHandeshake1 ;

	return TRUE ;



}
static BOOL processRecvHandshake1(CRTMPConnContext *pClient  , BOOL &bError)
{
	if( pClient->m_ctxRtmp == NULL )
	{
		return FALSE ;
	}

	CObjBuffer *buf = &( pClient->m_ctxRtmp->m_buf );
	CRTMPState *state = & ( pClient->m_ctxRtmp->m_state );


	if ( buf->DataLength() < (RTMP_SIG_SIZE ) ) 
	{
		return FALSE;
	}


	UINT8 *data = buf->Data + buf->ReadPos ;
	int nRet = 0 ;


	buf->ReadPos += RTMP_SIG_SIZE  ;
	

	nRet = (( CRTMPNetAsync  * ) pClient->m_net )->OnReceivedHandshake2( pClient , data , RTMP_SIG_SIZE   );

	if( nRet == -1 )
	{
		bError = TRUE ;
		LogPrint(_LOG_LEVEL_ERROR, LOG_TAG, "%s:%d,OnReceivedHandshake2 failed\n", __FUNCTION__, __LINE__);
		return FALSE ;

		
	}


	ObjBufferPack( *buf );
	state->m_handshake.m_bHandShaked = TRUE ;
	state->state = RTMP_STATE_NewPacket ;
	return TRUE ;



}
static void processInputBuffer(CRTMPConnContext *pClient , BOOL &bError) 
{
	if( pClient->m_ctxRtmp == NULL )
	{
		return  ;
	}

	//CObjBuffer *buf = &( pClient->m_ctxRtmp->m_buf );
	CRTMPState *state = & ( pClient->m_ctxRtmp->m_state );

	if( ! state->m_handshake.m_bHandShaked )
	{
		while ( !bError && ! pClient->m_bClosing ) 
		{

			if ( state->state == RTMP_STATE_RecvHandeshake0 ) 
			{
				if ( !processRecvHandshake0(pClient  , bError) ) 
				{
					break;
				}
				if( bError )
				{
					break ;
				}
				// If a close frame has been received, no further data should
				// have seen

			}

			if ( state->state == RTMP_STATE_RecvHandeshake1 ) 
			{
				if ( !processRecvHandshake1(pClient  , bError) ) 
				{
					break;
				}
				if( bError )
				{
					break ;
				}
				// If a close frame has been received, no further data should
				// have seen
				goto LABEL_RTMP_STATE_NewPacket ;

			}
		}
	}
	else
	{
LABEL_RTMP_STATE_NewPacket : ;
		while ( !bError && ! pClient->m_bClosing ) 
		{

			if ( state->state == RTMP_STATE_NewPacket) 
			{
				if ( !processInitialHeader(pClient  , bError) ) 
				{
					break;
				}
				if( bError )
				{
					break ;
				}
				// If a close frame has been received, no further data should
				// have seen

			}
			if ( state->state == RTMP_STATE_ExtHeader ) 
			{
				if ( !processExtHeader(pClient  , bError) ) 
				{
					break;
				}
				if( bError )
				{
					break ;
				}
			}

			
			if ( state->state == RTMP_STATE_PartHeader ) 
			{
				if ( !processRemainingHeader(pClient  , bError) ) 
				{
					break;
				}
				if( bError )
				{
					break ;
				}
			}
			if ( state->state == RTMP_STATE_ExtTimmer ) 
			{
				if ( !processExtTimmer(pClient  , bError) ) 
				{
					break;
				}
				if( bError )
				{
					break ;
				}
			}
			
			if ( state->state == RTMP_STATE_AmfData ) 
			{
				if ( !processData(pClient  , bError) ) 
				{
					break;
				}
			}
		}

	}
}

/////////////////////////////////////////////////////////////////////////////////////
static UINT8 _serversig[RTMP_SIG_SIZE] ;



CRTMPNetAsync::CRTMPNetAsync(int nWaitTimeoutSeconds  ,
						   int nStackSize , int nMaxConnections  ,
                           NetAsyncType netType) :
                           CObjNetAsync(nWaitTimeoutSeconds, nStackSize, nMaxConnections, netType)
{
	static BOOL  _bInitHandshake = FALSE ;
	if( !_bInitHandshake )
	{
		_bInitHandshake = TRUE ;
		int i = 0 ;
		UINT32 upTime = htonl((UINT32)OSTickCount());
		memcpy(_serversig, &upTime, 4);
		memset(&_serversig[4], 0, 4);
		for (i = 8; i < RTMP_SIG_SIZE; i++)
		{
			srand( upTime + i );
			_serversig[i] = (UINT8)(rand() % 256);
		}
	}

	m_Protocol = RTMPProtocolRTMP ;
	m_Protocol |= RTMPProtocolRTMPT ;


}
CRTMPNetAsync::~CRTMPNetAsync()
{

}

int CRTMPNetAsync::OnReadCompleted(CObjConnContext *pContext, CObjNetIOBuffer *pBuffer)
{
	CRTMPConnContext *pClient = _CPP_UTIL_DYNCAST(pContext,CRTMPConnContext);
	int i = -1 ;
	int iEnd = 0 ;
	char szTmp[200];
	CObjNetIOBuffer *io = NULL ;
	BOOL bSync = FALSE ;
	BOOL bError = FALSE ;
	BOOL bHttpBuf = FALSE ;
	if( pClient == NULL )
	{
		CloseConnection(pContext);
		printf("----if you use CRTMPNetAsync,make sure\n");
		printf("----AllocConnContext return CRTMPConnContext!\n");
		printf("----define your context inherit from CRTMPConnContext!\n");
		return 0;
	}

	if (pBuffer == NULL || pClient->m_bClosing)
	{
		return -1;
	}
	if( pBuffer->m_nDataLen < pBuffer->m_nBufLen )
	{
		pBuffer->m_pBuf[ pBuffer->m_nDataLen]  = 0 ;
	}


	CObjBuffer *objBuf = NULL ;



	if( pClient->m_type == CRTMPCtx::ConnTypeRTMP  )
	{
		if( pClient->m_ctxRtmp == NULL )
		{
			LogPrint(_LOG_LEVEL_ERROR , _TAGNAME ,"%s %d:conn is RTMP but m_ctxRtmp is NULL\n" , __FUNCTION__ , __LINE__  );
			return -1 ;
		}
		objBuf = & ( pClient->m_ctxRtmp->m_buf ) ;
		
	}
	else
	{
		objBuf = & ( pClient->m_bufHttp ) ;
		bHttpBuf = TRUE ;
	}

	if( pClient->m_ctxRtmp != NULL && objBuf->DataLength() > pClient->m_ctxRtmp->m_nMaxRTMPPacketSize )
	{
		printf("----RTMPServer receive too large data\n");
		CloseConnection(pContext);
		return -1;
	}
	





	

	

	



	objBuf->Write( pBuffer->m_pBuf, pBuffer->m_nDataLen , _ALIGN_ );
	if( objBuf->Data != NULL && objBuf->WritePos < objBuf->Allocated )
	{
		objBuf->Data[ objBuf->WritePos ] = 0 ;
	}
	else
	{
		UINT8 nullEnd = 0 ;
		objBuf->Write( &nullEnd , sizeof(nullEnd) , _ALIGN_ );
		objBuf->WritePos -= sizeof(nullEnd) ;
	}

	if( pClient->m_type  == CRTMPCtx::ConnTypeUnknown )
	{
		if( objBuf->DataLength() < 5 )
		{
			return 0 ;
		}

		{


			if( pClient->m_ctxRtmp != NULL )
			{
				LogPrint(_LOG_LEVEL_ERROR , _TAGNAME ,"%s %d:conn is Unknown but m_ctxRtmp is not NULL(%p)\n" , __FUNCTION__ , __LINE__ , pClient->m_ctxRtmp  );
				return -1 ;
			}

			pClient->m_ctxRtmp = pClient->CreateRTMPCtx();
			if( pClient->m_ctxRtmp == NULL )
			{
				return -1 ;
			}
			pClient->m_ctxRtmp->m_type = CRTMPCtx::ConnTypeRTMP ;
			pClient->m_type = CRTMPCtx::ConnTypeRTMP ;
			pClient->m_ctxRtmp->RTMPConnCtxAdd( pClient );
			pClient->m_ctxRtmp->m_buf.Write( objBuf->Data , objBuf->WritePos ,_ALIGN_  );
			objBuf->Drop();
		}

	}


	
	processInputBuffer( pClient  , bError );
	if( bError )
	{
		CloseConnection( pClient );
		if( !pClient->m_bTriggeredClosedEvent )
		{
			this->OnConnectionClosed(pClient);
			pClient->m_bTriggeredClosedEvent = TRUE ;
			pClient->OnClose();
		}
	}

	if( bSync )
	{
		if( pClient->m_ctxRtmp != NULL )
		{
			pClient->m_ctxRtmp->Sync();
		}
		else
		{
			pClient->SyncIoBuffer();
		}
		
	}
	
	

	



	return 0 ;
}
int CRTMPNetAsync::OnInvalidRTMPIOBuffer(CObjConnContext *pContext, CObjNetIOBuffer *pBuffer,const char *szMsg)
{
	return 0 ;
}
int CRTMPNetAsync::OnHttpOpen(CRTMPConnContext *pContext)
{
	return 0 ;
}
int CRTMPNetAsync::OnHttpClose(CRTMPConnContext *pContext)
{
	return 0 ;
}
int CRTMPNetAsync::OnHttpIdle(CRTMPConnContext *pContext, const char *Id, const char *sequence)
{
	return 0 ;
}
int CRTMPNetAsync::OnHttpSend(CRTMPConnContext *pContext, const char *Id, const char *sequence)
{
	return 0 ;
}
CObjConnContext  * CRTMPNetAsync::AllocConnContext(CObj *par)
{
	return CRTMPConnContext::CreateObj();
}

int CRTMPNetAsync::OnReceivedHandshake1(CRTMPConnContext *pClient ,const unsigned char * data, int length)
{
	if( data[ 0 ]  != 3 )
	{
		LogPrint(_LOG_LEVEL_ERROR, _TAGNAME, "%s %d:  data[0]  != 3  (data[0]=%d)\n", __FUNCTION__, __LINE__, data[0] );

		return -1 ;
	}


	const UINT8 * clientsig = data + 1 ;




	/* decode client response */
	UINT32 upTimeClt = 0 ;
	UINT8  verClt[4];
	memcpy(&upTimeClt, clientsig, 4);
	upTimeClt = ntohl(upTimeClt);
	memcpy(verClt, clientsig + 4 , 4);



	CObjNetIOBufferDynamic *ioBuf = CObjNetIOBufferDynamic::CreateObj( RTMP_SIG_SIZE * 2 + 1 , SIZE_1KB );
	if( ioBuf == NULL )
	{
		LogPrint(_LOG_LEVEL_ERROR, _TAGNAME, "%s %d:  CObjNetIOBufferDynamic::CreateObj faield\n", __FUNCTION__, __LINE__);
		return -1 ;
	}
	ioBuf->m_pBuf[ 0 ] = data[ 0 ];
	memcpy( ioBuf->m_pBuf+1, _serversig , RTMP_SIG_SIZE );
	memcpy( ioBuf->m_pBuf + RTMP_SIG_SIZE + 1 , clientsig , RTMP_SIG_SIZE  );
	ioBuf->m_nDataLen = RTMP_SIG_SIZE * 2 + 1 ;

	if( pClient->m_type == CRTMPCtx::ConnTypeRTMPT  )
	{
		CObjNetIOBuffer *ioHeader =  pClient->SendHttpContentLength( ioBuf->m_nDataLen + 1 ) ;
		if( ioHeader == NULL )
		{
			ioBuf->ReleaseObj();
			LogPrint(_LOG_LEVEL_ERROR, _TAGNAME, "%s %d:  CreateObj SendHttpContentLength faield\n", __FUNCTION__, __LINE__);
			return -1 ;
		}
		ioHeader->m_pBuf[ ioHeader->m_nDataLen ] = 1 ;
		ioHeader->m_nDataLen ++ ;

		if( !pClient->SendIoBuffer ( ioHeader )  )
		{
			ioHeader->ReleaseObj();
			ioBuf->ReleaseObj();
			LogPrint(_LOG_LEVEL_ERROR, _TAGNAME, "%s %d:  SendIoBuffer faield\n", __FUNCTION__, __LINE__);
			return -1 ;
		}

		pClient->DecHttpRequests();
	}


	if( !pClient->SendIoBuffer ( ioBuf )  )
	{
		ioBuf->ReleaseObj();
		CloseConnection(pClient);
		LogPrint(_LOG_LEVEL_ERROR, _TAGNAME, "%s %d:  SendIoBuffer faield\n", __FUNCTION__, __LINE__);
		return -1 ;
	}

	return 0 ;

}
int CRTMPNetAsync::OnReceivedHandshake2(CRTMPConnContext *pClient ,const unsigned char * data, int length)
{
	const UINT8 * clientsig = data ;
	BOOL bMatch  = ( memcmp(_serversig, clientsig, RTMP_SIG_SIZE) == 0 );
	int nRet = 0 ;



	if( bMatch )
	{
		return 0 ;
	}
	else
	{
		return -1 ;
	}

}

int CRTMPNetAsync::OnRTMPReadCompleted(CRTMPConnContext *pClient, CRTMPHeader *header, CObjNetIOBuffer *pBuffer)
{

	return 0 ;
}
CObjNetIOBuffer *  CRTMPNetAsync::CreateRTMPIoBuffer( int Size )
{

	return CObjNetIOBufferSharedMemory::CreateObj( NULL , AlignLength( Size  , 1024 ) , TRUE );
}























































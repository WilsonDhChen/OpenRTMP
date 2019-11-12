


#include "libcpputil_def.h"





#include "libcpputil_net_udp.h"
#include "libcpputil_net_select.h"

CObjNetError::CObjNetError()
{
	m_error = RDE_Error();
    m_errno = RDE_ErrorCode(-1);
}
CObjNetError::~CObjNetError()
{
	if( m_error != NULL )
	{
		RDE_ErrorFree( m_error );
	}

	
}
CObjNetError::operator int () const
{
	return m_errno ;
}
CObjNetError::operator const char *() const
{
	static char _szNULL[2] = { 0, 0 };
	if( m_error == NULL )
	{
		return _szNULL ;
	}
	else
	{
		return m_error ;
	}

}

/************************************************************************/
/*                          CObjNetIOBuffer                             */
/************************************************************************/

static INT32 _nCObjNetIOBuffer = 0 ;

_CPP_UTIL_DYNAMIC_IMP(CObjNetIOBuffer)
_CPP_UTIL_CLASSNAME_IMP(CObjNetIOBuffer)


CObjNetIOBuffer::CObjNetIOBuffer()
{

     MtxInterlockedInc(&_nCObjNetIOBuffer) ;

     m_nDataLen = 0 ;
     m_pBuf = NULL ;
     m_nBufLen = 0;
     m_nLenTrans = 0 ;
	 
     
     m_opType = IOOpTypeUnkown ;

#ifdef WIN32
	 m_bV6 = FALSE;
     memset( &m_ol,0,sizeof(m_ol) );
     m_socAccept_Connect = INVALID_RDE ;
     m_nSequenceNumber = 0 ;
#endif

}
CObjNetIOBuffer::~CObjNetIOBuffer()
{
    MtxInterlockedDec(&_nCObjNetIOBuffer) ;
}
int CObjNetIOBuffer::ObjCount()
{
	return _nCObjNetIOBuffer ;
}
void * CObjNetIOBuffer::operator new( size_t cb )
{
    return malloc2_align(cb , 0 , 4 );
}
void CObjNetIOBuffer::operator delete( void* p )
{
    free2(p);
}
/*
BOOL CObjNetIOBuffer::ResetDataPtr()
{
    return FALSE ;
}
*/
CObjNetIOBuffer::CObjNetIOBuffer(const CObjNetIOBuffer &src)
{
    m_pBuf = NULL ;
    m_nBufLen = 0;
    m_nLenTrans = 0 ;
    
    m_opType = IOOpTypeUnkown ;

#ifdef WIN32
    memset( &m_ol,0,sizeof(m_ol) );
    m_socAccept_Connect = INVALID_RDE ;
    m_nSequenceNumber = 0 ;
#endif

    *this = src ;
}
CObjNetIOBuffer & CObjNetIOBuffer::operator =(  const CObjNetIOBuffer& src )
{

    if( m_pBuf != NULL && src.m_pBuf != NULL )
    {
        m_nBufLen = MinInt(m_nBufLen,src.m_nBufLen);
        memcpy(m_pBuf,src.m_pBuf,m_nBufLen) ;

    }
    

    m_nLenTrans = MinInt(m_nBufLen,src.m_nLenTrans) ;


    
    m_opType = src.m_opType ;

#ifdef WIN32
    m_ol = src.m_ol ;
    m_socAccept_Connect = src.m_socAccept_Connect ;
#endif

    return *this;
}
const char *CObjNetIOBuffer::IOOpType2Str( CObjNetIOBuffer::IOOpType  ioType)
{

    static char szIOOpTypeUnkown[] = "IOOpTypeUnkown";
    static char szIOOpTypeAccept[] = "IOOpTypeAccept";
    static char szIOOpTypeRead[] = "IOOpTypeRead";
    static char szIOOpTypeWrite[] = "IOOpTypeWrite";
    static char szIOOpTypeConnect[] = "IOOpTypeConnect";
     
    switch ( ioType )
    {
    case  IOOpTypeAccept:
        {
            return szIOOpTypeAccept;
        }
    case IOOpTypeRead:
        {
            return szIOOpTypeRead;
        }
    case IOOpTypeWrite:
        {
            return szIOOpTypeWrite;
        }
    case IOOpTypeConnect:
        {
            return szIOOpTypeConnect;
        }
    default:
        {
            return  szIOOpTypeUnkown;
        }
    }
}
void * CObjNetIOBuffer::QueryObj(const char *szObjName)
{
    if( szObjName == NULL )
    {
        return NULL ;
    }

    if( strcmp( szObjName, GetObjName() ) == 0 )
    {
        return this;
    }
    else
    {
        return CObj::QueryObj(szObjName) ;
    }
}

/************************************************************************/
/*                          CObjNetIOBufferDefault                      */
/************************************************************************/
#if defined(_DEBUG_OBJ_REF_)
INT32 _nCObjNetIOBufferDefault = 0 ;
#endif
_CPP_UTIL_DYNAMIC_IMP(CObjNetIOBufferDefault)
_CPP_UTIL_CLASSNAME_IMP(CObjNetIOBufferDefault)
_CPP_UTIL_QUERYOBJ_IMP( CObjNetIOBufferDefault , CObjNetIOBuffer)
CObjNetIOBufferDefault::CObjNetIOBufferDefault()
{
#if defined(_DEBUG_OBJ_REF_)
    MtxInterlockedInc(&_nCObjNetIOBufferDefault) ;
#endif
	m_pBuf = m_szBuf ;
	m_nBufLen = sizeof(m_szBuf)-1 ; 

	m_szBuf[0] = 0 ;
	m_szBuf[ sizeof(m_szBuf)-1   ] = 0 ; 

}
CObjNetIOBufferDefault::~CObjNetIOBufferDefault()
{
#if defined(_DEBUG_OBJ_REF_)
    MtxInterlockedDec(&_nCObjNetIOBufferDefault) ;
#endif
}



/************************************************************************/
/*                          CObjNetIOBufferUDPDefault                      */
/************************************************************************/
#if defined(_DEBUG_OBJ_REF_)
INT32 _nCObjNetIOBufferUDPDefault = 0 ;
#endif
_CPP_UTIL_DYNAMIC_IMP(CObjNetIOBufferUDPDefault)
_CPP_UTIL_CLASSNAME_IMP(CObjNetIOBufferUDPDefault)
_CPP_UTIL_QUERYOBJ_IMP( CObjNetIOBufferUDPDefault , CObjNetIOBufferUDP)
CObjNetIOBufferUDPDefault::CObjNetIOBufferUDPDefault()
{
#if defined(_DEBUG_OBJ_REF_)
	MtxInterlockedInc(&_nCObjNetIOBufferUDPDefault) ;
#endif
	memset(m_szBuf,0,sizeof(m_szBuf) );
	m_pBuf = m_szBuf ;
	m_nBufLen = sizeof(m_szBuf) -1 ; 

}
CObjNetIOBufferUDPDefault::~CObjNetIOBufferUDPDefault()
{
#if defined(_DEBUG_OBJ_REF_)
	MtxInterlockedDec(&_nCObjNetIOBufferUDPDefault) ;
#endif
}
/************************************************************************/
/*                          CObjNetIOBufferUDP                      */
/************************************************************************/
#if defined(_DEBUG_OBJ_REF_)
INT32 _nCObjNetIOBufferUDP = 0 ;
#endif
_CPP_UTIL_DYNAMIC_IMP(CObjNetIOBufferUDP)
_CPP_UTIL_CLASSNAME_IMP(CObjNetIOBufferUDP)
_CPP_UTIL_QUERYOBJ_IMP( CObjNetIOBufferUDP , CObjNetIOBuffer)
CObjNetIOBufferUDP::CObjNetIOBufferUDP()
{
#if defined(_DEBUG_OBJ_REF_)
	MtxInterlockedInc(&_nCObjNetIOBufferUDP) ;
#endif

	memset( &m_AddrRemote, 0, sizeof(m_AddrRemote) );

}
CObjNetIOBufferUDP::~CObjNetIOBufferUDP()
{
#if defined(_DEBUG_OBJ_REF_)
	MtxInterlockedDec(&_nCObjNetIOBufferUDP) ;
#endif
}

/************************************************************************/
/*                           CObjNetIOBufferDynamic                     */
/************************************************************************/
_CPP_UTIL_CLASSNAME_IMP( CObjNetIOBufferDynamic )
_CPP_UTIL_QUERYOBJ_IMP( CObjNetIOBufferDynamic , CObjNetIOBuffer)
CObjNetIOBufferDynamic * CObjNetIOBufferDynamic::CreateObj(int bufferSize,int Align )
{
	if( bufferSize < 1 )
	{
		return NULL ;
	}
	CObjNetIOBufferDynamic *pObj = new CObjNetIOBufferDynamic(bufferSize ,Align);
	if( pObj == NULL )
	{
		return NULL ;
	}
	if( pObj->m_BufAlloced == NULL )
	{
		pObj->ReleaseObj();
		return NULL ;
	}
	pObj->m_bAlloced = TRUE ;
	pObj->m_nObjSize = sizeof(CObjNetIOBufferDynamic) ;
	pObj->AddObjRef();
	return pObj ;
}
#if defined(_DEBUG_OBJ_REF_)
INT32 _nCObjNetIOBufferDynamic = 0 ;
#endif
CObjNetIOBufferDynamic::CObjNetIOBufferDynamic(int bufferSize,int Align )
{
#if defined(_DEBUG_OBJ_REF_)
	MtxInterlockedInc(&_nCObjNetIOBufferDynamic) ;
#endif
	if( Align < 0 )
	{
		Align = 0 ;
	}
	Align--;
	bufferSize = (bufferSize  + Align ) & ~Align;

	m_BufAlloced = (char *)malloc2(bufferSize);

	if( m_BufAlloced != NULL  )
	{
		m_nBufLen = bufferSize ;
		m_pBuf = m_BufAlloced ;
		m_BufAlloced[0] = 0 ;
		m_BufAlloced[bufferSize-1] = 0 ;
	}
	else
	{
		printf("CObjNetIOBufferDynamic malloc %d bytes failed\n" , bufferSize);
	}


}
CObjNetIOBufferDynamic::~CObjNetIOBufferDynamic()
{
#if defined(_DEBUG_OBJ_REF_)
	MtxInterlockedDec(&_nCObjNetIOBufferDynamic) ;
#endif
	if( m_BufAlloced != NULL  )
	{
		free2( m_BufAlloced );

	}
}
/************************************************************************/
/*                           CObjNetIOBufferSharedMemory                     */
/************************************************************************/
_CPP_UTIL_CLASSNAME_IMP( CObjNetIOBufferSharedMemory )
_CPP_UTIL_QUERYOBJ_IMP( CObjNetIOBufferSharedMemory , CObjNetIOBuffer)
CObjNetIOBufferSharedMemory * CObjNetIOBufferSharedMemory::CreateObj(void *memShared,int memSize,BOOL bMalloc2)
{
	if( memSize < 1 && memShared == NULL )
	{
		return NULL ;
	}
	CObjNetIOBufferSharedMemory *pObj = new CObjNetIOBufferSharedMemory(memShared,memSize, bMalloc2);
	if( pObj == NULL )
	{
		return NULL ;
	}

	pObj->m_bAlloced = TRUE ;
	pObj->m_nObjSize = sizeof(CObjNetIOBufferSharedMemory) ;
	pObj->AddObjRef();
	return pObj ;
}
#if defined(_DEBUG_OBJ_REF_)
INT32 _nCObjNetIOBufferSharedMemory = 0 ;
#endif
CObjNetIOBufferSharedMemory::CObjNetIOBufferSharedMemory(void *memShared,int memSize,BOOL bMalloc2)
{
#if defined(_DEBUG_OBJ_REF_)
	MtxInterlockedInc(&_nCObjNetIOBufferSharedMemory) ;
#endif
	m_bMalloc2 = bMalloc2 ;

	if( m_bMalloc2 )
	{

		if( memShared != NULL )
		{
			SharedMemAddRef(memShared);
			m_nDataLen = memSize ;
		}
		else
		{
			memShared = SharedMemCreate( memSize );
			m_nDataLen = 0 ;
		}
		


	}
	else
	{

		if( memShared != NULL )
		{
			MemoryAddRef(memShared);
			m_nDataLen = memSize ;
		}
		else
		{
			memShared = MemoryCreate( memSize , NULL );
			m_nDataLen = 0 ;
		}

		

	}
	

	
	m_BufShared = (char *)memShared;
	

	if( m_bMalloc2 )
	{

		m_nBufLen = SharedMemSize(memShared);


	}
	else
	{

		m_nBufLen = MemorySize(memShared);

	}

	
	m_pBuf =   m_BufShared ;



}
CObjNetIOBufferSharedMemory::~CObjNetIOBufferSharedMemory()
{
#if defined(_DEBUG_OBJ_REF_)
	MtxInterlockedDec(&_nCObjNetIOBufferSharedMemory) ;
#endif

	if( m_bMalloc2 )
	{

		SharedMemRelease(m_BufShared);


	}
	else
	{

		MemoryRelease(m_BufShared);

	}
	
}
CObjNetIOBufferSharedMemory *CObjNetIOBufferSharedMemory::Copy()
{
	return CObjNetIOBufferSharedMemory::CreateObj( m_BufShared , m_nDataLen, m_bMalloc2 );
}
CObjNetIOBufferSharedMemory *CObjNetIOBufferSharedMemory::CopyNoShare()
{
	CObjNetIOBufferSharedMemory *ioBuf = CObjNetIOBufferSharedMemory::CreateObj(NULL, m_nBufLen, m_bMalloc2);
	if (ioBuf == NULL)
	{
		return NULL;
	}

	memcpy(ioBuf->m_pBuf, m_pBuf, m_nDataLen);

	ioBuf->m_nDataLen = m_nDataLen;

	return ioBuf;
}
/************************************************************************/
/*                           CObjConnContext                            */
/************************************************************************/


INT32 _nCObjConnContext = 0 ;

_CPP_UTIL_DYNAMIC_IMP(CObjConnContext)
_CPP_UTIL_CLASSNAME_IMP(CObjConnContext)


CObjConnContext::CObjConnContext()
{

    MtxInterlockedInc(&_nCObjConnContext );

    m_ssl = NULL;
	m_bConnecting = FALSE;
    m_bInited = FALSE ;
	m_nWrittingBytes = 0;
    memset( &m_AddrLocal, 0, sizeof(m_AddrLocal) );
    memset( &m_AddrRemote, 0, sizeof(m_AddrRemote) );
    m_bBadConntext = FALSE ;
    m_soc = INVALID_RDE ;
    m_bClosing = FALSE ;
    m_bTriggeredClosedEvent = FALSE ;



    m_ObjUser = NULL;
    m_bAutoReleaseUserObj = FALSE;
    m_tmConnected = libcpputil_UpSeconds();
    m_tmLastRead = libcpputil_UpSeconds();
    m_tmLastWrite = libcpputil_UpSeconds();
    m_tmLastActive = libcpputil_UpSeconds() ;

#ifdef WIN32
    m_nOutstandingSend = 0 ;
    m_nReadSequence = 0 ;			
    m_nCurrentReadSequence = 0;
#else
    m_bEvent = FALSE ;
#endif
	m_net = NULL ;
	MtxInit(&m_mtx,0) ;

    
}
TIME_T CObjConnContext::UpSeconds()
{
	return libcpputil_UpSeconds();
}
BOOL CObjConnContext::Send(CObjNetIOBuffer *pBuf)
{
	if( m_bClosing || m_net == NULL )
	{
		return FALSE ;
	}
	return m_net->Send(this ,pBuf );
}
BOOL CObjConnContext::InitLater()
{
    if( m_bInited )
    {
        return TRUE ;
    }
    m_bInited = TRUE ;
    return m_bInited ;
}
CObjConnContext::CObjConnContext(const CObjConnContext &src)
{
	MtxInterlockedInc(&_nCObjConnContext);
    memset( &m_AddrLocal, 0, sizeof(m_AddrLocal) );
    memset( &m_AddrRemote, 0, sizeof(m_AddrRemote) );
    m_soc = INVALID_RDE ;
    m_bClosing = FALSE ;
    m_bTriggeredClosedEvent = FALSE ;

    m_ObjUser = NULL;
    m_bAutoReleaseUserObj = FALSE;
    m_tmConnected = libcpputil_UpSeconds();
    m_tmLastRead = 0;
    m_tmLastWrite = 0;
    m_tmLastActive = 0 ;
	m_net = NULL ;
    m_ssl = NULL;


#ifdef WIN32
    m_nOutstandingSend = 0 ;
    m_nReadSequence = 0 ;			
    m_nCurrentReadSequence = 0;
#else
    m_bEvent = src.m_bEvent ;;
#endif
	m_nWrittingBytes = 0;

    *this = src ;
}
CObjConnContext::~CObjConnContext()
{

    MtxInterlockedDec(&_nCObjConnContext) ;

	CObj * obj = NULL ;
    if ( m_bAutoReleaseUserObj && m_ObjUser != NULL )
    {
        m_ObjUser->ReleaseObj() ;
    }

    if( m_soc != INVALID_RDE )
    {
        libcpputil_close_soc( m_soc );
        m_soc = INVALID_RDE ;
    }
#ifdef WIN32

    while( ( obj = m_listOutOfOrderReads.ListRemoveHead() ) != NULL )
    {
        obj->ReleaseObj() ;
    }
#endif
    
    Lock();
    while( ( obj = m_listSendBufs.ListRemoveHead() ) != NULL )
    {
        obj->ReleaseObj() ;
    }
	Unlock();
    

    if (m_ssl != NULL)
    {
        SSLFree(this);
        m_ssl = NULL;
    }
	MtxDestroy(&m_mtx);
	


}
int CObjConnContext::PendingBuffers()
{
	return m_listSendBufs.ListItemsCount();
}
void CObjConnContext::ClearPendingBuffers()
{
	CObj * obj = NULL ;
	while( ( obj = m_listSendBufs.ListRemoveHead() ) != NULL )
	{
		obj->ReleaseObj() ;
	}
}
int CObjConnContext::ObjCount()
{
	return _nCObjConnContext;
}
void * CObjConnContext::operator new( size_t cb )
{

    return malloc2(cb);

}
void CObjConnContext::operator delete( void* p )
{


    free2(p);


}
CObj * CObjConnContext::AttachUserObj( CObj * objUsr, BOOL bAutoRelease  ) 
{
    CObj *obj = m_ObjUser ;
    m_ObjUser = objUsr ;
    m_bAutoReleaseUserObj = bAutoRelease ;

    return obj ;
}
CObj * CObjConnContext::ClearUserObj()
{
    CObj *obj = m_ObjUser ;

    m_ObjUser = NULL ;
    m_bAutoReleaseUserObj = FALSE ;

    return obj ;
}
CObj * CObjConnContext::GetUserObj()
{
    return m_ObjUser ;
}
void CObjConnContext::Lock()
{
    MtxLock(&m_mtx);
}
void CObjConnContext::Unlock()
{
    MtxUnLock(&m_mtx);
}


CObjConnContext & CObjConnContext::operator =(  const CObjConnContext& src )
{
    if ( m_bAutoReleaseUserObj && m_ObjUser != NULL )
    {
        m_ObjUser->ReleaseObj() ;
    }

    m_AddrLocal =  src.m_AddrLocal ;
    m_AddrRemote = src.m_AddrRemote ;
    m_soc = src.m_soc ;

    m_ObjUser =  src.m_ObjUser ;
    m_bAutoReleaseUserObj = src.m_bAutoReleaseUserObj ;
    if ( m_bAutoReleaseUserObj && m_ObjUser != NULL )
    {
        m_ObjUser->AddObjRef();
    }
    return *this;
}
void * CObjConnContext::QueryObj(const char *szObjName)
{
    if( szObjName == NULL )
    {
        return NULL ;
    }

    if( strcmp( szObjName, GetObjName() ) == 0 )
    {
        return this;
    }
    else
    {
        return CObj::QueryObj(szObjName) ;
    }
}
void CObjConnContext::OnClose()
{

}

/************************************************************************/
/*                  CObjNetAsync                                      */
/************************************************************************/

CObjNetAsync::CObjNetAsync(const CObjNetAsync &src)
{


}
CObjNetAsync & CObjNetAsync::operator =(  const CObjNetAsync& src )
{
    return *this;
}

CObjNetAsync::CObjNetAsync(int nWaitTimeoutSeconds,
						   int nStackSize,int nMaxConnections,
                           NetAsyncType netType)
{

    m_pNetObj = (void *)new   CObjNetAsyncImp(this,nWaitTimeoutSeconds, nStackSize,
        nMaxConnections, netType);

}
CObjNetAsync::~CObjNetAsync()
{

    if( m_pNetObj != NULL )
    {
        delete  ((CObjNetAsyncImp*)m_pNetObj);
    }

    
}
BOOL CObjNetAsync::IsSupportSsl()
{
    return _libCpputilSupportSSL;
}
BOOL CObjNetAsync::SetSslCertAndKey(const char *szCertFile, const char *szKeyFile)
{
    return ((CObjNetAsyncImp*)m_pNetObj)->SetSSLCertAndKey(szCertFile, szKeyFile);
}
void CObjNetAsync::SetSslType(NetSsl ssl )
{
    ((CObjNetAsyncImp*)m_pNetObj)->SetSslType(ssl);
}

CObj *CObjNetAsync::GetThreadContext()
{
    return ((CObjNetAsyncImp*)m_pNetObj)->GetThreadContext();
}

CObjConnContext  * CObjNetAsync::AllocConnContext(CObj *par)
{
    return  CObjConnContext::CreateObj();
}
CObjNetIOBuffer  * CObjNetAsync::AllocReadIOBuffer()
{
    return  CObjNetIOBufferDefault::CreateObj();
}

CObj * CObjNetAsync::CreateThreadContext()
{
	return NULL ;
}

int CObjNetAsync::OnNewConnectionIncoming(CObjConnContext *pContext)
{
#if defined(_DEBUG_) || defined(_DEBUG)
    printf("OnNewConnectionIncoming %s:%d\n",pContext->m_AddrRemote.szAddr,pContext->m_AddrRemote.nPort);
#endif
    return 0 ;
}
int CObjNetAsync::OnNewConnectionOutgoing(CObjConnContext *pContext)
{

    return 0 ;
}
int CObjNetAsync::OnConnected(CObjConnContext *pContext)
{
	return 0;
}
int CObjNetAsync::OnConnectionClosed(CObjConnContext *pContext)
{

#if defined(_DEBUG_) || defined(_DEBUG)
    printf("OnConnectionClosed %s:%d\n",pContext->m_AddrRemote.szAddr,pContext->m_AddrRemote.nPort);
#endif
    return 0 ;

}

int CObjNetAsync::OnReadCompleted(CObjConnContext *pContext, CObjNetIOBuffer *pBuffer)
{
#if defined(_DEBUG_) || defined(_DEBUG)
    printf("OnReadCompleted %s:%d %s\n",pContext->m_AddrRemote.szAddr,pContext->m_AddrRemote.nPort,pBuffer->m_pBuf);
#endif
    return 0 ;
}
int CObjNetAsync::OnWriteCompleted(CObjConnContext *pContext, CObjNetIOBuffer *pBuffer)
{
#if defined(_DEBUG_) || defined(_DEBUG)
    printf("OnWriteCompleted %s:%d %d/%d\n",pContext->m_AddrRemote.szAddr,pContext->m_AddrRemote.nPort,pBuffer->m_nLenTrans,pBuffer->m_nDataLen);
#endif
    return 0 ;
}
void CObjNetAsync::SetSessionTimeout(int nSeconds )
{
    ((CObjNetAsyncImp*)m_pNetObj)->SetSessionTimeout(nSeconds);
}
void CObjNetAsync::SetSessionTimeoutType(SessionTimeoutType type)
{
    ((CObjNetAsyncImp*)m_pNetObj)->SetSessionTimeoutType(type);
}
CObjNetAsync::SessionTimeoutType CObjNetAsync::GetSessionTimeoutType()
{
    return ((CObjNetAsyncImp*)m_pNetObj)->GetSessionTimeoutType();
}
void CObjNetAsync::SetMaxConnections(int nCount)
{
    ((CObjNetAsyncImp*)m_pNetObj)->SetSessionTimeout(nCount);

}
void CObjNetAsync::SetMaxSends(int nCount )
{
}
void CObjNetAsync::SetMaxWrittingBytes(int bytes)
{
}
int  CObjNetAsync::GetMaxConnections()
{
    return 0;
}
int CObjNetAsync::OnSessionTimeout(CObjConnContext *pContext)
{
    return 0 ;
}
int CObjNetAsync::OnEvent()
{
    return 0 ;
}
int CObjNetAsync::OnError(CObjConnContext *pContext, const char * szTxt)
{
	return 0;
}
int CObjNetAsync::OnMaxConnectionsLimited(CObjConnContext *pContext)
{
    return 0 ;
}

int CObjNetAsync::OnEventWaitTimeout(int nSeconds)
{
    
    return 0 ;
}



void CObjNetAsync::LockContextList()
{
    ((CObjNetAsyncImp*)m_pNetObj)->LockContextList();
}
void CObjNetAsync::UnlockContextList()
{
    ((CObjNetAsyncImp*)m_pNetObj)->UnlockContextList();
}
const CObj  *  CObjNetAsync::GetConetxtListHead()
{
    return ((CObjNetAsyncImp*)m_pNetObj)->GetConetxtListHead();

}

BOOL CObjNetAsync::BindAddr(int nPort, const char * szBindAddrIn, BOOL bIpV6)
{
    return ((CObjNetAsyncImp*)m_pNetObj)->BindAddr(nPort, szBindAddrIn, bIpV6);
}
BOOL CObjNetAsync::Start()
{
    return ((CObjNetAsyncImp*)m_pNetObj)->Start();
}
void CObjNetAsync::SetName(const char *szName)
{
    ((CObjNetAsyncImp*)m_pNetObj)->SetName(szName);
}
void CObjNetAsync::CloseAllConnections()
{
    ((CObjNetAsyncImp*)m_pNetObj)->CloseAllConnections();

}
void CObjNetAsync::CloseConnection(CObjConnContext *pContext)
{
    ((CObjNetAsyncImp*)m_pNetObj)->CloseConnection(pContext);
}
void CObjNetAsync::Shutdown()
{
    ((CObjNetAsyncImp*)m_pNetObj)->Shutdown();
}
int  CObjNetAsync::GetConnections( )
{
    return  ((CObjNetAsyncImp*)m_pNetObj)->GetConnections();
}
int  CObjNetAsync::GetBindPort(BOOL bIpV6)
{
    return  ((CObjNetAsyncImp*)m_pNetObj)->GetBindPort(bIpV6);
}
BOOL CObjNetAsync::Connect(const char *addr, int port, CObj *par)
{
    return  ((CObjNetAsyncImp*)m_pNetObj)->Connect(addr, port, par);
}
BOOL CObjNetAsync::Send(CObjConnContext *pContext, const char *szBuffer, int nLen)
{
    return ((CObjNetAsyncImp*)m_pNetObj)->Send(pContext, szBuffer, nLen);
}
BOOL CObjNetAsync::Send(CObjConnContext *pContext, CObjNetIOBuffer *pBuffer)
{
    return  ((CObjNetAsyncImp*)m_pNetObj)->Send(pContext, pBuffer);
}
BOOL CObjNetAsync::AddSocket2Asyn(SOCKET soc, INET_ADDR_STR *remoteAddr, INET_ADDR_STR *localAddr)
{
    return  ((CObjNetAsyncImp*)m_pNetObj)->AddSocket2Asyn(soc, remoteAddr, localAddr);
}
BOOL CObjNetAsync::AddSocket2Asyn(CObjConnContext *pSocketConnectByYourself)
{
    return  ((CObjNetAsyncImp*)m_pNetObj)->AddSocket2Asyn(pSocketConnectByYourself);
}
SOCKET CObjNetAsync::ConnectSocket(char *szAddr, int nPort, unsigned int timeout)
{
    return RDE_Connect( szAddr, nPort, timeout) ;
}
SOCKET  CObjNetAsync::ConnectAgentHttps(const char *szAgentUrl,const char *szIP,int nPort,unsigned int timeout )
{

    return RDE_ConnectAgentHttps( szAgentUrl, szIP, nPort, timeout ) ;

}
SOCKET CObjNetAsync::ConnectAgentSks4(const char *szAgentUrl,const char *szIP,int nPort,unsigned int timeout )
{
    return RDE_ConnectAgentSks4( szAgentUrl, szIP, nPort, timeout ) ;

}
SOCKET CObjNetAsync::ConnectAgentSks5(const char *szAgentUrl,const char *szIP,int nPort,unsigned int timeout )
{
    return RDE_ConnectAgentSks5( szAgentUrl, szIP, nPort, timeout   ) ;
}



/************************************************************************/
/*                  CObjNetUDP                                      */
/************************************************************************/

_CPP_UTIL_DYNAMIC_IMP(CObjNetUDP)
_CPP_UTIL_CLASSNAME_IMP(CObjNetUDP)
_CPP_UTIL_QUERYOBJ_IMP(CObjNetUDP,CObj)
CObjNetUDP::CObjNetUDP()
{
	m_pNetObj = NULL ;
#if defined(_NET_UDP_)
	CNetAsyncUDP *pObj =  CNetAsyncUDP::CreateObj();
	if( pObj == NULL )
	{
		return ;
	}

	pObj->m_pNetAsync = this;
	m_pNetObj = pObj ;
#endif
}
CObjNetUDP::~CObjNetUDP()
{
	if( m_pNetObj != NULL )
	{
		m_pNetObj->ReleaseObj() ;
	}
}
CObjNetUDP::CObjNetUDP(const CObjNetUDP &src)
{

}
CObjNetUDP & CObjNetUDP::operator =(  const CObjNetUDP& src )
{
	return *this;
}


CObjNetIOBufferUDP  * CObjNetUDP::AllocReadIOBuffer()
{

	return CObjNetIOBufferUDPDefault::CreateObj() ;
}

int CObjNetUDP::OnReadCompleted(CObjNetIOBufferUDP *pBuffer)
{
	printf("CObjNetUDP::OnReadCompleted<%s>\n",pBuffer->m_pBuf );
	return 0 ;
}
BOOL CObjNetUDP::BindAddr(int nPort ,const char * szBindAddr )
{
#if defined(_NET_UDP_)
	CNetAsyncUDP * pObj = _CPP_UTIL_FORCECAST(m_pNetObj,CNetAsyncUDP);
	if( pObj == NULL )
	{
		return FALSE ;
	}
	return pObj->BindAddr(nPort,szBindAddr);
#else
    return FALSE ;
#endif
}
BOOL CObjNetUDP::Start()
{
#if defined(_NET_UDP_)
	CNetAsyncUDP * pObj = _CPP_UTIL_FORCECAST(m_pNetObj,CNetAsyncUDP);
	if( pObj == NULL )
	{
		return FALSE ;
	}
	return pObj->Start();
#else
    return FALSE ;
#endif
}
void CObjNetUDP::Shutdown()
{
#if defined(_NET_UDP_)
	CNetAsyncUDP * pObj = _CPP_UTIL_FORCECAST(m_pNetObj,CNetAsyncUDP);
	if( pObj == NULL )
	{
		return  ;
	}
	pObj->Shutdown();
#endif
}
int  CObjNetUDP::GetBindPort()
{
#if defined(_NET_UDP_)
	CNetAsyncUDP * pObj = _CPP_UTIL_FORCECAST(m_pNetObj,CNetAsyncUDP);
	if( pObj == NULL )
	{
		return FALSE ;
	}
	return pObj->GetBindPort();
#else
    return 0 ;
#endif
}

BOOL  CObjNetUDP::Send(CObjNetIOBufferUDP *pBuffer)
{
#if defined(_NET_UDP_)
	CNetAsyncUDP * pObj = _CPP_UTIL_FORCECAST(m_pNetObj,CNetAsyncUDP);
	if( pObj == NULL )
	{
		return FALSE ;
	}
	return pObj->Send(pBuffer);
#else
    return FALSE ;
#endif
}




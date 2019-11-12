

#include "libcpputil_def.h"

#include "libcpputil_net_udp.h"



_CPP_UTIL_DYNAMIC_IMP(CNetAsyncUDP)
_CPP_UTIL_CLASSNAME_IMP(CNetAsyncUDP)
_CPP_UTIL_QUERYOBJ_IMP(CNetAsyncUDP,CObj)

CNetAsyncUDP::CNetAsyncUDP()
{
	m_socListen = INVALID_RDE ;

#ifdef WIN32
	m_hThreadSelect = NULL ;
#else
	m_nThreadIDSelect = 0 ;
#endif

	m_bShutDown = FALSE ;

	m_ioBuffer = NULL ;

}
CNetAsyncUDP::~CNetAsyncUDP()
{
	if( m_socListen != INVALID_RDE)
	{
		libcpputil_close_soc(m_socListen);
		m_socListen = INVALID_RDE;
	}

	if( m_ioBuffer != NULL )
	{
		m_ioBuffer->ReleaseObj();
	}

}

BOOL CNetAsyncUDP::BindAddr(int nPort ,const char * szBindAddr )
{
	if( m_socListen != INVALID_RDE )
	{
		return FALSE ;
	}
	m_socListen = libcpputil_bind_udp(szBindAddr,nPort,TRUE) ;
	if( m_socListen == INVALID_RDE )
	{
		return FALSE ;
	}

	return TRUE;
}
int  CNetAsyncUDP::GetBindPort()
{
	int port = 0 ;
	if( m_socListen == INVALID_RDE )
	{
		return 0 ;
	}
	struct sockaddr_in sock_addr;
	int nRet;
	socklen_t size_addr;
	size_addr=sizeof(struct sockaddr);

	nRet= getsockname( m_socListen,(struct sockaddr*)&sock_addr,&size_addr);
	if( nRet )
	{
		return 0;
	}
	port = ntohs(sock_addr.sin_port);

	return port;
}

#ifdef WIN32
unsigned int WINAPI CNetAsyncUDP::_ThreadProcWorker( void * inPar) 
#else
void *CNetAsyncUDP::_ThreadProcWorker( void * inPar) 
#endif
{
	CNetAsyncUDP *_this = (CNetAsyncUDP *)inPar ;
	_this->ThreadWorker();
#ifdef WIN32
	_endthreadex( 0 );
#else
	pthread_exit(0);
#endif

	return 0 ;
}
BOOL CNetAsyncUDP::Start()
{

	if( m_socListen == INVALID_RDE )
	{
		return FALSE ;
	}

#ifdef WIN32
	unsigned int threadID = 0 ;
	if( m_hThreadSelect != NULL )
	{
		return TRUE ;
	}
	m_hThreadSelect = (HANDLE)_beginthreadex( NULL, 0, &CNetAsyncUDP::_ThreadProcWorker, this, 0, &threadID );
	if( m_hThreadSelect == NULL )
	{
		return FALSE ;
	}
	else
	{
		return TRUE ;
	}
#else
	int nRet = -1 ;
	if( m_nThreadIDSelect != 0 )
	{
		return TRUE ;
	}
	nRet = pthread_create((pthread_t*)&m_nThreadIDSelect,NULL,&CNetAsyncUDP::_ThreadProcWorker,(void *)this);
	if( nRet )
	{
		return FALSE ;
	}
	return TRUE ;
#endif
}
void CNetAsyncUDP::Shutdown()
{


#ifdef WIN32
	if( m_hThreadSelect == NULL )
	{
		return ;
	}
	m_bShutDown = TRUE;

	if( m_socListen != INVALID_RDE )
	{
		shutdown(m_socListen,SD_BOTH);
	}


	::WaitForSingleObject(m_hThreadSelect, INFINITE);
	::CloseHandle(m_hThreadSelect);
	m_hThreadSelect = NULL;

#else
	void *value = NULL ;
	if( m_nThreadIDSelect == 0 )
	{
		return ;
	}
	m_bShutDown = TRUE;

	if( m_socListen != INVALID_RDE )
	{
		shutdown(m_socListen,SD_BOTH);
	}

	pthread_join(m_nThreadIDSelect,&value);
	m_nThreadIDSelect = 0 ;
#endif
	if( m_socListen != INVALID_RDE )
	{
		RDEU_Close(m_socListen);
		m_socListen= INVALID_RDE ;
	}
	m_bShutDown = FALSE ;

}

int CNetAsyncUDP::ThreadWorker()
{
	int  ret = 0 ;
	int  retHandle = 0 ;
	fd_set  fdsetRead;
	fd_set  fdsetWrite;
	fd_set  fdsetError;
	SOCKET maxfd = 0 ;
	struct timeval timeout;

	memset( &timeout, 0 , sizeof(timeout) ) ;
	memset( &fdsetRead, 0 , sizeof(fdsetRead) ) ;
	memset( &fdsetWrite, 0 , sizeof(fdsetWrite) ) ;
	memset( &fdsetError, 0 , sizeof(fdsetError) ) ;
	timeout.tv_usec = 10000 ;


	while ( ! m_bShutDown )
	{

		FD_ZERO( &fdsetRead );
		FD_ZERO( &fdsetWrite );
		FD_ZERO( &fdsetError );
		if( m_socListen != INVALID_RDE )
		{
			FD_SET( m_socListen, &fdsetRead );
			maxfd = m_socListen ;
		}
		else
		{
			SleepMilli(10);
			continue ;
		}

		ret = select( maxfd + 1 ,&fdsetRead, &fdsetWrite, &fdsetError , &timeout ) ;
		if( ret == -1 )
		{
			break ;
		}
		else if( ret == 0 )
		{
			continue ;
		}
		else
		{
			if( !FD_ISSET(m_socListen,&fdsetRead) )
			{
				continue ;
			}
			HandleEventRead( m_socListen );
		}

	}
	return 0 ;
}
int CNetAsyncUDP::HandleEventRead( SOCKET soc)
{

	if( m_ioBuffer == NULL )
	{
		m_ioBuffer = m_pNetAsync->AllocReadIOBuffer();
	}

	if( m_ioBuffer == NULL )
	{
		return -1 ;
	}
	
	struct    sockaddr_in DestAddr;
	socklen_t nAddrSize=_SIZE_OF(DestAddr);
	m_ioBuffer->m_nLenTrans=recvfrom(soc,(char *)m_ioBuffer->m_pBuf,
		m_ioBuffer->m_nBufLen,0,(struct sockaddr*)&DestAddr,&nAddrSize);
	if( m_ioBuffer->m_nLenTrans > 0 )
	{
		m_ioBuffer->m_nDataLen = m_ioBuffer->m_nLenTrans ;

	}

	m_ioBuffer->m_AddrRemote.nPort = ntohs(DestAddr.sin_port);
	strcpy(m_ioBuffer->m_AddrRemote.szAddr,(char *)inet_ntoa(DestAddr.sin_addr));

	m_pNetAsync->OnReadCompleted( m_ioBuffer );
	m_ioBuffer->ReleaseObj();
	m_ioBuffer = NULL ;

	return 0;
}

BOOL CNetAsyncUDP::Send(CObjNetIOBufferUDP *pBuffer)
{
	if( m_socListen == INVALID_RDE )
	{
		return FALSE ;
	}
	struct    sockaddr_in DestAddr;

	DestAddr.sin_family=AF_INET;
	DestAddr.sin_port=htons((unsigned short)pBuffer->m_AddrRemote.nPort);
	DestAddr.sin_addr.s_addr=inet_addr(pBuffer->m_AddrRemote.szAddr); 
	int ret = sendto(m_socListen,
		(char *)pBuffer->m_pBuf,
		pBuffer->m_nDataLen,0,
		(struct    sockaddr*)&DestAddr,sizeof(DestAddr));

	if( ret == pBuffer->m_nDataLen )
	{
		pBuffer->ReleaseObj();
		return TRUE ;
	}
	else
	{
		return FALSE ;
	}
}
















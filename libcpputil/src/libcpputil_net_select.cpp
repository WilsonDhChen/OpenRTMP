
#include "libcpputil_def.h"

#include "libcpputil_net_select.h"

#if defined(WIN32)
#include <ws2tcpip.h>
#endif

_CPP_UTIL_DYNAMIC_IMP(CNetAsyncSelect)
_CPP_UTIL_CLASSNAME_IMP(CNetAsyncSelect)
_CPP_UTIL_QUERYOBJ_IMP(CNetAsyncSelect,CObj)

CNetAsyncSelect::CNetAsyncSelect()
{
	m_nStackSize = 0 ;
	m_nWorkerThreadsCount = 1 ;

	m_socListenIp6 = INVALID_RDE;
	m_socListen = INVALID_RDE ;
	
#ifdef WIN32
	m_hThreadSelect = NULL ;
#else
	m_nThreadIDSelect = 0 ;
#endif

	m_bShutDown = FALSE ;
	MtxInit(&m_mtxListContext, 0);
    MtxInit(&m_mtxThreadCtx, 0);
    
	m_szName[0] = 0 ;
}
CNetAsyncSelect::~CNetAsyncSelect()
{
    Shutdown();
	if( m_socListen != INVALID_RDE)
	{
		libcpputil_close_soc(m_socListen);
		m_socListen = INVALID_RDE;
	}

	if (m_socListenIp6 != INVALID_RDE)
	{
		libcpputil_close_soc(m_socListenIp6);
		m_socListenIp6 = INVALID_RDE;
	}

	FreeAllConnContext();

	MtxDestroy(&m_mtxListContext);
    MtxDestroy(&m_mtxThreadCtx);
    
}


void CNetAsyncSelect::FreeAllConnContext()
{


	LockContextList();
	CObj * obj ;

	while( ( obj = m_listContext.ListRemoveHead() ) != NULL )
	{
		obj->ReleaseObj() ;
	}
	UnlockContextList();
}
void CNetAsyncSelect::LockContextList()
{
	MtxLock( &m_mtxListContext );
}
void CNetAsyncSelect::UnlockContextList()
{
	MtxUnLock( &m_mtxListContext );
}
BOOL CNetAsyncSelect::AddSocket2Asyn(CObjConnContext *pContext)
{
    if( pContext != NULL )
    {
        pContext->m_net = m_pNetAsync->m_pNetAsync;
        pContext->m_bClosing = FALSE ;
        pContext->m_tmConnected = pContext->m_tmLastActive = libcpputil_UpSeconds() ;
    }
    
    if( pContext->m_AddrRemote.szAddr[0] == 0 )
    {
		libcpputil_GetPeerName(pContext->m_soc, &(pContext->m_AddrRemote.nPort), pContext->m_AddrRemote.szAddr);
    }
    
    if( pContext->m_AddrLocal.szAddr[0] == 0 )
    {
		libcpputil_GetBindName(pContext->m_soc, &(pContext->m_AddrLocal.nPort), pContext->m_AddrLocal.szAddr);
    }
    
    if( m_pNetAsync->OnNewConnectionIncoming(pContext)  == -1 )
    {
        return  FALSE ;
    }

	return AddConnContext(pContext) ;
}
BOOL CNetAsyncSelect::BindAddr(int nPort ,const char * szBindAddr )
{
	if( m_socListen != INVALID_RDE )
	{
		return FALSE ;
	}
	m_socListen = libcpputil_bind(szBindAddr,nPort,TRUE) ;
	if( m_socListen == INVALID_RDE )
	{
		return FALSE ;
	}

	return TRUE;
}
BOOL CNetAsyncSelect::BindAddrIp6(int nPort, const char * szBindAddr)
{
	if (m_socListenIp6 != INVALID_RDE)
	{
		return FALSE;
	}
	m_socListenIp6 = libcpputil_bindIp6(szBindAddr, nPort, TRUE);
	if (m_socListenIp6 == INVALID_RDE)
	{
		return FALSE;
	}

	return TRUE;
}
CObj *CNetAsyncSelect::GetThreadContext()
{
    CMtxAutoLock lock(&m_mtxThreadCtx);
    if (m_threadCtx.p != NULL)
    {
        m_threadCtx->AddObjRef();
    }
    return m_threadCtx.p;
}
int CNetAsyncSelect::GetConnections( )
{
    return m_listContext.ListItemsCount();
}
void CNetAsyncSelect::CloseAllConnections()
{
	LockContextList();


	CObj * const pItemHead = &m_listContext;
	CObjConnContext *pItemTmp = NULL;
	CObj *pItem = NULL;

	pItem = pItemHead->m_pNextObj ; 

	while( pItem != NULL && pItem != pItemHead )
	{
		pItemTmp = (CObjConnContext *)pItem ;
		pItem = pItem->m_pNextObj ;
		// pItemTmp->Lock();
		pItemTmp->m_bClosing = TRUE ;
		if( pItemTmp->m_soc != INVALID_RDE )
		{
			shutdown(pItemTmp->m_soc,SD_BOTH);
		}
		// pItemTmp->Unlock();

	}
	UnlockContextList();
}
void CNetAsyncSelect::CloseConnection(CObjConnContext *pContext)
{

	if( pContext == NULL || pContext->m_bClosing )
	{
		return ;
	}
	//pContext->Lock() ;

	pContext->m_bClosing = TRUE ;
	if( pContext->m_soc != INVALID_RDE )
	{
		shutdown(pContext->m_soc,SD_BOTH);
	}
	//pContext->Unlock() ;
}
BOOL CNetAsyncSelect::Connect(const char *host, int port, CObj *par)
{

	struct addrinfo hints;
	struct addrinfo *res = NULL, *cur = NULL;
	int nRet = 0;
	SOCKET soc = INVALID_RDE;


	CObjConnContext *pClientConnect = NULL;
	BOOL bTrigerClose = FALSE;

	memset(&hints, 0, sizeof(struct addrinfo));
#if 1
	hints.ai_family = 0; /* Allow IPv4 */
	hints.ai_flags = AI_PASSIVE; /* For wildcard IP address */
	hints.ai_protocol = 0; /* Any protocol */
	hints.ai_socktype = SOCK_STREAM;
#endif
	nRet = getaddrinfo(host, NULL, &hints, &res);
	if (nRet != 0 || res == NULL)
	{
		return FALSE;
	}

	soc = socket(res->ai_family, SOCK_STREAM, 0);
	if (soc == INVALID_RDE)
	{
		goto LABEL_ERROR;
	}

	libcpputil_setblocking(soc, FALSE );








	pClientConnect = m_pNetAsync->AllocConnContext(par);
	if (pClientConnect != NULL)
	{
        pClientConnect->m_net = m_pNetAsync->m_pNetAsync;
		pClientConnect->m_soc = soc;
		pClientConnect->m_bConnecting = TRUE;
		soc = INVALID_RDE;
	}

	if (pClientConnect == NULL || !pClientConnect->InitLater())
	{
		goto LABEL_ERROR;
	}


	if (res->ai_family == AF_INET6)
	{
		struct sockaddr_in6 *addr = (struct sockaddr_in6 *)res->ai_addr;
		addr->sin6_port = htons(port);

		nRet = connect(pClientConnect->m_soc, (struct sockaddr*)addr, sizeof(struct sockaddr_in6));

	}
	else if (res->ai_family == AF_INET)
	{
		struct sockaddr_in *addr = (struct sockaddr_in *)res->ai_addr;
		addr->sin_port = htons(port);

		nRet = connect(pClientConnect->m_soc, (struct sockaddr*)addr, sizeof(struct sockaddr_in));
	}
	else
	{
		goto LABEL_ERROR;
	}

	if (0 != nRet)
	{
#if defined(WIN32)
		nRet = WSAGetLastError();
		if (nRet != ERROR_IO_PENDING && nRet != WSAEWOULDBLOCK )
		{
			goto LABEL_ERROR;
		}
#else
		if (errno != EINPROGRESS)
		{
			goto LABEL_ERROR;
		}
#endif
	}
	nRet = libcpputil_GetBindName(pClientConnect->m_soc, &(pClientConnect->m_AddrLocal.nPort), pClientConnect->m_AddrLocal.szAddr);
	if (m_pNetAsync->OnNewConnectionOutgoing(pClientConnect) == -1)
	{
		goto LABEL_ERROR;
	}

	pClientConnect->m_tmLastActive = pClientConnect->m_tmConnected
		= pClientConnect->m_tmLastRead = pClientConnect->m_tmLastWrite
		= libcpputil_UpSeconds();

	if (!AddConnContext(pClientConnect))
	{
		m_pNetAsync->OnMaxConnectionsLimited(pClientConnect);
		CloseConnection(pClientConnect);
		pClientConnect->Lock();
		if (!pClientConnect->m_bTriggeredClosedEvent)
		{
			pClientConnect->m_bTriggeredClosedEvent = TRUE;
			bTrigerClose = TRUE;
		}
		else
		{
			bTrigerClose = FALSE;
		}
		pClientConnect->Unlock();

		if (bTrigerClose)
		{
			m_pNetAsync->OnConnectionClosed(pClientConnect);
			pClientConnect->OnClose();
		}
		goto LABEL_ERROR;
	}
	freeaddrinfo(res);
	return TRUE;
LABEL_ERROR:;

	if (soc != INVALID_RDE)
	{
		closesocket(soc);
	}
	if (pClientConnect != NULL)
	{
		pClientConnect->ReleaseObj();
	}

	freeaddrinfo(res);
	return FALSE;
}
int  CNetAsyncSelect::GetBindPort()
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
int  CNetAsyncSelect::GetBindPortIp6()
{
	int port = 0;
	if (m_socListenIp6 == INVALID_RDE)
	{
		return 0;
	}
	struct sockaddr_in6 sock_addr;
	int nRet;
	socklen_t size_addr;
	size_addr = sizeof(struct sockaddr);

	nRet = getsockname(m_socListenIp6, (struct sockaddr*)&sock_addr, &size_addr);
	if (nRet)
	{
		return 0;
	}
	port = ntohs(sock_addr.sin6_port);

	return port;
}
BOOL CNetAsyncSelect::Send(CObjConnContext *pContext, CObjNetIOBuffer *pBuffer)
{

	int errorNo = 0 ;
	if( pContext == NULL || pContext->m_bClosing  || pBuffer == NULL || 
		pBuffer->m_pBuf == NULL || pContext->m_soc == INVALID_RDE )
	{
		return FALSE ;
	}
	if( pContext->m_listSendBufs.ListItemsCount() > m_pNetAsync->m_nMaxSends )
	{
		m_pNetAsync->OnError(pContext, "too more MaxSends");
		pContext->m_bClosing = TRUE;
		return FALSE ;
	}
	pBuffer->m_nLenTrans = 0 ;
	pBuffer->m_opType = CObjNetIOBuffer::IOOpTypeWrite ;

	// _DBG_PRNT("Send\n");
	BOOL bEmpty = FALSE ;
	int nRet = 0 ;
	BOOL bError = FALSE ;
	BOOL bActived = FALSE ;
	pContext->Lock();
	if (m_pNetAsync->m_nMaxWrittingBytes != -1 && pContext->m_nWrittingBytes >= m_pNetAsync->m_nMaxWrittingBytes)
	{
		char szMsg[200];
		snprintf(szMsg, sizeof(szMsg) - 1, "too more WrittingBytes, WrittingBytes(%u)>=MaxWrittingBytes(%d)",
			pContext->m_nWrittingBytes, m_pNetAsync->m_nMaxWrittingBytes);

		m_pNetAsync->OnError(pContext, szMsg);
		pContext->Unlock();
		return FALSE;
	}

	pContext->m_nWrittingBytes += pBuffer->m_nDataLen;
	bEmpty =   pContext->m_listSendBufs.ListIsEmpty() ;
	pContext->m_listSendBufs.ListAddTail(pBuffer);


	if( bEmpty )
	{

		nRet = libcpputil_loop_send(pContext->m_soc,pBuffer->m_pBuf+pBuffer->m_nLenTrans,pBuffer->m_nDataLen-pBuffer->m_nLenTrans , &bError );
		if( nRet > 0 )
		{
			pContext->m_nWrittingBytes -= nRet;
			pBuffer->m_nLenTrans += nRet ;
			if( pContext->m_nWrittingBytes < 0 )
			{
				LogPrint(_LOG_LEVEL_ERROR, _TAGNAME, "%s:%d system bug (m_nWrittingBytes < 0)\n", __FILE__ , __LINE__ );
				pContext->m_nWrittingBytes = 0 ;
			}
			bActived = TRUE ;
		}
		else 
		{
#ifdef WIN32
			errorNo = WSAGetLastError();
			if( errorNo != WSAEWOULDBLOCK )
			{
				//printf("CNetAsyncSelect::Send errno != EWOULDBLOCK 0x%X\n", errorNo);
				bError = TRUE ;
			}
#else
			if( errno != EWOULDBLOCK )
			{
				//printf("CNetAsyncSelect::Send errno != EWOULDBLOCK\n");
				bError = TRUE ;
			}
#endif
		}
	}
	pContext->Unlock() ;


	if( bError )
	{
		pBuffer->AddObjRef();
		CloseConnection(pContext); 
		return FALSE ;
	}
	else
	{
		if( bActived )
		{
			pContext->m_tmLastActive = pContext->m_tmLastWrite = libcpputil_UpSeconds();
			if( m_pNetAsync->m_SessionTimeoutType == CObjNetAsync::SessionTimeoutWrite 
				||m_pNetAsync->m_SessionTimeoutType == CObjNetAsync::SessionTimeoutAll )
			{
				MoveConnContext2Tail( pContext );
			}
			
		}
		return TRUE ;
	}


	
}

int CNetAsyncSelect::ThreadWorker()
{
    {
        CMtxAutoLock lock(&m_mtxThreadCtx);
        m_threadCtx.Attach(m_pNetAsync->CreateThreadContext());
    }
    

	int  ret = 0 ;
	int  retHandle = 0 ;
	CObjConnContext conn4;
	CObjConnContext conn6;

#if defined(_NET_SELECT_POLL)
	#if !defined(_MAX_CTXS_)
	#define  _MAX_CTXS_  5000
	#endif
	struct pollfd pollfds[_MAX_CTXS_];
	CObjConnContext *ctxsPoll[_MAX_CTXS_];
	int poll_i = 0;
#else
	#define  _MAX_CTXS_   ( FD_SETSIZE + 4 ) 
	fd_set  fdsetRead;
	fd_set  fdsetWrite;
	fd_set  fdsetError;
#endif

	SOCKET maxfd = 0 ;
	struct timeval timeout;
	struct timeval timeoutCpy;
	BOOL bRemoved = FALSE ;

	CObj * const pItemHead = &m_listContext;
	CObjConnContext *pItemTmp = NULL;
	CObj *pItem = NULL;


	CObjConnContext *ctxs[_MAX_CTXS_];
	int nCtxs = 0;
	int i = 0;
	int nSockets = 0 ;

	memset( &timeout, 0 , sizeof(timeout) ) ;
	memset( &timeoutCpy, 0 , sizeof(timeoutCpy) ) ;

#if !defined(_NET_SELECT_POLL)
	memset( &fdsetRead, 0 , sizeof(fdsetRead) ) ;
	memset( &fdsetWrite, 0 , sizeof(fdsetWrite) ) ;
	memset( &fdsetError, 0 , sizeof(fdsetError) ) ;
#endif

	timeoutCpy.tv_usec = 50000 ;

	memcpy( &timeout , &timeoutCpy , sizeof(timeout) );

#if defined(SIGPIPE) ||  ! defined(WIN32)
    signal(SIGPIPE,SIG_IGN);
#endif
	
#if defined(_DEBUG_) || defined(_DEBUG)
	printf("CNetAsyncSelect::ThreadWorker\n");
#endif
	while ( ! m_bShutDown )
	{
		nSockets = 0 ;
#if defined(_NET_SELECT_POLL)
		if (m_socListen != INVALID_RDE)
		{
			conn4.m_soc = m_socListen;
			pollfds[nSockets].fd = m_socListen ;
			pollfds[nSockets].events = POLLIN|POLLPRI|POLLERR|POLLHUP|POLLNVAL ;
			pollfds[nSockets].revents = 0 ;
			ctxsPoll[nSockets] = &conn4;
			nSockets++;
		}
		if (m_socListenIp6 != INVALID_RDE)
		{
			conn6.m_soc = m_socListenIp6;
			pollfds[nSockets].fd = m_socListenIp6;
			pollfds[nSockets].events = POLLIN | POLLPRI | POLLERR | POLLHUP | POLLNVAL;
			pollfds[nSockets].revents = 0;
			ctxsPoll[nSockets] = &conn6;
			nSockets++;
		}
#else
		FD_ZERO( &fdsetRead );
		FD_ZERO( &fdsetWrite );
		FD_ZERO( &fdsetError );
		if (m_socListen != INVALID_RDE)
		{
			FD_SET(m_socListen, &fdsetRead);
			maxfd = m_socListen;
			nSockets++;
		}
		if (m_socListenIp6 != INVALID_RDE)
		{
			FD_SET(m_socListenIp6, &fdsetRead);
			if (m_socListenIp6 > maxfd)
			{
				maxfd = m_socListenIp6;
			}
			
			nSockets++;
		}
#endif

		
		//SleepMilli(1);


		{
			LockContextList();
			pItemTmp = NULL ;
			pItem = pItemHead->m_pNextObj ; 

			while( pItem != NULL && pItem != pItemHead )
			{
				pItemTmp = (CObjConnContext *)pItem ;
				pItem = pItem->m_pNextObj ;

				if( pItemTmp->m_bClosing )
				{
					
					bRemoved = RemoveConnContext(pItemTmp);
					
					if( !pItemTmp->m_bTriggeredClosedEvent )
					{

						m_pNetAsync->OnConnectionClosed(pItemTmp);
						pItemTmp->m_bTriggeredClosedEvent = TRUE ;
						pItemTmp->OnClose();
					}
					libcpputil_close_ctx(  pItemTmp );
					if( bRemoved )
					{
						pItemTmp->ReleaseObj();
					}
					
					

					continue ;
				}
				if( pItemTmp->m_soc == INVALID_RDE )
				{
					LogPrint(_LOG_LEVEL_ERROR, _TAGNAME, "%s %d: CNetAsyncSelect::ThreadWorker system bug\n", __FUNCTION__, __LINE__);
					continue ;
				}
#if defined(_NET_SELECT_POLL)
				pollfds[nSockets].fd = pItemTmp->m_soc ;
				pollfds[nSockets].events = POLLIN | POLLPRI  | POLLERR | POLLHUP | POLLNVAL;
				if( pItemTmp->m_listSendBufs.ListItemsCount() > 0 )
				{
					pollfds[nSockets].events |= POLLOUT ;
				}
				pollfds[nSockets].revents = 0;
				ctxsPoll[nSockets] = pItemTmp;
				nSockets++;
				if (nSockets > (_MAX_CTXS_ - 2))
				{
					break;
				}
#else

				FD_SET( pItemTmp->m_soc, &fdsetRead );
				FD_SET( pItemTmp->m_soc, &fdsetError );

				if (pItemTmp->m_listSendBufs.ListItemsCount() > 0 || pItemTmp->m_bConnecting)
				{
					FD_SET( pItemTmp->m_soc, &fdsetWrite );
				}
				if( pItemTmp->m_soc > maxfd )
				{
					maxfd = pItemTmp->m_soc ;
				}
				nSockets ++ ;
				if( nSockets > ( FD_SETSIZE - 2 ) )
				{
					break ;
				}
#endif
		

			}
			UnlockContextList();
		}


		if( nSockets < 1 )
		{
			SleepMilli(100);
			continue ;
		}
		memcpy( &timeout , &timeoutCpy , sizeof(timeout) );
#if defined(_NET_SELECT_POLL)
		ret = poll(pollfds,nSockets,50 );
#else
		ret = select( maxfd + 1 ,&fdsetRead, &fdsetWrite, &fdsetError , &timeout ) ;
#endif

		if( ret == -1 )
		{
			SleepMilli(100);
			//LogPrint(_LOG_LEVEL_DEBUG,_TAGNAME, "%s %d: select = %d\n" , __FUNCTION__ , __LINE__  , ret );
			continue ;
		}
		else if( ret == 0 )
		{
			HandleEventWaitTimeout( 0 );
			HandleSessionTimeout() ;

#if defined(_DEBUG_) || defined(_DEBUG)
	//		printf("CNetAsyncSelect::ThreadWorker select timeout\n");
#endif
			continue ;
		}
		else
		{
#if defined(_NET_SELECT_POLL)

#else
			if( m_socListen != INVALID_RDE && FD_ISSET(m_socListen,&fdsetRead) )
			{
				HandleEventAccept( NULL, FALSE );
			}
			if (m_socListenIp6 != INVALID_RDE && FD_ISSET(m_socListenIp6, &fdsetRead))
			{
				HandleEventAccept(NULL,TRUE);
			}
#endif
			//else
			{
				LockContextList();


				CObjConnContext *pConn = NULL;
				int bActiveRead = FALSE ;
				int bActiveWrite = FALSE ;

				nCtxs = 0;
#if defined(_NET_SELECT_POLL)
				for (poll_i = 0; poll_i < nSockets; poll_i ++ )
				{
					pConn = ctxsPoll[poll_i];
					if (pConn == NULL)
					{
						continue;
					}
#else
				pItem = pItemHead->m_pNextObj ; 

				while( pItem != NULL && pItem != pItemHead )
				{
					pConn = (CObjConnContext *)pItem ;
					pItem = pItem->m_pNextObj ;
#endif

					bActiveRead = FALSE ;
					bActiveWrite = FALSE ;
					if( pConn->m_bClosing )
					{
						goto LABEL_REMOVE ;
					}
					if( pConn->m_soc == INVALID_RDE )
					{
						goto LABEL_REMOVE ;
					}

					retHandle = 0 ;
#if defined(_NET_SELECT_POLL)
					if (pollfds[poll_i].revents&POLLIN || pollfds[poll_i].revents&POLLPRI )
#else
					if( FD_ISSET( pConn->m_soc, &fdsetRead ) )
#endif
					{
#if defined(_NET_SELECT_POLL)
						if (pConn->m_soc == m_socListen )
						{
							HandleEventAccept(NULL, FALSE);
							continue ;
						}
						if (pConn->m_soc == m_socListenIp6 )
						{
							HandleEventAccept(NULL, TRUE);
							continue;
						}
#endif
						if (pConn->m_bConnecting)
						{
							retHandle = HandleEventConnected(pConn, bRemoved);
						}
						else
						{
							retHandle = HandleEventRead(pConn, bRemoved);
						}
						
						if( bRemoved )
						{
							continue ;
						}
						if ( retHandle > 0 )
						{
							bActiveRead = TRUE ;
						}

						
					}
#if defined(_NET_SELECT_POLL)	
					if (pollfds[poll_i].revents&POLLERR || pollfds[poll_i].revents&POLLHUP || pollfds[poll_i].revents&POLLNVAL  )		
#else
					if( pConn->m_bClosing || FD_ISSET( pConn->m_soc, &fdsetError )  )
#endif
					{

					LABEL_REMOVE: ;
						pConn->m_bClosing = TRUE ;
						bActiveRead = FALSE ;
						bActiveWrite = FALSE ;
						
						bRemoved = RemoveConnContext(pConn);
						
						if( !pConn->m_bTriggeredClosedEvent )
						{

							m_pNetAsync->OnConnectionClosed(pConn);
							pConn->m_bTriggeredClosedEvent = TRUE ;
							pConn->OnClose();

						}
						libcpputil_close_ctx(  pConn );
						
						if( bRemoved )
						{
							pConn->ReleaseObj();
						}
						
						continue ;
					}
#if defined(_NET_SELECT_POLL)
					if (pollfds[poll_i].revents&POLLOUT)
#else
					if ( FD_ISSET( pConn->m_soc, &fdsetWrite) )
#endif
					{
						if (pConn->m_bConnecting)
						{
							retHandle = HandleEventConnected(pConn, bRemoved);
						}
						else
						{
							retHandle = HandleEventWrite(pConn, bRemoved);

						}

						if (bRemoved)
						{
							continue;
						}

						if ( retHandle > 0 )
						{
							bActiveWrite = TRUE ;
						}

					}

					if( (bActiveWrite || bActiveRead) && m_pNetAsync->m_SessionTimeoutType == CObjNetAsync::SessionTimeoutAll )
					{
						ctxs[nCtxs] = pConn ;
						nCtxs ++ ;
					}
					else if(  bActiveRead  && m_pNetAsync->m_SessionTimeoutType == CObjNetAsync::SessionTimeoutRead )
					{
						ctxs[nCtxs] = pConn ;
						nCtxs ++ ;
					}
					else if(  bActiveWrite  && m_pNetAsync->m_SessionTimeoutType == CObjNetAsync::SessionTimeoutWrite )
					{
						ctxs[nCtxs] = pConn ;
						nCtxs ++ ;
					}


				}
				if( nCtxs != m_listContext.ListItemsCount() )
				{
					for( i = 0 ; i < nCtxs ; i ++ )
					{
						MoveConnContext2Tail( ctxs[i] );
					}
				}

				UnlockContextList();
			}
		}

		HandleSessionTimeout() ;
		
	}



	LockContextList();


	pItemTmp = NULL;
	pItem = pItemHead->m_pNextObj ; 

	while( pItem != NULL && pItem != pItemHead  )
	{
		pItemTmp = (CObjConnContext *)pItem ;
		pItem = pItem->m_pNextObj ;

	
        pItemTmp->m_bClosing = TRUE ;
		bRemoved = RemoveConnContext(pItemTmp);
		if( !pItemTmp->m_bTriggeredClosedEvent )
		{

			m_pNetAsync->OnConnectionClosed(pItemTmp);
			pItemTmp->m_bTriggeredClosedEvent = TRUE ;
			pItemTmp->OnClose();

		}
		libcpputil_close_ctx(  pItemTmp );
		if( bRemoved )
		{
			pItemTmp->ReleaseObj();
		}
	}

	UnlockContextList();

	m_bShutDown = FALSE; 


    {
        CMtxAutoLock lock(&m_mtxThreadCtx);
        m_threadCtx = NULL;
    }

	
	return 0 ;
}
int CNetAsyncSelect::HandleEventConnected(CObjConnContext *pConn, BOOL &bRemoved)
{
	int nRet = 0;
	pConn->m_bConnecting = FALSE;
	nRet = libcpputil_GetPeerName(pConn->m_soc, &(pConn->m_AddrRemote.nPort), pConn->m_AddrRemote.szAddr);
	if (nRet != 0 || m_pNetAsync->OnConnected(pConn) == -1)
	{
		bRemoved = RemoveConnContext(pConn);
		if (!pConn->m_bTriggeredClosedEvent)
		{
			m_pNetAsync->OnConnectionClosed(pConn);
			pConn->m_bTriggeredClosedEvent = TRUE;
			pConn->OnClose();
		}
		libcpputil_close_ctx(pConn);

		if (bRemoved)
		{
			pConn->ReleaseObj();
		}

		return 0;
	}
	else
	{
		return 1;
	}
	
}
int CNetAsyncSelect::HandleEventWrite(CObjConnContext *pConn,BOOL &bRemoved)
{

	int  nRefCount = 0 ;
	BOOL bActive = FALSE ;
	int  nRet = 0 ;
	bRemoved = FALSE ;
	pConn->Lock();
	//LogPrint(_LOG_LEVEL_DEBUG,_TAGNAME, "%s %d: m_listSendBufs=%d\n" , __FUNCTION__ , __LINE__ ,pConn->m_listSendBufs.ListItemsCount() );

	
	if( pConn->m_listSendBufs.ListIsEmpty() )
	{
		pConn->Unlock();
		return nRet ;
	}
	CObjNetIOBuffer *pBuffer = NULL ;
	int nRemain = 0 ;
	BOOL bError = 0 ;


LABEL_LOOP:

	pBuffer = (CObjNetIOBuffer *)pConn->m_listSendBufs.ListRemoveHead() ;

	if( pBuffer == NULL )
	{
		if( bActive )
		{
			pConn->m_tmLastActive = pConn->m_tmLastWrite = libcpputil_UpSeconds();
			nRet = 1 ;
		}
		pConn->Unlock() ;   
		return nRet ;
	}
	nRemain = pBuffer->m_nDataLen -  pBuffer->m_nLenTrans ;


	if( nRemain == 0 )
	{


		m_pNetAsync->OnWriteCompleted(pConn,pBuffer);
		nRefCount = pBuffer->ReleaseObj();
		//LogPrint(_LOG_LEVEL_DEBUG,_TAGNAME, "%s %d: pBuffer->ReleaseObj() = %d\n" , __FUNCTION__ , __LINE__ , nRefCount );

		if( pConn->m_bClosing || pConn->m_soc == INVALID_RDE )
		{
			if( bActive )
			{
				pConn->m_tmLastActive = pConn->m_tmLastWrite = libcpputil_UpSeconds();
				nRet = 1 ;
			}
			pConn->Unlock() ;

			return nRet ;
		}

		goto   LABEL_LOOP ;
	}
	else 
	{

		nRet = libcpputil_loop_send(pConn->m_soc,pBuffer->m_pBuf + pBuffer->m_nLenTrans , nRemain ,&bError ) ;
		if( bError )
		{
			pConn->Unlock();
			bRemoved = RemoveConnContext(pConn);
			if( !pConn->m_bTriggeredClosedEvent )
			{
				m_pNetAsync->OnConnectionClosed(pConn);
				pConn->m_bTriggeredClosedEvent = TRUE ;
				pConn->OnClose();
			}
			libcpputil_close_ctx(  pConn );

			if( bRemoved )
			{
				pConn->ReleaseObj() ;
			}
			return -1 ;
		}
		if( nRet > 0 )
		{
			pConn->m_nWrittingBytes -= nRet;
			pBuffer->m_nLenTrans += nRet ;

			if( pConn->m_nWrittingBytes < 0 )
			{
				LogPrint(_LOG_LEVEL_ERROR, _TAGNAME, "%s:%d system bug (m_nWrittingBytes < 0)\n", __FILE__ , __LINE__ );
				pConn->m_nWrittingBytes = 0 ;
			}

			bActive = TRUE ;
		}



		nRemain = pBuffer->m_nDataLen -  pBuffer->m_nLenTrans ;
		if( nRemain == 0 )
		{

			m_pNetAsync->OnWriteCompleted(pConn,pBuffer);
			nRefCount = pBuffer->ReleaseObj();
			//LogPrint(_LOG_LEVEL_DEBUG,_TAGNAME, "%s %d: pBuffer->ReleaseObj() = %d\n" , __FUNCTION__ , __LINE__ , nRefCount );
			if( pConn->m_bClosing || pConn->m_soc == INVALID_RDE )
			{
				if( bActive )
				{
					pConn->m_tmLastActive = pConn->m_tmLastWrite = libcpputil_UpSeconds();
					nRet = 1 ;
				}
				pConn->Unlock() ;

				return nRet ;
			}
			goto   LABEL_LOOP ;
		}

	}


	if( pBuffer == NULL )
	{
		if( bActive )
		{
			pConn->m_tmLastActive = pConn->m_tmLastWrite = libcpputil_UpSeconds();
			nRet = 1 ;
		}
		pConn->Unlock() ;

		return 0 ;
	}



	pConn->m_listSendBufs.ListAddHead(pBuffer);

	if( bActive )
	{
		pConn->m_tmLastActive = pConn->m_tmLastWrite = libcpputil_UpSeconds();
		nRet = 1 ;
	}

	pConn->Unlock() ;

	return 0 ;
}
int CNetAsyncSelect::HandleEventAccept(CObjConnContext *pConn, BOOL ip6)
{

	BOOL bRemoved = FALSE ;
	//LogPrint(_LOG_LEVEL_DEBUG,_TAGNAME, "%s %d: Entry\n" , __FUNCTION__ , __LINE__  );
	if( m_bShutDown )
	{
		return 0 ;
	}

	char szBuf[sizeof(struct sockaddr_in) + sizeof(struct sockaddr_in6)];
	struct sockaddr_in *sock_addr = (struct sockaddr_in *)szBuf;
	struct sockaddr_in6 *sock_addr6 = (struct sockaddr_in6 *)szBuf;
	struct sockaddr *socaddr = (struct sockaddr *)szBuf;

	socklen_t size_addr = sizeof(szBuf);
	SOCKET soc;
	

	if (ip6)
	{
		soc = accept(m_socListenIp6,
			socaddr,
			&size_addr);
	}
	else
	{
		soc = accept(m_socListen,
			socaddr,
			&size_addr);
	}


	if(soc==INVALID_RDE)
	{
#ifdef WIN32
		LogPrint(_LOG_LEVEL_ERROR,_TAGNAME, "%s %d: accept error\n" , __FUNCTION__ , __LINE__  );
#else
		LogPrint(_LOG_LEVEL_ERROR,_TAGNAME, "%s %d: accept error(%d) %s\n" , __FUNCTION__ , __LINE__  , errno , strerror(errno) );
#endif
		return 0;
	}


	libcpputil_setblocking(soc , FALSE );



	CObjConnContext * pClient = m_pNetAsync->AllocConnContext(NULL);
	if( pClient == NULL )
	{
		libcpputil_close_soc(soc);
		LogPrint(_LOG_LEVEL_ERROR,_TAGNAME, "%s %d: AllocConnContext error\n" , __FUNCTION__ , __LINE__  );
		return 0;
	}
    pClient->m_net = m_pNetAsync->m_pNetAsync;
	if( !pClient->InitLater() )
	{
		libcpputil_close_soc(soc);
		pClient->ReleaseObj();
		LogPrint(_LOG_LEVEL_ERROR,_TAGNAME, "%s %d: InitLater error\n" , __FUNCTION__ , __LINE__  );
		return 0;
	}

	pClient->m_soc = soc ;

	libcpputil_GetBindName(soc, &(pClient->m_AddrLocal.nPort), pClient->m_AddrLocal.szAddr);

	if (socaddr->sa_family == AF_INET6)
	{
		pClient->m_AddrRemote.nPort = ntohs(sock_addr6->sin6_port);
		inet_ntop(AF_INET6, &sock_addr6->sin6_addr, pClient->m_AddrRemote.szAddr, sizeof(pClient->m_AddrRemote.szAddr) - 1);
	}
	else
	{
		pClient->m_AddrRemote.nPort = ntohs(sock_addr->sin_port);
		inet_ntop(AF_INET, &sock_addr->sin_addr, pClient->m_AddrRemote.szAddr, sizeof(pClient->m_AddrRemote.szAddr) - 1);

	}



	pClient->m_tmLastActive = pClient->m_tmConnected = libcpputil_UpSeconds();




	if (m_pNetAsync->OnNewConnectionIncoming(pClient) == -1)
	{
		libcpputil_close_ctx(pClient);
		pClient->ReleaseObj();
		return 0;

	}




	if( !AddConnContext(pClient) )
	{
		m_pNetAsync->OnMaxConnectionsLimited(pClient);
		libcpputil_close_ctx(pClient);
		pClient->ReleaseObj();
		LogPrint(_LOG_LEVEL_ERROR, _TAGNAME, "%s %d: AddConnContext error\n", __FUNCTION__, __LINE__);
	}



	return 0;
}
void CNetAsyncSelect::Shutdown()
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
	if (m_socListenIp6 != INVALID_RDE)
	{
		shutdown(m_socListenIp6, SD_BOTH);
	}
	CloseAllConnections();

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
	CloseAllConnections();

	pthread_join(m_nThreadIDSelect,&value);
	m_nThreadIDSelect = 0 ;
#endif
	if( m_socListen != INVALID_RDE )
	{
		RDE_Close(m_socListen);
		m_socListen= INVALID_RDE ;
	}

	if (m_socListenIp6 != INVALID_RDE)
	{
		RDE_Close(m_socListenIp6);
		m_socListenIp6 = INVALID_RDE;
	}

}
BOOL CNetAsyncSelect::Start()
{
#ifdef WIN32
	unsigned int threadID = 0 ;
	if( m_hThreadSelect != NULL )
	{
		return TRUE ;
	}
	m_hThreadSelect = (HANDLE)_beginthreadex( NULL, 0, &CNetAsyncSelect::_ThreadProcWorker, this, 0, &threadID );
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
	nRet = pthread_create((pthread_t*)&m_nThreadIDSelect,NULL,&CNetAsyncSelect::_ThreadProcWorker,(void *)this);
	if( nRet )
	{
		return FALSE ;
	}
	return TRUE ;
#endif
}



#ifdef WIN32
 unsigned int WINAPI CNetAsyncSelect::_ThreadProcWorker( void * inPar) 
#else
void *CNetAsyncSelect::_ThreadProcWorker( void * inPar) 
#endif
{
	
	CNetAsyncSelect *_this = (CNetAsyncSelect *)inPar ;
	if (_this->m_szName[0] != 0)
	{
		LogPrint(_LOG_LEVEL_INFO, _TAGNAME, "CObjNetAsync::_ThreadProcWorker started, %s\n", _this->m_szName);
	}
	
	_this->ThreadWorker();
	if (_this->m_szName[0] != 0)
	{
		LogPrint(_LOG_LEVEL_INFO, _TAGNAME, "CObjNetAsync::_ThreadProcWorker exited, %s\n", _this->m_szName);
	}
	
#ifdef WIN32
	_endthreadex( 0 );
#else
	 pthread_exit(0);
#endif

	return 0 ;
}

BOOL CNetAsyncSelect::AddConnContext( CObjConnContext *pContext)
{
	if( pContext == NULL ||  pContext->m_soc == INVALID_RDE )
	{
		return FALSE;
	}

	BOOL bOk = FALSE ;
	LockContextList();
	if(m_pNetAsync->m_nMaxConnections == -1 || m_listContext.ListItemsCount() < m_pNetAsync->m_nMaxConnections )
	{

		libcpputil_setblocking(pContext->m_soc , FALSE );
		if( pContext->m_AddrRemote.szAddr[0] == 0 )
		{
			libcpputil_GetPeerName(pContext->m_soc, &(pContext->m_AddrRemote.nPort), pContext->m_AddrRemote.szAddr);
		}

		if( pContext->m_AddrLocal.szAddr[0] == 0 )
		{
			libcpputil_GetBindName(pContext->m_soc, &(pContext->m_AddrLocal.nPort), pContext->m_AddrLocal.szAddr);
		}
		m_listContext.ListAddTail(pContext);
		bOk = TRUE ;
	}
	else
	{
		bOk = FALSE ;
	}
	UnlockContextList();
	return bOk ;
}
BOOL CNetAsyncSelect::RemoveConnContext( CObjConnContext *pContext)
{

	BOOL bOK = FALSE ;
	LockContextList();
	bOK = m_listContext.ListDel(pContext);
	UnlockContextList();
	return bOK ;
}
BOOL CNetAsyncSelect::MoveConnContext2Tail( CObjConnContext *pContext)
{
	if( pContext == NULL  )
	{
		return FALSE;
	}

	LockContextList();
	m_listContext.ListMoveTail(pContext);
	UnlockContextList();
	
	return TRUE ;
}
int CNetAsyncSelect::HandleEventRead(CObjConnContext *pConn,BOOL &bRemoved)
{

	int errorNo = 0;
	int nRet = 0 ;
	BOOL bActive = FALSE ;
	bRemoved = FALSE ;
	if( pConn->m_bClosing )
	{
		return -1 ;
	}
	



	int nReaden = 0 ;
	CObjNetIOBuffer *pBuffer = NULL ;
	pBuffer = m_pNetAsync->AllocReadIOBuffer();
	if( pBuffer == NULL )
	{
		return -1 ;
	}
	pBuffer->m_opType = CObjNetIOBuffer::IOOpTypeRead ;

	while( ( nReaden = recv(pConn->m_soc,pBuffer->m_pBuf,pBuffer->m_nBufLen,0) ) > 0 )
	{
		bActive = TRUE ;
		pBuffer->m_nDataLen = nReaden ;
		pBuffer->m_nLenTrans = nReaden ;
		nRet = m_pNetAsync->OnReadCompleted(pConn,pBuffer);
		pBuffer->ReleaseObj() ;
		if( nRet == -1 )
		{
			CloseConnection(pConn);
			return -1 ;
		}
		pBuffer = m_pNetAsync->AllocReadIOBuffer() ;
		if( pBuffer == NULL )
		{
			CloseConnection(pConn);
			return -1 ;
		}
		pBuffer->m_opType = CObjNetIOBuffer::IOOpTypeRead ;

		if( pConn->m_bClosing || pConn->m_soc == INVALID_RDE )
		{
			break ;
		}
		// pBuffer->ResetDataPtr();

	}
#ifdef WIN32
    errorNo = WSAGetLastError();
    if ((nReaden == 0) || (errorNo != WSAEWOULDBLOCK && nReaden < 0) || pConn->m_bClosing)
#else
    errorNo = errno ;
	if( (nReaden == 0  ) || (EWOULDBLOCK != errno && nReaden < 0 ) || pConn->m_bClosing )
#endif
	{
        
		pBuffer->m_nDataLen = 0 ;
		pBuffer->m_nLenTrans = 0 ;

		bRemoved = RemoveConnContext(pConn);
		libcpputil_close_ctx(  pConn );
		if( !pConn->m_bTriggeredClosedEvent )
		{
#ifndef WIN32
			errno = errorNo ;
#endif
			m_pNetAsync->OnConnectionClosed(pConn);
			pConn->m_bTriggeredClosedEvent = TRUE ;
			pConn->OnClose();
		}

		if( bRemoved )
		{
			pConn->ReleaseObj() ;
		}
		
		nRet = -1 ;
	}
	else
	{
		if( bActive )
		{
			pConn->m_tmLastActive =  pConn->m_tmLastRead = libcpputil_UpSeconds();
			nRet = 1 ;
		}
		else
		{
			nRet = 0 ;
		}
		
		
	}


	pBuffer->ReleaseObj();
	return nRet ;

}

int CNetAsyncSelect::HandleEventWaitTimeout(int nSeconds)
{
	// _DBG_PRNT("HandleEventWaitTimeout\n");
	if( m_pNetAsync->OnEventWaitTimeout(nSeconds) == -1 )
	{
		return 0 ;
	}
	return 0 ;
}
void CNetAsyncSelect::SetName(const char *szName)
{
	strcpyn( m_szName, szName , sizeof(m_szName));
}
int CNetAsyncSelect::HandleSessionTimeout()
{
	if( m_pNetAsync->m_nSessionTimeoutSeconds < 0 || m_listContext.ListIsEmpty() )
	{
		return 0;
	}
	LockContextList() ;

	CObj * const pItemHead = &m_listContext;
	CObjConnContext *pItemTmp = NULL;
	CObj *pItem = NULL;
	time_t tmNow = libcpputil_UpSeconds();
	BOOL bRelease = FALSE ;

	pItem = pItemHead->m_pNextObj ; 

	while( pItem != NULL && pItem != pItemHead )
	{
		pItemTmp = (CObjConnContext *)pItem ;
		pItem = pItem->m_pNextObj ;

		if (!pItemTmp->m_bClosing)
		{
			if (m_pNetAsync->m_SessionTimeoutType == CObjNetAsync::SessionTimeoutRead)
			{
				if ((tmNow - pItemTmp->m_tmLastRead) < m_pNetAsync->m_nSessionTimeoutSeconds)
				{
					break;
				}
			}
			else if (m_pNetAsync->m_SessionTimeoutType == CObjNetAsync::SessionTimeoutWrite)
			{
				if ((tmNow - pItemTmp->m_tmLastWrite) < m_pNetAsync->m_nSessionTimeoutSeconds)
				{
					break;
				}
			}
			else
			{
				if ((tmNow - pItemTmp->m_tmLastActive) < m_pNetAsync->m_nSessionTimeoutSeconds)
				{
					break;
				}
			}

			if (m_pNetAsync->OnSessionTimeout(pItemTmp) == -1)
			{
				continue ;
			}
		}

#if defined(_DEBUG_) || defined(_DEBUG)
		printf("Session timeout,closed! %s:%d\n", pItemTmp->m_AddrRemote.szAddr, pItemTmp->m_AddrRemote.nPort ) ;
#endif
		bRelease = TRUE ;
		pItemTmp->Lock();
		pItemTmp->m_bClosing = TRUE ;
		if( pItemTmp->m_soc != INVALID_RDE )
		{
			shutdown(pItemTmp->m_soc,SD_BOTH);
		}
		bRelease = m_listContext.ListDel(pItemTmp);
		pItemTmp->Unlock();

        if( !pItemTmp->m_bTriggeredClosedEvent )
        {
            pItemTmp->m_bTriggeredClosedEvent = TRUE ;
            m_pNetAsync->OnConnectionClosed(pItemTmp );
            pItemTmp->OnClose();
        }
        
		if( bRelease )
		{
			pItemTmp->ReleaseObj();
		}


	}

	UnlockContextList() ;
	return 0 ;
}







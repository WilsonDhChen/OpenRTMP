
#include "libcpputil_def.h"

#if defined(WIN32)
#include <ws2tcpip.h>
#endif

#if defined(WIN32)

HANDLE libcpputil_RemoveHandle(HANDLE *pArray, int nCount,int index2remove)
{
    HANDLE handle =  pArray[index2remove] ;
    if( handle == NULL )
    {
        return NULL ;
    }
    nCount -- ;

    for( ; index2remove < nCount  ; index2remove ++ )
    {
        pArray[index2remove]  = pArray[index2remove+1] ;
    }
    pArray[ nCount ] = NULL ;
    return handle ;
}
#endif

int  libcpputil_loop_send(SOCKET h_rde,const void *buffer,int size , BOOL *bError)
{
    const char *buf=(const char *)buffer;
    int i=0;
    int nRet;
    if( h_rde == INVALID_RDE )
    {
        return -1 ;
    }
	if( bError != NULL )
	{
		*bError = FALSE ;
	}

    while( (size-i) > 0 )
    {
        nRet = send( h_rde, (char *)buf+i, size-i, 0);
        if( nRet <= 0 )
        {
			if( bError != NULL )
			{
#ifdef WIN32
				*bError = ( WSAGetLastError() != WSAEWOULDBLOCK ) ;
#else
				*bError = (errno != EAGAIN );
#endif
			}

            if( i == 0 )
			{
				return nRet;
			}
            else 
			{
				return i;
			}
        }
        i+=nRet;
    }

    return  i;     
}

BOOL  libcpputil_setblocking(SOCKET sock ,BOOL bBlock)
{
	if( ! bBlock )
	{
#if defined(__WIN32__) || defined(_WIN32) || defined(IMN_PIM)
		unsigned long arg = 1;
		return ioctlsocket(sock, FIONBIO, &arg) == 0;
#elif defined(VXWORKS)
		int arg = 1;
		return ioctl(sock, FIONBIO, (int)&arg) == 0;
#else
		int curFlags = fcntl(sock, F_GETFL, 0);
		return fcntl(sock, F_SETFL, curFlags|O_NONBLOCK) >= 0;
#endif
	}
	else
	{
#if defined(__WIN32__) || defined(_WIN32) || defined(IMN_PIM)
		unsigned long arg = 0;
		return ioctlsocket(sock, FIONBIO, &arg) == 0;
#elif defined(VXWORKS)
		int arg = 0;
		return ioctl(sock, FIONBIO, (int)&arg) == 0;
#else
		int curFlags = fcntl(sock, F_GETFL, 0);
		return fcntl(sock, F_SETFL, curFlags&(~O_NONBLOCK)) >= 0;
#endif
	}

}

int  libcpputil_GetPeerName(SOCKET h_rde, int *PeerPort, char *szPeerIP)
{
	char szBuf[sizeof(struct sockaddr_in) + sizeof(struct sockaddr_in6)];
	struct sockaddr_in *sock_addr = (struct sockaddr_in *)szBuf;
	struct sockaddr_in6 *sock_addr6 = (struct sockaddr_in6 *)szBuf;
	struct sockaddr *socaddr = (struct sockaddr *)szBuf;
	int nRet = 0 ;
	socklen_t size_addr;
	if (h_rde == INVALID_RDE)
	{
		return -1;
	}
	size_addr = sizeof(szBuf);

	if (szPeerIP != NULL)
	{
		szPeerIP[0] = 0;
	}
	if (PeerPort != NULL)
	{
		*PeerPort = 0;
	}

	nRet = getpeername(h_rde, socaddr, &size_addr);
	if (nRet != 0)
	{
		return nRet;
	}
	if (socaddr->sa_family == AF_INET)
	{
		if (PeerPort != NULL)
		{
			*PeerPort = ntohs(sock_addr->sin_port);
		}

		if (szPeerIP != NULL)
		{
			inet_ntop(AF_INET, &sock_addr->sin_addr, szPeerIP, _INET_ADDR_STR_LEN-1);
		}
	}
	else if (socaddr->sa_family == AF_INET6)
	{
		if (PeerPort != NULL)
		{
			*PeerPort = ntohs(sock_addr6->sin6_port);
		}
		if (szPeerIP != NULL)
		{
			inet_ntop(AF_INET6, &sock_addr6->sin6_addr, szPeerIP, _INET_ADDR_STR_LEN-1);
		}
	}
	
	return 0;
}

int  libcpputil_GetBindName(SOCKET h_rde, int *PeerPort, char *szPeerIP)
{
	char szBuf[sizeof(struct sockaddr_in) + sizeof(struct sockaddr_in6)];
	struct sockaddr_in *sock_addr = (struct sockaddr_in *)szBuf;
	struct sockaddr_in6 *sock_addr6 = (struct sockaddr_in6 *)szBuf;
	struct sockaddr *socaddr = (struct sockaddr *)szBuf;
	int nRet = 0;
	socklen_t size_addr;
	if (h_rde == INVALID_RDE)
	{
		return -1;
	}
	size_addr = sizeof(szBuf);

	if (szPeerIP != NULL)
	{
		szPeerIP[0] = 0;
	}
	if (PeerPort != NULL)
	{
		*PeerPort = 0;
	}

	nRet = getsockname(h_rde, socaddr, &size_addr);
	if (nRet != 0)
	{
		return nRet;
	}
	if (socaddr->sa_family == AF_INET)
	{
		if (PeerPort != NULL)
		{
			*PeerPort = ntohs(sock_addr->sin_port);
		}

		if (szPeerIP != NULL)
		{
			inet_ntop(AF_INET, &sock_addr->sin_addr, szPeerIP, _INET_ADDR_STR_LEN-1);
		}
	}
	else if (socaddr->sa_family == AF_INET6)
	{
		if (PeerPort != NULL)
		{
			*PeerPort = ntohs(sock_addr6->sin6_port);
		}
		if (szPeerIP != NULL)
		{
			inet_ntop(AF_INET6, &sock_addr6->sin6_addr, szPeerIP, _INET_ADDR_STR_LEN-1);
		}
	}

	return 0;
}

SOCKET  libcpputil_bind( const char *szBindAddr,int nPort,BOOL bNoBlock)
{

    SOCKET SockOld;
    socklen_t LengthServerAddr;
    struct    sockaddr_in   ServerAddr;
    int ret;


    ServerAddr.sin_family=AF_INET;
    ServerAddr.sin_port=htons((unsigned short)nPort);

    SockOld=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(SockOld==INVALID_RDE)
    {
        return INVALID_RDE ;
    }
    if( bNoBlock )
    {
         if( !libcpputil_setblocking(SockOld , FALSE ) )
         {
             libcpputil_close_soc(SockOld);
             return INVALID_RDE ;
         }
    }
	if (szBindAddr == NULL ||
		szBindAddr[0] == 0
		|| strcmpix(szBindAddr, "*") == 0
		|| strcmpix(szBindAddr, "global") == 0
		|| strcmpix(szBindAddr, "any") == 0
		)
    {
        ServerAddr.sin_addr.s_addr=0;
    }
	else if (strcmpix(szBindAddr, "local") == 0 || strcmpix(szBindAddr, "localhost") == 0)
    {

        ServerAddr.sin_addr.s_addr=inet_addr("127.0.0.1");
    }		
    else
    {
		ServerAddr.sin_addr.s_addr = inet_addr(szBindAddr);
    }


    #ifndef WIN32
    ret=1;
    if(setsockopt(SockOld, SOL_SOCKET,SO_REUSEADDR,(const char *) &ret, sizeof(ret))<0)
    {
        
    }
    #endif

    ret=bind(SockOld,(struct sockaddr*)&ServerAddr,
        sizeof(ServerAddr));
    if(ret==SOCKET_ERROR)
    {
#ifdef WIN32
        ret = WSAGetLastError();
        libcpputil_close_soc( SockOld );
        WSASetLastError( ret );

#else
        ret= errno;
        libcpputil_close_soc( SockOld );
        errno = ret;
#endif
        

        return INVALID_RDE ;
    }
    LengthServerAddr=sizeof(ServerAddr);

    if(getsockname(SockOld,
        (struct sockaddr*)&ServerAddr,&LengthServerAddr)==SOCKET_ERROR)
    {    
        libcpputil_close_soc(SockOld);
        return INVALID_RDE ;    
        //////////////////////////////////
    }
    if( listen(SockOld, 2000) != 0 )
    {
#ifdef WIN32
        ret = WSAGetLastError( );
        libcpputil_close_soc( SockOld );
        WSASetLastError( ret );

#else
        ret = errno;
        libcpputil_close_soc( SockOld );
        errno = ret;
#endif


        return INVALID_RDE;
    }

    return SockOld;
}
SOCKET  libcpputil_bind_udp6(const char *szBindAddr, int nPort, BOOL bNoBlock)
{

    SOCKET SockOld=-1;
    struct    sockaddr_in6   ServerAddr;
    int ret;
#ifdef WIN32
    DWORD   dwBytesReturned = 0;
    BOOL     bNewBehavior = FALSE;
    DWORD   status;
#define   IOC_VENDOR   0x18000000   
#define   _WSAIOW(x,y)   (IOC_IN|(x)|(y))   
#define   SIO_UDP_CONNRESET   _WSAIOW(IOC_VENDOR,12) 
#endif

    memset(&ServerAddr, 0, sizeof(ServerAddr));

    SockOld = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    if (SockOld == INVALID_RDE)
    {
        return INVALID_RDE;
    }
    if (bNoBlock)
    {
        if (!libcpputil_setblocking(SockOld, FALSE))
        {
            libcpputil_close_soc(SockOld);
            return INVALID_RDE;
        }
    }
    ServerAddr.sin6_family = AF_INET6;

    if (szBindAddr == NULL ||
        szBindAddr[0] == 0
        || strcmpix(szBindAddr, "*") == 0
        || strcmpix(szBindAddr, "global") == 0
        || strcmpix(szBindAddr, "any") == 0
        || strcmpix(szBindAddr, "[*]") == 0
        || strcmpix(szBindAddr, "[global]") == 0
        || strcmpix(szBindAddr, "[any]") == 0
        )
    {
        ServerAddr.sin6_addr = in6addr_any;
    }
    else
    {
        if (szBindAddr[0] == '[')
        {
            char szIP[80];
            strcpyn_stop(szIP, szBindAddr + 1, sizeof(szIP), "]\r\n");
            inet_pton(AF_INET6, szIP, &ServerAddr.sin6_addr);
        }
        else
        {
            inet_pton(AF_INET6, szBindAddr, &ServerAddr.sin6_addr);
        }

    }


    ServerAddr.sin6_port = htons(nPort);

#ifdef WIN32
    status = WSAIoctl(SockOld,   SIO_UDP_CONNRESET, 
        &bNewBehavior,  sizeof   (bNewBehavior),   
        NULL,   0,   &dwBytesReturned,NULL,   NULL);
#endif


#ifndef WIN32    
    ret = 1;
    setsockopt(SockOld, SOL_SOCKET, SO_REUSEADDR, (const char *)&ret, sizeof(ret));
#endif   
    ret = 1;
    setsockopt(SockOld, SOL_SOCKET, SO_BROADCAST, (const char *)&ret, sizeof(ret));
    
    ret = 1;
    setsockopt(SockOld, SOL_SOCKET, IP_MULTICAST_TTL, (const char *)&ret, sizeof(ret));



    ret = 1;
    if (setsockopt(SockOld, IPPROTO_IPV6, IPV6_V6ONLY, (const char *)&ret, sizeof(ret)) < 0)
    {

    }

    ret = bind(SockOld, (struct sockaddr*)&ServerAddr,
        sizeof(ServerAddr));
    if (ret == SOCKET_ERROR)
    {
#ifdef WIN32
        ret = WSAGetLastError();
        libcpputil_close_soc(SockOld);
        WSASetLastError(ret);
#else
        ret = errno;
        libcpputil_close_soc(SockOld);
        errno = ret;
#endif
        return INVALID_RDE;
    }

    return SockOld;
}
SOCKET  libcpputil_bindIp6(const char *szBindAddr, int nPort, BOOL bNoBlock)
{

	
	SOCKET SockOld;
	socklen_t LengthServerAddr;
	struct    sockaddr_in6   ServerAddr;
	int ret;


	memset(&ServerAddr, 0, sizeof(ServerAddr));

	SockOld = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
	if (SockOld == INVALID_RDE)
	{
		return INVALID_RDE;
	}
	if (bNoBlock)
	{
		if (!libcpputil_setblocking(SockOld, FALSE))
		{
			libcpputil_close_soc(SockOld);
			return INVALID_RDE;
		}
	}
	ServerAddr.sin6_family = AF_INET6;

	if (szBindAddr == NULL ||
		szBindAddr[0] == 0
		|| strcmpix(szBindAddr, "*") == 0
		|| strcmpix(szBindAddr, "global") == 0
		|| strcmpix(szBindAddr, "any") == 0
		|| strcmpix(szBindAddr, "[*]") == 0
		|| strcmpix(szBindAddr, "[global]") == 0
		|| strcmpix(szBindAddr, "[any]") == 0
		)
	{
		ServerAddr.sin6_addr = in6addr_any;
	}
	else
	{
		if (szBindAddr[0] == '[')
		{
			char szIP[80];
			strcpyn_stop(szIP, szBindAddr + 1, sizeof(szIP), "]\r\n");
			inet_pton(AF_INET6, szIP, &ServerAddr.sin6_addr);
		}
		else
		{
			inet_pton(AF_INET6, szBindAddr, &ServerAddr.sin6_addr);
		}
		
	}


	ServerAddr.sin6_port = htons(nPort);

#ifndef WIN32
	ret = 1;
	if (setsockopt(SockOld, SOL_SOCKET, SO_REUSEADDR, (const char *)&ret, sizeof(ret))<0)
	{

	}
#endif

	ret  = 1;
	if (setsockopt(SockOld, IPPROTO_IPV6, IPV6_V6ONLY, (const char *)&ret, sizeof(ret)) < 0)
	{
		
	}

	ret = bind(SockOld, (struct sockaddr*)&ServerAddr,
		sizeof(ServerAddr));
	if (ret == SOCKET_ERROR)
	{
#ifdef WIN32
		ret = WSAGetLastError();
		libcpputil_close_soc(SockOld);
		WSASetLastError(ret);

#else
		ret = errno;
		libcpputil_close_soc(SockOld);
		errno = ret;
#endif


		return INVALID_RDE;
	}
	LengthServerAddr = sizeof(ServerAddr);

	if (getsockname(SockOld,
		(struct sockaddr*)&ServerAddr, &LengthServerAddr) == SOCKET_ERROR)
	{
		libcpputil_close_soc(SockOld);
		return INVALID_RDE;
		//////////////////////////////////
	}
	if (listen(SockOld, 2000) != 0)
	{
#ifdef WIN32
		ret = WSAGetLastError();
		libcpputil_close_soc(SockOld);
		WSASetLastError(ret);

#else
		ret = errno;
		libcpputil_close_soc(SockOld);
		errno = ret;
#endif


		return INVALID_RDE;
	}

	return SockOld;
}


SOCKET  libcpputil_bind_udp( const char *szBindAddr,int nPort,BOOL bNoBlock)
{
	char szIP[100];
	INT_IP nBindAddr = 0;
	SOCKET soc = INVALID_RDE ;

	RDE_IP_PORT(szBindAddr,szIP,100,&nPort);
	if(!szBindAddr||szBindAddr[0]==0
		||szIP[0]==0||stricmp(szIP,"global")==0
		||stricmp(szIP,"*")==0)
	{
		nBindAddr = 0;
	}
	else if(stricmp(szIP,"local")==0||stricmp(szIP,"localhost")==0)
	{

		nBindAddr = (INT_IP)inet_addr("127.0.0.1");
	}				
	else
	{
		nBindAddr = (INT_IP)inet_addr(szIP);
	}
	soc = libcpputil_udp_open(nBindAddr,nPort);

	if( soc != INVALID_RDE )
	{
		libcpputil_setblocking( soc,!bNoBlock );
	}
	return soc;
}
SOCKET  libcpputil_udp_open(INT_IP nBindAddr,int nPort)
{
	
    SOCKET    SockOld;
    socklen_t LengthServerAddr;
    struct    sockaddr_in   ServerAddr;
    int ret;  
#ifdef WIN32
    DWORD   dwBytesReturned   =   0;   
    BOOL     bNewBehavior   =   FALSE;   
    DWORD   status;
#define   IOC_VENDOR   0x18000000   
#define   _WSAIOW(x,y)   (IOC_IN|(x)|(y))   
#define   SIO_UDP_CONNRESET   _WSAIOW(IOC_VENDOR,12) 
#endif
    ServerAddr.sin_family=AF_INET;
    ServerAddr.sin_port=htons((unsigned short)nPort);
	
    SockOld=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if(SockOld==INVALID_SOCKET)
    {
        return INVALID_RDE ;
    }
#ifdef WIN32
    status = WSAIoctl(SockOld,   SIO_UDP_CONNRESET, 
					  &bNewBehavior,  sizeof   (bNewBehavior),   
					  NULL,   0,   &dwBytesReturned,NULL,   NULL);
#endif
    ServerAddr.sin_addr.s_addr=nBindAddr;
	
#ifndef WIN32    
    ret=1;
    setsockopt(SockOld, SOL_SOCKET,SO_REUSEADDR,(const char *) &ret, sizeof(ret));
#endif   
    ret=1;
    setsockopt(SockOld, SOL_SOCKET,SO_BROADCAST,(const char *) &ret, sizeof(ret));
    ret = 1;
    setsockopt(SockOld, SOL_SOCKET, IP_MULTICAST_TTL, (const char *)&ret, sizeof(ret));



	
    ret=bind(SockOld,(struct sockaddr*)&ServerAddr,
			 sizeof(ServerAddr));
    if(ret==SOCKET_ERROR)
    {    
#ifndef WIN32
        ret= errno;
        libcpputil_close_soc(SockOld);
        errno = ret ;
#else
        libcpputil_close_soc(SockOld);
#endif
        
        return INVALID_RDE ;
    }
    LengthServerAddr=sizeof(ServerAddr);
	
    if(getsockname(SockOld,
				   (struct sockaddr*)&ServerAddr,&LengthServerAddr)==SOCKET_ERROR)
    {    
        libcpputil_close_soc(SockOld);
        return INVALID_RDE ;    
        //////////////////////////////////
    }
	
    return SockOld;
}



TIME_T  libcpputil_UpSeconds()
{
#if defined(_WIN32) || defined(WIN32)
	return (TIME_T)( GetTickCount()/1000 );
#elif defined(_IOS_)
	static uint64_t _tickStart = 0 ;
	static mach_timebase_info_data_t _tickInfo;
	uint64_t nanosecs = 0 ;
	uint64_t elapsed = 0 ;
	if ( _tickStart == 0 )
	{
		_tickStart = mach_absolute_time() ;
		if (mach_timebase_info (&_tickInfo) != KERN_SUCCESS) 
		{
			printf ("mach_timebase_info failed\n");
		}
	}
	elapsed =  mach_absolute_time() - _tickStart ;

	nanosecs = elapsed * _tickInfo.numer / _tickInfo.denom;
	int millisecs = nanosecs / 1000000;

	return (millisecs / 1000) ;
#else

	return (OSTickCount() / 1000) ;
	/*
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (ts.tv_sec  + ts.tv_nsec / 1000000 / 1000 ) ;
	*/
	
#endif
}


void libcpputil_close_ctx(CObjConnContext *ctx )
{
	if( ctx == NULL || ctx->m_soc == INVALID_RDE )
	{
		return ;
	}
	ctx->m_bClosing = TRUE;
	//LogPrint(_LOG_LEVEL_DETAIL,_TAGNAME, "%s %d: errno=%d soc=%d,ctx=%p\n" , __FUNCTION__ , __LINE__ ,errno , ctx->m_soc,ctx );
	closesocket( ctx->m_soc );
	ctx->m_soc = INVALID_RDE ;

}
void libcpputil_close_soc(SOCKET soc )
{
	if( soc != INVALID_RDE )
	{
	//	LogPrint(_LOG_LEVEL_DEBUG,_TAGNAME, "%s %d: errno=%d soc=%d\n" , __FUNCTION__ , __LINE__ ,errno , soc );
		closesocket( soc );
	}

}

























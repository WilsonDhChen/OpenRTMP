
#include "rde_def.h"

#ifndef SO_MAX_MSG_SIZE
#define SO_MAX_MSG_SIZE 4096
#endif


int MYAPI RDE_SetReadTimeout(SOCKET h_rde,int timeout)
{

#if  defined(_WIN32_WCE)  
    return 0;
#elif defined(WIN32)
    if(h_rde==INVALID_RDE)
    {
        return 0;
    }
    if( timeout < 0 )
    {
        timeout = 0 ;
    }
    return setsockopt(h_rde,SOL_SOCKET,SO_RCVTIMEO,(char *)&timeout,
        sizeof(timeout));
#else


    struct timeval TimeOut;
    if( timeout < 0 )
    {
        timeout = 0 ;
    }
    timeout = timeout *1000;
    TimeOut.tv_sec = (int)(timeout/1000000);
    TimeOut.tv_usec = timeout%1000000;

    if(h_rde==INVALID_RDE)
    {
        return 0;
    }
    return setsockopt(h_rde,SOL_SOCKET,SO_RCVTIMEO,(char *)&TimeOut,
        sizeof(TimeOut));
#endif


}

int MYAPI RDE_SetRecvBuf(SOCKET h_rde,int bufsize)
{
    if (bufsize == 0)
    {
        return 0;
    }
	if( bufsize == -1 )
	{
		bufsize = SO_MAX_MSG_SIZE ;
	}
	return setsockopt( h_rde,SOL_SOCKET,SO_RCVBUF,(const char*)&bufsize,sizeof(int));
}
int MYAPI RDE_SetSendBuf(SOCKET h_rde,int bufsize)
{
    if (bufsize == 0)
    {
        return 0;
    }
	if( bufsize == -1 )
	{
		bufsize = SO_MAX_MSG_SIZE ;
	}
	return setsockopt( h_rde,SOL_SOCKET,SO_SNDBUF,(const char*)&bufsize,sizeof(int));
}
int MYAPI RDE_SetWriteTimeout(SOCKET h_rde,int timeout)
{
#if  defined(_WIN32_WCE)  
    return 0;
#elif defined(WIN32)
    if(h_rde==INVALID_RDE)
    {
        return 0;
    }
    if( timeout < 0 )
    {
        timeout = 0 ;
    }
    return setsockopt(h_rde,SOL_SOCKET,SO_SNDTIMEO,(char *)&timeout,
        sizeof(timeout));
#else
    struct timeval TimeOut;
    if( timeout < 0 )
    {
        timeout = 0 ;
    }
    timeout = timeout *1000;
    TimeOut.tv_sec = (int)(timeout/1000000);
    TimeOut.tv_usec = timeout%1000000;
    if(h_rde==INVALID_RDE)
    {
        return 0;
    }
    return setsockopt(h_rde,SOL_SOCKET,SO_SNDTIMEO,(char *)&TimeOut,
        sizeof(TimeOut));
#endif

}
int MYAPI RDE_SoError(SOCKET h_rde)
{
    int nRet=0;
    socklen_t len=_SIZE_OF(nRet);
    if(getsockopt(h_rde,SOL_SOCKET,SO_ERROR,(char *)&nRet,
        &len)==SOCKET_ERROR)
    {
        return SOCKET_ERROR;
    }
    return nRet;
}

const char *MYAPI RDE_Error()
{
#ifdef WIN32
    LPVOID lpMsgBuf = NULL;
    FormatMessageA( 
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM | 
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        WSAGetLastError(),
        MAKELANGID( LANG_ENGLISH, SUBLANG_DEFAULT ), // Default language
        (LPTSTR) &lpMsgBuf,
        0,
        NULL 
    );
    StrTrimRight((char *)lpMsgBuf);
    return (char *)lpMsgBuf;
#else
	const char * str = strerror(errno);
	int len = strlen(str) ;
	char * p = (char *)malloc(len+4);
	if( p != NULL )
	{
		strcpy( p, str );
		p[ len ] = 0 ;
	}
	return p ;
#endif
}
void MYAPI RDE_ErrorFree(const void *p)
{
	if( p == NULL )
	{
		return ;
	}
#ifdef WIN32
    LocalFree( (HLOCAL)p );
#else
    free( (void*)p );
#endif
}

int MYAPI RDE_ErrorCode(SOCKET h_rde)
{
#if defined(WIN32) || defined(_WIN32_WCE)
    errno = WSAGetLastError();
    WSASetLastError(errno);
#endif
    return (  errno );
}
BOOL MYAPI RDE_IsTimeout(SOCKET h_rde)
{
#if defined(WIN32) || defined(_WIN32_WCE)
	int socError = WSAGetLastError();
    errno = socError ;
    WSASetLastError(socError);
    return ( socError == WSAETIMEDOUT );
#else
    return ( ETIMEDOUT == errno || EAGAIN  == errno );
#endif
    /*
    int error;
    socklen_t errorlen = sizeof(error);
    getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &errorlen);
    */

}
int MYAPI RDE_SetReadTimeoutSec(SOCKET h_rde,int timeoutSec)
{

    return RDE_SetReadTimeout(h_rde,timeoutSec*1000);
}
int MYAPI RDE_SetWriteTimeoutSec(SOCKET h_rde,int timeoutSec)
{
    return RDE_SetWriteTimeout(h_rde,timeoutSec*1000);
}
int MYAPI RDE_GetBindName(SOCKET h_rde,INT_IP *BindIp,int *BindPort,char *szBindIP)
{
    struct sockaddr_in sock_addr;
    int nRet;
    socklen_t size_addr;
    size_addr=sizeof(struct sockaddr);
    if(szBindIP)szBindIP[0]=0;
    if(BindPort)*BindPort=0;
    if(BindIp)*BindIp=0;
    nRet=getsockname( h_rde,(struct sockaddr*)&sock_addr,&size_addr);
    if(nRet)return nRet;
    if(BindPort)*BindPort=ntohs(sock_addr.sin_port);
    if(szBindIP)
    {
        strcpy(szBindIP,(char *)inet_ntoa(sock_addr.sin_addr)); 
    }
    if(BindIp)
    {
        *BindIp=*(INT_IP *)(&sock_addr.sin_addr);
    }
    return 0;
}
char *MYAPI INET_ADDR2Str(INET_ADDR *pAddr,char *szOut ,int nLen)
{
    char szBuf[80];
    if( szOut )
    {
        szOut[0] = 0 ;
    }
    if( !szOut || !pAddr ||  nLen < 2 )
    {
        return szOut ;
    }
    szOut[nLen-1] = 0 ;

    snprintf(szOut,nLen-1,"%s:%d",INT_IP2Str(pAddr->addr,szBuf,_SIZE_OF(szBuf)), pAddr->port );

    return szOut ;

}
char *MYAPI INT_IP2Str(INT_IP ip,char *szOut ,int nLen)
{
    struct in_addr addr;
    addr.s_addr=ip;
    if(szOut)
    {
        strcpyn(szOut,(char *)inet_ntoa( addr),nLen); 
    }
    return szOut;
}
int MYAPI RDE_GetPeerName(SOCKET h_rde,INT_IP *PeerIp,int *PeerPort,char *szPeerIP)
{
    struct sockaddr_in sock_addr;
    int nRet;
    socklen_t size_addr;
    if(h_rde==INVALID_RDE)
    {
        return -1;
    }
    size_addr=sizeof(struct sockaddr);
    if( szPeerIP )szPeerIP[0]=0;
    if( PeerPort )*PeerPort=0;
    if( PeerIp )*PeerIp=0;
    nRet=getpeername( h_rde,(struct sockaddr*)&sock_addr,&size_addr);
    if(nRet)return nRet;
    if(PeerPort)*PeerPort=ntohs(sock_addr.sin_port);
    if(szPeerIP)
    {
        strcpyn(szPeerIP,(char *)inet_ntoa(sock_addr.sin_addr),LEN_DOMAIN); 
    }
    if(PeerIp)
    {
        *PeerIp=*(INT_IP *)(&sock_addr.sin_addr);
    }
    return 0;
}
int MYAPI RDE_Init()
{
#ifdef WIN32
    WSADATA WsaData;
    int nRet;
#endif
    static int bInit = 0;

#if defined(SIGPIPE) ||  ! defined(WIN32)
    signal(SIGPIPE,SIG_IGN);
#endif

    if( bInit )
    {
        return 0;
    }
    bInit = 1;

#ifdef WIN32
    nRet=WSAStartup(MAKEWORD(2,2),&WsaData);
    switch(nRet)
    {
    case 0:
        break;
    case WSASYSNOTREADY:
        strcpy(szRDE_ERROR,"WSASYSNOTREADY");
        break;
    case WSAVERNOTSUPPORTED :
        strcpy(szRDE_ERROR,"WSAVERNOTSUPPORTED");
        break;
    case WSAEPROCLIM :
        strcpy(szRDE_ERROR,"WSAEPROCLIM");
        break;
    case WSAEFAULT :
        strcpy(szRDE_ERROR,"WSAEFAULT");
        break;
    }
#endif



    return 0;
}

BOOL MYAPI RDE_SetSocketBlocking(SOCKET sock ,BOOL bBlock)
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

void MYAPI UrlParseUPDP(const char *szAgentUrlIn,
						char *szAgentUser,int nUserLen,char *szAgentPass,int nPassLen,
						char *szDomain,int nDomainLen,int *pnPort)
{
	int iAt = -1;
	int iSlash = -1;
	int iDomain = -1;
	int iColon = -1;
	char *szAgentUrl = 0;
	if( !szAgentUrlIn || !szAgentUrlIn[0] )
	{
		goto LABLE_FAIL;
	}
	szAgentUrl = (char *)alloca(strlenx(szAgentUrlIn)+2);
	if(!szAgentUrl)
	{
		goto LABLE_FAIL;
	}
	strcpyx(szAgentUrl,szAgentUrlIn);
	StrTrimX(szAgentUrl);
	if( !szAgentUrl[0] )
	{
		goto LABLE_FAIL; 
	}


	iAt = strcmpifind(szAgentUrl,"@");
	iSlash = strcmpifind(szAgentUrl,"://");
	if( iSlash>=0 )
	{
		iSlash += 3;
	}


	if( iAt >= 0 )
	{
		iDomain = iAt+1;
	}
	else if( iSlash>=0 )
	{
		iDomain = iSlash;
	}
	else
	{
		iDomain = 0;
	}
	if( iSlash<0 )
	{
		iSlash = 0;
	}
	if( iAt < 1 )
	{
		if(szAgentUser)
		{
			szAgentUser[0] = 0;
		}
		if(szAgentPass)
		{
			szAgentPass[0] = 0;
		}

	}
	else
	{
		szAgentUrl[iAt] = 0;
		iColon = strcmpifind(szAgentUrl+iSlash,":");
		if( iColon <0 )
		{
			if(szAgentUser)
			{
				strcpyn(szAgentUser,szAgentUrl+iSlash,nUserLen);
			}

		}
		else
		{
			szAgentUrl[iSlash+iColon] = 0;
			if(szAgentUser)
			{
				strcpyn(szAgentUser,szAgentUrl+iSlash,nUserLen);
			}

			if(szAgentPass)
			{
				strcpyn(szAgentPass,szAgentUrl+iSlash+iColon+1,nPassLen);
			}

		}

	}
	RDE_IP_PORT(szAgentUrl+iDomain,szDomain,nDomainLen,pnPort);

	StrTrimX(szAgentUser);
	StrTrimX(szDomain);
	return ;
LABLE_FAIL:
	if(szAgentUser)
	{
		szAgentUser[0] = 0;
	}
	if(szAgentPass)
	{
		szAgentPass[0] = 0;
	}
	if(szDomain)
	{
		szDomain[0] = 0;
	}
	if(pnPort)
	{
		*pnPort = 0;
	}
	return ;

}

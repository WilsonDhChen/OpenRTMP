
#include "rde_def.h"


char szRDE_ERROR[LEN_ERROR]="\0\0";
const char * MYAPI GetRdeError()
{ 
 return szRDE_ERROR;
}
int MYAPI ClearRdeError() 
{
 szRDE_ERROR[0]=0;
 return 0;
}
/************************* tcp function *************************************/
#ifndef WIN32
void closesocket(int h_Rde)
{
 close(h_Rde);
}
#endif 

UINT16 MYAPI h2ns(UINT16 nValue)
{

	return htons(nValue);
}
UINT16 MYAPI n2hs(UINT16 nValue)
{
	return ntohs(nValue);
}



const char * MYAPI RDE_GetHostName(char *szHost,int nHostLen)
{
    gethostname(szHost,nHostLen);
	return szHost;
}
INT_IP MYAPI RDE_GetIPByName(const char *szHost,char *szIP,int nLen)
{

#if defined(_Linux_)
	char    buf[1024];
	int     ret = 0 ;
	INT_IP ip = INADDR_NONE;
	char *host = NULL;
	int len = 0;
	struct  hostent hostinfo;
	struct  hostent *pHost = NULL;

	if (szIP)
	{
		szIP[0] = 0;
	}
	if (szHost == NULL || szHost[0] == 0)
	{
		return INADDR_NONE;
	}

	len = strlenx(szHost) + 1;
	host = (char *)alloca(len);
	strcpyn_stop(host, szHost, len, ":\r\n");
	ip = (INT_IP)inet_addr(host);
	if (ip != INADDR_NONE)
	{
		return ip;
	}

	if ( gethostbyname_r(host, &hostinfo, buf, sizeof(buf), &pHost, &ret) != 0 )
	{
		return ip;
	}

	if (pHost == NULL || pHost->h_addr_list[0] == NULL || pHost->h_addrtype != AF_INET)
	{
		return INADDR_NONE;
	}
	if(szIP)
	{
		strcpyn(szIP, (char *)inet_ntoa(*(struct in_addr *)pHost->h_addr_list[0]), nLen);
	}
	ip = (INT_IP)(((struct in_addr *)pHost->h_addr_list[0])->s_addr);
	return ip;
#else
	struct hostent *pHost = NULL;
	char *host = NULL;
	int len = 0;
	INT_IP ip = INADDR_NONE;

	if (szIP)
	{
		szIP[0] = 0;
	}
	if ( szHost == NULL || szHost[0] == 0 )
	{
		return INADDR_NONE;
	}




	len = strlenx(szHost) + 1;
	host = (char *)alloca(len);
	strcpyn_stop(host, szHost, len, ":\r\n");
	ip = (INT_IP)inet_addr(host);
	if (ip != INADDR_NONE)
	{
		return ip;
	}

    pHost=gethostbyname(host);
	if (pHost == NULL || pHost->h_addr_list[0] == NULL || pHost->h_addrtype != AF_INET)
	{
		return INADDR_NONE;
	}
	if(szIP)
    {
        strcpyn(szIP,(char *)inet_ntoa(*(struct in_addr *)pHost->h_addr_list[0]),nLen);
    }
	ip = (INT_IP)(((struct in_addr *)pHost->h_addr_list[0])->s_addr); 
	return ip;
#endif
}
/////////////////////////////////////////////////////////////// 
SOCKET MYAPI RDE_Open( const char *szBindAddr,int nPort)
{

	char szIP[100];
    SOCKET SockOld;
    socklen_t LengthServerAddr;
    struct    sockaddr_in   ServerAddr;
    int ret;
	RDE_IP_PORT(szBindAddr,szIP,100,&nPort);

    ServerAddr.sin_family=AF_INET;
    ServerAddr.sin_port=htons((unsigned short)nPort);

    SockOld=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(SockOld==INVALID_SOCKET)
    {
        strcpyx(szRDE_ERROR,"RDE_Open socket Error");
        return INVALID_SOCKET ;
    }

	RDE_SoError(SockOld);


    if(!szBindAddr||szBindAddr[0]==0
				||szIP[0]==0||strcmpix(szIP,"global")==0
				||strcmpix(szIP,"*")==0)
    {
        ServerAddr.sin_addr.s_addr=0;
    }
    else if(strcmpix(szIP,"local")==0||strcmpix(szIP,"localhost")==0)
    {

        ServerAddr.sin_addr.s_addr=inet_addr("127.0.0.1");
    }		
    else
    {
        ServerAddr.sin_addr.s_addr=inet_addr(szIP);
    }

#ifndef WIN32
    ret=1;
    if(setsockopt(SockOld, SOL_SOCKET,SO_REUSEADDR,(const char *) &ret, sizeof(ret))<0)
    {
        strcpyx(szRDE_ERROR,"RDE_Open setsockopt Error");
    }
#endif
    ret=bind(SockOld,(struct sockaddr*)&ServerAddr,
        sizeof(ServerAddr));
    if(ret==SOCKET_ERROR)
    {    
        closesocket(SockOld);
        strcpyx(szRDE_ERROR,"RDE_Open bind Error");
        return INVALID_SOCKET ;
    }
    LengthServerAddr=sizeof(ServerAddr);

    if(getsockname(SockOld,
    (struct sockaddr*)&ServerAddr,&LengthServerAddr)==SOCKET_ERROR)
    {    
        closesocket(SockOld);
        strcpyx(szRDE_ERROR,"RDE_Open getsockname Error");
        return INVALID_SOCKET ;    
    //////////////////////////////////
    }

    return SockOld;
}

#if !defined( _NO_RDE_TIMEOUT )
BOOL MYAPI RDE_ConnectWithTimeout( SOCKET soc, struct sockaddr* addr,int sizeofaddr ,int timeoutMilliseconds, BOOL *bCancel )
{
    fd_set readfd;
    fd_set writefd;
    fd_set errorfd;
    struct timeval timeout;
    int ret = 0;
    TIME_T tmBegin = 0;


    if( timeoutMilliseconds == -1 )
    {
        if( connect( soc, addr, sizeofaddr ) == 0 )
        {
            return TRUE ;
        }
        else
        {
            return FALSE ;
        }
    }

    timeout.tv_sec = 0;
    timeout.tv_usec = 10000 ;


   

    RDE_SetSocketBlocking( soc  , FALSE );

    tmBegin = OSTickCount();

    if( connect( soc, addr, sizeofaddr ) == 0 )
    {

        RDE_SetSocketBlocking( soc, TRUE );

        return TRUE;
    }

#ifdef WIN32
    if( WSAGetLastError( ) != WSAEWOULDBLOCK )
#else
    if( errno != EINPROGRESS )
#endif
    {
        RDE_SetSocketBlocking( soc, TRUE );
        return FALSE ;
    }

    while( ( OSTickCount( ) - tmBegin ) < timeoutMilliseconds )
    {

        if( bCancel != NULL && ( *bCancel ) )
        {
            break ;
        }

        FD_ZERO( &readfd );
        FD_ZERO( &writefd );
        FD_ZERO( &errorfd );


        FD_SET( soc, &readfd );
        FD_SET( soc, &writefd );
        FD_SET( soc, &errorfd );

        ret = select( soc + 1, &readfd, &writefd, &errorfd, &timeout );

        if( ret == 0 )
        {
            continue;
        }
        else if( ret == -1 )
        {

            break;

        }
        if( FD_ISSET( soc, &errorfd ) )
        {
            break;
        }

        if( FD_ISSET( soc, &readfd ) || FD_ISSET( soc, &writefd ) )
        {

            if( RDE_GetPeerName( soc , NULL, NULL ,NULL ) == 0 )
            {

                RDE_SetSocketBlocking( soc, TRUE );

                return TRUE;
            }

            break;


        }


    }

    RDE_SetSocketBlocking( soc, TRUE );

    return FALSE ;
}
#endif

BOOL MYAPI RDE_ConnectXX(SOCKET hRdeClient,INT_IP nIp,int port)
{
    struct sockaddr_in ServerAddr;
    //int nIo=1;
	if( nIp == 0 )
	{
		return FALSE ;
	}

    
    ServerAddr.sin_family=AF_INET;
    ServerAddr.sin_port=htons((unsigned short)port);
    
    
    ServerAddr.sin_addr.s_addr=nIp;

#if !defined( _NO_RDE_TIMEOUT )
    if( !RDE_ConnectWithTimeout( hRdeClient, ( struct sockaddr* )&ServerAddr, sizeof( ServerAddr ), 10000, NULL ) )
    {
        strcpyx( szRDE_ERROR, "RDE_ConnectX connect Error" );
        return FALSE;
    }
#else
    if( connect( hRdeClient, ( struct sockaddr* )&ServerAddr, sizeof( ServerAddr ) ) != 0 )
    {
        strcpyx( szRDE_ERROR, "RDE_ConnectX connect Error" );
        return FALSE;
    }
#endif

    return TRUE;
}
BOOL MYAPI RDE_ConnectX(SOCKET hRdeClient,const char *IpAddr,int nPort)
{
	 RDE_IP_PORT(IpAddr,0,0,&nPort);
   return  RDE_ConnectXX(hRdeClient,RDE_GetIPByName(IpAddr,0,0),nPort);
}
int MYAPI RDE_IP_PORT(const char *szAddr,char *szIPOut,int nIpSize,
                       int *nPortOut)
{
    int i;
	char szPort[20];
	int  nPort=0;
	int  bV6 = 0;
	if (szIPOut != NULL)
	{
		szIPOut[0] = 0;
	}

	if (szAddr == NULL || szAddr[0] == 0)
	{
		return 0;
	}
	if (szAddr[0] == '[')
	{
		bV6 = 1;
		szAddr++;
		strcpyn_stop(szIPOut, szAddr, nIpSize, "]/ \r\n\t//");
	}
	else
	{
		strcpyn_stop(szIPOut, szAddr, nIpSize, ":/ \r\n\t//");
	}
	
	StrTrimX( szIPOut);
	if (szIPOut && szIPOut[0] == '*')
	{
		szIPOut[0] = 0;
	}

	szPort[0] = 0;
    for(i=0;

		*(szAddr+i)!=0&&
		*(szAddr+i)!='/'&&
		*(szAddr+i)!=' '&&
		*(szAddr+i)!='\r'&&
		*(szAddr+i)!='\n'&&
		*(szAddr+i)!='\t'&&
		*(szAddr+i)!='\\'

		;i++)
    {
        if(   *(szAddr+i)==':')
        {
			if (bV6)
			{
				if (szAddr[i - 1] != ']')
				{
					continue;
				}
			}
            if(nPortOut)
            {
				strcpyn_number(szPort,szAddr+i+1,20);
				StrTrimX( szPort);
				nPort=atoix(szPort);
				if(nPort!=0 && szPort[0] )
				{
					*nPortOut=nPort;
					return 1;
				}
				else
				{
					return 0;
				}

            }
			else
			{
				return 0;
			}
        }

    }
    return 0;
}
SOCKET MYAPI RDE_Connect2(const char *IpAddr,int nPort,unsigned int timeout, SOCKET *pSocConnecting)
{
    RDE_IP_PORT(IpAddr,0,0,&nPort);
    return RDE_ConnectEx(RDE_GetIPByName(IpAddr,0,0),nPort,timeout , pSocConnecting, TRUE);
}
SOCKET MYAPI RDE_Connect(const char *IpAddr,int nPort,unsigned int timeout)
{

	RDE_IP_PORT(IpAddr,0,0,&nPort);
	return RDE_ConnectEx(RDE_GetIPByName(IpAddr,0,0),nPort,timeout , NULL, FALSE);
 
}



int MYAPI RDE_WriteStr(SOCKET h_rde,const char *buffer)
{
    return RDE_Write(h_rde,(char *)buffer,strlenx(buffer));
}
int MYAPI RDE_WriteStr0(SOCKET h_rde,const char *buffer)
{

    return RDE_Write(h_rde,(char *)buffer,strlenx(buffer)+1);
}
int MYAPI RDE_CancelIO(SOCKET h_rde)
{
    if( h_rde == INVALID_RDE )
    {
        return -1 ;
    }
    RDE_SetWriteTimeout(h_rde,2);
    RDE_SetReadTimeout(h_rde,2);
    RDE_Shutdown(h_rde);
    return 0; 
}


int MYAPI RDE_ReadLine(SOCKET  h_rde,void *buffer,int size,int *pIsLineEnd)
{
	int nSumReaded=0,nReaded=0,nWillRead=0;
	char *szBuff=(char *)buffer;
	do
	{
		nSumReaded+=nReaded;
		if(nReaded>0&&(szBuff[nSumReaded-1]=='\r'||
				szBuff[nSumReaded-1]=='\n'||
				szBuff[nSumReaded-1]==0
				))
		{
			if(pIsLineEnd)
			{
				*pIsLineEnd=1;
			}
			break;
		}
		nWillRead=SIZE_2KB<size-nSumReaded ? SIZE_2KB:size-nSumReaded;

		if(nWillRead<1)
		{

			break;

		}
		nReaded=RDE_Read(h_rde,szBuff+nSumReaded,nWillRead);
		if(nSumReaded==0&&nReaded<0)
		{
			return -1;
		}

	}while(nReaded>0);
	if(nSumReaded<size)szBuff[nSumReaded]=0;
	return nSumReaded;

}



int MYAPI RDE_SendRead(SOCKET h_rde,
                       const void *vSend ,int nSendSize,
                       void *vRec,int nRecSize)
{
    int nRet;
    if(nSendSize==-1)nSendSize=strlenx((char *)vSend);
    if((nRet=RDE_Write(h_rde,vSend,nSendSize))<0)return -2;
    if(vRec)*((char *)vRec)=0;
    nRet=RDE_Read(h_rde,vRec,nRecSize);
    if(nRet<0)nRet=-3;
    return nRet;
}
int MYAPI RDE_SendReadLine(SOCKET h_rde,
					   const void *vSend ,int nSendSize,
					   void *vRec,int nRecSize)
{
	int nRet;
	if(nSendSize==-1)nSendSize=strlenx((char *)vSend);
	if((nRet=RDE_Write(h_rde,vSend,nSendSize))<0)return -2;
	if(vRec)*((char *)vRec)=0;
	nRet=RDE_ReadLine(h_rde,vRec,nRecSize,0);
	if(nRet<0)nRet=-3;
	return nRet;
}

#if !defined(_RDE_SOCKS5)
SOCKET MYAPI RDE_ConnectWithAgent(const char *szAgentUrl,
                                 const char *szIP,
                                 int nPort,
                                 unsigned int timeout)
{
    return INVALID_SOCKET;
    
}

SOCKET MYAPI RDE_ConnectAgentHttps(const char *szAgentUrl,
                                   const char *szIP,int nPort,
                                   unsigned int timeout)
{

    return INVALID_SOCKET;
}

SOCKET MYAPI RDE_ConnectAgentSks4(const char *szAgentUrl,
                                  const char *szIP,int nPort,
                                  unsigned int timeout)
{

    return INVALID_SOCKET;
}
SOCKET MYAPI RDE_ConnectAgentSks5(const char *szAgentUrl,
                                  const char *szIP,
                                  int nPort,
                                  unsigned int timeout)
{
    return INVALID_SOCKET;
}

SOCKET MYAPI RDE_Socks5Auth(const char *szAgentUrl,unsigned int timeout)
{

    return INVALID_SOCKET;
}
int MYAPI RDE_Socks5Connect(SOCKET hRde,const char *szDomainIn,INT_IP nIp,int nPort)
{
    return 0;
}



#endif





#if !defined(_RDE_DNS)

char * MYAPI DNS_GetOSDnsAddr(char *szOutDnsAddr,int nLen)
{
    return NULL;
}
int MYAPI DNS_GetInfoWithType(const char *szDNSIP,const char*szHostIn,
                              const UINT16 nDnsType,char *szInfoOut,int nLen)
{
    return 0 ;
}
int MYAPI DNS_GetMXAddr(const char *szDNSIP,const char*szHost,
                        char *szMXOut,int nLen)
{
    return  0 ;
}
#endif




#if !defined(_RDE_NTP)

void MYAPI SetNetworkTimeUpdatedFunc(_OnNetworkTimeUpdated _func)
{

}
SYSTEMTIME *MYAPI NetworkTime( SYSTEMTIME *pSysTime )
{
	static SYSTEMTIME _netTime = {0,0,0,0,0,0,0,0};
	if( pSysTime == NULL )
	{
		return NULL ;
	}
	SystemTime(pSysTime , FALSE);
	memcpy( pSysTime, &_netTime, sizeof(SYSTEMTIME) );

	return pSysTime ;
}
BOOL MYAPI RDE_GetNtpTime(SYSTEMTIME *pSysTime,SOCKET hRdeU,const char *szTimeHost)
{
    return FALSE ;
}
BOOL MYAPI NetworkTimeUpdate()
{
    return FALSE ;
}
BOOL MYAPI RDE_GetTime(SYSTEMTIME *pSysTime,SOCKET hRdeU,const char *szTimeHost)
{
    return FALSE ;    
}
void MYAPI StartTimeGuard()
{
    
}
void MYAPI StopTimeGuard()
{

}

#endif



















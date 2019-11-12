
#include "rde_def.h"

#ifndef HAVE_RDE_ERROR_BUF
static char szRDE_ERROR[100];
#endif


#ifndef INVALID_RDE
#define  INVALID_RDE  (-1)
#endif

#ifndef DEF_TYPE_INT_IP
#define DEF_TYPE_INT_IP
typedef UINT32 INT_IP ;
#endif

#ifndef NULL
#define NULL 0
#endif


SOCKET MYAPI RDE_WaitConnect(SOCKET h_rde,char *szOutIP,int *nOutPort)
{

    struct sockaddr_in sock_addr;
    socklen_t size_addr;
    SOCKET soc;

    if(INVALID_RDE==h_rde)
    {
        return INVALID_RDE;
    }
    size_addr=sizeof(struct sockaddr);


    if(listen(h_rde,2)==SOCKET_ERROR)
    {
        //  strcpyx(szRDE_ERROR,"RDE_WaitConnect listen Error");
        //  return INVALID_RDE;
    }



    soc=accept(h_rde,
        (struct sockaddr*)&sock_addr,
        &size_addr);
    if(soc==INVALID_RDE)
    {
        strcpyx(szRDE_ERROR,"RDE_WaitConnect accept Error");
        return INVALID_RDE;
    }

    if(nOutPort)
    {
        *nOutPort=ntohs(sock_addr.sin_port);
    }
    if(szOutIP)
    {
        strcpy(szOutIP,(char *)inet_ntoa(sock_addr.sin_addr));
    }
    return soc;
}


int MYAPI RDE_ReadLoopNetHead(SOCKET  h_rde,void *buffer,int size)
{
    int nSumReaded=0,nReaded=0,nWillRead=0;
    char *szBuff=(char *)buffer;
    size -- ;
    if( size < 1 )
    {
        return 0 ;
    }
    do
    {
        nSumReaded+=nReaded;

        nWillRead=SIZE_2KB<size-nSumReaded ? SIZE_2KB:size-nSumReaded;

        if(nWillRead<1)
        {

            break;

        }
        if( nSumReaded > 0 )
        {
            szBuff[nSumReaded] =  0 ;
            if( strcmpifind(szBuff ,"\r\n\r\n") >= 0 )
            {
                return nSumReaded;
            }
        }
        nReaded=RDE_Read(h_rde,szBuff+nSumReaded,nWillRead);
        if(nSumReaded==0&&nReaded<0)
        {
            return -1;
        }

    }while(nReaded>0);

    return nSumReaded;

}
int MYAPI RDE_ReadLoopNetHead2(SOCKET  h_rde,void *buffer,int size)
{
	int nSumReaded=0,nReaded=0,nWillRead=0;
	char *szBuff=(char *)buffer;
	size -- ;
	if( size < 1 )
	{
		return 0 ;
	}
	do
	{
		nSumReaded += nReaded;

		nWillRead=1<size-nSumReaded ? 1:size-nSumReaded;

		if(nWillRead<1)
		{

			break;

		}
		if( nSumReaded >= 4 )
		{
			szBuff[nSumReaded] =  0 ;
			if( szBuff[nSumReaded-1] == '\n' && szBuff[nSumReaded-2] == '\r'&&
				szBuff[nSumReaded-3] == '\n' && szBuff[nSumReaded-4] == '\r' )
			{
				return nSumReaded;
			}
		}
		szBuff[nSumReaded] = 0 ;
		nReaded=RDE_Read(h_rde,szBuff+nSumReaded,nWillRead);
		if(nSumReaded == 0 && nReaded<0)
		{
			return -1;
		}

	}while(nReaded>0);

	return nSumReaded;

}
int MYAPI RDE_ReadLoop(SOCKET  h_rde,void *buffer,int size)
{
    int nSumReaded=0,nReaded=0,nWillRead=0;
    char *szBuff=(char *)buffer;
    do
    {
        nSumReaded+=nReaded;

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

    return nSumReaded;

}
int MYAPI RDE_Write(SOCKET h_rde,const void *buffer,int size)
{
    const char *buf=(const char *)buffer;
    int i=0;
    int nRet;
    if( h_rde == INVALID_RDE )
    {
        return -1 ;
    }
    if(!buf||size==0)
    {
        return 0;
    }

    while((size-i)>0)
    {
        nRet=send(h_rde,(char *)buf+i,size-i,0);
        if(nRet<0)
        {
            if(i==0)return nRet;
            else return i;
        }
        i+=nRet;
    }

    return  i;
}
int MYAPI RDE_DataAvailable( SOCKET  h_rde )
{
#ifdef WIN32
	u_long   len = 0 ;
	if( h_rde == INVALID_RDE )
	{
		return 0 ;
	}
	if( ioctlsocket(h_rde,   FIONREAD,   &len) < 0 )
	{
		len = -1;
	}
	return (int)len ;
#else
	#ifndef SO_NREAD
	return 65535;
	#else
	int   len = 0 ;
	socklen_t size = sizeof(len);
	if( h_rde == INVALID_RDE )
	{
		return 0 ;
	}
	getsockopt(h_rde,  SOL_SOCKET, SO_NREAD,   &len , &size);
	return (int)len ;
	#endif
#endif
}
int MYAPI RDE_Read(SOCKET  h_rde,void *buffer,int size)
{
    if( h_rde == INVALID_RDE )
    {
        return -1 ;
    }
    if( !buffer )
    {
        return 0;
    }
    return recv(h_rde,(char *)buffer,size,0);
}

int MYAPI RDE_Shutdown(SOCKET h_rde)
{

    if(h_rde==INVALID_RDE)
    {
        return 0;
    }
#ifdef SD_BOTH
    return shutdown(h_rde,SD_BOTH);
#else
    return shutdown(h_rde,2);
#endif
}
int MYAPI RDE_Close( SOCKET h_rde)
{
    if(h_rde!=INVALID_RDE)
    {
        closesocket(h_rde);
    }
    return 1;
}
int MYAPI RDE_CloseEx( SOCKET *phRde)
{
    if(phRde)
    {
        if(*phRde != INVALID_RDE)
        {
            closesocket(*phRde);
        }
        *phRde=INVALID_RDE;

    }
    return 1;
}


SOCKET MYAPI RDE_ConnectEx(INT_IP nIp,int port,unsigned int timeout, SOCKET *pSocConnecting , BOOL bAsync)
{
    SOCKET soc;
    struct sockaddr_in ServerAddr;
    int nIo=1;


    ServerAddr.sin_family=AF_INET;
    ServerAddr.sin_port=htons((unsigned short)port);



    ServerAddr.sin_addr.s_addr=nIp;

    soc=socket(AF_INET,SOCK_STREAM,0);
    if(soc==INVALID_RDE)
    {
        strcpy(szRDE_ERROR,"RDE_Connect socket Error");
        return INVALID_RDE;
    }

    if( pSocConnecting != NULL )
    {
        *pSocConnecting = soc ;
    }


	if (bAsync)
	{
		LogPrint(_LOG_LEVEL_DETAIL ,"" ,"%s %d pSocConnecting=%p\n", __FUNCTION__ , __LINE__ , pSocConnecting );
		if( !RDE_ConnectWithTimeout( soc, ( struct sockaddr* )&ServerAddr, sizeof( ServerAddr ), timeout, NULL ) )
		{
			LogPrint(_LOG_LEVEL_DETAIL ,"" ,"%s %d pSocConnecting=%p\n", __FUNCTION__ , __LINE__ , pSocConnecting );
			strcpy( szRDE_ERROR, "RDE_Connect connect Error" );
	
			if( pSocConnecting != NULL )
			{
				*pSocConnecting = INVALID_RDE;
			}
			closesocket( soc );
			return INVALID_RDE;
		}
		LogPrint(_LOG_LEVEL_DETAIL ,"" ,"%s %d pSocConnecting=%p\n", __FUNCTION__ , __LINE__ , pSocConnecting );
	}
	else
	{
		if( timeout != 0 )
		{
			RDE_SetWriteTimeout(soc,timeout);
			RDE_SetReadTimeout(soc,timeout);
		}

		if( connect( soc,  ( struct sockaddr* )&ServerAddr, sizeof( ServerAddr ) ) != 0 )
		{
			strcpy( szRDE_ERROR, "RDE_Connect connect Error" );
			if( pSocConnecting != NULL )
			{
				*pSocConnecting = INVALID_RDE;
			}
			closesocket( soc );
			return INVALID_RDE;
		}
		RDE_SetReadTimeout( soc, 0 );
		RDE_SetWriteTimeout( soc, 0 );
	}



    return soc;
}

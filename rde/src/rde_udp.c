

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


#ifndef WIN32
#define  stricmp  (strcasecmp)
#endif 

#ifndef MYAPI
#define MYAPI
#endif
/****************************** udp function ****************************************************************************/

SOCKET MYAPI RDEU_OpenEx(INT_IP nBindAddr,int nPort)
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
        strcpyx(szRDE_ERROR,"make socket error");
        return INVALID_SOCKET ;
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

    ret=bind(SockOld,(struct sockaddr*)&ServerAddr,
        sizeof(ServerAddr));
    if(ret==SOCKET_ERROR)
    {    
        closesocket(SockOld);
        strcpyx(szRDE_ERROR,"bind socket error");
        return INVALID_SOCKET ;
    }
    LengthServerAddr=sizeof(ServerAddr);

    if(getsockname(SockOld,
        (struct sockaddr*)&ServerAddr,&LengthServerAddr)==SOCKET_ERROR)
    {    
        closesocket(SockOld);
        strcpyx(szRDE_ERROR,"getsockname error");
        return INVALID_SOCKET ;    
        //////////////////////////////////
    }

    return SockOld;
}

#ifdef HAVE_RDE_IP_PORT_
SOCKET MYAPI RDEU_Open(const char *szBindAddr,int nPort)
{

    char szIP[100];
    INT_IP nBindAddr = 0;

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

    return RDEU_OpenEx( nBindAddr ,nPort);

}
#endif



int MYAPI RDEU_Close( SOCKET h_rde)
{
    if(h_rde!=INVALID_RDE)
    {
#ifdef SD_BOTH
        return shutdown(h_rde,SD_BOTH);
#else
        return shutdown(h_rde,2);
#endif
        closesocket(h_rde);
    }
    return 1; 
}

int MYAPI RDEU_WriteXX(SOCKET h_rde,const void *buf,int nBufSize,PINET_ADDR pUdpAddr)
{
    struct    sockaddr_in DestAddr;
    if( !pUdpAddr )
    {
        return -1;
    }
    if( nBufSize == -1 )
    {
        nBufSize = strlenx((const char *)buf);
    }

    DestAddr.sin_family=AF_INET;
    DestAddr.sin_port=htons((unsigned short)(pUdpAddr->port));
    DestAddr.sin_addr.s_addr=pUdpAddr->addr; 

    return sendto(h_rde,(char *)buf,nBufSize,0,
        (struct    sockaddr*)&DestAddr,sizeof(DestAddr));
}
int MYAPI RDEU_WriteEx(SOCKET h_rde,const void *buf,int nBufSize,INT_IP nDestIp,int nPort)
{
    struct    sockaddr_in DestAddr;
    //	int iMode = 0;

    DestAddr.sin_family=AF_INET;
    DestAddr.sin_port=htons((unsigned short)nPort);
    DestAddr.sin_addr.s_addr=nDestIp; 

    //	ioctlsocket(h_rde, FIONBIO, (u_long FAR*) &iMode); 
    return sendto(h_rde,(char *)buf,nBufSize,0,
        (struct    sockaddr*)&DestAddr,sizeof(DestAddr));
}
int MYAPI RDEU_Write(SOCKET h_rde,const void *buf,int nBufSize,const char *szIp,int nPort)
{
    struct    sockaddr_in DestAddr;

    DestAddr.sin_family=AF_INET;
    DestAddr.sin_port=htons((unsigned short)nPort);
    DestAddr.sin_addr.s_addr=inet_addr(szIp); 
    return sendto(h_rde,(char *)buf,nBufSize,0,
        (struct    sockaddr*)&DestAddr,sizeof(DestAddr));
}
int MYAPI RDEU_Read(SOCKET h_rde,void *buf,
                    int nBufSize,char *szOutIp,
                    int *nOutPort)
{
    struct    sockaddr_in DestAddr;
    int nReaded;
    int socError;
    socklen_t nAddrSize=_SIZE_OF(DestAddr);
    nReaded=recvfrom(h_rde,(char *)buf,nBufSize,0,(struct sockaddr*)&DestAddr,&nAddrSize);
    if (nReaded < 1)
    {
#if defined(WIN32) || defined(_WIN32_WCE)
        socError = WSAGetLastError();
#else
        socError = errno;
#endif
    }

    if(nOutPort)*nOutPort=ntohs(DestAddr.sin_port);
    if(szOutIp)strcpy(szOutIp,(char *)inet_ntoa(DestAddr.sin_addr));

    if (nReaded < 1)
    {
#if defined(WIN32) || defined(_WIN32_WCE)
        WSASetLastError(socError);
#endif
        errno = socError;
    }

    return nReaded;
}
int MYAPI RDEU_ReadEx(SOCKET h_rde,void *buf,
                      int nBufSize,INT_IP *nOutSrcIp,
                      int *nOutPort)
{
    struct    sockaddr_in DestAddr;
    int nReaded;
    int socError;
    socklen_t nAddrSize=_SIZE_OF(DestAddr);
    nReaded=recvfrom(h_rde,(char *)buf,nBufSize,0,(struct sockaddr*)&DestAddr,&nAddrSize);
    if (nReaded < 1)
    {
#if defined(WIN32) || defined(_WIN32_WCE)
        socError = WSAGetLastError();
#else
        socError = errno;
#endif
    }

    if(nOutPort)*nOutPort=ntohs(DestAddr.sin_port);
    if(nOutSrcIp)*nOutSrcIp=*(INT_IP *)(&DestAddr.sin_addr);

    if (nReaded < 1)
    {
#if defined(WIN32) || defined(_WIN32_WCE)
        WSASetLastError(socError);
#endif
        errno = socError;
    }

    return nReaded;
}
int MYAPI RDEU_ReadXX(SOCKET h_rde,void *buf,int nBufSize,PINET_ADDR pUdpAddr)
{
    struct    sockaddr_in DestAddr;
    int nReaded;
    int socError;

    socklen_t nAddrSize=_SIZE_OF(DestAddr);
    nReaded=recvfrom(h_rde,(char *)buf,nBufSize,0,(struct sockaddr*)&DestAddr,&nAddrSize);
    if (nReaded < 1)
    {
#if defined(WIN32) || defined(_WIN32_WCE)
        socError = WSAGetLastError();
#else
        socError = errno;
#endif
    }


    if( pUdpAddr )
    {
        pUdpAddr->port = ntohs(DestAddr.sin_port);
        pUdpAddr->addr = *(INT_IP *)(&DestAddr.sin_addr);
    }
    if (nReaded < 1)
    {
#if defined(WIN32) || defined(_WIN32_WCE)
        WSASetLastError(socError);
#endif
        errno = socError;
    }



    return nReaded;
}



















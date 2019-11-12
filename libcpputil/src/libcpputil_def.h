
#ifndef _LIBCPPUTIL_DEF_H_
#define _LIBCPPUTIL_DEF_H_

//#define _SELECT_ONLY


#ifdef _SELECT_ONLY
#define _NO_NET_ASYNC 
#endif

#define _NET_SELECT_

#if !defined(_IOS_) && !defined(_android_) && !defined(ARM)

	#if defined(WIN32)
	#define FD_SETSIZE      500
	#else
	#define _NET_SELECT_POLL
	#define FD_SETSIZE      65535
	#endif

#endif

#ifdef WIN32
    #ifdef _WIN32_LIBCPPUTIL_EXPORTS
        #undef  _CPP_UTIL_EXPORT
        #define _CPP_UTIL_EXPORT __declspec(dllexport)
    #endif
#else

#undef  _CPP_UTIL_EXPORT
//#define _CPP_UTIL_EXPORT    __attribute__((visibility("default")))
#define _CPP_UTIL_EXPORT

#endif

#ifdef WIN32



#include <winsock2.h>
#include <windows.h>
#include <direct.h>
#include <tchar.h>
#include <process.h>
#pragma comment(lib,"Ws2_32.lib")

typedef int socklen_t ;

#endif

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>

#if !defined(_FreeBSD_) && !defined(__DARWIN_UNIX03) 
#include <malloc.h>
#endif

#include <signal.h>



#ifndef WIN32
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>

#if defined(_NET_SELECT_POLL)
#include <poll.h>
#endif

#if !defined(_FreeBSD_)
#include <alloca.h>
#endif


    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>


#include <pthread.h>

#endif

#include "adtypes.h"
#include "adapi.h"
#include "rde.h"
#include "libmalloc2.h"




#include "libcpputil.h"
#include "libcpputil_var.h"
#include "libcpputil_net.h"
#include "libcpputil_thread.h"
#include "libcpputil_event.h"
#include "libcpputil_wait.h"
#include "libcpputil_net_imp.h"
//#define _WIN32_MEM_RQ_
#ifdef _WIN32_MEM_RQ_
void * Win32_AllocMem(size_t sb);
void Win32_FreeMem(void * p);

#undef  _MEM_ALLOC
#undef  _MEM_FREE
#undef  _MEM_FREE_
#define _MEM_ALLOC(n) Win32_AllocMem(n)
#define _MEM_FREE(n) Win32_FreeMem(n)
#define _MEM_FREE_(n) do{ if((n) && ((void*)(n))!=((void*)-1) ) Win32_FreeMem(n);n=0;}while(0)

#endif


#if !defined(_NO_NET_ASYNC)

    #if defined(WIN32)
        #define _NET_IO_IOCP_
    #elif defined(__DARWIN_UNIX03) || defined(_Darwin_) || defined(_FreeBSD_)
        #include <sys/event.h>
        #include <mach/mach_time.h>
        #define _NET_IO_KQUEUE_
    #else
        #include <sys/epoll.h>
        #define _NET_IO_EPOLL_
    #endif

#endif



#ifndef SD_BOTH
#define SD_BOTH 2
#endif

#define _TAGNAME "libcpputil"

#if defined(_DEBUG_) || defined(_DEBUG)
#define  _DBG_PRNT(x)  printf(x)
#else
#define  _DBG_PRNT(x)
#endif

//#if defined(_DEBUG_) || defined(_DEBUG)
#ifndef _EM86X_
//#define _DEBUG_OBJ_REF_
#endif
//#endif

#if defined(_DEBUG_OBJ_REF_)

extern  INT32 _nCObjConnContext ;

extern  INT32 _nCObjNetIOBufferDefault ;
extern  INT32 _nCObjUtil ;
extern  INT32 _nCPkgConnContext ;
extern  INT32 _nCObjEventQueue;
extern  INT32 _nCObjThread;


#endif


#ifndef WIN32
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#define closesocket(x) close(x)
#define stricmp  (strcasecmp)
#endif



#if defined(WIN32)
HANDLE libcpputil_RemoveHandle(HANDLE *pArray, int nCount,int index2remove);
    #ifndef  _MSC_VER
    typedef BOOL (PASCAL FAR * LPFN_ACCEPTEX)(
        IN SOCKET sListenSocket,
        IN SOCKET sAcceptSocket,
        IN PVOID lpOutputBuffer,
        IN DWORD dwReceiveDataLength,
        IN DWORD dwLocalAddressLength,
        IN DWORD dwRemoteAddressLength,
        OUT LPDWORD lpdwBytesReceived,
        IN LPOVERLAPPED lpOverlapped
        );
    typedef VOID
    (PASCAL FAR * LPFN_GETACCEPTEXSOCKADDRS)(
        IN PVOID lpOutputBuffer,
        IN DWORD dwReceiveDataLength,
        IN DWORD dwLocalAddressLength,
        IN DWORD dwRemoteAddressLength,
        OUT struct sockaddr **LocalSockaddr,
        OUT LPINT LocalSockaddrLength,
        OUT struct sockaddr **RemoteSockaddr,
        OUT LPINT RemoteSockaddrLength
        );
    #define WSAID_GETACCEPTEXSOCKADDRS \
            {0xb5367df2,0xcbac,0x11cf,{0x95,0xca,0x00,0x80,0x5f,0x48,0xa1,0x92}}
    #define WSAID_ACCEPTEX \
        {0xb5367df1,0xcbac,0x11cf,{0x95,0xca,0x00,0x80,0x5f,0x48,0xa1,0x92}}

    #endif

#else


void libcpputil_maketimeout_milli(struct timespec *tsp, int nMilli);
#endif

int  libcpputil_GetBindName(SOCKET h_rde, int *PeerPort, char *szPeerIP);
int  libcpputil_GetPeerName(SOCKET h_rde, int *PeerPort, char *szPeerIP);
SOCKET  libcpputil_udp_open(INT_IP nBindAddr,int nPort);


int  libcpputil_loop_send(SOCKET h_rde,const void *buffer,int size,BOOL *bError);
TIME_T  libcpputil_UpSeconds();
void libcpputil_close_ctx(CObjConnContext *ctx );
void SSLFree(CObjConnContext *ctx);
void InitSSL();

extern BOOL _libCpputilSupportSSL;

#endif





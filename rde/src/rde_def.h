



#if  defined(_WIN32_WCE)
//#pragma comment (lib,"adapi.lib")  
#pragma comment (lib,"coredll.lib")
#pragma comment (lib,"commctrl.lib")
#pragma comment (lib,"Mmtimer.lib")
#pragma comment (lib,"ws2.lib")
typedef int socklen_t;
#elif defined(_WIN32)
typedef int socklen_t;
#include "Winsock2.h"
#pragma comment (lib,"Ws2_32.lib")

#else

#include "sys/select.h"
#include "sys/types.h"
#include "sys/socket.h"
#include "sys/stat.h"
#include "unistd.h"

#include "errno.h"

#if !defined(_FreeBSD_)  && !defined(__DARWIN_UNIX03) 
#include "alloca.h"
#endif
#include  <signal.h>

#include "fcntl.h"

#include "netinet/in.h"
#include "arpa/inet.h"
#include "netdb.h"

#define SOCKET_ERROR -1
#define INVALID_SOCKET -1

#endif


#ifndef _NO_STDARG_
#include<stdio.h>
#endif

#include <time.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#if !defined(_FreeBSD_)   && !defined(__DARWIN_UNIX03) 
#include <malloc.h>
#endif
 


#include "adtypes.h"
#include "adapi.h"
#include "rde.h"
#include "memdbg.h"




#define LEN_ERROR  100
#define HAVE_RDE_ERROR_BUF 1
extern char szRDE_ERROR[];

#ifndef WIN32
void closesocket(int h_Rde);
#endif

#ifdef WIN32
#define MAX_HOSTNAME_LEN                128 // arb.
#define MAX_DOMAIN_NAME_LEN             128 // arb.
#define MAX_SCOPE_ID_LEN                256 // arb.
typedef struct {
    char String[4 * 4];
} IP_ADDRESS_STRING, *PIP_ADDRESS_STRING, IP_MASK_STRING, *PIP_MASK_STRING;

//
// IP_ADDR_STRING - store an IP address with its corresponding subnet mask,
// both as dotted decimal strings
//

typedef struct _IP_ADDR_STRING {
    struct _IP_ADDR_STRING* Next;
    IP_ADDRESS_STRING IpAddress;
    IP_MASK_STRING IpMask;
    DWORD Context;
} IP_ADDR_STRING, *PIP_ADDR_STRING;


typedef struct {
    char HostName[MAX_HOSTNAME_LEN + 4] ;
    char DomainName[MAX_DOMAIN_NAME_LEN + 4];
    PIP_ADDR_STRING CurrentDnsServer;
    IP_ADDR_STRING DnsServerList;
    UINT NodeType;
    char ScopeId[MAX_SCOPE_ID_LEN + 4];
    UINT EnableRouting;
    UINT EnableProxy;
    UINT EnableDns;
} FIXED_INFO, *PFIXED_INFO;

typedef DWORD (WINAPI *_GetNetworkParams)(PFIXED_INFO pFixedInfo, PULONG pOutBufLen);
#endif







BOOL MYAPI RDE_ConnectWithTimeout( SOCKET soc, struct sockaddr* addr, int sizeofaddr, int timeoutMilliseconds, BOOL *bCancel );





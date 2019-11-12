

#include "libcpputil_def.h"



static int  Socket_Init()
{
#ifdef WIN32
	WSADATA WsaData;
	int nRet;
#endif
	static BOOL bInit = FALSE;

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
		//strcpy(szRDE_ERROR,"WSASYSNOTREADY");
		break;
	case WSAVERNOTSUPPORTED :
		//strcpy(szRDE_ERROR,"WSAVERNOTSUPPORTED");
		break;
	case WSAEPROCLIM :
		//strcpy(szRDE_ERROR,"WSAEPROCLIM");
		break;
	case WSAEFAULT :
		//strcpy(szRDE_ERROR,"WSAEFAULT");
		break;
	}
#endif



	return 0;
}





void  MYAPI LibCppUtilInit(INT64 nMaxFreeMem)
{
	static BOOL bInited = FALSE ;


	malloc2_setmaxfreesize( nMaxFreeMem );


	if( bInited )
	{
		return ;
	}
	bInited = TRUE ;
	Socket_Init();
	AdAPIInit_Mtx();
	RDE_Init();
    InitSSL();

}
void MYAPI LibCppUtilGetMemState(INT64 *pnMemFree, INT64 *pnMemUsed, INT64 *pnMemMax)
{

	malloc2_getstate( pnMemFree , pnMemUsed , pnMemMax );

}


#ifndef WIN32
void libcpputil_maketimeout_milli(struct timespec *tsp, int nMilli)

{

	struct timeval now;

	gettimeofday(&now,NULL);

	tsp->tv_sec = now.tv_sec ;

	tsp->tv_nsec = now.tv_usec * 1000;

	tsp->tv_sec += nMilli/1000;
	tsp->tv_nsec += (nMilli%1000) * 1000 * 1000;

	if (tsp->tv_nsec >= 1000000000L) 
	{
		tsp->tv_sec++;
		tsp->tv_nsec -= 1000000000L;
	}
}
#endif

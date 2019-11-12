


#ifndef _libcpputil_net_udp_h_
#define _libcpputil_net_udp_h_





class    CNetAsyncUDP :
	public CObj 
{
public:
	_CPP_UTIL_DYNAMIC_DECLARE(CNetAsyncUDP)  ;
	_CPP_UTIL_CLASSNAME_DECLARE(CNetAsyncUDP) ;
	_CPP_UTIL_QUERYOBJ_DECLARE(CNetAsyncUDP) ;

	CNetAsyncUDP();
	~CNetAsyncUDP();
	BOOL BindAddr(int nPort = 80,const char * szBindAddr = NULL);
	int  GetBindPort();

	void Shutdown();
	BOOL Start();
	BOOL Send(CObjNetIOBufferUDP *pBuffer);

#ifdef WIN32
	static unsigned int WINAPI _ThreadProcWorker( void * inPar) ;
	HANDLE m_hThreadSelect ;
#else
	static void *_ThreadProcWorker( void * inPar) ;
	pthread_t m_nThreadIDSelect;
#endif
	int ThreadWorker();
	int HandleEventRead( SOCKET soc);

public:
	SOCKET m_socListen;
	BOOL   m_bShutDown;

	CObjNetUDP *m_pNetAsync;
	CObjNetIOBufferUDP *m_ioBuffer;
};










#endif








#ifndef _libcpputil_net_select_h_
#define _libcpputil_net_select_h_


class    CNetAsyncSelect :
	public CObj 
{
public:
	_CPP_UTIL_DYNAMIC_DECLARE(CNetAsyncSelect)  ;
	_CPP_UTIL_CLASSNAME_DECLARE(CNetAsyncSelect) ;
	_CPP_UTIL_QUERYOBJ_DECLARE(CNetAsyncSelect) ;

	CNetAsyncSelect();
	virtual ~CNetAsyncSelect();

    int GetConnections( );
	void LockContextList();
	void UnlockContextList();
	BOOL AddSocket2Asyn(CObjConnContext *pSocketConnectByYourself);
	BOOL BindAddr(int nPort = 80,const char * szBindAddr = NULL);
	BOOL BindAddrIp6(int nPort = 80, const char * szBindAddr = NULL);
	void CloseAllConnections();
	void CloseConnection(CObjConnContext *pContext);
	int  GetBindPort();
	int  GetBindPortIp6();
	BOOL Send(CObjConnContext *pContext, CObjNetIOBuffer *pBuffer);
	void Shutdown();
	BOOL Start();
	void SetName(const char *szName);
	BOOL Connect(const char *addr, int port, CObj *par);

	void FreeAllConnContext();

	int HandleEventAccept(CObjConnContext *pConn,BOOL ip6);
	int HandleEventRead(CObjConnContext *pConn,BOOL &bRemoved);
	int HandleEventWrite(CObjConnContext *pConn,BOOL &bRemoved);
	int HandleEventConnected(CObjConnContext *pConn, BOOL &bRemoved);
	int HandleEventWaitTimeout(int nSeconds);
	int HandleSessionTimeout();
    CObj *GetThreadContext();



	BOOL AddConnContext( CObjConnContext *pContext);
	BOOL RemoveConnContext( CObjConnContext *pContext);
	BOOL MoveConnContext2Tail( CObjConnContext *pContext);
#ifdef WIN32
	static unsigned int WINAPI _ThreadProcWorker( void * inPar) ;
	HANDLE m_hThreadSelect ;
#else
	static void *_ThreadProcWorker( void * inPar) ;
	pthread_t m_nThreadIDSelect;
#endif
	int ThreadWorker();
public:
	int m_nStackSize;
	int m_nWorkerThreadsCount;
	CObj m_listContext;
	MTX_OBJ m_mtxListContext;
	SOCKET m_socListen;
	SOCKET m_socListenIp6;
	BOOL m_bShutDown ;
	char m_szName[24];


    CObjPtr<CObj> m_threadCtx;
    MTX_OBJ m_mtxThreadCtx;

    CObjNetAsyncImp *m_pNetAsync;
};



#endif






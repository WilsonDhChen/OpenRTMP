

#if !defined(_libcpputil_net_imp_h_)
#define _libcpputil_net_imp_h_




class CSslObj;
class    CObjNetAsyncImp
{
public:


    CObjNetAsyncImp(CObjNetAsync *netObj,int nWaitTimeoutSeconds = -1,
        int nStackSize = 0, int nMaxConnections = 2000,
        CObjNetAsync::NetAsyncType netType = CObjNetAsync::NetAsyncTypeDefault);
    virtual ~CObjNetAsyncImp();
    void CleanSSL();

    void SetSslType(CObjNetAsync::NetSsl ssl );
    BOOL SetSSLCertAndKey(const char *szCertFile, const char *szKeyFile);
    void SetSessionTimeout(int nSeconds = -1);
    void SetMaxConnections(int nCount = 2000);
    BOOL BindAddr(int nPort = 80, const char * szBindAddr = NULL, BOOL bIpV6 = FALSE);
    BOOL Start();
    void Shutdown();
    int  GetBindPort(BOOL bIpV6 = FALSE);
    int  GetConnections();

    void SetSessionTimeoutType(CObjNetAsync::SessionTimeoutType type);
    CObjNetAsync::SessionTimeoutType GetSessionTimeoutType();

    BOOL SendRaw(CObjConnContext *pContext, CObjNetIOBuffer *pBuffer);
    BOOL SendRaw(CObjConnContext *pContext, const char *szBuffer, int nLen);

    BOOL Send(CObjConnContext *pContext, CObjNetIOBuffer *pBuffer);
    BOOL Send(CObjConnContext *pContext, const char *szBuffer, int nLen);



    const CObj  *  GetConetxtListHead();
    void LockContextList();
    void UnlockContextList();



    void CloseConnection(CObjConnContext *pContext);
    void CloseAllConnections();
    void SetName(const char *szName);



public:

    BOOL AddSocket2Asyn(CObjConnContext *pSocketConnectByYourself);
    BOOL AddSocket2Asyn(SOCKET soc, INET_ADDR_STR *remoteAddr = NULL, INET_ADDR_STR *localAddr = NULL);

    BOOL Connect(const char *addr, int port, CObj *par = NULL);

public:
    CObjConnContext  * AllocConnContext(CObj *par);
    CObjNetIOBuffer  * AllocReadIOBuffer();
    CObj * CreateThreadContext();
    CObj *GetThreadContext();

    // return -1 , if you want block the connection
    int OnNewConnectionIncoming(CObjConnContext *pContext);
    int OnNewConnectionOutgoing(CObjConnContext *pContext);



    int OnConnected(CObjConnContext *pContext);
    int OnConnectionClosed(CObjConnContext *pContext);




    int OnReadCompleted(CObjConnContext *pContext, CObjNetIOBuffer *pBuffer);
    int OnWriteCompleted(CObjConnContext *pContext, CObjNetIOBuffer *pBuffer);



                                  
    
    // return -1 , if you no want  close the session
    int OnEventWaitTimeout(int nSeconds);
    int OnSessionTimeout(CObjConnContext *pContext);
    int OnMaxConnectionsLimited(CObjConnContext *pContext);
    int OnEvent();
    int OnError(CObjConnContext *pContext, const char * szTxt);


public:
    CObjNetAsync::SessionTimeoutType m_SessionTimeoutType;
    int m_nMaxConnections;
    int m_nMaxSends;
    int m_nMaxWrittingBytes;
    int m_nWaitTimeoutSeconds;
    int m_nSessionTimeoutSeconds;

public:
    CRefString m_certFile;
    CRefString m_keyFile;
    CObjNetAsync::NetAsyncType m_netType;
    CObj *m_pNetObj;
    CObjNetAsync *m_pNetAsync;
    CObjNetAsync::NetSsl m_sslType;
    CSslObj *m_ssl;

    
};


































#endif






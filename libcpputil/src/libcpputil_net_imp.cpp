

#include "libcpputil_def.h"
#include "libcpputil_net_imp.h"



#include "libcpputil_net_udp.h"
#include "libcpputil_net_select.h"

BOOL _libCpputilSupportSSL = FALSE;
/************************************************************************/
/*                  CObjNetAsyncImp                                      */
/************************************************************************/

CObjNetAsyncImp::CObjNetAsyncImp(CObjNetAsync *netObj, int nWaitTimeoutSeconds,
    int nStackSize, int nMaxConnections,
    CObjNetAsync::NetAsyncType netType
    )
{
    m_ssl = NULL;
    m_pNetAsync = netObj;
    m_sslType = CObjNetAsync::NetSslNone;
    m_netType = netType;
    m_SessionTimeoutType = CObjNetAsync::SessionTimeoutAll;
    m_pNetObj = NULL;
    int nWorkerThreadsCount = 1;


    m_netType = CObjNetAsync::NetAsyncTypeSelect;

    CNetAsyncSelect *pObj = CNetAsyncSelect::CreateObj();
    pObj->m_pNetAsync = this;
    m_pNetObj = pObj;
    m_netType = CObjNetAsync::NetAsyncTypeSelect;
    pObj->m_nStackSize = nStackSize;
    pObj->m_nWorkerThreadsCount = nWorkerThreadsCount;



    m_nSessionTimeoutSeconds = -1;

    m_nMaxConnections = nMaxConnections;

    if (m_nMaxConnections > 56000)
    {
        m_nMaxConnections = 56000;
    }



    m_nMaxSends = 20000;
    m_nMaxWrittingBytes = SIZE_1M;
    m_nWaitTimeoutSeconds = nWaitTimeoutSeconds;


}
CObjNetAsyncImp::~CObjNetAsyncImp()
{


    if (m_pNetObj != NULL)
    {
        m_pNetObj->ReleaseObj();
    }

    CleanSSL();

}
void CObjNetAsyncImp::SetSessionTimeoutType(CObjNetAsync::SessionTimeoutType type)
{
    m_SessionTimeoutType = type;
}
CObjNetAsync::SessionTimeoutType CObjNetAsyncImp::GetSessionTimeoutType()
{
    return m_SessionTimeoutType;
}

void CObjNetAsyncImp::SetSslType(CObjNetAsync::NetSsl ssl)
{
    m_sslType = ssl;
}

CObj *CObjNetAsyncImp::GetThreadContext()
{

    CNetAsyncSelect * pObj = _CPP_UTIL_FORCECAST(m_pNetObj, CNetAsyncSelect);
    if (pObj == NULL)
    {
        return NULL;
    }
    return pObj->GetThreadContext();


}

CObjConnContext  * CObjNetAsyncImp::AllocConnContext(CObj *par)
{
    return  m_pNetAsync->AllocConnContext(par);
}
CObjNetIOBuffer  * CObjNetAsyncImp::AllocReadIOBuffer()
{
    return  m_pNetAsync->AllocReadIOBuffer();
}

CObj * CObjNetAsyncImp::CreateThreadContext()
{
    return m_pNetAsync->CreateThreadContext();
}
BOOL CObjNetAsyncImp::Send(CObjConnContext *pContext, CObjNetIOBuffer *pBuffer)
{

    return  SendRaw(pContext, pBuffer);

}
BOOL CObjNetAsyncImp::Send(CObjConnContext *pContext, const char *szBuffer, int nLen)
{

     return  SendRaw(pContext, szBuffer, nLen);

}


BOOL CObjNetAsyncImp::SendRaw(CObjConnContext *pContext, const char *szBuffer, int nLen)
{
    if (szBuffer == NULL)
    {
        return FALSE;
    }
    CObjNetIOBuffer    * pBuf = CObjNetIOBufferDynamic::CreateObj(nLen,256);
    if (pBuf == NULL)
    {
        return FALSE;
    }
    if (nLen == -1)
    {
        nLen = strlen(szBuffer);
    }
    if (nLen > pBuf->m_nBufLen)
    {
        pBuf->ReleaseObj();
        return FALSE;
    }
    memcpy(pBuf->m_pBuf, szBuffer, nLen);
    pBuf->m_nDataLen = nLen;

    if (SendRaw(pContext, pBuf))
    {
        return TRUE;
    }
    else
    {
        pBuf->ReleaseObj();
        return FALSE;
    }

}
BOOL CObjNetAsyncImp::SendRaw(CObjConnContext *pContext, CObjNetIOBuffer *pBuffer)
{
    if (pBuffer->m_nDataLen > pBuffer->m_nBufLen)
    {
        return FALSE;
    }


    CNetAsyncSelect * pObj = _CPP_UTIL_FORCECAST(m_pNetObj, CNetAsyncSelect);
    if (pObj == NULL || pBuffer == NULL)
    {
        return FALSE;
    }
    pBuffer->m_nLenTrans = 0;
    return pObj->Send(pContext, pBuffer);


}

int CObjNetAsyncImp::OnNewConnectionIncoming(CObjConnContext *pContext)
{

    return   m_pNetAsync->OnNewConnectionIncoming(pContext);

}
int CObjNetAsyncImp::OnNewConnectionOutgoing(CObjConnContext *pContext)
{

    return   m_pNetAsync->OnNewConnectionOutgoing(pContext);

}
int CObjNetAsyncImp::OnConnected(CObjConnContext *pContext)
{

    return   m_pNetAsync->OnConnected(pContext);

}
int CObjNetAsyncImp::OnConnectionClosed(CObjConnContext *pContext)
{

    return   m_pNetAsync->OnConnectionClosed(pContext);
 
}

int CObjNetAsyncImp::OnReadCompleted(CObjConnContext *pContext, CObjNetIOBuffer *pBuffer)
{

     return   m_pNetAsync->OnReadCompleted(pContext, pBuffer);

}
int CObjNetAsyncImp::OnWriteCompleted(CObjConnContext *pContext, CObjNetIOBuffer *pBuffer)
{

     return   m_pNetAsync->OnWriteCompleted(pContext, pBuffer);

}

void CObjNetAsyncImp::SetSessionTimeout(int nSeconds)
{
    m_nSessionTimeoutSeconds = nSeconds;
}
void CObjNetAsyncImp::SetMaxConnections(int nCount)
{
    m_nMaxConnections = nCount;


}
int CObjNetAsyncImp::OnSessionTimeout(CObjConnContext *pContext)
{
    return m_pNetAsync->OnSessionTimeout(pContext);
}
int CObjNetAsyncImp::OnEvent()
{
    return m_pNetAsync->OnEvent();
}
int CObjNetAsyncImp::OnError(CObjConnContext *pContext, const char * szTxt)
{
    return m_pNetAsync->OnError(pContext, szTxt);
}
int CObjNetAsyncImp::OnMaxConnectionsLimited(CObjConnContext *pContext)
{
    return m_pNetAsync->OnMaxConnectionsLimited(pContext);
}

int CObjNetAsyncImp::OnEventWaitTimeout(int nSeconds)
{

    return m_pNetAsync->OnEventWaitTimeout(nSeconds);
}



void CObjNetAsyncImp::LockContextList()
{


    CNetAsyncSelect * pObj = _CPP_UTIL_FORCECAST(m_pNetObj, CNetAsyncSelect);
    if (pObj == NULL)
    {
        return;
    }
    pObj->LockContextList();

}
void CObjNetAsyncImp::UnlockContextList()
{

    CNetAsyncSelect * pObj = _CPP_UTIL_FORCECAST(m_pNetObj, CNetAsyncSelect);
    if (pObj == NULL)
    {
        return;
    }
    pObj->UnlockContextList();


}
const CObj  *  CObjNetAsyncImp::GetConetxtListHead()
{

    CNetAsyncSelect * pObj = _CPP_UTIL_FORCECAST(m_pNetObj, CNetAsyncSelect);
    if (pObj == NULL)
    {
        return  NULL;
    }
    return &(pObj->m_listContext);


}

BOOL CObjNetAsyncImp::BindAddr(int nPort, const char * szBindAddrIn, BOOL bIpV6)
{
    char szBindAddr[120];
    RDE_IP_PORT(szBindAddrIn, szBindAddr, sizeof(szBindAddr), &nPort);

        CNetAsyncSelect * pObj = _CPP_UTIL_FORCECAST(m_pNetObj, CNetAsyncSelect);
        if (pObj == NULL)
        {
            return FALSE;
        }

      
         return pObj->BindAddr(nPort, szBindAddr);



}
BOOL CObjNetAsyncImp::Start()
{

        CNetAsyncSelect * pObj = _CPP_UTIL_FORCECAST(m_pNetObj, CNetAsyncSelect);
        if (pObj == NULL)
        {
            return FALSE;
        }
        return pObj->Start();

}
void CObjNetAsyncImp::SetName(const char *szName)
{

        CNetAsyncSelect * pObj = _CPP_UTIL_FORCECAST(m_pNetObj, CNetAsyncSelect);
        if (pObj == NULL)
        {
            return;
        }
        pObj->SetName(szName);

}
void CObjNetAsyncImp::CloseAllConnections()
{

        CNetAsyncSelect * pObj = _CPP_UTIL_FORCECAST(m_pNetObj, CNetAsyncSelect);
        if (pObj == NULL)
        {
            return;
        }
        pObj->CloseAllConnections();

}
void CObjNetAsyncImp::CloseConnection(CObjConnContext *pContext)
{

        CNetAsyncSelect * pObj = _CPP_UTIL_FORCECAST(m_pNetObj, CNetAsyncSelect);
        if (pObj == NULL)
        {
            return;
        }
        pObj->CloseConnection(pContext);

}
void CObjNetAsyncImp::Shutdown()
{

        CNetAsyncSelect * pObj = _CPP_UTIL_FORCECAST(m_pNetObj, CNetAsyncSelect);
        if (pObj == NULL)
        {
            return;
        }
        pObj->Shutdown();


}
int  CObjNetAsyncImp::GetConnections()
{

        CNetAsyncSelect * pObj = _CPP_UTIL_FORCECAST(m_pNetObj, CNetAsyncSelect);
        if (pObj == NULL)
        {
            return  0;
        }
        return pObj->GetConnections();


}
int  CObjNetAsyncImp::GetBindPort(BOOL bIpV6)
{

        CNetAsyncSelect * pObj = _CPP_UTIL_FORCECAST(m_pNetObj, CNetAsyncSelect);
        if (pObj == NULL)
        {
            return  0;
        }

        {
            return pObj->GetBindPort();
        }


}
BOOL CObjNetAsyncImp::Connect(const char *addr, int port, CObj *par)
{
    if (addr == NULL || addr[0] == 0)
    {
        return FALSE;
    }


        CNetAsyncSelect * pObj = _CPP_UTIL_FORCECAST(m_pNetObj, CNetAsyncSelect);
        if (pObj == NULL)
        {
            return FALSE;
        }
        return pObj->Connect(addr, port, par);


}
BOOL CObjNetAsyncImp::AddSocket2Asyn(SOCKET soc, INET_ADDR_STR *remoteAddr, INET_ADDR_STR *localAddr)
{

        return FALSE;



}
BOOL CObjNetAsyncImp::AddSocket2Asyn(CObjConnContext *pSocketConnectByYourself)
{
    return FALSE;

}

////////////////////////////////////////
#if !defined(_OPENSSL_SUPPORT)

void CObjNetAsyncImp::CleanSSL()
{
}
BOOL CObjNetAsyncImp::SetSSLCertAndKey(const char *szCertFile, const char *szKeyFile)
{
    return FALSE;
}

void SSLFree(CObjConnContext *ctx)
{

}
void InitSSL()
{
}
#endif
















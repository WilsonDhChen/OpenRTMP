

#include "libcpputil_def.h"


static int _nBufCount = 0 ;
class    CObjNetIOBufferTest :
    public CObjNetIOBuffer
{
public:
    _CPP_UTIL_DYNAMIC(CObjNetIOBufferTest)  


    CObjNetIOBufferTest()
    {
        m_pBuf =  m_szBuf ;
        m_nBufLen = sizeof(m_szBuf);
        memset(m_szBuf,'A',sizeof(m_szBuf) );
        m_pBuf[ sizeof(m_szBuf) - 1 ] = 0 ;
        m_nDataLen = m_nBufLen ;
        _nBufCount ++ ;
    }
    ~CObjNetIOBufferTest()
    {
       m_pBuf = m_pBuf ;
       _nBufCount -- ;
    }
    //virtual BOOL ResetDataPtr();
private:
    char m_szBuf[SIZE_1KB];
};
    
CObjNetAsync *NetTest = NULL;
CObjEventQueue * _EventQueue;
extern "C" void MYAPI LibCppUtilTest()
{
    LibCppUtilInit(SIZE_20M * 5);

	

    return ;
	

}
extern "C" void MYAPI LibCppUtilTestSend()
{
    _EventQueue->PostEvent(CObjEvent::CreateObj());
    BOOL bOK = FALSE ;
    if( NetTest == NULL )
    {
        return ;
    }
    if( NetTest->GetConetxtListHead()->ListIsEmpty() )
    {
        return ;
    }
    CObjConnContext *pClient = (CObjConnContext *)NetTest->GetConetxtListHead()->m_pNextObj ; 
    CObjNetIOBufferTest *pBuf =   CObjNetIOBufferTest::CreateObj();
    bOK = NetTest->Send(pClient,pBuf);
    if( !bOK )
    {
        pBuf->ReleaseObj();
    }
    return ;
}
extern "C" void MYAPI LibCppUtilTestFinish()
{
   
    if( NetTest == NULL )
    {
        return ;
    }
    NetTest->Shutdown();
    delete NetTest ;
    NetTest = NULL ;

	INT64 nMemFree, nMemUsed;
    LibCppUtilGetMemState(&nMemFree,&nMemUsed,NULL);
#ifdef WIN32
//    _asm{int 3}
#endif
    printf("------memory pool state-------\n");
    printf("nMemFree = %d\n",nMemFree);
    printf("nMemUsed = %d\n",nMemUsed);
    printf("------------------------------\n");

    DbgPrintfObjRef();
    return ;
}





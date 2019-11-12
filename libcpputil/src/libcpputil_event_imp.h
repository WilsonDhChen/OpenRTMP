
#ifndef _LIBCPPUTIL_EVENT_IMP_H_
#define _LIBCPPUTIL_EVENT_IMP_H_

class CEventImp;
class CEventThread :
	public CObj ,
    public CObjThread 
{

    _CPP_UTIL_CLASSNAME(CEventThread)  ;
    _CPP_UTIL_QUERYOBJ(CObj)  ;
public:
    static CEventThread * CreateObj(CObjEventQueue *pQueue,int nStackSize,CEventImp *pImp )
    {
        CEventThread *pObj = new CEventThread(pQueue,nStackSize,pImp);
        if( pObj == NULL )
        {
            return NULL ;
        }
        pObj->m_bAlloced = TRUE ;
        pObj->m_nObjSize = sizeof(CEventThread) ;
        pObj->AddObjRef();
        return pObj ;
    }
    CEventThread(CObjEventQueue *pQueue,int nStackSize,CEventImp *pImp)
    {
        m_nTriggeredEmptyQueue = 0 ;
        MtxInit(&m_mtxQueue, 0) ;
        m_bShutdown = FALSE ;
        m_pQueue = NULL ;
#ifdef  WIN32
        m_hEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
#else
        pthread_mutex_init( &m_mtxEvent ,NULL );
        pthread_cond_init(&m_cond,NULL);
#endif
        m_pQueue =  pQueue ;
        m_pImp = pImp ;
        m_nStackSize = nStackSize ;
    }
    virtual ~CEventThread()
    {
        WaitForExit();
#ifdef  WIN32
        ::CloseHandle(m_hEvent) ;
#else
        pthread_mutex_destroy(&m_mtxEvent);
        pthread_cond_destroy(&m_cond);
#endif
        MtxDestroy(&m_mtxQueue);

        CObj *pItemTmp = NULL ;
        while ( ( pItemTmp = m_listQueue.ListRemoveHead() ) != NULL  )
        {
            pItemTmp->ReleaseObj() ;
        }
    }
    virtual int Run();
    void Start();
    CObjEvent *GetNextEvent()
    {
        CObjEvent *pObj = NULL ;
        MtxLock(&m_mtxQueue) ;
        pObj = (CObjEvent *)m_listQueue.ListRemoveHead() ;
        if( pObj != NULL )
        {
            pObj->OnLeaveQueue();
        }
        MtxUnLock(&m_mtxQueue) ;
        return  pObj ;
    }
    void Shutdown()
    {

        m_bShutdown = TRUE ;
        NotifyEvent();

    }
    void NotifyEvent()
    {
#ifdef WIN32
        ::SetEvent(m_hEvent);
#else
        pthread_cond_signal(&m_cond);
#endif
    }
    int GetEventCount()
    {
        int nCount = 0 ;
        nCount = m_listQueue.ListItemsCount() ;
        return nCount ;
    }
    BOOL PostEvent(CObjEvent *pEvent,BOOL bNotify)
    {
        if ( pEvent == NULL || m_bShutdown )
        {
            return FALSE ;
        }
        MtxLock(&m_mtxQueue) ;
        m_listQueue.ListAddTail(pEvent);
        MtxUnLock(&m_mtxQueue)  ;

        MtxInterlockedExchange(&m_nTriggeredEmptyQueue,0);

        if( bNotify )
        {
            NotifyEvent();
        }
        
        return TRUE ;
    }
public:
    BOOL m_bShutdown ;
#ifdef WIN32
    HANDLE m_hEvent ;
#else
    pthread_mutex_t m_mtxEvent;
    pthread_cond_t m_cond;
#endif
    int m_nStackSize;
    INT32 m_nTriggeredEmptyQueue;
    CObjEventQueue *m_pQueue;
    CEventImp *m_pImp;

    CObj m_listQueue;
    MTX_OBJ m_mtxQueue;
};

typedef CEventThread *  CEventThreadPtr;

class CEventImp :
    public CObj 
{
    _CPP_UTIL_CLASSNAME(CEventImp)  ;
    _CPP_UTIL_QUERYOBJ(CObj)  ;
    static CEventImp * CreateObj(CObjEventQueue *pQueue,int nThreads ,int nStackSize,int nWaitMilliSecond  )
    {
        CEventImp *pObj = new CEventImp(pQueue,nThreads,nStackSize,nWaitMilliSecond);
        if( pObj == NULL )
        {
            return NULL ;
        }
        pObj->m_bAlloced = TRUE ;
        pObj->m_nObjSize = sizeof(CEventImp) ;
        pObj->AddObjRef();
        return pObj ;
    }
    const CObj * GetIdleThreadObj()
    {

        CObj *obj = NULL ;
        MtxLock(&m_mtxThread);
        obj =  m_listThread.m_pNextObj ;
        MtxUnLock(&m_mtxThread);

        return  obj ;
    }
	const CObj * GetNextThreadObj()
	{
		CObj *obj = NULL ;
		MtxLock(&m_mtxThread);
		if( m_curThreadIndex >= m_nThreads )
		{
			m_curThreadIndex = 0 ;
		}
		obj =  m_threads[m_curThreadIndex];

		m_curThreadIndex++ ;

		MtxUnLock(&m_mtxThread);

		return  obj ;
	}
    void SetStackSize( int nStackSize  )
    {
        m_nStackSize = nStackSize ;
    }
    void MoveThread2Head(CObj *obj)
    {
        MtxLock(&m_mtxThread);
        m_listThread.ListMoveHead(obj);
        MtxUnLock(&m_mtxThread);
    }
    void MoveThread2Tail(CObj *obj)
    {
        MtxLock(&m_mtxThread);
        m_listThread.ListMoveTail(obj);
        MtxUnLock(&m_mtxThread);
    }
    CEventImp(CObjEventQueue *pQueue,int nThreads ,int nStackSize,int nWaitMilliSecond  )
    {
		m_curThreadIndex = 0 ;
        m_nStackSize = 0 ;
        m_bShutdown = FALSE ;
        m_nTriggeredEmptyQueue = 0 ;
        MtxInit(&m_mtxQueue, 0) ;
        MtxInit(&m_mtxThread, 0) ;
		m_szName[0] = 0 ;
        m_pQueue = pQueue ;
        m_nWaitMilliSecond= nWaitMilliSecond ;
        if( nThreads < 1 )
        {
            nThreads = 1;
        }
		m_threads = new CEventThreadPtr[nThreads];
		m_nThreads =  nThreads ;
        int i = 0 ;
        CEventThread *pEventThread = NULL ;
        for( i =0 ; i < nThreads ; i ++ )
        {
            pEventThread = CEventThread::CreateObj(pQueue,nStackSize,this) ;
			m_threads[i] = pEventThread ;
            m_listThread.ListAddTail( pEventThread );
        }



    }
    BOOL Start()
    {

        //Shutdown();
        //_DBG_PRNT("CEventImp::Start\n");
        CObj * const pItemHead = &m_listThread;
        CEventThread *pItemTmp = NULL;
        CObj *pItem = NULL;

        pItem = pItemHead->m_pNextObj ; 
        while( pItem != NULL && pItem != pItemHead )
        {
            pItemTmp = (CEventThread *)pItem ;
            pItem = pItem->m_pNextObj ;
            pItemTmp->Start();
        }

        return TRUE ;
    }
    void Shutdown()
    {
        m_bShutdown = TRUE ;
        CObj * const pItemHead = &m_listThread;
        CEventThread   *pItemTmp = NULL;
        CObj  *pItem = NULL;

        pItem = pItemHead->m_pNextObj ; 
        while( pItem != NULL && pItem != pItemHead )
        {
            pItemTmp = (CEventThread *)pItem ;
            pItem = pItem->m_pNextObj ;
            pItemTmp->Shutdown();
        }

        pItem = pItemHead->m_pNextObj ; 
        while( pItem != NULL && pItem != pItemHead )
        {
            pItemTmp = (CEventThread *)pItem ;
            pItem = pItem->m_pNextObj ;
            pItemTmp->WaitForExit();
        }
        m_bShutdown = FALSE ;
    }

    int GetEventCount()
    {
        int nCount = 0 ;
        nCount = m_listQueue.ListItemsCount() ;
        return nCount ;
    }
    BOOL PostEvent(CObjEvent *pEvent,BOOL bNotify )
    {
        if ( pEvent == NULL || m_bShutdown )
        {
            return FALSE ;
        }
        int nCount = 0 ;
        MtxLock(&m_mtxQueue) ;
        m_listQueue.ListAddTail(pEvent);
        nCount = m_listQueue.ListItemsCount() ;
        MtxUnLock(&m_mtxQueue) ;

        MtxInterlockedExchange(&m_nTriggeredEmptyQueue,0);

        if( !bNotify )
        {
            return TRUE ;
        }

        CObj * const pItemHead = &m_listThread;
        CEventThread *pItemTmp = NULL;
        CObj *pItem = NULL;
        int i = 0  ;

        pItem = pItemHead->m_pNextObj ; 
        while( pItem != NULL && i < nCount && pItem != pItemHead )
        {
            pItemTmp = (CEventThread *)pItem ;
            pItem = pItem->m_pNextObj ;
            pItemTmp->NotifyEvent();
            i ++ ;
        }


        return TRUE ;
    }
    void NotifyEvent()
    {
        int nCount =  m_listQueue.ListItemsCount() ;
        CObj * const pItemHead = &m_listThread;
        CEventThread *pItemTmp = NULL;
        CObj *pItem = NULL;
        int i = 0  ;

        pItem = pItemHead->m_pNextObj ; 
        while( pItem != NULL && i < nCount && pItem != pItemHead )
        {
            pItemTmp = (CEventThread *)pItem ;
            pItem = pItem->m_pNextObj ;
            pItemTmp->NotifyEvent();
            i ++ ;
        }
    }
    CObjEvent *GetNextEvent()
    {
        CObjEvent *pObj = NULL ;
        MtxLock(&m_mtxQueue) ;
        pObj = (CObjEvent *)m_listQueue.ListRemoveHead() ;
        if( pObj != NULL )
        {
            pObj->OnLeaveQueue();
        }
        MtxUnLock(&m_mtxQueue) ;
        return  pObj ;
    }
	void SetName(const char *szName)
	{
		strcpyn( m_szName, szName , sizeof(m_szName));
	}

    virtual ~CEventImp()
    {

        Shutdown();
        CObj *pItemTmp = NULL ;

		delete [] m_threads ;

        while ( ( pItemTmp = m_listThread.ListRemoveHead() ) != NULL  )
        {
            pItemTmp->ReleaseObj() ;
        }
        while ( ( pItemTmp = m_listQueue.ListRemoveHead() ) != NULL  )
        {
            pItemTmp->ReleaseObj() ;
        }
		
        MtxDestroy(&m_mtxQueue) ;
        MtxDestroy(&m_mtxThread) ;
    }
public:
    int m_nWaitMilliSecond;
    BOOL m_bShutdown ;
    CObj m_listQueue;
    MTX_OBJ m_mtxQueue;

    CObj m_listThread;
    MTX_OBJ m_mtxThread;
    INT32 m_nTriggeredEmptyQueue;
    int m_nStackSize ;

    CObjEventQueue *m_pQueue;
	CEventThreadPtr *m_threads;
	int  m_nThreads;
	int  m_curThreadIndex;
	char m_szName[24];
};











#endif








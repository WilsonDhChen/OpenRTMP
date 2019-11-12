
#include "libcpputil_def.h"
#include "libcpputil_event_imp.h"



static INT32 _nCObjEvent = 0 ;


_CPP_UTIL_DYNAMIC_IMP(CObjEvent)
_CPP_UTIL_CLASSNAME_IMP(CObjEvent)
CObjEvent::CObjEvent()
{

    MtxInterlockedInc(&_nCObjEvent);


}
CObjEvent::~CObjEvent()
{
    MtxInterlockedDec(&_nCObjEvent);

}
int CObjEvent::ObjCount()
{
	return _nCObjEvent ;
}
void * CObjEvent::operator new( size_t cb )
{


    return malloc2(cb);


}
void CObjEvent::operator delete( void* p )
{


    free2(p);


}
void * CObjEvent::QueryObj(const char *szObjName)
{
    if( szObjName == NULL )
    {
        return NULL ;
    }

    if( strcmp( szObjName, GetObjName() ) == 0 )
    {
        return this;
    }
    else
    {
        return CObj::QueryObj(szObjName) ;
    }
}
void CObjEvent::OnLeaveQueue()
{

}
int CObjEvent::OnExecute( CObj *sender  )
{

    return 0 ;
}
BOOL CObjEvent::Post(BOOL bNotify )
{
	return FALSE ;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/

void CEventThread::Start()
{
    m_bShutdown = FALSE ;
    CObjThread::Start( m_pImp->m_nStackSize !=0 ? m_pImp->m_nStackSize : m_nStackSize );
}
int CEventThread::Run()
{
	if (m_pImp->m_szName[0] != 0)
	{
	//	LogPrint(_LOG_LEVEL_INFO, _TAGNAME, "CEventThread::Run started, %s\n", m_pImp->m_szName);
	}
	
	

    CObj *pThreadContext = m_pQueue->CreateThreadContext(this) ;
    CObjEvent  *pEvent = NULL ;
    int nTriggeredEmpty = 0 ;
    BOOL bEmptyCalled = FALSE ;
#ifdef WIN32
    DWORD nRet = 0 ;
    DWORD dwMilliseconds =  INFINITE ;
#else
    int nRet = 0 ;
    struct timespec timeout;
#endif
	goto LABEL_EVENT_THREAD;

    // _DBG_PRNT("CEventThread::Run\n");
LABEL_LOOP:

    m_pImp->MoveThread2Head(this);
#ifdef WIN32
    dwMilliseconds =  INFINITE ;
    if( m_pImp->m_nWaitMilliSecond != -1 )
    {
        dwMilliseconds = m_pImp->m_nWaitMilliSecond;
    }
    nRet = WaitForSingleObject(m_hEvent,dwMilliseconds);
    if( nRet == WAIT_FAILED )
    {
        goto LABEL_END ;
    }
    if( nRet == WAIT_TIMEOUT )
    {
        m_pQueue->OnQueueWaitTimeout(pThreadContext,this,m_pImp->m_nWaitMilliSecond ) ;
    }
#else

    if( m_pImp->m_nWaitMilliSecond == -1 )
    {
        pthread_mutex_lock(&m_mtxEvent);
        nRet = pthread_cond_wait(&m_cond,&m_mtxEvent);
        pthread_mutex_unlock(&m_mtxEvent);
        if( nRet != 0 )
        {
#if defined(_DEBUG_) || defined(_DEBUG)
            printf("pthread_cond_wait=%d;erro=%s\n",nRet,strerror(errno));
#endif
            goto LABEL_END ;
        }

    }
    else
    {


        pthread_mutex_lock(&m_mtxEvent);
        libcpputil_maketimeout_milli(&timeout,m_pImp->m_nWaitMilliSecond);
        nRet = pthread_cond_timedwait(&m_cond,&m_mtxEvent,&timeout);
        pthread_mutex_unlock(&m_mtxEvent);

		
        if(nRet == ETIMEDOUT)
        {
            m_pQueue->OnQueueWaitTimeout(pThreadContext,this,m_pImp->m_nWaitMilliSecond ) ;
			//printf("pthread_cond_timedwait ETIMEDOUT\n");
        }
        else if( nRet != 0  )
        {
#if defined(_DEBUG_) || defined(_DEBUG)
            printf("pthread_cond_timedwait=%d;erro=%s\n",nRet,strerror(nRet));
#endif
          //  goto LABEL_END ;
        }


    }

#endif
    if( m_bShutdown )
    {
        // _DBG_PRNT("CEventThread::Run m_bShutdown = TRUE\n");
        goto LABEL_END ;
    }
    m_pImp->MoveThread2Tail(this);
    bEmptyCalled = FALSE ;

LABEL_EVENT_THREAD:
    while( ( pEvent = GetNextEvent() ) != NULL )
    {
        m_pQueue->ProcessEvent(pEvent,pThreadContext);
        if( m_bShutdown )
        {
            goto LABEL_END ;
        }
    }


    nTriggeredEmpty = MtxInterlockedExchange(&m_nTriggeredEmptyQueue,1);
    if( nTriggeredEmpty == 0 )
    {
        m_pQueue->OnQueueEmptyInThread(pThreadContext,this) ;
        bEmptyCalled = TRUE ;
    }


LABEL_EVENT_ALL:
    while( ( pEvent = m_pQueue->GetNextEvent() ) != NULL )
    {
        m_pQueue->ProcessEvent(pEvent,pThreadContext);
        if( m_bShutdown )
        {
            goto LABEL_END ;
        }
    }

    nTriggeredEmpty = MtxInterlockedExchange(&(m_pImp->m_nTriggeredEmptyQueue),1);
    if( nTriggeredEmpty == 0 )
    {
        m_pQueue->OnQueueEmpty(pThreadContext,this) ;
        bEmptyCalled = TRUE  ;
    }

    if( bEmptyCalled )
    {
        if( m_pQueue->GetEventCount(this) > 0 )
        {
            goto LABEL_EVENT_THREAD ;
        }
        else if( m_pQueue->GetEventCount() > 0 )
        {
            goto LABEL_EVENT_ALL ;
        }

    }

    goto LABEL_LOOP ;

LABEL_END:
    if( pThreadContext != NULL )
    {
        pThreadContext->ReleaseObj() ;
    }
    m_bShutdown = FALSE ;
	if (m_pImp->m_szName[0] != 0)
	{
	//	LogPrint(_LOG_LEVEL_INFO, _TAGNAME, "CEventThread::Run exited, %s\n", m_pImp->m_szName);
	}
    
    return 0 ;

}


/************************************************************************/
/*                       CObjEventQueue                                 */
/************************************************************************/
#if defined(_DEBUG_OBJ_REF_)
INT32 _nCObjEventQueue = 0 ;
#endif

CObjEventQueue::CObjEventQueue(int nThreads ,int nStackSize ,int nWaitMilliSecond )
{
#if defined(_DEBUG_OBJ_REF_)
    MtxInterlockedInc(&_nCObjEventQueue) ;
#endif
    m_pEventObj = CEventImp::CreateObj( this,nThreads,nStackSize,nWaitMilliSecond ) ;
}
CObjEventQueue::~CObjEventQueue()
{
#if defined(_DEBUG_OBJ_REF_)
    MtxInterlockedDec(&_nCObjEventQueue) ;
#endif
    if( m_pEventObj != NULL )
    {

        m_pEventObj->ReleaseObj() ;
    }
}
void CObjEventQueue::SetName(const char *szName)
{
	CEventImp * pObj = _CPP_UTIL_FORCECAST(m_pEventObj,CEventImp);
    if( pObj == NULL )
    {
        _DBG_PRNT("CEventImp = NULL\n");
        return  ;
    }
	pObj->SetName(szName);
}

void CObjEventQueue::OnQueueEmpty(CObj *pThreadContext,const CObj *threadObj)
{
   // _DBG_PRNT("CObjEventQueue::OnQueueEmpty\n");
    return ;
}
void CObjEventQueue::OnQueueWaitTimeout(CObj *pThreadContext,const CObj *threadObj,int nWaitMilliSecond)
{
 //  _DBG_PRNT("CObjEventQueue::OnQueueWaitTimeout\n");
   return ;
}
void CObjEventQueue::OnQueueEmptyInThread(CObj *pThreadContext,const CObj *threadObj)
{
    //_DBG_PRNT("CObjEventQueue::OnQueueEmptyInThread\n");
    return ;
}
CObj * CObjEventQueue::CreateThreadContext(const CObj *objThread)
{
    return NULL ;
}
int CObjEventQueue::ProcessEvent(CObjEvent  * pEvent,CObj *pThreadContext)
{
    //_DBG_PRNT("CObjEventQueue::ProcessEvent\n");
    if( pEvent != NULL )
    {
        pEvent->OnExecute(pThreadContext);
        pEvent->ReleaseObj() ;
    }
    return 0 ;
}

BOOL CObjEventQueue::Start()
{
   // _DBG_PRNT("CObjEventQueue::Start\n");
    CEventImp * pObj = _CPP_UTIL_FORCECAST(m_pEventObj,CEventImp);
    if( pObj == NULL )
    {
        _DBG_PRNT("CEventImp = NULL\n");
        return FALSE ;
    }
    return pObj->Start() ;
}
void CObjEventQueue::SetStackSize( int nStackSize  )
{
    CEventImp * pObj = _CPP_UTIL_FORCECAST(m_pEventObj,CEventImp);
    if( pObj == NULL )
    {
        return  ;
    }
    pObj->SetStackSize(nStackSize) ;
    return  ;
}
void CObjEventQueue::Shutdown()
{
    CEventImp * pObj = _CPP_UTIL_FORCECAST(m_pEventObj,CEventImp);
    if( pObj == NULL )
    {
        return  ;
    }
    pObj->Shutdown() ;
    return  ;
}
void CObjEventQueue::NotifyEvent()
{
    CEventImp * pObj = _CPP_UTIL_FORCECAST(m_pEventObj,CEventImp);
    if( pObj == NULL )
    {
        return  ;
    }
    pObj->NotifyEvent() ;
	return ;
}
void CObjEventQueue::NotifyEvent(const CObj *threadObj)
{
    CEventThread * pObj =  _CPP_UTIL_FORCECAST(((CObj *)threadObj),CEventThread);
    if( pObj == NULL )
    {
        return  ;
    }
    pObj->NotifyEvent() ;
	return ;
}
BOOL CObjEventQueue::PostEvent(CObjEvent *pEvent,BOOL bNotify)
{
    CEventImp * pObj = _CPP_UTIL_FORCECAST(m_pEventObj,CEventImp);
    if( pObj == NULL )
    {
        return FALSE ;
    }
    return pObj->PostEvent(pEvent,bNotify) ;
}
const CObj * CObjEventQueue::GetIdleThreadObj()
{
    CEventImp * pObj = _CPP_UTIL_FORCECAST(m_pEventObj,CEventImp);
    if( pObj == NULL )
    {
        return FALSE ;
    }
    return pObj->GetIdleThreadObj();
}
const CObj * CObjEventQueue::GetNextThreadObj()
{
	CEventImp * pObj = _CPP_UTIL_FORCECAST(m_pEventObj,CEventImp);
	if( pObj == NULL )
	{
		return FALSE ;
	}
	return pObj->GetNextThreadObj();
}
BOOL CObjEventQueue::PostEvent(CObjEvent *pEvent,const CObj *objThread,BOOL bNotify)
{
    CEventThread * pObj =  _CPP_UTIL_FORCECAST(((CObj *)objThread),CEventThread);
    if( pObj == NULL )
    {
        return FALSE ;
    }
    return pObj->PostEvent(pEvent,bNotify);
}
CObjEvent *CObjEventQueue::GetNextEvent()
{
    CEventImp * pObj = _CPP_UTIL_FORCECAST(m_pEventObj,CEventImp);
    if( pObj == NULL )
    {
        return NULL ;
    }
    return pObj->GetNextEvent() ;
}
CObjEvent *CObjEventQueue::GetNextEvent(const CObj *threadObj)
{
    CEventThread * pObj =  _CPP_UTIL_FORCECAST(((CObj *)threadObj),CEventThread);
    if( pObj == NULL )
    {
        return FALSE ;
    }
    return pObj->GetNextEvent();
}
int CObjEventQueue::GetEventCount()
{
    CEventImp * pObj = _CPP_UTIL_FORCECAST(m_pEventObj,CEventImp);
    if( pObj == NULL )
    {
        return 0 ;
    }
    return pObj->GetEventCount() ;
}
int CObjEventQueue::GetEventCount(const CObj *threadObj)
{
    CEventThread * pObj =  _CPP_UTIL_FORCECAST(((CObj *)threadObj),CEventThread);
    if( pObj == NULL )
    {
        return FALSE ;
    }
    return pObj->GetEventCount();
}















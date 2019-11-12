

#include"adapi_def.h"

#define TIME_WAIT       10
#define TIME_WAIT_CONDITION       10
#define QUEUE_STATUS_UNLOCK 0
#define QUEUE_STATUS_LOCK 1
#define LEN_ERROR_QUEUE  40


#define _USE_OS_MUTEX_ONLY
//#define _USE_OS_MUTEX_ 
  

#ifdef _USE_OS_MUTEX_ONLY
    #ifndef _USE_OS_MUTEX_
        #define  _USE_OS_MUTEX_ 
    #endif
#endif





typedef struct tagHANDLE_QUEUE
{
    BOOL bHeapMemory;
    BOOL bCriticalSectionOnly;
    volatile unsigned int nFirst;
    volatile unsigned int nLast;
    volatile unsigned int nQueueLength;

#ifdef _USE_OS_MUTEX_
    #ifdef WIN32
        CRITICAL_SECTION winCriticalSection; 
    #else
        pthread_mutex_t  pthreadMtx;
    #endif
#else
    int nQueueStatus ;
#endif

	unsigned int nMAX_QUEUE_LENGTH;
	void *UserData;
	int  nErrorCode;
	char szErrorTxt[LEN_ERROR_QUEUE];
}HANDLE_QUEUE,*PHANDLE_QUEUE;

int MYAPI QueueHandleSize()
{
	return (int)(_SIZE_OF(HANDLE_QUEUE)+4);
}
static QUEUE_HANDLE  QueueInitSys(void *pHandleMem,const unsigned int nMaxQueue,BOOL bHeapMemory,BOOL bCriticalSectionOnly)
{
    PHANDLE_QUEUE pQueue=(PHANDLE_QUEUE)pHandleMem;
    if(!nMaxQueue||!pQueue)
    {
        return NULL ;
    }

#ifdef _USE_OS_MUTEX_ONLY
    bCriticalSectionOnly = TRUE ;
#endif

    memset(pQueue,0,sizeof(HANDLE_QUEUE));
    pQueue->nMAX_QUEUE_LENGTH=nMaxQueue;
    pQueue->bHeapMemory = !! bHeapMemory ;
    pQueue->bCriticalSectionOnly = !! bCriticalSectionOnly ;
#ifdef _USE_OS_MUTEX_
    #ifdef WIN32
        InitializeCriticalSection( &(pQueue->winCriticalSection) );
    #else
        pthread_mutex_init( &(pQueue->pthreadMtx) ,NULL );
    #endif
#else
    pQueue->nQueueStatus=QUEUE_STATUS_UNLOCK;
#endif
    return pQueue;
}
QUEUE_HANDLE MYAPI QueueInit(void *pHandleMem,const unsigned int nMaxQueue, BOOL bCriticalSectionOnly)
{
    return  QueueInitSys( pHandleMem, nMaxQueue, FALSE, bCriticalSectionOnly );
}
QUEUE_HANDLE MYAPI QueueAlloc(const unsigned int nMaxQueue, BOOL bCriticalSectionOnly)
{
	PHANDLE_QUEUE pQueue=0;
	if(!nMaxQueue)
    {
        return NULL ;
    }
	pQueue=(PHANDLE_QUEUE)_MEM_ALLOC(sizeof(HANDLE_QUEUE));
	if(!pQueue)
    {
        return NULL ;
    }
	return QueueInitSys( pQueue, nMaxQueue, TRUE ,bCriticalSectionOnly);
}
int MYAPI QueueFree(QUEUE_HANDLE pQueueHandle,int IsForce)
{
	PHANDLE_QUEUE pQueue=0;
	INVALID_PTR_RETURN(pQueueHandle,0);
	pQueue=(PHANDLE_QUEUE)(pQueueHandle);
	if( !pQueue )
    {
        return QUEUE_ERROR_OK ;
    }
    if( pQueue->nQueueLength > 0 )
    {
        return QUEUE_ERROR_USED ;
    }

#ifdef _USE_OS_MUTEX_
    #ifdef WIN32
        DeleteCriticalSection( &(pQueue->winCriticalSection) );
    #else
        pthread_mutex_destroy( &(pQueue->pthreadMtx) );
    #endif
#endif

    if( !pQueue->bHeapMemory )
    {
        return QUEUE_ERROR_OK ;
    }

    _MEM_FREE(pQueue);

    return QUEUE_ERROR_OK ;
}

void MYAPI QueueSetUserData(QUEUE_HANDLE pQueue,void *UserData)
{
	INVALID_PTR(pQueue);
	((PHANDLE_QUEUE)pQueue)->UserData=UserData;

}
void * MYAPI QueueGetUserData(QUEUE_HANDLE pQueue)
{
	INVALID_PTR_RETURN(pQueue,0);
	return ((PHANDLE_QUEUE)pQueue)->UserData;
}
unsigned int MYAPI QueueCurLength(QUEUE_HANDLE pQueue)
{
	INVALID_PTR_RETURN(pQueue,0);
	return ((PHANDLE_QUEUE)pQueue)->nQueueLength;
}
const char * MYAPI QueueErrorTxt(QUEUE_HANDLE pQueueHandle)
{
	PHANDLE_QUEUE pQueue=0;
	INVALID_PTR_RETURN(pQueueHandle,0);
	pQueue=(PHANDLE_QUEUE)pQueueHandle;
	return pQueue->szErrorTxt;
}
int MYAPI QueueErrorCode(QUEUE_HANDLE pQueueHandle)
{
	PHANDLE_QUEUE pQueue=0;
	INVALID_PTR_RETURN(pQueueHandle,0);
	pQueue=(PHANDLE_QUEUE)pQueueHandle;
	return pQueue->nErrorCode;
}
void MYAPI QueueUnlock(QUEUE_HANDLE pQueueHandle)
{
	PHANDLE_QUEUE pQueue=0;
	INVALID_PTR(pQueueHandle);
	pQueue=(PHANDLE_QUEUE)pQueueHandle;

#ifdef  _USE_OS_MUTEX_
    if ( pQueue->bCriticalSectionOnly )
    {
    #ifdef WIN32 
            LeaveCriticalSection(  &(pQueue->winCriticalSection) );
    #else
            pthread_mutex_unlock( &(pQueue->pthreadMtx) );
            
    #endif
        return ;
    }
#endif

	pQueue->nQueueLength--;
	pQueue->nFirst++;
	if(pQueue->nFirst>pQueue->nMAX_QUEUE_LENGTH)
	{
		pQueue->nFirst=0;
	}
    return ;
}

int MYAPI WaitConditionEqual(int *pCondition,int nValue,unsigned int nWaitTime,int *IsStop)
{
	unsigned int nTimeOut=0;
	int nStop=0;
	if(!pCondition)
	{
		return _WAIT_ERROR;
	}
	if(!IsStop)
	{
		IsStop=&nStop;
	}
	while ( *(volatile int *)pCondition != nValue )
	{
		if(nWaitTime!=(unsigned int)(-1) && nTimeOut>nWaitTime)
		{
			return _WAIT_TIMEOUT;
		}
		if(*IsStop)
		{
			return _WAIT_CANCEL;
		}
		SleepMilli(TIME_WAIT_CONDITION);
		nTimeOut+=TIME_WAIT_CONDITION;

	}
	return _WAIT_OK;
}
int MYAPI WaitConditionNoEqual(int *pCondition,int nValue,unsigned int nWaitTime,int *IsStop)
{
	unsigned int nTimeOut=0;
	int nStop=0;
	if(!pCondition)
	{
		return _WAIT_ERROR;
	}
	if(!IsStop)
	{
		IsStop=&nStop;
	}
	while ( *(volatile int *)pCondition == nValue )
	{
		if(nWaitTime!=(unsigned int)(-1) && nTimeOut>nWaitTime)
		{
			return _WAIT_TIMEOUT;
		}
		if(*IsStop)
		{
			return _WAIT_CANCEL;
		}
		SleepMilli(TIME_WAIT_CONDITION);
		nTimeOut+=TIME_WAIT_CONDITION;

	}
	return _WAIT_OK;
}
int MYAPI QueueWaitLockEx(QUEUE_HANDLE pQueueHandle,unsigned int nWaitTime,int *pIsStop)
{
	PHANDLE_QUEUE pQueue=0;
	unsigned int nTimeOut=0;
	int nMyQueue,nQueueLeaved;
	unsigned int nFirst;
	INVALID_PTR_RETURN(pQueueHandle,QUEUE_ERROR_INVALID_PARAMETER);
	pQueue=(PHANDLE_QUEUE)pQueueHandle;

#ifdef  _USE_OS_MUTEX_

    #ifdef WIN32 
        EnterCriticalSection( &(pQueue->winCriticalSection) );
    #else
        pthread_mutex_lock( &(pQueue->pthreadMtx) );
    #endif

    if ( pQueue->bCriticalSectionOnly )
    {
        return QUEUE_ERROR_OK ;
    }

#else
    
    while( *(( volatile int *)&(pQueue->nQueueStatus))!=QUEUE_STATUS_UNLOCK)
    {
        if(pIsStop&&*pIsStop)
        {
            return QUEUE_ERROR_CANCEL_BYUSER;
        }
        SleepMilli(TIME_WAIT);
        nTimeOut+=TIME_WAIT;
        if(nWaitTime!=(unsigned int)(-1)&&nTimeOut>nWaitTime)
        {
            pQueue->nErrorCode=QUEUE_ERROR_TIMEOUT;
            strcpyn(pQueue->szErrorTxt,"Queue is loacked ,wait timeout",LEN_ERROR_QUEUE);
            return QUEUE_ERROR_TIMEOUT;
        }
    }
    pQueue->nQueueStatus=QUEUE_STATUS_LOCK;
        
#endif

	if(pQueue->nQueueLength >= pQueue->nMAX_QUEUE_LENGTH)
	{
        pQueue->nErrorCode=QUEUE_ERROR_FULL;
        strcpyn(pQueue->szErrorTxt,"Queue is full",LEN_ERROR_QUEUE);

    #ifdef  _USE_OS_MUTEX_
        #ifdef WIN32 
                LeaveCriticalSection(  &(pQueue->winCriticalSection) );
        #else
                pthread_mutex_unlock( &(pQueue->pthreadMtx) );
        #endif
    #else
        pQueue->nQueueStatus=QUEUE_STATUS_UNLOCK;
    #endif

		return QUEUE_ERROR_FULL;
	}
	nFirst=pQueue->nFirst;
	nMyQueue=pQueue->nQueueLength;
	pQueue->nQueueLength++;
	pQueue->nLast++;
	if(pQueue->nLast >= pQueue->nMAX_QUEUE_LENGTH)
	{
		pQueue->nLast=0;
	}
#ifdef  _USE_OS_MUTEX_
    #ifdef WIN32 
        LeaveCriticalSection(  &(pQueue->winCriticalSection) );
    #else
        pthread_mutex_unlock( &(pQueue->pthreadMtx) );    
    #endif
#else
    pQueue->nQueueStatus=QUEUE_STATUS_UNLOCK;
#endif
	/*************************************/	

	while(1)
	{
		nQueueLeaved=pQueue->nFirst-nFirst;

		if(nQueueLeaved<0)
		{
			nQueueLeaved+=pQueue->nMAX_QUEUE_LENGTH;
		}
		if(nQueueLeaved<nMyQueue)
		{
			if(pIsStop&&*pIsStop)
			{
				QueueUnlock(pQueue);
				pQueue->nErrorCode=QUEUE_ERROR_CANCEL_BYUSER;
				strcpyn(pQueue->szErrorTxt,"Queuing Stoped by user",LEN_ERROR_QUEUE);
				return QUEUE_ERROR_CANCEL_BYUSER;
			}
			SleepMilli(TIME_WAIT);
			nTimeOut+=TIME_WAIT;
			if(nWaitTime!=(unsigned int)(-1)&&nTimeOut>nWaitTime)
			{
				//Wait Queue time out
				QueueUnlock(pQueue);
				pQueue->nErrorCode=QUEUE_ERROR_TIMEOUT;
				strcpyn(pQueue->szErrorTxt,"Queuing is timeout",LEN_ERROR_QUEUE);
				return QUEUE_ERROR_TIMEOUT;
			}
		}
		else
		{
			break;
		}
	}

	return QUEUE_ERROR_OK;

}


int MYAPI QueueWaitLock(QUEUE_HANDLE pQueueHandle,unsigned int nWaitTime)
{
	return QueueWaitLockEx(pQueueHandle,nWaitTime,0);
}


const char * MYAPI QueueErrStr(int nErrorCode)
{
    static char szQUEUE_ERROR_OK[] = "QUEUE_ERROR_OK";
    static char szQUEUE_ERROR_INVALID_PARAMETER[] = "QUEUE_ERROR_INVALID_PARAMETER";
    static char szQUEUE_ERROR_TIMEOUT[]="QUEUE_ERROR_TIMEOUT";
    static char szQUEUE_ERROR_FULL[]="QUEUE_ERROR_FULL";
    static char szQUEUE_ERROR_CANCEL_BYUSER[] ="QUEUE_ERROR_CANCEL_BYUSER";
    static char szQUEUE_ERROR_USED[] = "QUEUE_ERROR_USED";
    static char szQUEUE_ERROR_UNKNOWN[] = "QUEUE_ERROR_UNKNOWN";
    switch(nErrorCode)
    {
    case QUEUE_ERROR_OK:
        {
            return szQUEUE_ERROR_OK ;
        }
    case QUEUE_ERROR_INVALID_PARAMETER:
        {
            return szQUEUE_ERROR_INVALID_PARAMETER;
        }
    case QUEUE_ERROR_TIMEOUT:
        {
            return szQUEUE_ERROR_TIMEOUT;
        }
    case QUEUE_ERROR_FULL:
        {
            return szQUEUE_ERROR_FULL;
        }
    case QUEUE_ERROR_CANCEL_BYUSER:
        {
            return szQUEUE_ERROR_CANCEL_BYUSER;
        }
    case QUEUE_ERROR_USED:
        {
            return szQUEUE_ERROR_USED;
        }
    default:
        {
            return szQUEUE_ERROR_UNKNOWN;
        }
    }
}

#ifndef _NO_STDARG_
int MYAPI QueueLogFileX(QUEUE_HANDLE pQueueHandle,
						unsigned int nWaitTime,
						const char *Path,
						const char *fmt, 
						...)
{
    FILE *pFile;
    va_list args;
	
	if(!Path||!Path[0])return 0;
	if(pQueueHandle)
	{
		if(QueueWaitLock(pQueueHandle,nWaitTime)!=QUEUE_ERROR_OK)
		{
			return 0;
		}
	}
	
    pFile=fopen(Path,"a");
    if(!pFile)
    {
		if(pQueueHandle)QueueUnlock(pQueueHandle);
		return 0;
    }

    va_start(args, fmt);
    vfprintf(pFile,fmt,args);
    va_end(args);

    /////////////////////////
    fclose(pFile);
	if(pQueueHandle)QueueUnlock(pQueueHandle);
    return 1;
}
#endif

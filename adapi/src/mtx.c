




#include"adapi_def.h"



BOOL MYAPI MtxInit( MTX_OBJ * pMtxObj,int nMtxOpt)
{

    return MtxInitEx(pMtxObj,4000,nMtxOpt);
}



BOOL MYAPI MtxInitEx( MTX_OBJ * pMtxObj,int nSpinCount,int nMtxOpt)
{

#ifdef WIN32
    if( !pMtxObj )
    {
        return FALSE ;
    }

  //  if( ! InitializeCriticalSectionAndSpinCount(pMtxObj,nSpinCount) )
    {
        InitializeCriticalSection( pMtxObj ) ;
    }
    return TRUE; 
#else
    pthread_mutexattr_t attr;
    int threadOpt;
    switch(nMtxOpt)
    {
    case _MTX_OPT_FAST:
        {
            /*
            pthread_mutexattr_init(&attr);
            pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_DEFAULT);
            pthread_mutex_init( pMtxObj ,&attr );
            pthread_mutexattr_destroy(&attr);
      */
            pthread_mutex_init( pMtxObj ,NULL );
            break ;
        }

    case _MTX_OPT_RECURSIVE:
        {
            
#if   defined(_USE_PTHREAD_MUTEX_RECURSIVE_NP)
			pthread_mutexattr_init(&attr);
            pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE_NP);
			pthread_mutex_init( pMtxObj ,&attr );
			pthread_mutexattr_destroy(&attr);
#elif defined(_USE_PTHREAD_MUTEX_RECURSIVE) 
			pthread_mutexattr_init(&attr);
			pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE);
			pthread_mutex_init( pMtxObj ,&attr );
			pthread_mutexattr_destroy(&attr);
#else
			goto LABEL_DEFAULT ;
#endif

            break ;
        }

#if  defined(PTHREAD_MUTEX_ERRORCHECK_NP)
    case _MTX_OPT_ERRORCHECK:
        {
            pthread_mutexattr_init(&attr);
            pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_ERRORCHECK_NP);
            pthread_mutex_init( pMtxObj ,&attr );
            pthread_mutexattr_destroy(&attr);
            break ;
        }
#endif
    default:
        {
	
LABEL_DEFAULT: ;
			
#if defined(_USE_PTHREAD_MUTEX_RECURSIVE_NP)
			pthread_mutexattr_init(&attr);
			pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE_NP);
			pthread_mutex_init( pMtxObj ,&attr );
			pthread_mutexattr_destroy(&attr);
#elif defined(_USE_PTHREAD_MUTEX_RECURSIVE) 
			
			pthread_mutexattr_init(&attr);
			pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE);
			pthread_mutex_init( pMtxObj ,&attr );
			pthread_mutexattr_destroy(&attr);
#else
            pthread_mutex_init( pMtxObj ,NULL );
#endif
            break ;
        }

    }

    return TRUE ;
#endif

}

BOOL MYAPI MtxTryLock( MTX_OBJ * pMtxObj)
{
    int nRet = 0 ;
    if( !pMtxObj )
    {
        return FALSE ;
    }
#ifdef WIN32 
    #if(_WIN32_WINNT >= 0x0400)
    nRet = TryEnterCriticalSection( pMtxObj ); 
    return nRet ;
    #else
    return FALSE ;
    #endif
#else
    nRet = pthread_mutex_trylock( pMtxObj ) ;
    return (  nRet == 0 );
#endif
}

BOOL MYAPI MtxLock( MTX_OBJ * pMtxObj)
{
    int nRet = 0 ;
    if( !pMtxObj )
    {
        return FALSE ;
    }
    //printf("---------------------MtxLock--------------------\n");
#ifdef WIN32 
    EnterCriticalSection( pMtxObj ); 
    return TRUE  ;
#else
    nRet = pthread_mutex_lock( pMtxObj ) ;
    return (  nRet == 0 );
#endif
}

BOOL MYAPI MtxUnLock( MTX_OBJ * pMtxObj)
{
    int nRet = 0 ;
    if( !pMtxObj )
    {
        return FALSE ;
    }
    //printf("---------------------MtxUnLock--------------------\n");
#ifdef WIN32 
    LeaveCriticalSection( pMtxObj ); 
    return TRUE  ;
#else
    nRet = pthread_mutex_unlock( pMtxObj ) ;
    return (  nRet == 0 );
#endif
}

BOOL MYAPI MtxDestroy( MTX_OBJ * pMtxObj)
{
    if( !pMtxObj )
    {
        return FALSE ;
    }
#ifdef WIN32 
    DeleteCriticalSection( pMtxObj ); 
    return TRUE  ;
#else
    pthread_mutex_destroy( pMtxObj ) ;
    return TRUE;
#endif
}



static MTX_OBJ _MtxObjInternal;
static BOOL _bMtxInited = FALSE ;
static MTX_OBJ _MtxObjInternalCount;

void MYAPI AdAPIInit_Mtx()
{
    if( ! _bMtxInited )
    {
        _bMtxInited = TRUE ;
        MtxInit(&_MtxObjInternal,0);
        MtxInit(&_MtxObjInternalCount,0);
    }

}
void MYAPI MtxInternalLock()
{
    MtxLock( &_MtxObjInternal );
}
void MYAPI MtxInternalUnlock()
{
    MtxUnLock( &_MtxObjInternal );
}

INT32 MYAPI MtxInterlockedExchange(INT32 volatile *pCount,INT32 nNewValue)
{
#ifdef WIN32
    return InterlockedExchange(pCount , nNewValue );
//#elif defined(_Linux_)
//	return __sync_fetch_and_set((pCount),nNewValue);
#else
    int nRet = 0 ;
    if( pCount == NULL )
    {
        return 0 ;
    }

    MtxLock( &_MtxObjInternalCount );
    nRet = (*pCount) ;
    (*pCount) = nNewValue ;
    MtxUnLock( &_MtxObjInternalCount );

    return   nRet ;
#endif
}
INT32 MYAPI MtxInterlockedInc(INT32 volatile *pCount)
{
#ifdef WIN32
    return InterlockedIncrement( pCount );
#else
    long nRet = 0 ;
    if( pCount == NULL )
    {
        return 0 ;
    }

    MtxLock( &_MtxObjInternalCount );
    (*pCount) ++ ;
    nRet = (*pCount) ;
    MtxUnLock( &_MtxObjInternalCount );

    return   nRet ;
#endif

}
INT32 MYAPI MtxInterlockedDec(INT32 volatile *pCount)
{
#ifdef WIN32
    return InterlockedDecrement( pCount );
#else
    long nRet = 0 ;
    if( pCount == NULL )
    {
        return 0 ;
    }

    MtxLock( &_MtxObjInternalCount );
    (*pCount) -- ;
    nRet = (*pCount) ;
    MtxUnLock( &_MtxObjInternalCount );

    return   nRet ;
#endif
}

#if 1
INT64 MYAPI MtxInterlockedInc64(INT64 volatile *pCount)
{
#ifdef _WIN64
	return InterlockedIncrement64(pCount);
#else
	INT64 nRet = 0;
	if (pCount == NULL)
	{
		return 0;
	}

	MtxLock(&_MtxObjInternalCount);
	(*pCount)++;
	nRet = (*pCount);
	MtxUnLock(&_MtxObjInternalCount);

	return   nRet;
#endif

}
INT64 MYAPI MtxInterlockedDec64(INT64 volatile *pCount)
{
#ifdef _WIN64
	return InterlockedDecrement64(pCount);
#else
	INT64 nRet = 0 ;
	if( pCount == NULL )
	{
		return 0 ;
	}

	MtxLock(&_MtxObjInternalCount);
	(*pCount)--;
	nRet = (*pCount);
	MtxUnLock(&_MtxObjInternalCount);

	return   nRet;
#endif
}

#endif

#ifndef  WIN32
typedef struct tagOBJ_WAIT
{
	pthread_mutex_t mtx;
	pthread_cond_t cond;
	int  bSignal;
	int bManualReset;
}OBJ_WAIT;
#endif
HEVWAIT MYAPI EvWaitCreate( BOOL  bManualReset )
{
	
#ifdef WIN32
	return (HEVWAIT)CreateEvent(NULL, bManualReset, FALSE, NULL);
#else

	 OBJ_WAIT *obj = NULL ;
	 pthread_mutexattr_t attr;
	 obj = (OBJ_WAIT*)_MEM_ALLOC(sizeof(OBJ_WAIT));
	 if( obj == NULL )
	 {
		 return NULL ;
	 }
	 memset( obj, 0 , sizeof(OBJ_WAIT) ) ;
	 obj->bManualReset = bManualReset ;
#if defined(_USE_PTHREAD_MUTEX_RECURSIVE_NP)
	 pthread_mutexattr_init(&attr);
	 pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE_NP);
	 pthread_mutex_init( &(obj->mtx) ,&attr );
	 pthread_mutexattr_destroy(&attr);
#elif defined(_USE_PTHREAD_MUTEX_RECURSIVE) 

	 pthread_mutexattr_init(&attr);
	 pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE);
	 pthread_mutex_init( &(obj->mtx) ,&attr );
	 pthread_mutexattr_destroy(&attr);
#else
	 pthread_mutex_init( &(obj->mtx) ,NULL );
#endif

	 pthread_cond_init(&(obj->cond),NULL);
	 return obj ;

#endif

	

}
void    MYAPI EvWaitClose( HEVWAIT hWait )
{

#ifdef WIN32
	CloseHandle( (HANDLE)hWait );
#else
	OBJ_WAIT *obj = (OBJ_WAIT *)hWait ;

	if( obj == NULL )
	{
		return ;
	}
	pthread_cond_destroy(& (obj->cond) );
	pthread_mutex_destroy(& (obj->mtx) );
	_MEM_FREE( obj );
#endif
	
}
EvWaitResult     MYAPI EvWaitForEvent(HEVWAIT hWait , int nMilliseconds)
{
#ifdef WIN32
	DWORD nRet = 0 ;
	DWORD    dwMilliseconds =  INFINITE ;
	if( nMilliseconds == -1 )
	{
		dwMilliseconds =  INFINITE ;
	}
	else
	{
		dwMilliseconds = nMilliseconds ;
	}


	nRet = WaitForSingleObject((HANDLE)hWait,dwMilliseconds);
	if( nRet == WAIT_FAILED )
	{
		return EvWaitResultError;
	}
	else if( nRet == WAIT_TIMEOUT )
	{
		return EvWaitResultTimeout ;
	}
	else
	{
		return EvWaitResultOk ;
	}

#else
	int nRet = 0 ;
	struct timespec timeout;
	OBJ_WAIT *obj = (OBJ_WAIT *)hWait ;
	if( obj == NULL )
	{
		return EvWaitResultError ;
	}
	
	if( nMilliseconds == -1 )
	{
		pthread_mutex_lock( &(obj->mtx) );

		if( obj->bSignal )
		{
			nRet = 0 ;
		}
		else
		{
			nRet = pthread_cond_wait(&(obj->cond) , &(obj->mtx)  );
		}
		
		if( !obj->bManualReset )
		{
			obj->bSignal = FALSE ;
		}
		pthread_mutex_unlock( &(obj->mtx) );


		if( nRet != 0 )
		{
			return EvWaitResultError ;
		}
		else
		{
			return EvWaitResultOk ;
		}

	}
	else
	{
		pthread_mutex_lock( &(obj->mtx) );
		//maketimeout_milli(&timeout,nMilliseconds);

		{

			struct timeval now;

			gettimeofday(&now,NULL);

			timeout.tv_sec = now.tv_sec ;

			timeout.tv_nsec = now.tv_usec * 1000;

			timeout.tv_sec += nMilliseconds/1000;
			timeout.tv_nsec += (nMilliseconds%1000) * 1000 * 1000;

			if (timeout.tv_nsec >= 1000000000L) 
			{
				timeout.tv_sec++;
				timeout.tv_nsec -= 1000000000L;
			}

		}
		if( obj->bSignal )
		{
			nRet = 0 ;
		}
		else
		{
			nRet = pthread_cond_timedwait( &(obj->cond) ,&(obj->mtx),&timeout);
		}

		
		if( !obj->bManualReset )
		{
			obj->bSignal = FALSE ;
		}
		pthread_mutex_unlock( &(obj->mtx) );



		if(nRet == ETIMEDOUT) 
		{
			return EvWaitResultTimeout ;
		}
		else if( nRet != 0  )
		{
			return EvWaitResultError ;
		}
		else
		{
			return EvWaitResultOk ;

		}

	}

#endif
}
void    MYAPI EvWaitSetEvent( HEVWAIT hWait )
{

#ifdef WIN32
	SetEvent( (HANDLE)hWait );
#else
	OBJ_WAIT *obj = (OBJ_WAIT *)hWait ;
	if( obj == NULL )
	{
		return  ;
	}
	obj->bSignal = TRUE ;
	pthread_cond_signal(& (obj->cond) );
#endif
}

BOOL    MYAPI  EvWaitResetEvent( HEVWAIT hWait )
{
#ifdef WIN32
	return ResetEvent( (HANDLE)hWait );
#else
	pthread_mutexattr_t attr;
	OBJ_WAIT *obj = (OBJ_WAIT *)hWait ;
	if( obj == NULL )
	{
		return  FALSE ;
	}
	pthread_mutex_lock( &(obj->mtx) );
	obj->bSignal = FALSE ;
	pthread_mutex_unlock( &(obj->mtx) );
	return TRUE ;
#endif
}



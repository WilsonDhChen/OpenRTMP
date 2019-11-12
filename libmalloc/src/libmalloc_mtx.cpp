



#include "libmalloc_def.h"

static BOOL_MALLOC _bMtxInited = FALSE_MALLOC ;
static MTX_OBJ _MtxObjInternalCount;

void  libmalloc2initcounter()
{
	if( ! _bMtxInited )
	{
		_bMtxInited = TRUE_MALLOC ;
		libmalloc2MtxInit(&_MtxObjInternalCount,0);
	}

}

BOOL_MALLOC   libmalloc2MtxInit( MTX_OBJ * pMtxObj,int nMtxOpt)
{

    return libmalloc2MtxInitEx(pMtxObj,4000,nMtxOpt);
}



BOOL_MALLOC   libmalloc2MtxInitEx( MTX_OBJ * pMtxObj,int nSpinCount,int nMtxOpt)
{

#ifdef WIN32
    if( !pMtxObj )
    {
        return FALSE_MALLOC ;
    }

  //  if( ! InitializeCriticalSectionAndSpinCount(pMtxObj,nSpinCount) )
    {
        InitializeCriticalSection( pMtxObj ) ;
    }
    return TRUE_MALLOC; 
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

    return TRUE_MALLOC ;
#endif

}
BOOL_MALLOC   libmalloc2MtxLock( MTX_OBJ * pMtxObj)
{
    int nRet = 0 ;
    if( !pMtxObj )
    {
        return FALSE_MALLOC ;
    }
    //printf("---------------------libmalloc2MtxLock--------------------\n");
#ifdef WIN32 
    EnterCriticalSection( pMtxObj ); 
    return TRUE_MALLOC  ;
#else
    nRet = pthread_mutex_lock( pMtxObj ) ;
    return (  nRet == 0 );
#endif
}

BOOL_MALLOC   libmalloc2MtxUnLock( MTX_OBJ * pMtxObj)
{
    int nRet = 0 ;
    if( !pMtxObj )
    {
        return FALSE_MALLOC ;
    }
    //printf("---------------------libmalloc2MtxUnLock--------------------\n");
#ifdef WIN32 
    LeaveCriticalSection( pMtxObj ); 
    return TRUE_MALLOC  ;
#else
    nRet = pthread_mutex_unlock( pMtxObj ) ;
    return (  nRet == 0 );
#endif
}

BOOL_MALLOC   libmalloc2MtxDestroy( MTX_OBJ * pMtxObj)
{
    if( !pMtxObj )
    {
        return FALSE_MALLOC ;
    }
#ifdef WIN32 
    DeleteCriticalSection( pMtxObj ); 
    return TRUE_MALLOC  ;
#else
    pthread_mutex_destroy( pMtxObj ) ;
    return TRUE_MALLOC;
#endif
}



long  libmalloc2MtxInterlockedInc(INT32 volatile *pCount)
{
#ifdef WIN32
#if  _MSC_VER  >= 1800
	return InterlockedIncrement((unsigned long volatile*)pCount);
#else
	return InterlockedIncrement(( long volatile*)pCount);
#endif
#elif defined(__DARWIN_UNIX03)
	return OSAtomicIncrement32( (int32_t *)pCount );
#elif defined(_Linux_) && defined(_GCC_ATOMIC_) 
	return __sync_add_and_fetch_4((pCount),1);
#else
	long nRet = 0 ;
	if( pCount == NULL )
	{
		return 0 ;
	}

	libmalloc2MtxLock( &_MtxObjInternalCount );
	(*pCount) ++ ;
	nRet = (*pCount) ;
	libmalloc2MtxUnLock( &_MtxObjInternalCount );

	return   nRet ;
#endif

}
long  libmalloc2MtxInterlockedDec(INT32 volatile *pCount)
{
#ifdef WIN32
#if  _MSC_VER  >= 1800
	return InterlockedDecrement((unsigned long volatile*)pCount);
#else
	return InterlockedDecrement(( long volatile*)pCount);
#endif
#elif defined(__DARWIN_UNIX03)
	return OSAtomicDecrement32( (int32_t *)pCount );
#elif defined(_Linux_) && defined(_GCC_ATOMIC_) 
	return __sync_sub_and_fetch_4((pCount),1);
#else
	long nRet = 0 ;
	if( pCount == NULL )
	{
		return 0 ;
	}

	libmalloc2MtxLock( &_MtxObjInternalCount );
	(*pCount) -- ;
	nRet = (*pCount) ;
	libmalloc2MtxUnLock( &_MtxObjInternalCount );

	return   nRet ;
#endif
}






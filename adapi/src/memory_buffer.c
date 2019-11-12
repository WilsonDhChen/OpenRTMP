




#if defined(_KERNEL_LINUX_)
	#define MYAPI
	#define _MEM_FREE(p)              (kfree)(p)
	#define _MEM_ALLOC(size)       (kmalloc)((size),GFP_KERNEL)
	typedef int BOOL ;
	#elif defined(_KERNEL_DARWIN_)
	#elif defined(_KERNEL_WIN32_)
	#elif defined(_KERNEL_WIN_CE_)
#else
	#include"adapi_def.h"
	#define _MEMORY_BUFFER_MTX_
#endif


#if defined(_MEMORY_BUFFER_MTX_)
#define _MEMORY_BUFFER_LOCK()   MtxLock( &(pMem->mtx) );    
#else
#define _MEMORY_BUFFER_LOCK()
#endif

#if defined(_MEMORY_BUFFER_MTX_)
#define _MEMORY_BUFFER_UNLOCK() MtxUnLock( &(pMem->mtx) ); 
#else
#define _MEMORY_BUFFER_UNLOCK()
#endif

typedef struct tagMEMORY_BUFFER
{
#if defined(_MEMORY_BUFFER_MTX_)
    MTX_OBJ mtx;
#endif
    int nTotalSize;
    int iFirst;
    int nDataLen;
    int nBufSize;
    int bAutoClear;
    char * buf;
}MEMORY_BUFFER;



void  MYAPI   MemoryBufferFree(void *hMem)
{
    MEMORY_BUFFER *pMem = NULL ;
    pMem = (MEMORY_BUFFER *)hMem ;
    if( hMem != NULL )
    {
#ifdef _MEMORY_BUFFER_MTX_

        MtxDestroy( &(pMem->mtx) ) ;
#endif
        _MEM_FREE(hMem);
    }
}

void *  MYAPI MemoryBufferAlloc( int nBufSize,BOOL bAutoClear)
{
    int nTotalSize = 0 ;
    MEMORY_BUFFER *pMem = NULL ;
    nTotalSize = sizeof(MEMORY_BUFFER) + nBufSize ;
    pMem = (MEMORY_BUFFER *)_MEM_ALLOC(nTotalSize);

    if( pMem != NULL )
    {
        pMem->nTotalSize = nTotalSize ;
        pMem->iFirst =  0 ;
        pMem->nDataLen = 0 ;
        pMem->nBufSize = nBufSize ;
        pMem->bAutoClear = bAutoClear ;
        pMem->buf = ((char *)pMem) + sizeof(MEMORY_BUFFER) ;
#ifdef _MEMORY_BUFFER_MTX_
        MtxInit( &(pMem->mtx) , 0 ) ;
#endif
    }
    return pMem ;
}

void MYAPI MemoryBufferReset( void *hMem )
{
    MEMORY_BUFFER *pMem = NULL ;
    pMem = (MEMORY_BUFFER *)hMem ; 
    if( pMem == NULL )
    {
        return  ;
    }
    else
    {
        pMem->nDataLen = 0 ;
        pMem->iFirst = 0 ;
    }
}
int  MYAPI MemoryBufferLength( void *hMem )
{
    MEMORY_BUFFER *pMem = NULL ;
    pMem = (MEMORY_BUFFER *)hMem ; 
    if( pMem == NULL )
    {
        return -1 ;
    }
    else
    {
        return pMem->nDataLen ;
    }
}
int MYAPI MemoryBufferRead(void *hMem,void *buf,int nSize,BOOL bCopyToUser)
{
    MEMORY_BUFFER *pMem = NULL ;
    int nCpyn = 0 ;
    int nCpyed = 0 ;
    int nMaxCpy = 0 ;
    int nTmp = 0 ;
    int nTmp2 = 0 ;

    pMem = (MEMORY_BUFFER *)hMem ; 
    if( pMem == NULL || buf == NULL )
    {
        return -1 ;
    }



    if( pMem->nDataLen < 0  )
    {
        return -1 ;
    }
    else if( pMem->nDataLen == 0  )
    {
        return 0 ;
    }
    _MEMORY_BUFFER_LOCK();

    nMaxCpy = min(nSize,pMem->nDataLen);

    while( nCpyed < nMaxCpy )
    {
        nTmp =  pMem->nBufSize -  pMem->iFirst ;
        nTmp2 = nMaxCpy - nCpyed ;
        nCpyn = min( nTmp,nTmp2 ) ;
#if defined(_KERNEL_LINUX_)
		if( bCopyToUser )
		{
			if( copy_to_user( ((char *)buf)+nCpyed,pMem->buf + pMem->iFirst, nCpyn ) )
			{
				_MEMORY_BUFFER_UNLOCK();
				return -1;
			}
		}
		else
		{
			memcpy( ((char *)buf)+nCpyed,pMem->buf + pMem->iFirst, nCpyn );
		}
#else
        memcpy( ((char *)buf)+nCpyed,pMem->buf + pMem->iFirst, nCpyn );
#endif

        nCpyed += nCpyn ;
        pMem->iFirst += nCpyn ;
        pMem->nDataLen -= nCpyn ;
        if( pMem->iFirst > pMem->nBufSize )
        {
            _MEMORY_BUFFER_UNLOCK();
            return -1;
        }
        else if( pMem->iFirst == pMem->nBufSize )
        {
            pMem->iFirst = 0 ;
        }
    }



    _MEMORY_BUFFER_UNLOCK();
    return nCpyed ;
}
int MYAPI MemoryBufferWrite(void *hMem,const void *buf,int nSize,BOOL bCopyFromUser)
{
    MEMORY_BUFFER *pMem = NULL ;
    int nCpyn = 0 ;
    int nCpyed = 0 ;
    int nMaxCpy = 0 ;
    int nTmp = 0 ;
    int nTmp2 = 0 ;
    int iCpy = 0 ;

    pMem = (MEMORY_BUFFER *)hMem ; 
    if( pMem == NULL || buf == NULL )
    {
        return -1 ;
    }
    if( nSize == 0 )
    {
        return 0 ;
    }

    _MEMORY_BUFFER_LOCK();
    if( (pMem->nDataLen + nSize ) > pMem->nBufSize  )
    {
        if( pMem->bAutoClear && nSize <=  pMem->nBufSize )
        {
#if defined(_KERNEL_LINUX_)
			printk("usbhidraw MemoryBuffer is full,cleared\n");
#endif
            pMem->nDataLen = 0 ;
            pMem->iFirst = 0 ; 
            goto LABEL_GO_ON ;
        }
        else
        {
            _MEMORY_BUFFER_UNLOCK();
            return 0 ;
        }
    }
LABEL_GO_ON:
    nMaxCpy = nSize ;

    while( nCpyed < nMaxCpy )
    {
        iCpy = ( pMem->iFirst + pMem->nDataLen ) % pMem->nBufSize ;
        nTmp =  pMem->nBufSize -  iCpy ;
        nTmp2 = nMaxCpy - nCpyed ;
        nCpyn = min( nTmp,nTmp2 ) ;
#if defined(_KERNEL_LINUX_)
		if( bCopyFromUser )
		{
			if( copy_from_user( pMem->buf + iCpy,((char *)buf)+nCpyed, nCpyn ) )
			{
				_MEMORY_BUFFER_UNLOCK();
				return -1 ;
			}
			
		}
		else
		{
			memcpy( pMem->buf + iCpy,((char *)buf)+nCpyed, nCpyn );
		}
#else
        memcpy( pMem->buf + iCpy,((char *)buf)+nCpyed, nCpyn );
#endif

        nCpyed += nCpyn ;
        pMem->nDataLen += nCpyn ;
    }

    _MEMORY_BUFFER_UNLOCK();
    return nCpyed ;
}


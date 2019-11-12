

#include "libmalloc_def.h"

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

static INT32 _sharedMemCounter = 0;
typedef struct tagSHAREDMEM
{
	INT32 ref;
	size_t cb ;
	_Destory destroy;
	int bCached;
}SHAREDMEM;
INT32    SharedMemCounter()
{
	return _sharedMemCounter;
}
void *   SharedMemCreate( size_t cb )
{
	return SharedMemCreate2( cb , NULL, TRUE );
}
void *   SharedMemCreate2( size_t cb,_Destory destroy,int bCached )
{
	static int _initCounter = 0 ;
	SHAREDMEM *p = NULL ;
	if( !_initCounter )
	{
		libmalloc2initcounter();
	}
	
	if( bCached )
	{
		p = (SHAREDMEM*)malloc2( sizeof(SHAREDMEM) + cb ) ;
	}
	else
	{
		p = (SHAREDMEM*)malloc( sizeof(SHAREDMEM) + cb ) ;
	}
	
	if( p == NULL )
	{
		return NULL ;
	}

	libmalloc2MtxInterlockedInc(&_sharedMemCounter);
	p->cb = cb ;
	p->ref = 1 ;
	p->destroy = destroy ;
	p->bCached = bCached ;

	return (((char *)p)+ sizeof(SHAREDMEM)) ;
}
size_t   SharedMemSize( void *p1 )
{
	SHAREDMEM *p = NULL ;
	if( p1 == NULL )
	{
		return 0 ;
	}
	p = (SHAREDMEM *)((char *)p1 - sizeof(SHAREDMEM) );
	if( p == NULL )
	{
		return 0 ;
	}
	return p->cb ;
}
INT32     SharedMemAddRef(void *p1)
{
	SHAREDMEM *p = NULL ;
	if( p1 == NULL )
	{
		return 0 ;
	}
	p = (SHAREDMEM *)((char *)p1 - sizeof(SHAREDMEM) );
	if( p == NULL )
	{
		return 0 ;
	}
	return libmalloc2MtxInterlockedInc(&(p->ref));
}
INT32     SharedMemRelease(void *p1)
{
	SHAREDMEM *p = NULL ;
	long ref = 0 ;
	if( p1 == NULL )
	{
		return 0 ;
	}
	p = (SHAREDMEM *)((char *)p1 - sizeof(SHAREDMEM) );
	if( p == NULL )
	{
		return 0 ;
	}
	ref =  libmalloc2MtxInterlockedDec(&(p->ref));
	if( ref < 1 )
	{
		if( p->destroy != NULL )
		{
			p->destroy(p1);
		}
		if( p->bCached )
		{
			free2(p);
		}
		else
		{
			free(p);
		}
		libmalloc2MtxInterlockedDec(&_sharedMemCounter);
	}
	return ref ;
}













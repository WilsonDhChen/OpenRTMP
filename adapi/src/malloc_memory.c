

#include "adapi_def.h"



typedef struct tagMEMTAG
{
	UINT32 ref ;
	size_t cb ;
	__Destory destroy;
}MEMTAG;


char *  MYAPI  MemoryEmptyString( )
{
    static char *szNULL = NULL ;

    if( szNULL != NULL )
    {
        MemoryAddRef( szNULL  );
        return szNULL ;
    }

    szNULL = ( char * )MemoryCreate( 8, NULL );
    memset( szNULL, 0 , 8 ) ;
    MemoryAddRef( szNULL );
    return szNULL;
}
char *  MYAPI  MemoryString( const char *str , int nLen , int align )
{
    char *pStr = NULL ;
    if( str == NULL || str[ 0 ] == 0 )
    {
        return MemoryEmptyString();
    }

    if( nLen == -1 )
    {
        nLen = strlen( str );
    }

    align--;

    align = (nLen + 1 + sizeof(int) + align) & (~align);

    pStr = ( char * ) MemoryCreate( align, NULL );

    if( pStr != NULL )
    {
        memcpy( pStr, str, nLen );
        pStr[ nLen ] = 0 ;

        memcpy(pStr + align - sizeof(int), &nLen, sizeof(int));
    }

    return pStr ;

}
int  MYAPI  MemoryStringLen(const char *str)
{
    int len = 0;
    int size = MemorySize((void *)str);
    if (size >= sizeof(int))
    {
        memcpy(&len, str + size - sizeof(int), sizeof(int));
    }


    return len;
}
void *  MYAPI  MemoryCreate( size_t cb,__Destory destroy )
{

	MEMTAG *p = NULL ;

	if( cb == 0 )
	{
		return NULL ;
	}
	p = (MEMTAG*)malloc( sizeof(MEMTAG) + cb  ) ;
	
	if( p == NULL )
	{
		return NULL ;
	}
	p->cb = cb ;
	p->ref = 1 ;
	p->destroy = destroy ;
	return (((char *)p)+ sizeof(MEMTAG)) ;
}
void *  MYAPI  MemoryAlloc( size_t cb )
{
	return MemoryCreate( cb , NULL );
}
void *  MYAPI  MemoryRealloc( void *p1,size_t cb )
{
	MEMTAG *p = NULL ;
	MEMTAG *pR = NULL ;
	if( p1 == NULL )
	{
		return MemoryAlloc( cb );
	}
	else
	{
		p = (MEMTAG *)((char *)p1 - sizeof(MEMTAG) );
		if( p == NULL )
		{
			return 0 ;
		}
		pR = realloc( p , cb + sizeof(MEMTAG)  );

		if( pR == NULL )
		{
			pR = malloc(  cb + sizeof(MEMTAG)  );
			if( pR == NULL )
			{
				return NULL ;
			}
			else
			{
				memcpy( pR, p, MinInt(cb + sizeof(MEMTAG),p->cb + sizeof(MEMTAG)) );
				
				free(p);
			}
		}
		pR->cb = cb;
		return (((char *)pR)+ sizeof(MEMTAG)) ;
	}
}
size_t  MYAPI  MemorySize( void *p1 )
{
	MEMTAG *p = NULL ;
	if( p1 == NULL )
	{
		return 0 ;
	}
	p = (MEMTAG *)((char *)p1 - sizeof(MEMTAG) );
	if( p == NULL )
	{
		return 0 ;
	}
	return p->cb ;
}
INT32    MYAPI  MemoryAddRef( void *p1   )
{
	MEMTAG *p = NULL ;
	if( p1 == NULL )
	{
		return 0 ;
	}
	p = (MEMTAG *)((char *)p1 - sizeof(MEMTAG) );
	if( p == NULL )
	{
		return 0 ;
	}
	return MtxInterlockedInc(&(p->ref));
}
INT32    MYAPI  MemoryRelease( void *p1   )
{
	MEMTAG *p = NULL ;
	INT32 ref = 0 ;
	if( p1 == NULL )
	{
		return 0 ;
	}
	p = (MEMTAG *)((char *)p1 - sizeof(MEMTAG) );
	if( p == NULL )
	{
		return 0 ;
	}
	ref =  MtxInterlockedDec(&(p->ref));
	if( ref < 1 )
	{
		if( p->destroy != NULL )
		{
			p->destroy(p1);
		}

		free(p);
		

	}
	return ref ;
}

void * MYAPI CreateMemoryPool()
{
#if defined(_Darwin_)
    return CreateMemoryPool_OSX();
#else
    return NULL ;
#endif
    
}
void MYAPI ReleaseMemoryPool(void *p)
{
#if defined(_Darwin_)
    ReleaseMemoryPool_OSX( p );
#else
  
#endif
}







#include "libmalloc_def.h"



#if !defined(_MALLOC_MEM_CATCH)

static INT64 _nMemUsed = 0;
static INT32 _Malloc2Couter = 0;

typedef struct tagMEMTAG
{
	size_t cb;
}MEMTAG;

static MTX_OBJ memCacheMtx;
static BOOL _bInit = FALSE;

int     malloc2_counter()
{
	return _Malloc2Couter;
}

static void update_memused_size(int size)
{
	if (!_bInit)
	{
		_bInit = TRUE;
		libmalloc2MtxInit(&memCacheMtx, 0);
	}
#if defined(_Linux_) && ( !defined(_TQ210_) ) && defined(_GCC_ATOMIC_) 
	__sync_add_and_fetch_8((&_nMemUsed), size);
#else
	libmalloc2MtxLock(&memCacheMtx);
	_nMemUsed += size;
	libmalloc2MtxUnLock(&memCacheMtx);
#endif
}
static int  min_int(int a, int b)
{
	if (a<b)
	{
		return a;
	}
	else
	{
		return b;
	}
}
void   malloc2_setmaxfreesize(INT64 maxSize)
{
}
void   malloc2_getstate(INT64 *pnMemFree, INT64 *pnMemUsed, INT64 *pnMemMax)
{
	if (pnMemFree != NULL)
	{
		*pnMemFree = 0;
	}
	if (pnMemUsed != NULL)
	{
		*pnMemUsed = _nMemUsed;
	}
	if (pnMemMax != NULL)
	{
		*pnMemMax = 0;
	}
}
void *  malloc2_align(size_t cb, int grow_count, int align)
{
	align--;
	cb = (cb + align) & ~align;

	return malloc2(cb);
}
void *  realloc2(void *p1, size_t cb)
{
	MEMTAG *p = NULL;
	MEMTAG *pR = NULL;
	int newSize = 0;
	int oldSize = 0;
	if (p1 == NULL)
	{
		return malloc2(cb);
	}
	else
	{
		p = (MEMTAG *)((char *)p1 - sizeof(MEMTAG));
		if (p == NULL)
		{
			return 0;
		}
		oldSize = p->cb;
		newSize = cb - oldSize;

		pR = (MEMTAG *)realloc(p, cb + sizeof(MEMTAG));

		if (pR == NULL)
		{

			pR = (MEMTAG *)malloc(cb + sizeof(MEMTAG));
			if (pR == NULL)
			{
				return NULL;
			}
			else
			{
				memcpy(pR, p, min_int(cb + sizeof(MEMTAG), p->cb + sizeof(MEMTAG)));

				update_memused_size(newSize);
				
				free(p);
			}
		}
		else
		{
			
			update_memused_size(newSize);
		}
		pR->cb = cb;
		return (((char *)pR) + sizeof(MEMTAG));
	}
}
void *   malloc2(size_t cb)
{


	MEMTAG *p = NULL;

	if (cb == 0)
	{
		return NULL;
	}
	p = (MEMTAG*)malloc(sizeof(MEMTAG) + cb);

	if (p == NULL)
	{
		return NULL;
	}
	p->cb = cb;
	update_memused_size(p->cb);

	libmalloc2MtxInterlockedInc(&_Malloc2Couter);

	return (((char *)p) + sizeof(MEMTAG));
}
void   free2(void *p1)
{
	MEMTAG *p = NULL;

	if (p1 == NULL)
	{
		return ;
	}
	p = (MEMTAG *)((char *)p1 - sizeof(MEMTAG));
	if (p == NULL)
	{
		return ;
	}
	update_memused_size(0 - (p->cb));

	libmalloc2MtxInterlockedDec(&_Malloc2Couter);
	free(p);
}


size_t   msize2(void *p1)
{
	MEMTAG *p = NULL;
	if (p1 == NULL)
	{
		return 0 ;
	}
	p = (MEMTAG *)((char *)p1 - sizeof(MEMTAG));
	if (p == NULL)
	{
		return 0;
	}
	return p->cb;
}




#endif


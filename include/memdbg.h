
#ifndef _MEMDBG_H_
#define _MEMDBG_H_


#ifndef _ADTYPES_H_
#include "adtypes.h"
#endif
	
#ifndef MEM_ERROR
#define MEM_ERROR
#define MEM_ERROR_TIMEOUT               -1
#define MEM_ERROR_QUEUE_IS_FULL         -2
#define MEM_ERROR_INVALID_POINTER       -3
#define MEM_ERROR_INVALID_PARAMETER     -4
#define MEM_ERROR_FILEOPEN              -5
#define MEM_ERROR_NOT_SUPPORT           -6
#endif

#ifdef  _MEM_DEBUG_

#define _MEM_ALLOC(n) (memalloc_dbg)((n),__FILE__,__LINE__)
#define _MEM_CALLOC(num,size) (memcalloc_dbg)((num),(size),__FILE__,__LINE__)
#define _MEM_REALLOC(memblock,size) (memrealloc_dbg)((memblock),(size),__FILE__,__LINE__)
#define _MEM_FREE(n) (memfree_dbg)((n),__FILE__,__LINE__)
#define _MEM_FREE_(n) (memfreex_dbg)(&(n),__FILE__,__LINE__)
#define _MEM_FREEX(n) (memfreex_dbg)((n),__FILE__,__LINE__)
#define _MEM_SNAPSHOT(n) mem_snapshot(n)

#else

#define _MEM_ALLOC(n) malloc(n)
#define _MEM_CALLOC(num,size) calloc((num),(size))
#define _MEM_REALLOC(memblock,size) realloc((memblock),(size))
	#ifndef _MEM_CPY_ALLOC

	#define _MEM_FREE(n) free(n)
	#define _MEM_FREE_(n) do{ if((n) && ((void*)(n))!=((void*)-1) ) free(n);n=0;}while(0)
	#define _MEM_FREEX(n) do{ if((n) && (*(n)) && ((void*)(*(n)))!=((void*)-1)) free((*(n)));(*(n))=0;}while(0)

	#else

	#define _MEM_FREE(n) memfree(n)
	#define _MEM_FREE_(n) do{if(n) memfree(n);n=0;}while(0)
	#define _MEM_FREEX(n) do{if((n)&&(*(n))) memfree((*(n)));(*(n))=0;}while(0)

	#endif
#define _MEM_SNAPSHOT(n) 

#endif


#if defined(__cplusplus)
extern "C"   
{
#endif

void * MYAPI memcpy_alloc(void * vData,int nDataLen);
int MYAPI memfree(void *p);
int MYAPI memfreex(void *p);

void * MYAPI memalloc(int  nSize);
void * MYAPI memalloc_dbg(int  nSize,const char *szFile,int nLine);
void * MYAPI memcalloc(int  nNum,int  nSize);
void * MYAPI memcalloc_dbg(int  nNum,int  nSize,
                           const char *szFile,int nLine);
void * MYAPI memrealloc( void *memblock, int  nSize );
void * MYAPI memrealloc_dbg(void *memblock,int  nSize,
                            const char *szFile,int nLine);
int MYAPI memfree_dbg(void *p,const char *szFile,int nLine);
int MYAPI memfreex_dbg(void *p,const char *szFile,int nLine);
int MYAPI mem_snapshot(const char *szFileName);
int MYAPI memfree_all();
int MYAPI mem_isvalid(void *p,const char *szFile,int nLine);
#if defined(__cplusplus)
}   
#endif


#endif


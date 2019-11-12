


//#define _NO_STLPORT_

#ifdef WIN32
#include <windows.h>
#include <tchar.h>
#endif

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>
#if !defined(_FreeBSD_) && !defined(__DARWIN_UNIX03)
#include <malloc.h>
#endif


#ifndef WIN32
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#if !defined(_FreeBSD_)
#include <alloca.h>
#endif

#include <pthread.h>

#if defined(_Darwin_) || defined(__DARWIN_UNIX03)
#include <mach/mach_time.h>
#define  _USE_PTHREAD_MUTEX_RECURSIVE
#include <libkern/OSAtomic.h> 
#else
#define  _USE_PTHREAD_MUTEX_RECURSIVE_NP
#endif


#endif

#include "adtypes.h"

#ifdef WIN32
typedef CRITICAL_SECTION MTX_OBJ;
#define   _MTX_OBJ_DEF
#else
#if defined(_PTHREAD_H) || defined(_PTHREAD_H_)
typedef pthread_mutex_t  MTX_OBJ;
#define   _MTX_OBJ_DEF
#endif
#endif

typedef int BOOL_MALLOC;

#define FALSE_MALLOC 0
#define TRUE_MALLOC 1
#define  _MTX_OPT_FAST          0x01
#define  _MTX_OPT_RECURSIVE     0x02
#define  _MTX_OPT_ERRORCHECK    0x04

#include "libmalloc2.h"

#if !defined(_NO_STLPORT_)
#define _MALLOC_MEM_CATCH
#include "libmalloc_tools.h"
#endif




BOOL_MALLOC  libmalloc2MtxInitEx( MTX_OBJ * pMtxObj,int nSpinCount,int nMtxOpt);
BOOL_MALLOC  libmalloc2MtxInit( MTX_OBJ * pMtxObj,int nMtxOpt);

BOOL_MALLOC  libmalloc2MtxLock( MTX_OBJ * pMtxObj);
BOOL_MALLOC  libmalloc2MtxUnLock( MTX_OBJ * pMtxObj);
BOOL_MALLOC  libmalloc2MtxDestroy( MTX_OBJ * pMtxObj);
void  libmalloc2initcounter();
long  libmalloc2MtxInterlockedInc(INT32 volatile *pCount);
long  libmalloc2MtxInterlockedDec(INT32 volatile *pCount);











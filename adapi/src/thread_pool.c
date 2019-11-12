

#include"adapi_def.h"


#ifndef _ADAPI_NO_THREAD_FUNC_

/************************************************************/
#define  _THPOOL_MAX_QUEUE     2048
#define  _THPOOL_DEF_LEN	1024

#define _TH_STATUS_RUN			1
#define _TH_STATUS_SUSPEND		2
#define _TH_STATUS_EXIT			3

#define _THPOOL_WAITTIME		10
#define _THPOOL_WAITTIME_PRE		10


typedef struct tagTH_OBJECT
{
	int nThreadStatus;
	unsigned int nWaitTime;
	void *Parameter;

}TH_OBJECT,*PTH_OBJECT;


typedef struct tagTHPOOL_SYS
{
    BOOL   bHeapAlloc;
    MTX_OBJ  MtxObj;
	unsigned int nStackSize;
	void *pfuncThreadProc;

	int nPoolLen;
	int iPoolPointer;
	int nThreadNumber;
	UINT32 Option;
	PTH_OBJECT pThObj[1];

}THPOOL_SYS,*PTHPOOL_SYS;


int MYAPI ThPoolHandleSize(int nPoolLen)
{
	if(nPoolLen==-1)
	{
		nPoolLen=_THPOOL_DEF_LEN;
	}
	return (sizeof(THPOOL_SYS)+((nPoolLen+1)*sizeof(PTH_OBJECT)));
}

THPOOL_HANDLE   ThPoolInitSys(void *pHandleMem,void *pfuncThreadProc,unsigned int nStackSize,int nPoolLen,BOOL bHeapAlloc)
{
	PTHPOOL_SYS pSys=(PTHPOOL_SYS)pHandleMem;
	if(!pSys)
    {
        return 0;
    }
    memset( pSys, 0, _SIZE_OF(THPOOL_SYS) );
	if(!nPoolLen||!pfuncThreadProc)
	{
		return 0;
	}
    pSys->bHeapAlloc = !! bHeapAlloc ;
	if(nPoolLen==-1)
	{
		nPoolLen=_THPOOL_DEF_LEN;
	}
#ifdef PTHREAD_STACK_MIN
#ifndef PAGE_SIZE
#define NO_PAGE_SIZE
#define PAGE_SIZE  (1024*512)
#endif
	if( nStackSize>0 && nStackSize<PTHREAD_STACK_MIN )
	{
		nStackSize = PTHREAD_STACK_MIN;
	}
#ifndef NO_PAGE_SIZE
#undef PAGE_SIZE
#endif
#endif

	pSys->nThreadNumber = 0;
	pSys->iPoolPointer=-1;
	pSys->nPoolLen=nPoolLen;

	pSys->nStackSize=nStackSize;
	pSys->pfuncThreadProc=pfuncThreadProc;

    MtxInit( &(pSys->MtxObj), 0 );
	

	return (THPOOL_HANDLE)pSys;
}
THPOOL_HANDLE   MYAPI ThPoolInit(void *pHandleMem,void *pfuncThreadProc,unsigned int nStackSize,int nPoolLen)
{
    return ThPoolInitSys(pHandleMem,pfuncThreadProc,nStackSize,nPoolLen,FALSE);
}
THPOOL_HANDLE MYAPI  ThPoolAlloc(void *pfuncThreadProc,unsigned int nStackSize,int nPoolLen)
{
	PTHPOOL_SYS pSys=0;
	THPOOL_HANDLE hThPool=0;

	if(!nPoolLen||!pfuncThreadProc)
	{
		return 0;
	}
	pSys=(PTHPOOL_SYS)_MEM_ALLOC(ThPoolHandleSize(nPoolLen));
	hThPool = ThPoolInitSys(pSys,pfuncThreadProc,nStackSize,nPoolLen,TRUE);

	if( !hThPool )
	{
		_MEM_FREEX(&pSys);
	}
	return hThPool;
}
UINT32 MYAPI ThPoolSetOption(THPOOL_HANDLE hPool,UINT32 NewOption)
{
	UINT32 opt = 0;
	PTHPOOL_SYS pSys=(PTHPOOL_SYS)hPool;
	if(!pSys)
	{
		return 0;
	}
	opt = pSys->Option;
	pSys->Option = NewOption;

	return opt;
}
UINT32 MYAPI ThPoolOption(THPOOL_HANDLE hPool)
{
	PTHPOOL_SYS pSys=(PTHPOOL_SYS)hPool;
	if(!pSys)
	{
		return 0;
	}
	return pSys->Option;
}
int MYAPI ThPoolCountWorking(THPOOL_HANDLE hPool)
{
	return (ThPoolCountAll(hPool) - ThPoolCountWaiting(hPool));
}
int MYAPI ThPoolCountAll(THPOOL_HANDLE hPool)
{
	PTHPOOL_SYS pSys=(PTHPOOL_SYS)hPool;
	if(!pSys)
	{
		return 0;
	}
	return pSys->nThreadNumber;
}
int MYAPI ThPoolCountWaiting(THPOOL_HANDLE hPool)
{
	PTHPOOL_SYS pSys=(PTHPOOL_SYS)hPool;
	if(!pSys)
	{
		return 0;
	}
	if(pSys->iPoolPointer<0)
	{
		return 0;
	}
	else
	{
		return (pSys->iPoolPointer+1);
	}

}

int MYAPI ThPoolRunTask(THPOOL_HANDLE hPool,void *ThreadParameter,void *pfuncThreadProc)
{
	PTHPOOL_SYS pSys=(PTHPOOL_SYS)hPool;
	int nRet;
	void * thread=0;
	unsigned int nStackSize=0;
	PTH_OBJECT pThObj=0;
	if(!pSys && !pfuncThreadProc)
	{
		return _THPOOL_ERROR;
	}
	if(!pSys && pfuncThreadProc)
	{
		goto CREATE_TH;
	}
	if(pfuncThreadProc && pfuncThreadProc!=pSys->pfuncThreadProc )
	{
		return _THPOOL_ERROR;
	}
	if( (_THPOOL_OPT_USEPOOLONLY&pSys->Option) && ( pSys->nThreadNumber >= pSys->nPoolLen ))
	{
		return _THPOOL_FULL;
	}
	pfuncThreadProc=pSys->pfuncThreadProc;
	nStackSize=pSys->nStackSize;

	nRet=MtxLock( &(pSys->MtxObj) );
	if(  !nRet )
	{
		goto CREATE_TH;
	}
	if((pSys->iPoolPointer)<0)
	{
		MtxUnLock( &(pSys->MtxObj) );
		goto CREATE_TH;
	}
	pThObj = pSys->pThObj[(pSys->iPoolPointer)--];

	if(pSys->iPoolPointer>=0)
	{
		pSys->pThObj[pSys->iPoolPointer]->nWaitTime=_THPOOL_WAITTIME;
	}
	MtxUnLock( &(pSys->MtxObj) );
	if(!pThObj)
	{
		goto CREATE_TH;
	}


	pThObj->Parameter=ThreadParameter;
	pThObj->nWaitTime=1;
	pThObj->nThreadStatus=_TH_STATUS_RUN;
	


	return _THPOOL_OK;

CREATE_TH:
	thread=Thread2Create(pfuncThreadProc,ThreadParameter,0,nStackSize,FALSE);
	if(Thread2IsValid(thread))
	{
		if(pSys)
		{
			pSys->nThreadNumber ++;
		}
		return _THPOOL_OK;
	}
	else
	{
		return _THPOOL_ERROR;
	}
}
int MYAPI ThPoolWaitTask(THPOOL_HANDLE hPool ,void **ppThreadParameterOut)
{
	PTHPOOL_SYS pSys=(PTHPOOL_SYS)hPool;
	TH_OBJECT	ThObj;
	PTH_OBJECT pThObj=&ThObj;
	int  nRet;

	if(!pSys)
	{
		return _THPOOL_FULL;
	}
	nRet=MtxLock( &(pSys->MtxObj) );
	if(  !nRet )
	{
		pSys->nThreadNumber --;
		
		return _THPOOL_ERROR;
	}
	if( (pSys->iPoolPointer) >= ( pSys->nPoolLen - 1 ) )
	{
		MtxUnLock( &(pSys->MtxObj) );
		pSys->nThreadNumber --;
		return _THPOOL_FULL;
	}
	if(pSys->iPoolPointer<0)
	{
		pSys->iPoolPointer=-1;
	}
	pSys->pThObj[++(pSys->iPoolPointer)]=pThObj;
	pThObj->nWaitTime=_THPOOL_WAITTIME;
	pThObj->nThreadStatus=_TH_STATUS_SUSPEND;
	pThObj->Parameter=0;

	if(pSys->iPoolPointer>0)
	{
		pSys->pThObj[(pSys->iPoolPointer)-1]->nWaitTime=_THPOOL_WAITTIME_PRE;
	}
	MtxUnLock( &(pSys->MtxObj) );
	
	/*---------------------------------------------------*/
	while(pThObj->nThreadStatus==_TH_STATUS_SUSPEND)
	{
		SleepMilli(pThObj->nWaitTime);
	}
	if(pThObj->nThreadStatus == _TH_STATUS_EXIT)
	{
		pSys->nThreadNumber --;
		return _THPOOL_EXIT;
	}
	else if(pThObj->nThreadStatus == _TH_STATUS_RUN)
	{
		if(ppThreadParameterOut)
		{
			*ppThreadParameterOut = pThObj->Parameter;
		}
		return _THPOOL_OK;
	}
	else
	{
		pSys->nThreadNumber --;
		return _THPOOL_ERROR;
	}

}


int MYAPI ThPoolCancelAll(THPOOL_HANDLE hPool)
{
	PTHPOOL_SYS pSys=(PTHPOOL_SYS)hPool;
	int  nRet,i,iPoolPointer;
	if(!pSys)
	{
		return _THPOOL_ERROR;
	}
	nRet=MtxLock( &(pSys->MtxObj) );
	if(  !nRet )
	{
		return _THPOOL_ERROR;
	}
	iPoolPointer=pSys->iPoolPointer;
	for(i=0;i<=iPoolPointer;i++)
	{
		pSys->pThObj[i]->Parameter = 0;
		pSys->pThObj[i]->nWaitTime=1;
		pSys->pThObj[i]->nThreadStatus=_TH_STATUS_EXIT;
	}
	pSys->iPoolPointer=-1;
	MtxUnLock( &(pSys->MtxObj) );
	return _THPOOL_OK;
}

void MYAPI ThPoolFree(THPOOL_HANDLE hPool)
{
    PTHPOOL_SYS pSys=(PTHPOOL_SYS)hPool;
	if( !pSys )
	{
		return ;
	}
	ThPoolCancelAll(hPool);

    if( pSys->bHeapAlloc )
    {
        _MEM_FREEX(&hPool);
    }
	
}
 

#endif




































#include "libcpputil_def.h"




/************************************************************************/
/*                                                                      */
/************************************************************************/
#if defined(_DEBUG_OBJ_REF_)
INT32 _nCObjWait = 0 ;
#endif

_CPP_UTIL_CLASSNAME_IMP(CObjWait)

CObjWait * CObjWait::CreateObj( BOOL bManualReset ) 
{ 
	CObjWait *pObj = NULL ; 
	pObj = new CObjWait(bManualReset) ; 
	if( pObj == NULL ) 
	{ 
		return NULL ; 
	} 
	pObj->m_bAlloced = TRUE ; 
	pObj->m_nObjSize = sizeof(CObjWait) ; 
	pObj->AddObjRef(); 
	return pObj ; 
}

CObjWait::CObjWait(BOOL bManualReset )
{
#if defined(_DEBUG_OBJ_REF_)
    MtxInterlockedInc(&_nCObjWait);
#endif

    m_pObj = EvWaitCreate( bManualReset ) ;

}
CObjWait::~CObjWait()
{
#if defined(_DEBUG_OBJ_REF_)
    MtxInterlockedDec(&_nCObjWait);
#endif
    if( m_pObj != NULL )
    {
        EvWaitClose(m_pObj);
    }

}
void * CObjWait::QueryObj(const char *szObjName)
{
    if( szObjName == NULL )
    {
        return NULL ;
    }

    if( strcmp( szObjName, GetObjName() ) == 0 )
    {
        return this;
    }
    else
    {
        return CObj::QueryObj(szObjName) ;
    }
}

CObjWait::WaitRet CObjWait::WaitForEvent(int dwMilliseconds)
{
    if(  m_pObj == NULL )
    {
        return CObjWait::WaitRetError ;
    }
    else
    {
		EvWaitResult Ret = EvWaitForEvent(m_pObj,dwMilliseconds);
        switch ( Ret )
        {
		case EvWaitResultError:
			{
				return WaitRetError ;
			}
		case EvWaitResultOk:
			{
				return WaitRetSuccess;
			}
		default:
			{
				return WaitRetTimeout ;
			}
        }
    }
}
void CObjWait::SetEvent()
{
    if(  m_pObj == NULL )
    {
        return  ;
    }
    else
    {
		EvWaitSetEvent(m_pObj);
        return  ;
    }
}
BOOL CObjWait::ResetEvent()
{
    if(  m_pObj == NULL )
    {
        return  FALSE;
    }
    else
    {
        return EvWaitResetEvent(m_pObj);
    }
}






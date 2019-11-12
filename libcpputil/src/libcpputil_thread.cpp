

#include "libcpputil_def.h"
#include "libcpputil_thread_imp.h"

#if defined(_DEBUG_OBJ_REF_)
INT32 _nCObjThread = 0 ;
#endif

CObjThread::CObjThread()
{
#if defined(_DEBUG_OBJ_REF_)
    MtxInterlockedInc(&_nCObjThread) ;
#endif
	m_obj = new CObjThreadImp( this );
}
CObjThread::~CObjThread()
{
#if defined(_DEBUG_OBJ_REF_)
    MtxInterlockedDec(&_nCObjThread) ;
#endif
	delete m_obj ;
}

int CObjThread::Run()
{

#if defined(_DEBUG_) || defined(_DEBUG)
        int i = 0 ;
        for( i = 0 ; i < 10 ; i ++ )
        {
            printf("CObjThread::Run %d\n",i);
            SleepMilli(1000);
        }
#else
	printf("CObjThread::Run not override\n");
#endif
        return 0 ;
    

}
BOOL CObjThread::IsRunning()
{
	if( m_obj == NULL )
	{
		return FALSE ;
	}
	return ((CObjThreadImp * )m_obj)->IsRunning();
}
INT_THREADID CObjThread::ThreadId()
{
	if( m_obj == NULL )
	{
		return 0 ;
	}
	return ((CObjThreadImp * )m_obj)->ThreadId();
}

void CObjThread::SetNativeObjThreadProc( _ObjThreadProc funcThread , void *vThreadParameter)
{
	if( m_obj != NULL )
	{
		((CObjThreadImp * )m_obj)->SetNativeObjThreadProc( funcThread , vThreadParameter );
	}
}
BOOL CObjThread::Start(int nStackSize )
{
	if( m_obj == NULL )
	{
		return FALSE ;
	}
	return ((CObjThreadImp * )m_obj)->Start(nStackSize);
}
BOOL CObjThread::Stop()
{
#ifdef WIN32
	if( m_obj == NULL )
	{
		return FALSE ;
	}
	UINT64 threadid = ((CObjThreadImp * )m_obj)->ThreadId() ;
	if( threadid != 0 )
	{
		PostThreadMessage( (DWORD)threadid ,WM_QUIT , 0 , 0 );
		return TRUE ;
	}
	
#else
#endif
	return FALSE ;
}
void CObjThread::WaitForExit()
{
	if( m_obj == NULL )
	{
		return  ;
	}

	((CObjThreadImp * )m_obj)->WaitForExit() ;
}













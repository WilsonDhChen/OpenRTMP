

#include "libcpputil_def.h"
#include "libcpputil_thread.h"
#include "libcpputil_thread_imp.h"

CObjThreadImp::CObjThreadImp(CObjThread * obj)
{
	m_ObjThreadProc  = NULL ;
	m_vThreadParameter = NULL ;
	m_bRunning = FALSE ;
	m_nThreadId = NULL ; 
	m_obj = obj ;
    m_bCreating = FALSE ;
	m_hEvent = EvWaitCreate( TRUE );
	EvWaitSetEvent( m_hEvent ) ;
}
CObjThreadImp::~CObjThreadImp()
{

	if( m_bRunning )
	{
		Stop();
		EvWaitForEvent( m_hEvent , -1 );
	}
	EvWaitClose( m_hEvent ) ;
	m_hEvent = NULL ;

}
BOOL CObjThreadImp::Stop()
{
	return m_obj->Stop();
}
void CObjThreadImp::SetNativeObjThreadProc( _ObjThreadProc funcThread , void *vThreadParameter)
{
	m_ObjThreadProc  = funcThread ;
	m_vThreadParameter = vThreadParameter ;
}
unsigned int MYAPI CObjThreadImp::_ThreadProc(void *p)
{
	CObjThreadImp *_this = (CObjThreadImp *)p ;
	_this->m_bRunning = TRUE ;
    _this->m_bCreating = FALSE ;
    void *pool = CreateMemoryPool();
#if !defined(WIN32)
    pthread_detach(pthread_self());
#endif

	if( _this->m_ObjThreadProc != NULL )
	{
		_this->m_ObjThreadProc( _this->m_vThreadParameter ) ;
	}
	else
	{
		_this->m_obj->Run() ;
	}
    
    ReleaseMemoryPool(pool);
    
	_this->m_bRunning = FALSE ;
	EvWaitSetEvent( _this->m_hEvent );
	Thread2Exit( 0 );
	return 0 ;
}
void CObjThreadImp::WaitForExit()
{
	EvWaitForEvent( m_hEvent , -1 );
}
BOOL CObjThreadImp::IsRunning()
{
	return m_bRunning ;
}
INT_THREADID CObjThreadImp::ThreadId()
{
	return m_nThreadId ;
}
BOOL CObjThreadImp::Start(int nStackSize )
{
    if( m_bRunning || m_bCreating )
	{
		return FALSE ;
	}

    m_bCreating = TRUE ;
	EvWaitResetEvent( m_hEvent  );
	void *hThread =   Thread2Create( (void *)(&CObjThreadImp::_ThreadProc), this , &m_nThreadId, nStackSize , FALSE );
	if( !Thread2IsValid( hThread ) )
	{
		EvWaitSetEvent( m_hEvent );
		return FALSE ;
	}
	else
	{
        m_bCreating = FALSE ;
		Thread2Close( hThread );
	}

	return TRUE ;

}
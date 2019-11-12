
#ifndef _libcpputil_thread_imp_h_
#define _libcpputil_thread_imp_h_



class    CObjThreadImp :
	public CObj
{
public:
	CObjThreadImp( CObjThread * obj);
	virtual ~CObjThreadImp();
	void SetNativeObjThreadProc( _ObjThreadProc funcThread , void *vThreadParameter);
	void WaitForExit();
	BOOL IsRunning();
	INT_THREADID ThreadId(); 
	BOOL Start(int nStackSize = 0);
	BOOL Stop();
	static unsigned int MYAPI _ThreadProc(void *p);

public:
	_ObjThreadProc m_ObjThreadProc ;
	void *m_vThreadParameter;
	BOOL m_bRunning;
    BOOL m_bCreating;
	INT_THREADID m_nThreadId; 
	CObjThread *m_obj;
	HEVWAIT m_hEvent;
};










#endif



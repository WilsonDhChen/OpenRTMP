

#ifndef _LIBMEMDBG_H_
#define _LIBMEMDBG_H_


#ifdef WIN32

#ifndef _LIBMEMDBG_EXPORT
#pragma comment(lib,"libmemdbg.lib")
#endif

#endif

typedef int (*_MemoryDbgCallback)(int i,int nTotal,const char *szFile,int nLine,size_t addr,int nBlockAlloced,const char *szCallStack,const char *szContent,const char *szTimeAlloced,const char *szTimeFreed,void *vUsrData);
typedef int (*_BSTRDbgCallback)(int i,int nTotal,const char *szAddr,const char *szBSTR,const char *szTimeAlloced,void *vUsrData);


#if defined(__cplusplus) 
extern "C"
{
#endif

    int  GetBreakPoints(int *pBrk,int nLen);
    void RemoveBreakPoint(int nBlock);
    void AddBreakPoint(int nBlock,BOOL bInt3);
    BOOL GetBreakPoint(int nBlock,BOOL *pInt3);

    int  Free_GetBreakPoints(int *pBrk,int nLen);
    void Free_RemoveBreakPoint(int nBlock);
    void Free_AddBreakPoint(int nBlock,BOOL bInt3);
    BOOL Free_GetBreakPoint(int nBlock,BOOL *pInt3);

    void EnableAllocINT3();
    void EnableFreeINT3();

    

	typedef int ( *_MemDbgNotifyProc)(const char *szFile,int nLine,const char *szNotfiyReason);

	void   MemDbgSetNotifyProc( _MemDbgNotifyProc pFunc );
    size_t GetCrtMemAlloced();
    size_t GetHeapMemAlloced();


	void *  MemDbgRecalloc( void * _Memory, size_t _Count, size_t _Size,const char *szFile,int nLine);
	void *  MemDbgRealloc(void * _Memory, size_t _NewSize,const char *szFile,int nLine);
	void *  MemDbgCalloc(size_t _Count, size_t _Size,const char *szFile,int nLine);
	void *   MemDbgAlloc( size_t cb,const char *szFile,int nLine);
	void   MemDbgFree(void *p,const char *szFile,int nLine);
    size_t    MemDbgMSize(void *p);
    void * MemDbgExpand(void * _Memory, size_t _NewSize,const char *szFile,int nLine);

	void   MemDbgInit( int nMaxFreeMem );
	void   MemDbgGetMemState(int *pnMemFree,int *pnMemUsed,int *pnMemMax);
	void   MemDbgExportAllocedMem2File(const char *szFileLog);
	
	
	void   MemDbgPrintfMemoryLeak();
    void  MemDbgExportAllocedMem( _MemoryDbgCallback _pfuncCallback,void * vData);

    LPVOID WINAPI Real_HeapAlloc(HANDLE hHeap, DWORD dwFlags,SIZE_T dwBytes);
    LPVOID WINAPI Real_HeapReAlloc(HANDLE hHeap,DWORD dwFlags,LPVOID lpMem,SIZE_T dwBytes);
    BOOL WINAPI Real_HeapFree(HANDLE hHeap,DWORD dwFlags,LPVOID lpMem);
    
    LPVOID WINAPI My_HeapAlloc(HANDLE hHeap, DWORD dwFlags,SIZE_T dwBytes);
    LPVOID WINAPI My_HeapReAlloc(HANDLE hHeap,DWORD dwFlags,LPVOID lpMem,SIZE_T dwBytes);
    BOOL WINAPI My_HeapFree(HANDLE hHeap,DWORD dwFlags,LPVOID lpMem);

    int SysStringCount();
    void  SysStringExportAllocedMem( _BSTRDbgCallback _pfuncCallback,void * vData);



    void  MemDbgExportBSTRAllocedMem( _BSTRDbgCallback _pfuncCallback,void * vData);

    void  MemDbgExportBSTRAllocedMemEx( _MemoryDbgCallback _pfuncCallback,void * vData);
    void  MemDbgExportGlobalAllocedMem( _MemoryDbgCallback _pfuncCallback,void * vData);
    void  MemDbgExportCoTaskAllocedMem( _MemoryDbgCallback _pfuncCallback,void * vData);
    void  MemDbgExportHeapAllocedMem( _MemoryDbgCallback _pfuncCallback,void * vData);
    void  MemDbgExportLocallAllocedMem( _MemoryDbgCallback _pfuncCallback,void * vData);

    void  MemDbgExportVirtualAllocedMem( _MemoryDbgCallback _pfuncCallback,void * vData);
    size_t GetVirtuaMemAlloced();

 

#if defined(__cplusplus)
}
#endif



#if defined(__cplusplus)


	#if !defined(_CRTBLD) && !defined(_LIBMEMDBG_EXPORT) && !defined(_DEBUG) && !defined(_CRTDBG_MAP_ALLOC) && !defined(_CRT_MEM_) && !defined(_CPP_RELEASE_MEM_DBG_TRACK_)
		
		inline void * operator new(size_t size,const char * szFile,int nLine)
		{
			return MemDbgAlloc(size,szFile,nLine);
		}
		inline void * operator new[](size_t size,const char * szFile,int nLine)
		{
			return MemDbgAlloc(size,szFile,nLine);
		}
		inline void operator delete(void* p,const char *szFile,int nLine)
		{
			MemDbgFree(p,szFile,nLine);
		}
		inline void operator delete[](void* p,const char *szFile,int nLine)
		{
			MemDbgFree(p,szFile,nLine);
		}
		/*
		inline void operator delete(void* p)
		{
			MemDbgFree(p,"",0);
		}
		*/
		#undef new
		#define new new(__FILE__,__LINE__)
		#define _CPP_RELEASE_MEM_DBG_TRACK_

	#endif

#endif



#endif

































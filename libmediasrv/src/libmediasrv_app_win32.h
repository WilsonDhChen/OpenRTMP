
#ifndef _libmediasrv_app_win32_h_
#define _libmediasrv_app_win32_h_ 

#if defined(WIN32)
#include "libmediasrv_app.h"

class  CMediaSrvAppWin32 :
	public CMediaSrvApp
{
	_CPP_UTIL_DYNAMIC(CMediaSrvAppWin32);
	_CPP_UTIL_CLASSNAME(CMediaSrvAppWin32);
	_CPP_UTIL_QUERYOBJ(CMediaSrvApp);
public:
	CMediaSrvAppWin32();
	virtual ~CMediaSrvAppWin32();
	virtual int  Run();
	virtual void Stop();
	void Win32Loop();
	void WndProc(UINT message, WPARAM wParam, LPARAM  lParam);
public:

	DWORD m_dwThreadID;
};

#endif
#endif














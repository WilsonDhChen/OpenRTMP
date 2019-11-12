
#include "libmediasrv_def.h"

#if defined(WIN32)
#include "libmediasrv_app_win32.h"

CMediaSrvAppWin32::CMediaSrvAppWin32()
{
	m_dwThreadID = 0;
}
CMediaSrvAppWin32::~CMediaSrvAppWin32()
{

}
int  CMediaSrvAppWin32::Run()
{
	LibCppUtilInit(_AppConfig.m_nMaxCachedMem);

	if (!StartNetworkAndQueue())
	{
		return 1;
	}
	m_dwThreadID = GetCurrentThreadId();

	Win32Loop();
	StopNetworkAndQueue();
	return 0;
}
void CMediaSrvAppWin32::Stop()
{
	if (m_dwThreadID != 0)
	{
		PostThreadMessage(m_dwThreadID, WM_QUIT, 0, 0);
	}
}
void CMediaSrvAppWin32::WndProc(UINT message, WPARAM wParam, LPARAM  lParam)
{


}
void CMediaSrvAppWin32::Win32Loop()
{
	MSG Msg;
	while (GetMessage(&Msg, NULL, 0, 0))
	{
		WndProc(Msg.message, Msg.wParam, Msg.lParam);
		if (WM_QUIT == Msg.message)
		{
			break;
		}
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
}




extern "C" int win_main(char *cmdline)
{

#ifdef WIN32
	if (cmdline == NULL)
	{
		cmdline = GetCommandLineA();
	}
#endif




    
	CMediaSrvApp App;
	_AppConfig.Init();
	CMediaSrvApp *theApp = _MediasrvApp;
	theApp->ParseCmdline(cmdline);
	theApp->Run();
	return 0;
}



#endif


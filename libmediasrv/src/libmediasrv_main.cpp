

#include "libmediasrv_def.h"











BOOL MYAPI MediaSrvStart()
{
	
	if (_MediasrvApp == NULL)
	{
		_AppConfig.Init();
		new CMediaSrvApp();
	}
	return _MediasrvApp->StartNetworkAndQueue();
}
void MYAPI MediaSrvStop()
{
	if (_MediasrvApp == NULL)
	{
		return;
	}
	_MediasrvApp->StopNetworkAndQueue();
}



extern "C" int MYAPI unix_main(int argc, char *argv[])
{


    CMediaSrvApp App;
	_AppConfig.Init(argc, argv);

	
	LogPrint(_LOG_LEVEL_DEBUG, _TAGNAME, "%s started\n", _APP_NAME);
	_MediasrvApp->Run();
    _MediasrvApp->CleanUp();
	return 0;
}





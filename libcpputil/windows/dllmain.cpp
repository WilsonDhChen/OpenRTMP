// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "Winsock2.h"
#include "adapi.h"
#include "rde.h"
#include "libcpputil.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{ 
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
        {
            RDE_Init();
            
            LibCppUtilInit(SIZE_10M*40);
            break ;
        }
	case DLL_THREAD_ATTACH:

	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}


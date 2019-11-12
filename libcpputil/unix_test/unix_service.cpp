
#ifdef WIN32
#include "stdafx.h"
#include <winsock2.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string>

#if !defined(WIN32) 
#include <sys/wait.h> 
#include <dlfcn.h>
#include <sys/types.h>
#include <unistd.h>
typedef int SOCKET;
#endif

#include "adtypes.h"
#include "adapi.h"
#include "libcpputil_net.h"
#ifdef WIN32
    #if defined(_DEBUG_) || defined(DEBUG) || defined(_DEBUG)
    //#define _FORK_CHILD_
    static const char SERVICE_DLLNAME[]="libcpputild.dll";
    #else
    //#define _FORK_CHILD_
#ifdef _WIN64
	static const char SERVICE_DLLNAME[]="libcpputil64.dll";
#else
    static const char SERVICE_DLLNAME[]="libcpputil.dll";
#endif
    #endif
#else


    #if defined(_DEBUG_) || defined(DEBUG) || defined(_DEBUG)
    	
    #if defined(_FreeBSD_)
    static const char SERVICE_DLLNAME[]="/mnt/unix/libso/FreeBSD/libcpputild.so";
	#elif defined(_Darwin_)
	static const char SERVICE_DLLNAME[]="/Volumes/FAT/program/unix/libso/Darwin/libcpputild.so";
    #else
    static const char SERVICE_DLLNAME[]="/mnt/hgfs/wine/program/unix/libso/Linux/libcpputild.so";
    #endif
    	
    #else
    //#define _FORK_CHILD_
    static const char SERVICE_DLLNAME[]="libcpputil.so";
    #endif
#endif

typedef void  ( MYAPI *_LibCppUtilTest)() ;
typedef void  ( MYAPI *_LibCppUtilTestFinish)() ;
typedef void  ( MYAPI *_LibCppUtilTestSend)() ;

static char SERVICE_NAME[]="libcpputil" ;
int mainTCP(int argc, char *argv[]);
int main(int argc, char *argv[])
{

	return mainTCP( argc , argv);
	CObjNetUDP udp;

	udp.BindAddr(900);
	udp.Start();

	printf("bind at %d\n",udp.GetBindPort());
	char c = 0 ;
	while(  c = getchar() )
	{
		if( c == 't' )
		{
			udp.Shutdown();

		}
	}
	return 0 ;
}
int mainTCP(int argc, char *argv[])
{

    DLL_HANDLE dlservice=0;
    _LibCppUtilTest  start = 0 ;
    _LibCppUtilTestFinish end= 0 ;
    _LibCppUtilTestSend  sendTest= 0 ;


    dlservice = DllOpen(SERVICE_DLLNAME,0);
    if(!dlservice)
    {
        fprintf(stderr,"[%s] dlopen %s\n",SERVICE_NAME,DllError());
        return 1;
    }

    start=(_LibCppUtilTest)DllSymbol(dlservice, "LibCppUtilTest" );
    if( ! start)
    {
        fprintf(stderr,"[%s] dlsym %s\n",SERVICE_NAME,DllError());
        DllClose(dlservice);
        return 1;
    }
    end=(_LibCppUtilTestFinish)DllSymbol(dlservice, "LibCppUtilTestFinish" );
    if( ! end)
    {
        fprintf(stderr,"[%s] dlsym %s\n",SERVICE_NAME,DllError());
        DllClose(dlservice);
        return 1;
    }
    sendTest =(_LibCppUtilTestSend)DllSymbol(dlservice, "LibCppUtilTestSend" );
    if( ! sendTest)
    {
        fprintf(stderr,"[%s] dlsym %s\n",SERVICE_NAME,DllError());
        DllClose(dlservice);
        return 1;
    }
	
 	printf("%s started\n",SERVICE_NAME);
	printf("r=Start\nt=Stop\ns=Send\nx=Exit\n\n");
	start();
	char c = 0 ;
	while(  c = getchar() )
	{
		if( c == 't' )
		{
			end();
			printf("%s stop\n",SERVICE_NAME);
			
		}
		else  if( c == 'x' )
		{
			end();
			printf("%s exit\n",SERVICE_NAME);
			exit(0);
			
		}
		else  if( c == 'r' )
		{
			start();
			printf("%s start\n",SERVICE_NAME);
			
		}
		else  if( c == 's' )
		{
			sendTest();
			printf("%s send\n",SERVICE_NAME);
			
		}
	}
		
	return 1;

}







#include "adapi_def.h"

#if !defined(MYAPI)
#define MYAPI
#endif

#if defined(__DARWIN_UNIX03)
#include <CoreFoundation/CoreFoundation.h>
#ifdef __OBJC__
#include <Foundation/Foundation.h>
#endif
#endif

int MYAPI GetCurrentAppPath(char *szPath, int nLen)
{
	if (nLen < 2)
	{
		szPath[0] = 0;
		return 0;
	}
#if defined(WIN32)
	return GetModuleFileNameA(NULL, szPath, nLen - 1);
#elif defined(__DARWIN_UNIX03)
    
    CFURLRef refPath = 0 ;
	int nLenPath = 0 ;
    
    if (nLen < 2)
    {
        szPath[0] = 0;
        return 0;
    }
    refPath = CFBundleCopyExecutableURL(CFBundleGetMainBundle());
    CFURLGetFileSystemRepresentation(refPath,1, (UInt8*)szPath, nLen-1 );
    CFRelease(refPath);
	nLenPath = strlen(szPath);


    
	return nLenPath;
#else
	char link[100];
	int nReaden = 0;
	sprintf(link, "/proc/%d/exe", getpid());/////////////
	nReaden = readlink(link, szPath, nLen - 1);//////////////
	if (nReaden < 0)
	{
		return 0;
	}
	szPath[nReaden] = 0;
	return nReaden;
#endif
}

int MYAPI GetCurrentAppDir(char *szPath, int nLen)
{
	int nLenPath = 0 ;
    GetCurrentAppPath(szPath,nLen);
    
    ChopPathFileName(szPath);
    
    
    
    nLenPath = strlen(szPath);
    
    if( nLenPath >= 3 )
    {
        if( szPath[nLenPath-1] == '/' &&  szPath[nLenPath-2] == '.')
        {
            szPath[nLenPath-2] = 0 ;
        }
    }
    
    return nLenPath ;
}




void MYAPI AddDllSearchPath(const char *path)
{
#if defined(WIN32)
	char DlVar[]="PATH";
#elif defined(__DARWIN_UNIX03)
	char DlVar[]="DYLD_LIBRARY_PATH";
#else
	char DlVar[] = "LD_LIBRARY_PATH";
#endif
	char szBuf[4096];
	char *envOld = NULL;
	szBuf[sizeof(szBuf)-1] = 0;
	envOld = getenv(DlVar);
#if defined(WIN32)
	if( envOld == NULL || envOld[0] == 0 )
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "%s=%s", DlVar, path);
	}
	else
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "%s=%s;%s", DlVar, path, envOld);
	}
	
#else
	if (envOld == NULL || envOld[0] == 0)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "%s=%s", DlVar, path);
	}
	else
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "%s=%s:%s", DlVar, path, envOld);
	}
#endif
	putenv(szBuf);
}

void MYAPI AddSearchPath(const char *path)
{
	char DlVar[] = "PATH";
	char szBuf[4096];
	char *envOld = NULL;
	szBuf[sizeof(szBuf) - 1] = 0;
	envOld = getenv(DlVar);
#if defined(WIN32)
	if (envOld == NULL || envOld[0] == 0)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "%s=%s", DlVar, path);
	}
	else
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "%s=%s;%s", DlVar, path, envOld);
	}

#else
	if (envOld == NULL || envOld[0] == 0)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "%s=%s", DlVar, path);
	}
	else
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "%s=%s:%s", DlVar, path, envOld);
	}
#endif
	putenv(szBuf);
}





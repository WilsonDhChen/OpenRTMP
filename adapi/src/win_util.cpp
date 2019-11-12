
#include"adapi_def.h"


#if defined(WIN32) && !defined(_WIN32_WCE)

#include"shlobj.h"
#include"shlwapi.h"
#include"tchar.h"
#include"atlbase.h"
#include"atlconv.h"

typedef HRESULT (WINAPI  *_DllGetClassObject)(REFCLSID rclsid, REFIID riid, LPVOID * ppvObj); 
HRESULT  WINAPI Win32CoCreateInstanceFromDLLW(__in     REFCLSID rclsid, 
								 __in_opt LPUNKNOWN pUnkOuter,
								 __in     DWORD dwClsContext, 
								 __in     REFIID riid, 
								 __deref_out LPVOID FAR* ppv,
								 const WCHAR *szDllPath)
{

	return 0 ;
}
HRESULT  WINAPI Win32CoCreateInstanceFromDLLA(__in     REFCLSID rclsid, 
										 __in_opt LPUNKNOWN pUnkOuter,
										 __in     DWORD dwClsContext, 
										 __in     REFIID riid, 
										 __deref_out LPVOID FAR* ppv,
										 const char *szDllPath)
{
	CA2W pszW(szDllPath);
	return Win32CoCreateInstanceFromDLLW( rclsid , pUnkOuter , dwClsContext , riid , ppv ,pszW );
}
const char * MYAPI Win32UserAppData(char *szPathOut,int nLen)
{
  
    return szPathOut ; 
}
const char * MYAPI Win32UserSettingAppData(char *szPathOut,int nLen)
{
 
    return szPathOut ;
}

const char * MYAPI Win32AllUserAppData(char *szPathOut,int nLen)
{
 
    return szPathOut ;
}

const char * MYAPI Win32AllUserDocuments(char *szPathOut,int nLen)
{
 
    return szPathOut ;
}

const char * MYAPI Win32UserDocuments(char *szPathOut,int nLen)
{

    return szPathOut ;
}


BOOL MYAPI Win32RegGetString(const char *szRegPath, const char *szValueName,
								 char *outvalue,int cb)
{

	return TRUE;
}

BOOL MYAPI Win32RegSetString(const  char *szRegPath, const char *szValueName,  const char *szValue)
{

	return TRUE;
}

BOOL MYAPI Win32RegDelKey(const char *szRegPath, const char *szSubKey)
{

	return TRUE;
}

BOOL MYAPI Win32RegDelValue(const char *szRegPath, const char *szValueName)
{

	return TRUE;
}

#endif





#include "adapi_def.h"

static char _szDbUser[] = "user" ;
static char _szDbPass[] = "Pass" ;
static char _szDbHost[] = "Host" ;
static char _szDbDatabase[] = "Database" ;
static char _szDbExtra[] = "Extra" ;
static char _szDbPort[] = "port" ;
static char _szDbTimeout[] = "timeout" ;

int IniReadDatabase(const char *szIniFile,const char *szSection,PDATABASE_INI pDblIni)
{
	static const char szDbSection[]="database";
	if(!szIniFile||!pDblIni)
	{
		return 0;
	}
	
	if(szSection==0 || szSection[0] == 0)
	{
		szSection=szDbSection;
	}
	IniRead(szSection,_szDbUser,pDblIni->szUser,LEN_DB_USER,szIniFile);
	IniRead(szSection,_szDbPass,pDblIni->szPass,LEN_DB_PASS,szIniFile);
	IniRead(szSection,_szDbHost,pDblIni->szHost,LEN_DB_HOST,szIniFile);
	IniRead(szSection,_szDbDatabase,pDblIni->szDatabase,LEN_DB_DATABASE,szIniFile);
	IniRead(szSection,_szDbExtra,pDblIni->szExtra,LEN_DB_EXTRA,szIniFile);

	pDblIni->nPort=IniReadInt(szSection,_szDbPort,szIniFile,0);
	pDblIni->nTimeOut=IniReadInt(szSection,_szDbTimeout,szIniFile,0);

	
	StrTrimX(pDblIni->szUser);
	StrTrimX(pDblIni->szPass);
	StrTrimX(pDblIni->szHost);
	StrTrimX(pDblIni->szDatabase);
	StrTrimX(pDblIni->szExtra);

	

	
	return 1;
}

int IniWriteDatabase(const char *szIniFile,const char *szSection,const PDATABASE_INI pDblIni)
{
    static const char szDbSection[]="database";
    if(!szIniFile||!pDblIni)
    {
        return 0;
    }

    if(szSection==0 || szSection[0] == 0)
    {
        szSection=szDbSection;
    }


    IniWrite(szSection,_szDbUser,pDblIni->szUser,szIniFile);
    IniWrite(szSection,_szDbPass,pDblIni->szPass,szIniFile);
    IniWrite(szSection,_szDbHost,pDblIni->szHost,szIniFile);
    IniWrite(szSection,_szDbDatabase,pDblIni->szDatabase,szIniFile);
    IniWrite(szSection,_szDbExtra,pDblIni->szExtra,szIniFile);
    IniWriteInt(szSection,_szDbPort,pDblIni->nPort,szIniFile);
    IniWriteInt(szSection,_szDbTimeout,pDblIni->nTimeOut,szIniFile);




    return 1;
}






















#include "adapi_def.h"



static const char FUNCTION_INIT[]="UserInit";
static const char FUNCTION_LOGIN[]="UserLogin";

PAUTH_USER MYAPI AuthUserAlloc(const char *szUser,const char *szPwd)
{
	int nLenUser=0,nLenPwd=0;
	PAUTH_USER pUser=0;
	if(!szUser||!szUser[0])
	{
		return 0;
	}
	nLenUser=strlenx(szUser);
	nLenPwd=strlenx(szPwd);

	pUser=(PAUTH_USER)_MEM_ALLOC(sizeof(AUTH_USER)+nLenUser+1+nLenPwd+1);
	if(!pUser)
	{
		return 0;
	}
	pUser->next=0;
	pUser->szUser = ((char *)(pUser+1));
	pUser->szPwd  = ((char *)(pUser+1))+nLenUser+1;
	((char *)(pUser->szPwd))[0]=0;

	strcpyx((char *)pUser->szUser,szUser);
	strcpyx((char *)pUser->szPwd,szPwd);

	return pUser;
}



void MYAPI AuthUserFree(PAUTH_USER *ppUser)
{
	if(!ppUser||!(*ppUser))return;
	_MEM_FREEX(ppUser);
	return;
}

void MYAPI AuthUserFreeList(PAUTH_USER pUser)
{
	PAUTH_USER pUserTmp=0;
	while(pUser)
	{
		pUserTmp=pUser;
		pUser=pUser->next;
		AuthUserFree(&pUserTmp);
	}
	return;
}
int MYAPI AuthUserLogin(PAUTH_USER pUserList,const char *szUser,const char *szPwd,int nUserType)
{
	while(pUserList)
	{
		if(strcmpix(szUser,pUserList->szUser)==0)
		{
			if(strcmp(szPwd,pUserList->szPwd)==0)
			{
				return _LOGIN_OK_;
			}
			else
			{
				return _LOGIN_PWD_ERROR_;
			}

		}
		pUserList=pUserList->next;
	}
	return _LOGIN_NO_USER_;
}
int MYAPI AuthPluginLogin(PAUTH_PLUGIN pPluginList,const char *szUser,const char *szPwd,int nUserType,void *ExtData)
{
	int nRet;
	if(pPluginList || !szUser || !szPwd)
	{
		return _LOGIN_SYS_ERROR_;
	}
	while(pPluginList)
	{
		if(pPluginList->UserLogin)
		{
			nRet=pPluginList->UserLogin(szUser,szPwd,nUserType,ExtData);
			if(nRet==_LOGIN_OK_ || nRet==_LOGIN_PWD_ERROR_ )
			{
				return nRet;
			}
		}

		pPluginList=pPluginList->next;
	}
	return _LOGIN_NO_USER_;
}
void MYAPI AuthUsersInitFromIni(PAUTH_USER *ppUserHead,const char *szIniSec,const char *szIniFile)
{
	char szBuf[200];
	char *pszUser=0,*pszPwd=0;
	int i=0,j=0;
	PAUTH_USER pUserTmp=0,*ppUser=0;
	ppUser=ppUserHead;
	if( !ppUser )
	{
		return ;
	}
	AuthUserFreeList(*ppUser);
	while(IniReadIndex(szIniSec,i,szBuf,200,szIniFile)>0)
	{
		pszUser=szBuf;
		pszPwd=0;
		for(j=0;szBuf[j];j++)
		{
			if( szBuf[j] ==',' || szBuf[j]==';' || szBuf[j]==':' )
			{
				szBuf[j]=0;
				pszPwd = szBuf+j+1;
				break;
			}
		}
		pUserTmp=AuthUserAlloc(pszUser,pszPwd );
		if(!pUserTmp)break;

		*ppUser=pUserTmp;
		ppUser=&(pUserTmp->next);
		i++;
	}
	return ;
}

int MYAPI AuthPluginLoad(PAUTH_PLUGIN pPlugin,const char *szPathPlugin,const char *szLoginFunction)
{
	DLL_HANDLE hDll=0;
	int IsPreLoaded=0;
	int nRet;
	_UserInit UserInit=0;
	if(!szPathPlugin)return 0;
	memset(pPlugin,0,sizeof(AUTH_PLUGIN));
	hDll=DllHandle(szPathPlugin);
	if(hDll)
	{
		//ServiceLog(_LOG_LEVEL_WARNING,"plugin <%s> already loaded\n",szPathPlugin);
		IsPreLoaded=1;
	}
	if(!hDll)
	{
		hDll=DllOpen(szPathPlugin,0);
		if(!hDll)
		{
			//ServiceLog(_LOG_LEVEL_ERROR,"plugin <%s> can't load\n",szPathPlugin);
			return 0;
		}
	}
	pPlugin->UserLogin=(_UserLogin)DllSymbol(hDll,szLoginFunction);
	if(!pPlugin->UserLogin)
	{
		if(szLoginFunction&&szLoginFunction[0])
		{
			//ServiceLog(_LOG_LEVEL_WARNING,"plugin <%s> not find user defined function <%s>\n",szPathPlugin,szLoginFunction);
		}
		pPlugin->UserLogin=(_UserLogin)DllSymbol(hDll,FUNCTION_LOGIN);
	}
	UserInit=(_UserInit)DllSymbol(hDll,FUNCTION_INIT);
	if(!pPlugin->UserLogin)
	{
		if( !IsPreLoaded )DllClose(hDll);
		//ServiceLog(_LOG_LEVEL_ERROR,"plugin <%s> not found UserLogin function\n",szPathPlugin);
		return 0;
	}
	if(!IsPreLoaded && UserInit)
	{
		nRet=UserInit();
		if(nRet!=0)
		{
			if( !IsPreLoaded )DllClose(hDll);
			//ServiceLog(_LOG_LEVEL_ERROR,"plugin <%s> init error,UserInit return %d,return 0 is success\n",szPathPlugin,nRet);
			return 0;
		}
	}
	else
	{
		//ServiceLog(_LOG_LEVEL_WARNING,"plugin <%s> not find function UserInit\n",szPathPlugin);

	}
	pPlugin->hPlugin=hDll;
	strcpyn(pPlugin->szPluginName,FileNameFromPath(szPathPlugin),LEN_AUTH_NAME);
	return 1;
}































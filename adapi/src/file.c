

//#define MYDEBUG_ADAPI

#include"adapi_def.h"

static const char RC_CRLF[]="\r\n";


#ifndef _POSIX_PATH_MAX
#define _POSIX_PATH_MAX     260
#endif
int MYAPI FileReadEx(const char *szFile,INT_FILE nSeek,void *buf,unsigned int nBufSize)
{
    FILE_HANDLE hFile;
    int nReaded;
#ifdef MYDEBUG_ADAPI
MyAdpiLogXX("FileReadEx\n");
#endif
    hFile=FileOpenXX(szFile,OX_RDONLY);
    if(FILE_IO_ERR==hFile)
    {
        sprintf(szADPI_ERROR,"FileOpenXX <%s> fail",szFile);
        return -1;
    }
    if(FileSeekXX( hFile, nSeek, SEEK_SET)==-1L)
    {
        sprintf(szADPI_ERROR,"FileOpenXX <%s> fail",szFile);
        FileCloseXX(hFile);
        return -1;
    }
    nReaded=FileReadXXLoop(hFile,buf,nBufSize);
    FileCloseXX(hFile);
    return nReaded;
}
int MYAPI FileWriteEx(const char *szFile,INT_FILE nSeek,const void *buf,unsigned int nBufSize)
{
    FILE_HANDLE hFile;
    int nWriten;
#ifdef MYDEBUG_ADAPI
MyAdpiLogXX("FileWriteEx\n");
#endif
    //chmod(szFile,S_IREAD|S_IWRITE);

    hFile=FileOpenXX(szFile,OX_CREAT|OX_RDWR);
    if(FILE_IO_ERR==hFile)
    {
        sprintf(szADPI_ERROR,"FileOpenXX <%s> fail",szFile);
        return -1;
    }
    if(FileSeekXX( hFile, nSeek, SEEK_SET)==-1L)
    {
        sprintf(szADPI_ERROR,"FileSeekXX <%s> fail",szFile);
        FileCloseXX(hFile);
        return -1;
    }
    nWriten=FileWriteXX(hFile,buf,nBufSize);
    FileCloseXX(hFile);
    return nWriten;
}

FILE_HANDLE  MYAPI FileOpenXX2(const char *szFile,int nMode)
{
	int nLen;
	char c;
	unsigned int dModeTmp;
	char *szFileName;

	dModeTmp=nMode&OX_CREAT;
    if(dModeTmp!=OX_CREAT)
    {
		return FileOpenXX(szFile,nMode);
    }
	nLen=strlenx(szFile)-1;
	szFileName=(char *)alloca(nLen+3);
	if(!szFileName)return FILE_HANDLE_INVALID;
	strcpyn(szFileName,szFile,nLen+3);
	for(;nLen>0;nLen--)
	{
		c=szFileName[nLen];
		if(c=='\\'||c=='/')
		{
			szFileName[nLen]=0;
			MakeDirRecur(szFileName, NULL);
			break;
		}
	}

	return FileOpenXX(szFile,nMode);

}
FILE_HANDLE  MYAPI FileOpenXX(const char *szFile,int nModeIn)
{
    unsigned int dModeAccess = 0;
    unsigned int dModeCreation = 0;
    unsigned int dModeTmp = 0;
    FILE_HANDLE hFile = FILE_HANDLE_INVALID;
    int nMode = 0;
#ifdef WIN32
    DWORD dwShareMode = FILE_SHARE_READ;
    SECURITY_ATTRIBUTES sa;
    SECURITY_DESCRIPTOR sd;
#endif
#if defined(_UNICODE) || defined(UNICODE)
	LPTSTR pszTmp=0;
	int nLenWideChar;

    if( !szFile || !szFile[0] )
    {
        return FILE_HANDLE_INVALID;
    }
	nLenWideChar=MultiByteToWideChar(CP_ACP,MB_COMPOSITE,(char*)szFile,-1,0,0);
	pszTmp=alloca((nLenWideChar+1)*2);
	pszTmp[nLenWideChar]=0;
	MultiByteToWideChar(CP_ACP,MB_COMPOSITE,(char*)szFile,-1,pszTmp,nLenWideChar);
#endif
    dModeAccess=0;
    dModeCreation=0;




#ifdef MYDEBUG_ADAPI
MyAdpiLogXX("FileOpenXX \n");
#endif

    if( !szFile || !szFile[0] )
    {
        return FILE_HANDLE_INVALID;
    }
    dModeTmp=nModeIn&OX_WRONLY;
    if(dModeTmp==OX_WRONLY)
    {
        nMode |= O_WRONLY;
#ifdef WIN32
        dModeAccess|=GENERIC_WRITE;
        dModeCreation=OPEN_EXISTING;
        dwShareMode |= FILE_SHARE_WRITE ;
#endif
    }
    dModeTmp=nModeIn&OX_RDONLY;
    if(dModeTmp==OX_RDONLY)
    {
        nMode |= O_RDONLY;
#ifdef WIN32
        dModeAccess|=GENERIC_READ;
        dModeCreation=OPEN_EXISTING;
		dwShareMode |= FILE_SHARE_READ ;
#endif
    }
    dModeTmp=nModeIn&OX_RDWR;
    if(dModeTmp==OX_RDWR)
    {
        nMode |= O_RDWR;
#ifdef WIN32
        dModeAccess=GENERIC_READ|GENERIC_WRITE;
        dModeCreation=OPEN_EXISTING;
        dwShareMode |= FILE_SHARE_WRITE ;
		dwShareMode |= FILE_SHARE_READ ;
#endif
    }

    dModeTmp=nModeIn&OX_CREAT;
    if(dModeTmp==OX_CREAT)
    {
        nMode |= O_CREAT;
#ifdef WIN32
        dModeCreation=OPEN_ALWAYS;
        dwShareMode |= FILE_SHARE_WRITE ;
#endif
        //dModeAccess=GENERIC_READ|GENERIC_WRITE;
    }
#ifdef WIN32
    dModeTmp=nModeIn&O_TRUNC;
    if(dModeTmp==O_TRUNC)
    {
        dModeCreation=CREATE_ALWAYS;
        dModeAccess=GENERIC_READ|GENERIC_WRITE;
        dwShareMode |= FILE_SHARE_WRITE ;
    }
#endif
    dModeTmp=nModeIn&OX_EXCL;
    if(dModeTmp==OX_EXCL)
    {
        nMode |= O_EXCL;
#ifdef WIN32
        dModeCreation=OPEN_EXISTING;
#endif
    }
    dModeTmp=nModeIn&OX_APPEND;
    if(dModeTmp==OX_APPEND)
    {
        nMode |= O_APPEND;
#ifdef WIN32
        dModeCreation=OPEN_EXISTING;
#endif
    }



#ifdef WIN32
    InitializeSecurityDescriptor(&sd,SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(&sd,TRUE,NULL,FALSE);
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = &sd;

	#if defined(_UNICODE) || defined(UNICODE)

    hFile = (FILE_HANDLE) CreateFile(pszTmp,dModeAccess,dwShareMode,
                            0,dModeCreation,FILE_ATTRIBUTE_NORMAL,&sa);
	#else
    hFile = (FILE_HANDLE) CreateFile(szFile,dModeAccess,dwShareMode,
                            0,dModeCreation,FILE_ATTRIBUTE_NORMAL,&sa);
	#endif

    if(hFile != FILE_HANDLE_INVALID && (nModeIn&OX_APPEND) == OX_APPEND )
    {
        SetFilePointer((HANDLE)hFile,0,0,FILE_END );
    }
#else

    hFile=open(szFile,nMode,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
#endif

    return hFile;

}
int MYAPI FileReadLineXX(FILE_HANDLE hFile,void *Buf,int nBufSize,BOOL *pIsEnd,BOOL *pIsLineEnd)
{
	BOOL IsEnd,i=0;
	char c;
	char *psz=(char *)Buf;
	BOOL *pEnd;
	nBufSize--;
	if(!psz||nBufSize<=0)return 0;
	if(pIsEnd==0)pEnd=&IsEnd;
	else pEnd=pIsEnd;
	psz[0]=0;

	while(CharPosInStr(c=FileGetChar(hFile,pEnd),RC_CRLF)>=0&&*pEnd==0);
	if(*pEnd)return 0;
	psz[i]=c;
	i++;
	while(i<nBufSize&&
		CharPosInStr(c=FileGetChar(hFile,pEnd),RC_CRLF)<0&&
		*pEnd==0)
	{
		psz[i]=c;
		i++;
	}
	if(pIsLineEnd&&CharPosInStr(c,RC_CRLF)>=0)*pIsLineEnd=TRUE;
	psz[i]=0;
	return i;
}
char MYAPI FileGetChar(FILE_HANDLE hFile,BOOL *IsFileEnd)
{
	int nRet;
	char c;
	if(IsFileEnd)*IsFileEnd=0;
	nRet=FileReadXX(hFile,&c,1);
	if(nRet<=0)
	{
		if(IsFileEnd)*IsFileEnd=TRUE;
		return (char)EOF;
	}
	else return c;
}
int MYAPI FileReadXXLoop(FILE_HANDLE hFile,void *Buf,unsigned int nBufSize)
{
	char *pBuf=(char *)Buf;
	int nReaded=0;
    int nSumReaded=0;
	int nWillRead=(int)nBufSize;

	nReaded=FileReadXX(hFile,pBuf+nSumReaded,nWillRead);
	if(nReaded<0)
	{
	//	if(pIsEOf)*pIsEOf=1;
		return -1;
	}
	do
	{
		nSumReaded+=nReaded;
		nWillRead=(int)nBufSize-nSumReaded;
		if(nWillRead<1)
		{
			break;
		}
		nReaded=FileReadXX(hFile,pBuf+nSumReaded,nWillRead);
	} while(nReaded>0);
//	if(nReaded<0&&pIsEOf)*pIsEOf=1;
	return nSumReaded;

}
BOOL MYAPI FileTruncateXX(FILE_HANDLE hFile,INT_FILE length )
{
#ifdef WIN32
    if( FileSeekXX(hFile , length, SEEK_SET ) == length )
    {
        return SetEndOfFile( hFile );
    }
    else
    {
        return FALSE ;
    }
#else
    return (ftruncate( hFile , length ) == 0 );
#endif
}

int MYAPI FileReadXX(FILE_HANDLE hFile,void *Buf,unsigned int nBufSize)
{
    int nRet=0;
    #ifdef WIN32
    DWORD nRead=0;
    #else
    int nRead=0;
    #endif
#ifdef MYDEBUG_ADAPI
MyAdpiLogXX("FileReadXX\n");
#endif

#ifdef WIN32
    nRead=0;
    nRet=ReadFile(hFile,Buf,nBufSize,&nRead,0);
    if(nRet==0){nRead=-1;}
    return nRead;
#else

	do
	{
		nRet=read((int)hFile,Buf,nBufSize);

	} while(nRet<0 && errno==EINTR);


    return nRet;


#endif
}
int MYAPI FileWriteXXLoop(FILE_HANDLE hFile,const void *Buf, unsigned int nBufSize)
{
    int nWritten = 0 ;
    int nRet = 0 ;
    const char *pBuf = (const char *)Buf ;
    if( pBuf == NULL )
    {
        return -1 ;
    }
    if( nBufSize == (unsigned int)-1 )
    {
        nBufSize = strlenx(pBuf);
    }


    while( nBufSize > 0 )
    {
        nRet = FileWriteXX(hFile,pBuf+nWritten,nBufSize);
        if( nRet < 0 )
        {
            if( nWritten == 0 )
            {
                nWritten = -1 ;
            }
            break ;
        }
        else if( nRet == 0 )
        {
            break ;
        }
        nWritten += nRet ;
        nBufSize -= nRet ;
    }
    return nWritten ;

}
int MYAPI FileWriteXX(FILE_HANDLE hFile,const void *Buf, unsigned int nBufSize)
{
    int nRet;
    #ifdef WIN32
    DWORD nWritten=0;
    #else
    int nWritten = 0 ;
    #endif
#ifdef MYDEBUG_ADAPI
MyAdpiLogXX("FileWriteXX\n");
#endif
	if( !Buf )
	{
		return 0;
	}
	if( nBufSize == (unsigned int)-1 )
	{
		nBufSize = strlenx((const char *)Buf);
	}
#ifdef WIN32
    nWritten=0;
    nRet=WriteFile(hFile,Buf,nBufSize,&nWritten,0);
    if(nRet==0){nWritten=-1;}
    return nWritten;
#else
    do
     {
         nWritten =  write(hFile,Buf,nBufSize);
     } while(nWritten<0 && errno==EINTR);
     return nWritten;

#endif

}
INT_FILE MYAPI FileSeekXX(FILE_HANDLE hFile, INT_FILE nOffSet,int nSet)
{


#ifdef MYDEBUG_ADAPI
MyAdpiLogXX("FileSeekXX\n");
#endif
#ifdef WIN32
    LARGE_INTEGER li;

	li.QuadPart = nOffSet;
    if(nSet==SEEK_SET)
    {
        nSet=FILE_BEGIN;
    }
    else if(nSet==SEEK_CUR)
    {
        nSet=FILE_CURRENT;
    }
    else if(nSet==SEEK_END)
    {
        nSet=FILE_END;
    }
    else
    {
        nSet=FILE_BEGIN;
    }
    li.LowPart=SetFilePointer(hFile, li.LowPart, &li.HighPart,nSet);
    if (li.LowPart == (DWORD)-1 && GetLastError() != NO_ERROR)
    {
	   li.QuadPart = -1;
    }
    return (INT_FILE)li.QuadPart;

#else
    return lseek(hFile,nOffSet,nSet);
#endif
}
BOOL MYAPI FileSyncXX( FILE_HANDLE hFile )
{
    if( hFile == FILE_HANDLE_INVALID )
    {
        return FALSE ;
    }
#ifdef WIN32
    return FlushFileBuffers(hFile);
#else
    return ( fsync(hFile) == 0 );
#endif
}
static BOOL _bAutoSync = FALSE ;
void MYAPI FileSetAutoSync(BOOL bAutoSync)
{
      _bAutoSync =    bAutoSync ;
}
int MYAPI FileCloseXX(FILE_HANDLE hFile)
{
#ifdef MYDEBUG_ADAPI
MyAdpiLogXX("FileCloseXX\n");
#endif
	if(hFile==FILE_HANDLE_INVALID)
    {
        return 0;
    }
    if( _bAutoSync )
    {
        FileSyncXX(hFile);
    }

#ifdef WIN32
    if(CloseHandle(hFile)==0)
    {
        return -1;
    }
    else
    {
        return 0;
    }
#else
    return close(hFile);
#endif

}

INT_FILE MYAPI FileLength(const char *szFile)
{

#ifdef MYDEBUG_ADAPI
MyAdpiLogXX("FileLength\n");
#endif
#if defined(WIN32)
	LARGE_INTEGER li;
	FILE_HANDLE hFile;
	hFile=FileOpenXX(szFile,OX_RDONLY);
	if(hFile==FILE_HANDLE_INVALID)
	{
		return -1;
	}

	li.LowPart=GetFileSize(hFile,(LPDWORD)&li.HighPart);
	FileCloseXX(hFile);
	return (INT_FILE)li.QuadPart;
#else
	struct stat FileStat;
    if(stat(szFile,&FileStat)!=-1)
    {
        return FileStat.st_size;
    }
    else
    {
        return -1;
    }
#endif
}
int MYAPI PathIsDirOrFileX(const char *szFile)
{

#ifdef MYDEBUG_ADAPI
MyAdpiLogXX("PathIsDirOrFileX\n");
#endif
    if(PathIsDirectoryX(szFile))
    {

		return 2;

    }
    else if(PathIsFileX(szFile))
    {
        return 1;
    }
	else
	{
		return 0;
	}
}
BOOL  MYAPI PathIsDirectoryX(const char *szPath)
{
#if defined(WIN32)
	DWORD dwAttr;
	#if defined(_UNICODE) || defined(UNICODE)

	LPTSTR pszTmp=0;
	int nLenWideChar;
	nLenWideChar=MultiByteToWideChar(CP_ACP,MB_COMPOSITE,(char*)szPath,-1,0,0);
	pszTmp=alloca((nLenWideChar+1)*2);
	pszTmp[nLenWideChar]=0;
	MultiByteToWideChar(CP_ACP,MB_COMPOSITE,(char*)szPath,-1,pszTmp,nLenWideChar);

	dwAttr=GetFileAttributes(pszTmp);
	#else
	dwAttr=GetFileAttributes(szPath);
	#endif
	if(dwAttr==(DWORD)-1)return 0;
	if(dwAttr&FILE_ATTRIBUTE_DIRECTORY)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
#else

    struct stat FileStat;
#ifdef MYDEBUG_ADAPI
MyAdpiLogXX("PathIsDirectoryX\n");
#endif
    if(stat(szPath,&FileStat)!=-1)
    {
		if(FileStat.st_mode&S_IFDIR)
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}

    }
    else
    {
        return FALSE;
    }

#endif
}
BOOL MYAPI PathIsFileX(const char *szFile)
{
#if defined(WIN32)
	DWORD dwAttr;
	#if defined(_UNICODE) || defined(UNICODE)
	LPTSTR pszTmp=0;
	int nLenWideChar;
	nLenWideChar=MultiByteToWideChar(CP_ACP,MB_COMPOSITE,(char*)szFile,-1,0,0);
	pszTmp=alloca((nLenWideChar+1)*2);
	pszTmp[nLenWideChar]=0;
	MultiByteToWideChar(CP_ACP,MB_COMPOSITE,(char*)szFile,-1,pszTmp,nLenWideChar);
	dwAttr=GetFileAttributes(pszTmp);
	#else
	dwAttr=GetFileAttributes(szFile);
	#endif
	if(dwAttr==(DWORD)-1)
        {
            return FALSE;
        }
	if(!(dwAttr&FILE_ATTRIBUTE_DIRECTORY))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
#else
    struct stat FileStat;
#ifdef MYDEBUG_ADAPI
MyAdpiLogXX("PathIsFileX\n");
#endif
    if(stat(szFile,&FileStat)!=-1)
    {
		if(FileStat.st_mode&S_IFDIR)
		{
			return FALSE;
		}
		else
		{
			return TRUE;
		}

    }
    else
    {
        return FALSE;
    }
#endif

}

int MYAPI FileOverWriteX(const char *szFile,const void *buf,int nBufSize)
{
    FILE_HANDLE hFile;
    int nWrite;
#ifdef MYDEBUG_ADAPI
MyAdpiLogXX("FileOverWriteX\n");
#endif
    //chmod(szFile,S_IREAD|S_IWRITE);
    hFile=FileOpenXX2(szFile,OX_CREAT|OX_TRUNC|OX_RDWR);

    if(FILE_HANDLE_INVALID==hFile)
    {
        sprintf(szADPI_ERROR,"open <%s> fail",szFile);
        return -1;
    }

    nWrite=FileWriteXX(hFile,buf,nBufSize);
    FileCloseXX(hFile);
    return nWrite;
}

BOOL MYAPI RemoveFileX(const char *szFilePath)
{
#ifdef MYDEBUG_ADAPI
MyAdpiLogXX("RemoveFileX\n");
#endif

#ifdef WIN32
	#if defined(_UNICODE) || defined(UNICODE)

	LPTSTR pszTmp=0;
	int nLenWideChar;
	nLenWideChar=MultiByteToWideChar(CP_ACP,MB_COMPOSITE,(char*)szFilePath,-1,0,0);
	pszTmp=alloca((nLenWideChar+1)*2);
	pszTmp[nLenWideChar]=0;
	MultiByteToWideChar(CP_ACP,MB_COMPOSITE,(char*)szFilePath,-1,pszTmp,nLenWideChar);
	DeleteFile(pszTmp);
	#else
	DeleteFile(szFilePath);
	#endif
#else
    //chmod(szFilePath,S_IREAD|S_IWRITE);
    unlink(szFilePath);
#endif
    if( FileLength(szFilePath) == -1 )
    {
        return TRUE ;
    }
    else
    {
        return FALSE ;
    }

}
BOOL MYAPI MakeDirX(const char *szFolderPath)
{
#ifdef MYDEBUG_ADAPI
MyAdpiLogXX("MakeDirX\n");
#endif

#ifdef WIN32
        SECURITY_ATTRIBUTES sa;
		SECURITY_ATTRIBUTES* lpSa = &sa;
		LPCTSTR path = NULL ; 
		LPTSTR pszTmp = 0;
		int nLenWideChar = 0 ;
		BOOL bOK = FALSE ;



		#if defined(_UNICODE) || defined(UNICODE)


		nLenWideChar=MultiByteToWideChar(CP_ACP,MB_COMPOSITE,(char*)szFolderPath,-1,0,0);
		pszTmp=alloca((nLenWideChar+1)*2);
		pszTmp[nLenWideChar]=0;
		MultiByteToWideChar(CP_ACP,MB_COMPOSITE,(char*)szFolderPath,-1,pszTmp,nLenWideChar);
		path = pszTmp ;

		#else

		path = szFolderPath ;


		#endif


		{
			TCHAR * szSD = _T("D:")       // Discretionary ACL
				_T("(A;OICI;GA;;;BU)")     // Allow full control to user
				_T("(A;OICI;GA;;;AN)")     // Allow full control to anonymous logon
				_T("(A;OICI;GA;;;AU)") // Allow full control to authenticated users
				_T("(A;OICI;GA;;;WD)")    // Allow full control to everyone
				_T("(A;OICI;GA;;;BA)");    // Allow full control to administrators

			sa.nLength = sizeof(SECURITY_ATTRIBUTES);
			sa.bInheritHandle = FALSE;
			sa.lpSecurityDescriptor = NULL ;
			if( ! ConvertStringSecurityDescriptorToSecurityDescriptor(
				szSD,
				SDDL_REVISION_1,
				&(sa.lpSecurityDescriptor),
				NULL) )
			{
				lpSa = NULL ;
			}

		}


		bOK  =  CreateDirectory( path, lpSa );

		if( lpSa != NULL )
		{
			LocalFree(lpSa->lpSecurityDescriptor);
		}

		return bOK;

		

#else
    return ( mkdir(szFolderPath,0777) == 0 ) ;
#endif

}
int MYAPI MakeDirRecur(const char *szFolderPath, char *szNewPathOut)
{

	char *pszPath;
	int iFolder=0,iPath=0,nLenPath;
	int nCpyed=0;
#if defined(WIN32)
	int IsWinNet=0,iLoopCount=0;
#endif
	static char szDIR_DIR[]="/\\";
	if(!szFolderPath||!szFolderPath[0])return 0;
	nLenPath=strlenx(szFolderPath)+10;
	pszPath=(char *)alloca(nLenPath);
	if(!pszPath)return 0;

	if(szFolderPath[0]=='\\'||szFolderPath[0]=='/')
	{
#if defined(WIN32)
		pszPath[0]='\\';
#else
		pszPath[0]='/';
#endif

		iPath=1;
		szFolderPath+=1;

#if defined(WIN32)
		if(szFolderPath[0]=='\\')
		{
			pszPath[0]='\\';
			pszPath[1]='\\';
			iPath=2;
			szFolderPath+=1;
			IsWinNet=1;
		}
#endif

	}

	for(;CharPosInStr(*szFolderPath,szDIR_DIR)>=0;szFolderPath++);
	do
	{
		nCpyed=strcpyn_stop(pszPath+iPath,szFolderPath,nLenPath-iPath,szDIR_DIR);
		szFolderPath+=nCpyed;
		if(nCpyed>0)
		{
			iPath+=nCpyed;
#if defined(WIN32)
			pszPath[iPath]='\\';
#else
			pszPath[iPath]='/';
#endif
			iPath++;
			pszPath[iPath]=0;

		}
#if defined(WIN32)
	    iLoopCount++;
		if(IsWinNet&&iLoopCount<3)
		{
			goto MYSKIP;
		}
#endif
		if(!PathIsDirectoryX(pszPath))
		{
			if(!MakeDirX(pszPath))
			{
				return 0;
			}
		}

#if defined(WIN32)
MYSKIP:
#endif
		for(;CharPosInStr(*szFolderPath,szDIR_DIR)>=0;szFolderPath++);


	}while(nCpyed>0);

	if (szNewPathOut != NULL)
	{
		strcpyx(szNewPathOut, pszPath);
	}
	return 1;
}


int MYAPI RemoveDirX(const char *szFolderPath)
{
#ifdef MYDEBUG_ADAPI
MyAdpiLogXX("RemoveDirX\n");
#endif
#ifdef WIN32
	#if defined(_UNICODE) || defined(UNICODE)

	LPTSTR pszTmp=0;
	int nLenWideChar;
	nLenWideChar=MultiByteToWideChar(CP_ACP,MB_COMPOSITE,(char*)szFolderPath,-1,0,0);
	pszTmp=alloca((nLenWideChar+1)*2);
	pszTmp[nLenWideChar]=0;
	MultiByteToWideChar(CP_ACP,MB_COMPOSITE,(char*)szFolderPath,-1,pszTmp,nLenWideChar);
	return  RemoveDirectory(pszTmp);
	#else
	return  RemoveDirectory(szFolderPath);
	#endif
#else
    return rmdir(szFolderPath);
#endif
}
/*
int MYAPI RemoveDirX2(const char *szFolderPath)
{
#ifdef MYDEBUG_ADAPI
MyAdpiLogXX("RemoveDirX\n");
#endif
#ifdef WIN32
	#if defined(_UNICODE) || defined(UNICODE)
	RemoveDirectoryA(szFolderPath);
	return  !PathIsDirectoryX(szFolderPath);
	#else
	RemoveDirectory(szFolderPath);
	return  !PathIsDirectoryX(szFolderPath);
	#endif
#else
    rmdir(szFolderPath);
	return !PathIsDirectoryX(szFolderPath);
#endif
}
*/
int MYAPI EnumLogicalDrives(_DeleteFileNotifyCallBack DeleteFileNotifyCallBack,
                            void *UserData,int *pIsStop)
#if defined(WIN32) && !defined(_WIN32_WCE)

{

	char   szDriverList[200];
	char   szDriveName[8];
	int i=0,nDriveNum=0;

	GetLogicalDriveStringsA(200,szDriverList);
	while(*(szDriverList+i)!=0&&i<200)
	{

		strcpyn(szDriveName,szDriverList+i,6);
		if(*szDriveName>='a'&&*szDriveName<='z')(*szDriveName)-=32;
		szDriveName[3]=0;
		for(;*(szDriverList+i)!=0&&i<200;i++);
		i++;
		/*************************************/
		if(DeleteFileNotifyCallBack)
		{
			if( DeleteFileNotifyCallBack(szDriveName,szDriveName,0,-1, UserData) == -1 )
			{
				break ;
			}

		}

		/*************************************/

		nDriveNum++;
	}
	return nDriveNum;


}
#else
{
    return 0 ;
}
#endif
int MYAPI RemoveDirRecur(const char *szFolderPath,
						 _DeleteFileNotifyCallBack DeleteFileNotifyCallBack,
						 void *UserData,int *pIsStop)
{
#if defined(WIN32)
	WIN32_FIND_DATA FindData;
	HANDLE hFind;
	char *pszPath;
	char *psPathFull;
	char  *pszTmpFile;
	int nLenFolder,nLenRemain=0;

#if defined(_UNICODE) || defined(UNICODE)
	LPTSTR pszPathWide;
	LPTSTR pszFolderPathWide;

	int nLenWideChar;
	int nLenFolderWide;
#endif

#if defined(WIN32) && !defined(_WIN32_WCE)
	if(strcmpix(szFolderPath,"/")==0||strcmpix(szFolderPath,"\\")==0)
	{
		return EnumLogicalDrives(DeleteFileNotifyCallBack,UserData,pIsStop);
	}
#endif

#if defined(_UNICODE) || defined(UNICODE)
	nLenWideChar=MultiByteToWideChar(CP_ACP,MB_COMPOSITE,(char*)szFolderPath,-1,0,0);
	pszFolderPathWide=alloca((nLenWideChar+1)*2);
	if(!pszFolderPathWide)return 0;
	pszFolderPathWide[nLenWideChar]=0;
	MultiByteToWideChar(CP_ACP,MB_COMPOSITE,(char*)szFolderPath,-1,pszFolderPathWide,nLenWideChar);

#endif


	if(pIsStop&&*pIsStop)return 0;

	if(!szFolderPath||!szFolderPath[0])return 0;
	if(PathIsFileX(szFolderPath))
	{
		if(DeleteFileNotifyCallBack)
		{
			if(DeleteFileNotifyCallBack(szFolderPath,0,1,
#if defined(_UNICODE) || defined(UNICODE)
			GetFileAttributes(pszFolderPathWide),
#else
			GetFileAttributesA(szFolderPath),
#endif
			UserData) == 1 )
			{
				if(pIsStop&&*pIsStop)return 0;
				return RemoveFileX(szFolderPath);
			}
			else
			{
				return 1;
			}

		}
		else
		{
			if(pIsStop&&*pIsStop)return 0;
			return RemoveFileX(szFolderPath);
		}

	}

	if(pIsStop&&*pIsStop)return 0;
	nLenFolder=strlenx(szFolderPath);
	pszPath=alloca(nLenFolder+10);
	nLenRemain=nLenFolder+MAX_PATH+100;
	psPathFull=alloca(nLenRemain);
	if(!pszPath||!psPathFull)return 0;
	strcpyx(pszPath,szFolderPath);


	nLenFolder=PathAddSlash(pszPath);
	nLenRemain-=nLenFolder;
	strcpyx(psPathFull,pszPath);
	strcpyx(pszPath+nLenFolder,"*");

#if defined(_UNICODE) || defined(UNICODE)
	if(pIsStop&&*pIsStop)return 0;
	pszTmpFile=alloca(MAX_PATH);
	if(!pszTmpFile)return 0;
	nLenWideChar=MultiByteToWideChar(CP_ACP,MB_COMPOSITE,(char*)pszPath,-1,0,0);
	pszPathWide=alloca((nLenWideChar+1)*2);
	if(!pszPathWide)return 0;
	nLenFolderWide=nLenWideChar;
	pszPathWide[nLenWideChar]=0;
	MultiByteToWideChar(CP_ACP,MB_COMPOSITE,(char*)pszPath,-1,pszPathWide,nLenWideChar);

#endif
	if(pIsStop&&*pIsStop)return 0;



#if defined(_UNICODE) || defined(UNICODE)
	hFind=FindFirstFile(pszPathWide,&FindData);
#else
	hFind=FindFirstFile(pszPath,&FindData);
#endif

 	if(hFind==INVALID_HANDLE_VALUE)
	{
		return 1;
	}

	do
	{
		if(pIsStop&&*pIsStop)goto MY_STOP;
		#if defined(_UNICODE) || defined(UNICODE)
		WideCharToMultiByte(CP_ACP,WC_COMPOSITECHECK | WC_DEFAULTCHAR,FindData.cFileName,-1,pszTmpFile,MAX_PATH,0,0);
		#else
		pszTmpFile=FindData.cFileName;
		#endif

		if(strcmpix(".",pszTmpFile)!=0
			&&strcmpix("..",pszTmpFile)!=0)
		{
			strcpyx(psPathFull+nLenFolder,pszTmpFile);

			if(DeleteFileNotifyCallBack)
			{
				if(DeleteFileNotifyCallBack(psPathFull,pszTmpFile,
					!(FindData.dwFileAttributes &FILE_ATTRIBUTE_DIRECTORY),
					FindData.dwFileAttributes,UserData))
				{

					goto MY_REMOVE;
				}
				else
				{
					continue;
				}

			}

MY_REMOVE:
			if(pIsStop&&*pIsStop)goto MY_STOP;
			if(FindData.dwFileAttributes &FILE_ATTRIBUTE_DIRECTORY)
			{
				RemoveDirRecur(psPathFull,DeleteFileNotifyCallBack,UserData,pIsStop);
				if(pIsStop&&*pIsStop)goto MY_STOP;
			}
			else
			{
				RemoveFileX(psPathFull);
			}


		}

	}while(FindNextFile(hFind,&FindData));

	FindClose(hFind);

	psPathFull[nLenFolder]=0;
	if(DeleteFileNotifyCallBack)
	{
		if(DeleteFileNotifyCallBack(psPathFull,0,0,
#if defined(_UNICODE) || defined(UNICODE)
			GetFileAttributes(pszFolderPathWide),
#else
			GetFileAttributesA(szFolderPath),
#endif
					UserData))
		{
			if(pIsStop&&*pIsStop)goto MY_STOP;
			return RemoveDirX(psPathFull);
		}
		else
		{
			return 1;
		}

	}
	else
	{
		if(pIsStop&&*pIsStop)goto MY_STOP;
		return RemoveDirX(psPathFull);
	}
	return 1;
MY_STOP:
	FindClose(hFind);
	return 2;
#else
	/************************unix code go here*********************************/
	DIR *pDir=0;
	struct dirent *pDirEntry;
	struct dirent *pDirRes=0;
	char *pszPath;
	char *psPathFull;
	int nLenFolder,nLenRemain=0;
    BOOL bIsFile = FALSE ;

	if(!szFolderPath||!szFolderPath[0])
    {
        return 0;
    }
	if(pIsStop&&*pIsStop)
    {
        return 0;
    }
	if( PathIsFileX(szFolderPath) )
	{
		if(DeleteFileNotifyCallBack)
		{
			if(DeleteFileNotifyCallBack(szFolderPath,0,0,0,UserData))
			{
				if(pIsStop&&*pIsStop)
                {
                    return 0;
                }
				return RemoveFileX(szFolderPath);
			}
			else
			{
				return 1;
			}

		}
		else
		{
			if(pIsStop&&*pIsStop)
            {
                return 0;
            }
			return RemoveFileX(szFolderPath);
		}

	}
	if(pIsStop&&*pIsStop)
    {
        return 0;
    }
	nLenFolder=strlenx(szFolderPath);
	pszPath=(char *)alloca(nLenFolder+10);
	nLenRemain=nLenFolder+_POSIX_PATH_MAX+10;
	psPathFull=(char *)alloca(nLenRemain);
	if(!pszPath||!psPathFull)
    {
        return 0;
    }
	strcpyx(pszPath,szFolderPath);
	nLenFolder = PathAddSlash(pszPath);
	nLenRemain -= nLenFolder;
	strcpyx(psPathFull,pszPath);


	pDirEntry=(struct dirent *)alloca(sizeof(struct dirent) + _POSIX_PATH_MAX+10);
	if(!pDirEntry)
    {
        return 0;
    }
	memset(pDirEntry,0,sizeof(struct dirent));
	if(pIsStop&&*pIsStop)
    {
        return 0;
    }
	pDir = opendir( pszPath );
	if(!pDir)
    {
        return 0;
    }
	while ( ( readdir_r(pDir, pDirEntry, &pDirRes) == 0 ) && pDirRes != 0 )
	{


       // printf("readdir_r %d: %s\n", pDirRes->d_type,pszPath);
		if(pIsStop&&*pIsStop)
        {
            goto MY_STOP;
        }
		if(strcmpix(".",pDirRes->d_name)==0
			||strcmpix("..",pDirRes->d_name)==0)
		{
			continue;
		}
		strcpyn(psPathFull+nLenFolder,pDirRes->d_name,nLenRemain);

        if( pDirRes->d_type == DT_UNKNOWN )
        {
            bIsFile = PathIsFileX( psPathFull ) ;
        }
        else
        {
            bIsFile = (pDirRes->d_type != DT_DIR);
        }

		if(DeleteFileNotifyCallBack)
		{
			if(DeleteFileNotifyCallBack(psPathFull,pDirRes->d_name,
				bIsFile,
				pDirRes->d_type,UserData))
			{

				goto MY_REMOVE;
			}
			else
			{
				/*
				if(pIsStop&&*pIsStop)goto MY_STOP;
				if(pDirRes->d_type==4)
				{
					RemoveDirRecur(psPathFull,DeleteFileNotifyCallBack,UserData,pIsStop);
				}
				*/
				continue;
			}

		}

MY_REMOVE:
		if(pIsStop&&*pIsStop)
        {
            goto MY_STOP;
        }
		if( !bIsFile )
		{
            //printf("RemoveDirRecur:%s\n",psPathFull);
			RemoveDirRecur(psPathFull,DeleteFileNotifyCallBack,UserData,pIsStop);
			if(pIsStop&&*pIsStop)
            {
                goto MY_STOP;
            }
		}
		else
		{
            //printf("RemoveFileX %d:%s\n", pDirRes->d_type,psPathFull);
			RemoveFileX(psPathFull);
		}



	}
	/////////////////////////////
	closedir(pDir);
    pDir = NULL ;

	psPathFull[nLenFolder]=0;

	if(DeleteFileNotifyCallBack)
	{
		if(DeleteFileNotifyCallBack(psPathFull,0,0,0,UserData))
		{
			if(pIsStop&&*pIsStop)
            {
                goto MY_STOP;
            }
			return RemoveDirX(psPathFull);
		}
		else
		{
			return 1;
		}

	}
	else
	{
		if(pIsStop&&*pIsStop)
        {
            goto MY_STOP;
        }
		return RemoveDirX(psPathFull);
	}
	return 1;

MY_STOP:
    if( pDir != NULL )
    {
	    closedir(pDir);
    }
	return 2;
#endif
}






BOOL MYAPI FileSatusUTC(const char *szFile,PFILE_STATX pFileStatus)
{
#ifdef WIN32
    FILETIME lstMdTime;
    FILE_HANDLE hFile ;
    BOOL bRet = FALSE;
    LARGE_INTEGER li;
    if( pFileStatus )
    {
        memset(pFileStatus,0,_SIZE_OF(FILE_STATX));
    }
    hFile = FileOpenXX(szFile,OX_RDONLY);
    if( hFile== FILE_HANDLE_INVALID )
    {
        return FALSE ;
    }
    bRet = GetFileTime(hFile,0,0,&lstMdTime);
    li.LowPart=GetFileSize(hFile,(LPDWORD)&li.HighPart);
    FileCloseXX(hFile);
    if( pFileStatus )
    {
        FileTimeToSystemTime(&lstMdTime,&(pFileStatus->tLstModify) );
        pFileStatus->tLstModify.wMilliseconds = 0;
        pFileStatus->nFileLength = (INT_FILE)li.QuadPart ;
    }
    return TRUE;
#else
    struct stat FileStat;
    struct tm tmLstModify;

    if( pFileStatus )
    {
        memset(pFileStatus,0,_SIZE_OF(FILE_STATX));
    }

    if( stat(szFile,&FileStat) != 0 )
    {
        return FALSE;
    }
    gmtime_r( &FileStat.st_mtime ,&tmLstModify);

    if( pFileStatus )
    {
        tm2SystemTime(&tmLstModify,&(pFileStatus->tLstModify));
        pFileStatus->nFileLength = (INT_FILE)FileStat.st_size;

    }
    return TRUE;
#endif


}

BOOL MYAPI FileSetLstModifyTime(const char *szFile,const SYSTEMTIME *pSysTime,BOOL bLocalTime)
{
#ifdef WIN32
    FILETIME lstMdTime,lstMdTimeSys;
    FILE_HANDLE hFile ;
    SYSTEMTIME systimeMd;
    BOOL bRet = FALSE;
    if( !szFile || !szFile[0] || !pSysTime )
    {
        return FALSE;
    }
    systimeMd = *pSysTime;
    systimeMd.wMilliseconds = 0;
    SystemTimeToFileTime(&systimeMd,&lstMdTime);
    lstMdTimeSys = lstMdTime;
    if( bLocalTime )
    {
        LocalFileTimeToFileTime(&lstMdTime,&lstMdTimeSys);
    }

    hFile = FileOpenXX(szFile,OX_WRONLY);
    if( hFile== FILE_HANDLE_INVALID )
    {
        return FALSE ;
    }
    bRet = SetFileTime(hFile,0,0,&lstMdTimeSys);
    FileSyncXX(hFile);
    FileCloseXX(hFile);
    return bRet;
#else
    struct utimbuf lstMdTime;
    time_t tLstMod;
    if(  !szFile || !szFile[0] || !pSysTime )
    {
        return FALSE;
    }
    tLstMod = SystemTime2time_t(pSysTime);
    if( !bLocalTime )
    {
        tLstMod = time_t2Local(tLstMod);
    }
    lstMdTime.modtime = tLstMod;
    lstMdTime.actime = tLstMod;

    if( utime(szFile,&lstMdTime) == 0 )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
    /*
    struct stat FileStat;
    struct tm tmLstModify;
    if( stat(szFile,&FileStat) == -1 )
    {
        return FALSE;
    }
    gmtime_r( &FileStat.st_mtime ,&tmLstModify);
    tm2SystemTime(&tmLstModify,pSysTime);
    */


#endif


}

BOOL MYAPI FileMoveX(const char *szFileSrc,const char *szFileDest)
{
    if( rename(szFileSrc,szFileDest) == 0 )
    {
#ifndef WIN32
       if( _bAutoSync )
       {
           sync();
       }
#endif
        return TRUE;
    }
    else
    {
        return FALSE ;
    }

}

//#define _FILE_CPY_LOG
BOOL MYAPI FileCopyXX(const char *szFileSrc,const char *szFileDest,_FileCopyProgress funcCall,void *vUserData)
{
	char szBuf[SIZE_2KB];
	const int nBufLen  = SIZE_2KB ;
	int nReaden = 0,nWritten = 0 ;
	INT_FILE nSumWritten = 0 ;
	INT_FILE nFileLength = 0 ;
	BOOL bOk = TRUE ;
	FILE_HANDLE hSrc;
	FILE_HANDLE hDest;
    FILE_STATX  FileStatus ;
	if( ! FileSatusUTC(szFileSrc,&FileStatus) )
	{
#ifdef _FILE_CPY_LOG
            printf("FileSatusUTC src faild <%s>\n",szFileSrc);
#endif
	    return FALSE ;
	}
        nFileLength = FileStatus.nFileLength ;

	hSrc = FileOpenXX2(szFileSrc,OX_RDONLY);
	if( hSrc == FILE_HANDLE_INVALID )
	{
#ifdef _FILE_CPY_LOG
            printf("FileOpenXX2 src faild <%s>\n",szFileSrc);
#endif
	    return FALSE ;
	}
	hDest = FileOpenXX2(szFileDest,OX_CREAT|OX_TRUNC|OX_RDWR);
	if( hDest == FILE_HANDLE_INVALID )
	{
#ifdef _FILE_CPY_LOG
            printf("FileOpenXX2 dst faild <%s>\n",szFileDest);
#endif
	    FileCloseXX(hSrc);
	    return FALSE ;
	}
	if( funcCall )
	{
	    funcCall(nSumWritten,nFileLength,_PROGRESS_STATUS_START,vUserData);
	}
	while( (nReaden = FileReadXX(hSrc, szBuf, nBufLen) ) > 0 )
	{
	    nWritten = FileWriteXX(hDest , szBuf , nReaden );
	    if( nWritten != nReaden )
	    {
	        bOk = FALSE ;
#ifdef _FILE_CPY_LOG
            printf("FileWriteXX dst faild <%s>\n",szFileDest);
#endif
		 break ;
	    }

	    nSumWritten += nWritten ;
	    if( funcCall )
	    {
	        funcCall(nSumWritten,nFileLength,_PROGRESS_STATUS_ING,vUserData);
	    }
	}

	FileCloseXX(hSrc);
    FileSyncXX(hDest) ;
	FileCloseXX(hDest);
	if( !bOk )
	{
	    RemoveFileX(szFileDest);
	}
    else
    {
        FileSetLstModifyTime( szFileDest, & (FileStatus.tLstModify), 0 );
    }
	if( funcCall )
	{
	    funcCall(nSumWritten,nFileLength,_PROGRESS_STATUS_END,vUserData);
	}
	return bOk ;
}

INT64 MYAPI GetDiskFreeSpaceX(const char *szPathIn, INT64 * pnTotal )
{
    INT64 nFree = -1 ;
    INT64 nTotal = -1 ;
#ifdef  WIN32
    char szPath[4];
    ULARGE_INTEGER FreeBytesAvailableToCaller ;
    ULARGE_INTEGER TotalNumberOfBytes ;
    ULARGE_INTEGER TotalNumberOfFreeBytes;
    BOOL bOk = FALSE ;


    #if defined(_UNICODE) || defined(UNICODE)
        LPTSTR pszTmp=0;
        int nLenWideChar;

        if( ! szPathIn || ! szPathIn[0] )
        {
            goto LABEL_END ;
        }

        memset(szPath,0,sizeof(szPath));
        szPath[0] = szPathIn [0] ;
        szPath[1] = ':' ;

        nLenWideChar=MultiByteToWideChar(CP_ACP,MB_COMPOSITE,(char*)szPath,-1,0,0);
        pszTmp=alloca((nLenWideChar+1)*2);
        pszTmp[nLenWideChar]=0;
        MultiByteToWideChar(CP_ACP,MB_COMPOSITE,(char*)szPath,-1,pszTmp,nLenWideChar);
        bOk = GetDiskFreeSpaceEx(pszTmp,&FreeBytesAvailableToCaller,&TotalNumberOfBytes,&TotalNumberOfFreeBytes);
    #else
        if( ! szPathIn || ! szPathIn[0] )
        {
            goto LABEL_END ;
        }
        memset(szPath,0,sizeof(szPath));
        szPath[0] = szPathIn [0] ;
        szPath[1] = ':' ;
        bOk = GetDiskFreeSpaceEx(szPath,&FreeBytesAvailableToCaller,&TotalNumberOfBytes,&TotalNumberOfFreeBytes);
    #endif
        if( !bOk )
        {
            goto LABEL_END ;
        }
        nFree = (INT64) TotalNumberOfFreeBytes.QuadPart ;
        nTotal = (INT64) TotalNumberOfBytes.QuadPart ;
        goto LABEL_END ;

#elif  defined(__DARWIN_UNIX03) || defined(_FreeBSD_)
#else
    struct statfs  fs_buf;

    if( !szPathIn || !szPathIn[0] )
    {
        goto LABEL_END ;
    }

    if (statfs(szPathIn, &fs_buf) < 0)
    {
        goto LABEL_END ;
    }
    nFree = (INT64)fs_buf.f_bsize * ((INT64)fs_buf.f_bavail);
    nTotal = (INT64)fs_buf.f_bsize * ( (INT64)fs_buf.f_blocks );

#endif

LABEL_END:
    if( pnTotal )
    {
        *pnTotal = nTotal ;
    }
    return nFree ;
}

#ifndef WIN32

BOOL is_path_match_device(const char *path,const char *szDevPath)
{
	struct stat st;
	dev_t dev;

	if( path == NULL || path[0] == 0 || szDevPath == NULL || szDevPath[0] == 0 )
	{
		return FALSE ;
	}
	if ( stat(path, &st) )
	{
		return FALSE;
	}
	dev = (st.st_mode & S_IFMT) == S_IFBLK ? st.st_rdev : st.st_dev;

	if ( stat(szDevPath, &st) )
	{
		return FALSE;
	}

	if(  S_ISBLK(st.st_mode) &&  st.st_rdev == dev  )
	{
		return TRUE ;
	}

	return FALSE ;

}
BOOL find_block_device(const char *path,char *szDevPath,int nLen)
{
    DIR *dir;
    struct dirent *entry;
    struct stat st;
    dev_t dev;
    BOOL bOk = FALSE ;
    if( szDevPath )
    {
        szDevPath[0] = 0 ;
    }
    if (stat(path, &st) || !(dir = opendir("/dev")))
    {
            return FALSE;
    }
    dev = (st.st_mode & S_IFMT) == S_IFBLK ? st.st_rdev : st.st_dev;
    while ((entry = readdir(dir)) != NULL)
    {
            char devpath[PATH_MAX];
            sprintf(devpath,"/dev/%s", entry->d_name);
            if (!stat(devpath, &st) && S_ISBLK(st.st_mode) && st.st_rdev == dev)
            {
                    if( strcmpix(devpath , "/dev/root") == 0 )
                    {
                        continue;
                    }
                    strcpyn(szDevPath,devpath,nLen);
                    bOk = TRUE ;
                    break;
            }
    }
    closedir(dir);

    return bOk;
}

#endif
























































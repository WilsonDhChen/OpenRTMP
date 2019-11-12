

#include "adapi_def.h"

static const unsigned short __endian_test = 0x0001;
#define is_little_endian (*((const unsigned char*)&__endian_test))

char szADPI_ERROR[LEN_ERROR]="\0\0";



static __LogPrintFunc _logPrint = NULL ;

void MYAPI SetLogPrint( __LogPrintFunc func )
{
	_logPrint = func ;
}
int LogPrint(int prio, const char *tag,  const char *fmt, ...)
{
	int errno_ = 0;
	va_list args;
#ifdef WIN32
	DWORD dwLastError = 0 ;
	dwLastError = GetLastError();
#endif

	errno_ = errno ;
	va_start(args, fmt);
	if( _logPrint != NULL )
	{
		_logPrint(  prio,  tag, fmt, args );
	}
	va_end(args);
	errno = errno_ ;

#ifdef WIN32
	SetLastError(dwLastError);
#endif
	return 0 ;

}
int  VLogPrint(int prio, const char *tag,  const char *fmt, va_list ap )
{
    int errno_ = 0;
#ifdef WIN32
    DWORD dwLastError = 0 ;
    dwLastError = GetLastError();
#endif
    
    errno_ = errno ;
    
    if( _logPrint != NULL )
    {
        _logPrint(  prio,  tag, fmt, ap );
    }
    
    errno = errno_ ;
    
#ifdef WIN32
    SetLastError(dwLastError);
#endif
    return 0 ;
    
}
int MYAPI CpuCoreNumber()
{
    return 4;
}
#if defined(_MSC_VER) &&  defined(_WIN64)
#include <intrin.h> 
#endif
BOOL MYAPI CpuId(char *szId , int nLen)
{
    strcpyn(szId, "123456", nLen);
}

int MYAPI GetMachineId(char *szOut, int nSize, const char *szEth)
{
    strcpyn(szOut, "L123456AAAAAAAAAAAAA", nSize);
	return (int)strlen(szOut);
}



int MYAPI AlignLength( int Size ,int Align  )
{
	Align -- ;
	Size = (Size  + Align) & ~Align;
	return Size ;
}

BOOL MYAPI IsBigEndian()
{


	return !(is_little_endian);
}
static __inline UINT16 ArchSwap16(UINT16 D) 
{
	return((D<<8)|(D>>8));
}
static __inline UINT32 ArchSwap32(UINT32 D) 
{
	return((D<<24)|((D<<8)&0x00FF0000)|((D>>8)&0x0000FF00)|(D>>24));
}
static __inline UINT64 ArchSwap64(UINT64 val) {
	UINT32 hi, lo;

	/* Separate into high and low 32-bit values and swap them */
	lo = (UINT32)(val&0xFFFFFFFF);//lo保存原64位的低32位
	val >>= 32;
	hi = (UINT32)(val&0xFFFFFFFF);//hi保存原64位的高32位
	val = ArchSwap32(lo);//lo按照32位大小端进行转换，val保存新的lo
	val <<= 32;//新的lo移位为新的高32位
	val |= ArchSwap32(hi);//hi按照32位大小端进行转换成新的lo（新的低位），并跟val（新的高位）或运算
	return(val);
}
INT16 MYAPI htole_16( INT16 s16 )
{
	if( is_little_endian )
	{
		return s16 ;
	}
	return ArchSwap16( s16 );
}
INT32 MYAPI htole_32( INT32 s32 )
{
	if( is_little_endian )
	{
		return s32 ;
	}
	return ArchSwap32( s32 );
}
INT16 MYAPI letoh_16( INT16 s16 )
{
	if( is_little_endian )
	{
		return s16 ;
	}
	return ArchSwap16( s16 );
}
INT32 MYAPI letoh_32( INT32 s32 )
{
	if( is_little_endian )
	{
		return s32 ;
	}
	return ArchSwap32( s32 );
}
INT64 MYAPI letoh_64( INT64 s64 )
{
	if( is_little_endian )
	{
		return s64 ;
	}
	return ArchSwap64( s64 );
}
INT64 MYAPI htole_64( INT64 s64 )
{
	if( is_little_endian )
	{
		return s64 ;
	}
	return ArchSwap64( s64 );
}
char * MYAPI GetAdapiError()
{
 return szADPI_ERROR;
}
int MYAPI ClearAdapiError()
{
 szADPI_ERROR[0]=0;
 return 0;
}
int MYAPI OS_Type()
{
#if  defined(_WIN32_WCE)
	return _OS_TYPE_WINCE;
#elif defined(WIN32)
	return _OS_TYPE_WINDOWS;
#else
	return _OS_TYPE_UNIX;
#endif
}

#ifndef _NO_STDARG_
void MYAPI MyAdpiLogXX(const char *fmt, ...)
{
    va_list args;
    printf("Adapi log -----");
    va_start(args, fmt);
    vprintf(fmt,args);
    va_end(args);
}
int MYAPI LogFileX(const char *Path,const char *fmt, ...)
{
    FILE *pFile = NULL;
    va_list args;
#ifdef MYDEBUG_ADAPI
MyAdpiLogXX("LogFile\n");
#endif

    pFile=fopen(Path,"a");
    if(!pFile)
    {
    sprintf(szADPI_ERROR,"fopen <%s> fail",Path);
    return 0;
    }

    va_start(args, fmt);
    vfprintf(pFile,fmt,args);
    va_end(args);

    /////////////////////////
    fclose(pFile);
    return 1;
}

#endif
const char * MYAPI LogLevelStr(int nLevel)
{
	static char szLEVEL_FATAL[]="LEVEL_FATAL";
	static char szLEVEL_ERROR[]="LEVEL_ERROR";
	static char szLEVEL_INFO[]="LEVEL_INFO";
	static char szLEVEL_WARNING[]="LEVEL_WARNING";
	static char szLEVEL_DEBUG[]="LEVEL_DEBUG";
	static char szLEVEL_DETAIL[] = "LEVEL_DETAIL";
	static char szLEVEL_UNKNOWN[]="LEVEL_UNKNOWN";
	switch(nLevel)
	{
	case _LOG_LEVEL_FATAL:
		{
			return szLEVEL_FATAL;
		}
	case _LOG_LEVEL_ERROR:
		{
			return szLEVEL_ERROR;
		}
	case _LOG_LEVEL_INFO:
		{
			return szLEVEL_INFO;
		}
	case _LOG_LEVEL_WARNING:
		{
			return szLEVEL_WARNING;
		}
	case _LOG_LEVEL_DEBUG:
		{
			return szLEVEL_DEBUG;
		}
	case _LOG_LEVEL_DETAIL:
		{
			return szLEVEL_DETAIL;
		}
	}
	return szLEVEL_UNKNOWN;
}
const char *MYAPI FileNameFromPath(const char *szFilePathName)
{
	int nLen,i;
	if(!szFilePathName||!szFilePathName[0])
	{
            return 0;
	}
	nLen=strlenx(szFilePathName);
	i=nLen-1;
//	if(szFilePathName[i]=='\\'||szFilePathName[i]=='/')return szFilePathName+i+1;

	for(;i>=0;i--)
	{
		if(szFilePathName[i]=='/'||szFilePathName[i]=='\\')
		{
			return szFilePathName+i+1;
		}
	}
	return szFilePathName;

}
const char *MYAPI FileNameExt(const char *szFileName)
{
	int nLen,i;
	if(!szFileName||!szFileName[0])return 0;
	nLen=strlenx(szFileName);
	i=nLen-1;
	if(szFileName[i]=='\\'||szFileName[i]=='/')return szFileName+i+1;

	for(;i>0;i--)
	{
		if(szFileName[i]=='.')
		{
			return szFileName+i+1;
		}
	}
	return szFileName+nLen;

}
int MYAPI ChopPathFileNameW(const WCHAR *szPath)
{
    int i;
    WCHAR *inout=(WCHAR *)szPath;
    if(!inout)
    {
        return 0;
    }
    i=strlenxW(inout);
    if(inout[i-1]=='\\'||inout[i-1]=='/')return 1L;
    i--;
    for(;i>-1;i--)
    {
        if(inout[i]=='\\'||inout[i]=='/')
        {
            //    ::ShowString(inout);
            inout[i+1]=0;
            return 1L;
        }
    }
    return 1L;
}
int MYAPI ChopPathFileName(const char *szPath)
{
    int i;
	char *inout=(char *)szPath;
    if(!inout)return 0;
    i=strlenx(inout);
    if(inout[i-1]=='\\'||inout[i-1]=='/')return 1L;
    i--;
    for(;i>-1;i--)
    {
        if(inout[i]=='\\'||inout[i]=='/')
        {
        //    ::ShowString(inout);
            inout[i+1]=0;
            return 1L;
        }
    }
    return 1L;
}
char * MYAPI UpLevelPath(const char *szPath)
{
	int i;
	char *inout=(char *)szPath;
	if(!inout)return 0;
	i=strlenx(inout);
	if(inout[i-1]=='\\'||inout[i-1]=='/')
	{
		i--;
	}
	i--;
	for(;i>-1;i--)
	{
		if(inout[i]=='\\'||inout[i]=='/')
		{
			//    ::ShowString(inout);
			inout[i+1]=0;
			return inout;
		}
	}
	return inout;
}

int MYAPI PathAddSlash(const char *szDir)
{
	char *szPath=(char *)szDir;
	char *pszPath;
	char *szFolderPath;
	int iFolder=0,iPath=0,nLenPath;
	int nCpyed=0;
	int nLenFolder;
	static char szDIR_DIR[]="/\\";
	szFolderPath=szPath;
	if(!szFolderPath||!szFolderPath[0])return 0;
	nLenFolder=strlenx(szFolderPath);
	nLenPath=nLenFolder+10;
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
		for(;CharPosInStr(*szFolderPath,szDIR_DIR)>=0;szFolderPath++);


	}while(nCpyed>0);
	strcpyn(szPath,pszPath,nLenFolder+3);
	return iPath;

}

int MYAPI PathAddSlashEx(const char *szDir,int nOsType)
{
	char *szPath=(char *)szDir;
	char *pszPath;
	char *szFolderPath;
	int iFolder=0,iPath=0,nLenPath;
	int nCpyed=0;
	int nLenFolder;
	static char szDIR_DIR[]="/\\";
	szFolderPath=szPath;
	if(!szFolderPath||!szFolderPath[0])return 0;
	nLenFolder=strlenx(szFolderPath);
	nLenPath=nLenFolder+10;
	pszPath=(char *)alloca(nLenPath);
	if(!pszPath)return 0;

	if(szFolderPath[0]=='\\'||szFolderPath[0]=='/')
	{
		if(nOsType==_OS_TYPE_WINDOWS||nOsType==_OS_TYPE_WINCE||nOsType==_OS_TYPE_UNKNOWN)
		{
			pszPath[0]='\\';
		}
		else
		{
			pszPath[0]='/';
		}
		


		iPath=1;
		szFolderPath+=1;

		if(nOsType==_OS_TYPE_WINDOWS||nOsType==_OS_TYPE_WINCE||nOsType==_OS_TYPE_UNKNOWN)
		{
			if(szFolderPath[0]=='\\')
			{
				pszPath[0]='\\';
				pszPath[1]='\\';
				iPath=2;
				szFolderPath+=1;
			}
		}



	}

	for(;CharPosInStr(*szFolderPath,szDIR_DIR)>=0;szFolderPath++);
	do
	{
		nCpyed=strcpyn_stop(pszPath+iPath,szFolderPath,nLenPath-iPath,szDIR_DIR);
		szFolderPath+=nCpyed;
		if(nCpyed>0)
		{
			iPath+=nCpyed;
			if(nOsType==_OS_TYPE_WINDOWS||nOsType==_OS_TYPE_WINCE||nOsType==_OS_TYPE_UNKNOWN)
			{
				pszPath[iPath]='\\';
			}
			else
			{
				pszPath[iPath]='/';
			}
						


			iPath++;
			pszPath[iPath]=0;

		}
		for(;CharPosInStr(*szFolderPath,szDIR_DIR)>=0;szFolderPath++);


	}while(nCpyed>0);
	strcpyn(szPath,pszPath,nLenFolder+3);
	return iPath;

}

int MYAPI HttpReplyCode(const char *szHead)
{
	int i = 0;
	char szNumber[40];
	if( !szHead || !szHead[0] )
	{
		return 0;
	}
	if( strcmpipre(szHead,"HTTP/") != 0 )
	{
		return 0;
	}
	for( i=0; szHead[i] && szHead[i]!='\r' && szHead[i]!='\n'; i++)
	{
		if(szHead[i]==' ')
		{
			break;
		}
	}
	for( ; szHead[i]==' '; i++);
	strcpyn_number(szNumber,szHead+i,40);
	return atoix(szNumber);
}
int MYAPI GetIpPort(const char *szAddr,char *szIPOut,int nIpSize,int *nPortOut)
{
    int i;
    strcpyn_stop(szIPOut,szAddr,nIpSize,":;");
    for(i=0;*(szAddr+i)!=0;i++)
    {
        if(*(szAddr+i)==':'||*(szAddr+i)==';')
        {
            if(nPortOut)
            {
            *nPortOut=atol(szAddr+i+1);
            return 1;
            }
        }

    }
    if(nPortOut)
    {
    *nPortOut=0;
    }
    return 0;
}

int MYAPI RandomEx(int Start,int End)
{
#if  defined(_WIN32_WCE)
	FILETIME FileTime;
	SYSTEMTIME systm;
	GetSystemTime(&systm);
	SystemTimeToFileTime(&systm,&FileTime);
	srand( (unsigned)FileTime.dwLowDateTime );
#else
	srand( (unsigned)time( NULL ) );
#endif

    return Start+rand()%(End-Start);
}






typedef int ( MYAPI *_FuncCmp)(void *Value1,void *Value2);
int MYAPI BinSearchFromFile( FILE_SEARCH *pSC, _FileSearchCallBack BinFindCallBack )
{
    int    bFound=0;
    INT_FILE    High,Low,Mid;
    INT_FILE    pFileData=0,pIndData;
    int    bFindSig=0;
    INT_FILE    pFileDataPre=0;
    int    cmpV;
    INT_FILE    nFileSize;
    FILE_HANDLE    hFile;


    _FuncCmp    FuncCmp;

    FuncCmp=(_FuncCmp)pSC->FuncCmp;

    if(!pSC->FuncCmp)
    {
        strcpyx(szADPI_ERROR,"FuncCmp is zero");
        return 0;
    }
    if(pSC->DataWillFind==0)
    {
        strcpyx(szADPI_ERROR,"DataWillFind is zero");
        return 0;
    }
    if(pSC->szFileName==0)
    {
        strcpyx(szADPI_ERROR,"szFileName is zero");
        return 0;
    }
    High=pSC->nHigh;
    Low=pSC->nLow;

    ///////////////////////////////////////////////////
    if(High==0)
    {
        nFileSize=FileLength(pSC->szFileName);
        if(nFileSize%pSC->nBlockSize!=0)
        {
            strcpyx(szADPI_ERROR,"nBlockSize is wrong");
            return 0;
        }
        High=nFileSize/pSC->nBlockSize-1;
    }
    ///////////////////////////////////////////////////
    hFile=FileOpenXX(pSC->szFileName,OX_RDONLY);
    if(FILE_IO_ERR==hFile)
    {
        sprintf(szADPI_ERROR,"FileOpenXX <%s> fail",pSC->szFileName);
        return 0;
    }


    Mid=(High+Low)/2;


    while((!bFound)&&(High>=Low))
    {

        pFileData=Mid*(pSC->nBlockSize);
        if(FileSeekXX( hFile, pFileData+(pSC->nDataWillFindOffset), SEEK_SET)==-1L)
        {
            sprintf(szADPI_ERROR,"FileSeekXX <%s> fail",pSC->szFileName);
            goto CLOSE;
            return 0;
        }

        if(FileReadXX(hFile,pSC->ReadBuf,pSC->nReadBufSize)!=( int)pSC->nReadBufSize)
        {
            sprintf(szADPI_ERROR,"FileReadXX <%s> fail",pSC->szFileName);
            goto CLOSE;
            return 0;
        }


        cmpV=FuncCmp(pSC->ReadBuf,pSC->DataWillFind);


        if(cmpV==0)
        {

        //    printf("funcmp:%s\n",(char*)pSC->ReadBuf);

            pFileDataPre=pFileData;
            pIndData=pFileData;
            pSC->nDataFindedPointer=pFileData;

            pFileData-=(pSC->nBlockSize);
            if(FileSeekXX( hFile, pFileData+(pSC->nDataWillFindOffset), SEEK_SET)==-1L)
            {

                goto FIND;
                return 0;
            }
            if(FileReadXX(hFile,pSC->ReadBuf,pSC->nReadBufSize)!=( int)pSC->nReadBufSize)
            {

                goto FIND;
                return 0;
            }


            cmpV=FuncCmp(pSC->ReadBuf,pSC->DataWillFind);

            if(cmpV==0)
            {
                pFileData=pFileDataPre;
                High=Mid-1;
            //    printf("dsdsd\n");
                //continue;

            }
            else if(cmpV<0)
            {
                ////////////////????
            FIND:
                FileSeekXX( hFile, pFileDataPre+(pSC->nDataWillFindOffset), SEEK_SET);
                FileReadXX(hFile,pSC->ReadBuf,pSC->nReadBufSize);

                if(!BinFindCallBack(pSC))
                {
                    FileCloseXX( hFile );
                    return 1;
                }
                ////////////////??????
                pFileData=pFileDataPre;

                while(1)
                {

                pFileData+=(pSC->nBlockSize);

                if(FileSeekXX( hFile, pFileData+(pSC->nDataWillFindOffset), SEEK_SET)==-1L)
                {

                    FileCloseXX( hFile );
                    return 0;
                }
                if(FileReadXX(hFile,pSC->ReadBuf,pSC->nReadBufSize)!=(int)pSC->nReadBufSize)
                {

                    FileCloseXX( hFile );
                    return 0;
                }

                    cmpV=FuncCmp(pSC->ReadBuf,pSC->DataWillFind);

                    if(cmpV==0)
                    {

                        pSC->nDataFindedPointer=pFileData;

                        if(!BinFindCallBack(pSC))
                        {
                            FileCloseXX( hFile );
                            return 1;
                        }

                    }
                    else
                    {
                        FileCloseXX( hFile );
                        return 1;

                    }
            }
                ////////////////////////////
                FileCloseXX( hFile );
                return 1;
                ////////////////end

            }
            else
            {
                FileCloseXX( hFile );
                return 0;
            }
            ////////////////////////////////////////////////////////////////


        }
        else if(cmpV>0)
        {
            High=Mid-1;
        }
        else if(cmpV<0)
        {
            Low=Mid+1;
        }

        Mid=(High+Low)/2;

    }


CLOSE:
    FileCloseXX( hFile );

    return 0;
}

void MYAPI SleepMicro(unsigned int us)
{
#ifdef WIN32
	LARGE_INTEGER freq;
	LARGE_INTEGER start, end;
	LONGLONG count;
	QueryPerformanceFrequency(&freq);
	count = (us * freq.QuadPart) / (1000 * 1000);
	QueryPerformanceCounter(&start);
	count = count + start.QuadPart ;
	do
	{
	   QueryPerformanceCounter(&end);

	}while(end.QuadPart< count);
#else
    usleep(us);
#endif
}
void  MYAPI SleepMilli(unsigned int nTime)
{
#ifdef MYDEBUG_ADAPI
MyAdpiLogXX("SleepMilli\n");
#endif
#ifdef WIN32
    Sleep(nTime);
#else
    nTime*=1000;
    usleep(nTime);
#endif
    return ;
}


unsigned char MYAPI Swap4Bit(unsigned char cValue)
{
    unsigned char cRet=cValue;
    cRet=cRet<<4;
    cValue=cValue>>4;
    cRet=cRet|cValue;
    return cRet;
}
unsigned short int MYAPI SwapByte(unsigned short  nWord)
{
     unsigned short nRet=0;
    nRet=nWord<<8;
    nWord=nWord>>8;
    nRet=nRet|nWord;
    return nRet;
}
int MYAPI SwapByteOrder(void *pData,int nSize)
{
    unsigned short int *pWords;
    int i;
    if(!pData)return 0;
    pWords=(unsigned short int *)pData;
    if(nSize%2)nSize--;
    nSize/=2;
    for(i=0;i<nSize;i++)
    {
        pWords[i]=SwapByte(pWords[i]);
    }
    return 1;
}

#ifdef WIN32
static char _szErrorDLL[500] = {0};
#endif

DLL_HANDLE MYAPI DllOpenW(const wchar_t *szFile,int nOption)
{

#if defined(WIN32) || defined(_WIN32_WCE)

    DLL_HANDLE hDll = NULL ;
    if(!szFile||!szFile[0])
    {
        strcpyn( _szErrorDLL ,"empty file name",sizeof(_szErrorDLL));
        return (DLL_HANDLE)0;
    }
    hDll = LoadLibraryExW(szFile,NULL,nOption);

    if( hDll == NULL )
    {
        FormatMessageA( 
            FORMAT_MESSAGE_FROM_SYSTEM | 
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            GetLastError(),
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
            _szErrorDLL,
            sizeof(_szErrorDLL)-1,
            NULL 
            );
    }
    return  hDll ;

#else
    char szPath[400];
    UnicodeToUTF8( szFile , szPath , sizeof(szPath) );
    return DllOpen( szPath , nOption);
#endif



}

DLL_HANDLE MYAPI DllOpen(const char *szFile,int nOption)
{

#if defined(WIN32) || defined(_WIN32_WCE)

    DLL_HANDLE hDll = NULL ;
	#if defined(_UNICODE) || defined(UNICODE)
		LPTSTR pszTmp=0;
		int nLenWideChar;
		if(!szFile||!szFile[0])return (DLL_HANDLE)0;
		nLenWideChar=MultiByteToWideChar(CP_ACP,MB_COMPOSITE,(char*)szFile,-1,0,0);
		pszTmp=alloca((nLenWideChar+1)*2);
		if(!pszTmp)return (DLL_HANDLE)0;
		pszTmp[nLenWideChar]=0;
		MultiByteToWideChar(CP_ACP,MB_COMPOSITE,(char*)szFile,-1,pszTmp,nLenWideChar);
		hDll =  LoadLibraryEx(pszTmp,NULL,nOption);
	#else
		if(!szFile||!szFile[0])
        {
            strcpyn( _szErrorDLL ,"empty file name",sizeof(_szErrorDLL));
            return (DLL_HANDLE)0;
        }
		hDll = LoadLibraryEx(szFile,NULL,nOption);
	#endif
        if( hDll == NULL )
        {
            FormatMessageA( 
                FORMAT_MESSAGE_FROM_SYSTEM | 
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                GetLastError(),
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                _szErrorDLL,
                sizeof(_szErrorDLL)-1,
                NULL 
                );
        }
        return  hDll ;

#else

#ifndef _NO_DLOPEN_
	if(!szFile||!szFile[0])
    {
        return (DLL_HANDLE)0;
    }
	if(nOption==0)
    {
        nOption=RTLD_NOW|RTLD_LOCAL;
#ifdef RTLD_DEEPBIND
		nOption |= RTLD_DEEPBIND ;
#endif
    }
	return dlopen(szFile,nOption);
#else
	return NULL;
#endif

#endif


	
}
DLL_HANDLE MYAPI DllHandle(const char *szFile)
{
#if defined(WIN32) || defined(_WIN32_WCE)
	#if defined(_UNICODE) || defined(UNICODE)
		LPTSTR pszTmp=0;
		int nLenWideChar;
		if(!szFile||!szFile[0])return (DLL_HANDLE)0;
		nLenWideChar=MultiByteToWideChar(CP_ACP,MB_COMPOSITE,(char*)szFile,-1,0,0);
		pszTmp=alloca((nLenWideChar+1)*2);
		pszTmp[nLenWideChar]=0;
		MultiByteToWideChar(CP_ACP,MB_COMPOSITE,(char*)szFile,-1,pszTmp,nLenWideChar);
	#else
		if(!szFile||!szFile[0])return (DLL_HANDLE)0;
	#endif

	#if defined(_UNICODE) || defined(UNICODE)
			return (DLL_HANDLE)GetModuleHandle(pszTmp);
	#else
			return (DLL_HANDLE)GetModuleHandle(szFile);
	#endif

	
#else

#ifndef _NO_DLOPEN_
	if(!szFile||!szFile[0])return (DLL_HANDLE)0;
	#ifdef RTLD_NOLOAD
	return (DLL_HANDLE)dlopen(szFile,RTLD_NOLOAD);
	#else
	return (DLL_HANDLE)dlopen(szFile,RTLD_NOW);
	#endif
#else
	return 0;
#endif

#endif
}
int MYAPI  DllClose(DLL_HANDLE DlHandle)
{
	if(!DlHandle)return 0;
#if defined(WIN32) || defined(_WIN32_WCE)
	return FreeLibrary(DlHandle);
#else
#ifndef _NO_DLOPEN_
	return dlclose(DlHandle);
#else
	return 0;
#endif
#endif
}

void * MYAPI DllSymbol(DLL_HANDLE DlHandle,const char *szSymbol)
{
#if defined(WIN32)

	#if defined(_WIN32_WCE) 
		LPTSTR szSymbolWide;
		int nLenWideChar;
		if(!DlHandle||!szSymbol||!szSymbol[0])return 0;
		nLenWideChar=MultiByteToWideChar(CP_ACP,MB_COMPOSITE,(char*)szSymbol,-1,0,0);
		szSymbolWide=alloca((nLenWideChar+1)*2);
		if(!szSymbolWide)return 0;
		szSymbolWide[nLenWideChar]=0;
		MultiByteToWideChar(CP_ACP,MB_COMPOSITE,(char*)szSymbol,-1,szSymbolWide,nLenWideChar);
		return GetProcAddress(DlHandle,szSymbolWide);
	#else
		if(!DlHandle||!szSymbol||!szSymbol[0])return 0;
		return GetProcAddress(DlHandle,szSymbol);
	#endif

	
#else

#ifndef _NO_DLOPEN_
	if(!DlHandle||!szSymbol || !szSymbol[0]) 
    {
        return 0;
    }
	return dlsym(DlHandle,szSymbol);
#else
	return 0;
#endif

#endif
	
}
void * MYAPI DllSymbolEx(const char *szLibraryPath,const char *szSymbol)
{
	void *hLib=0;
	void * pSymbol=0;
	int IsLoadLib=0;

	hLib=DllHandle(szLibraryPath);
	if(!hLib)
	{
		hLib=DllOpen(szLibraryPath,0);
		IsLoadLib=1;
	}
	if(!hLib)
	{
		return 0;
	}
	pSymbol=DllSymbol(hLib,szSymbol);
	if(pSymbol)
	{
		return pSymbol;
	}

	if(IsLoadLib)
	{
		DllClose(hLib);
	}
	return 0;
}
char * MYAPI DllError()
{
	static char STRNULL[]="\0";
	
#if defined(WIN32) || defined(_WIN32_WCE)

	return _szErrorDLL;
#else
#ifndef _NO_DLOPEN_
	char *szError;
	szError=(char *)dlerror();
	if(szError==0)
	{
		return STRNULL;
	}
	else
	{
		return szError;
	}
#else
	return STRNULL;
#endif

#endif

}

int MYAPI ExeProg(const char *szApplacation,const char *szCmdLine)
{
#if defined(WIN32) || defined(_WIN32_WCE) 
	PROCESS_INFORMATION PsInfo;
	STARTUPINFO StartInfo;


	#if defined(_WIN32_WCE) || defined(_UNICODE) || defined(UNICODE)
		LPTSTR szApplacationWide=0;
		LPTSTR szCmdLineWide=0;
		int nLenWideChar=0;

		if(szApplacation&&szApplacation[0])
		{
			nLenWideChar=MultiByteToWideChar(CP_ACP,MB_COMPOSITE,(char*)szApplacation,-1,0,0);
			szApplacationWide=alloca((nLenWideChar+1)*2);
			if(!szApplacationWide)return 0;
			szApplacationWide[nLenWideChar]=0;
			MultiByteToWideChar(CP_ACP,MB_COMPOSITE,(char*)szApplacation,-1,szApplacationWide,nLenWideChar);
		}

		if(szCmdLine&&szCmdLine[0])
		{
			nLenWideChar=MultiByteToWideChar(CP_ACP,MB_COMPOSITE,(char*)szCmdLine,-1,0,0);
			szCmdLineWide=alloca((nLenWideChar+1)*2);
			if(!szCmdLineWide)return 0;
			szCmdLineWide[nLenWideChar]=0;
			MultiByteToWideChar(CP_ACP,MB_COMPOSITE,(char*)szCmdLine,-1,szCmdLineWide,nLenWideChar);
		}

		
		memset(&StartInfo,0,sizeof(StartInfo));
		StartInfo.wShowWindow=SW_SHOWNORMAL;
	//	#if defined(_WIN32_WCE) 
	//		return CreateProcess(szCmdLineWide,szCmdLineWide,0,0,0,0,0,0,0,&PsInfo);
	//	#else
			return CreateProcess(szApplacationWide,szCmdLineWide,0,0,0,0,0,0,&StartInfo,&PsInfo);
	//	#endif
		

	#else

		memset(&StartInfo,0,sizeof(StartInfo));
		return CreateProcess(szApplacation,(char *)szCmdLine,0,0,0,0,0,0,&StartInfo,&PsInfo);

	#endif
#else

	return execlp(szApplacation,szCmdLine ,(char *)NULL );
#endif

} 

int MYAPI wstrlenx(const wchar_t *szBuf)
{
	int i=0;
	if(!szBuf)
	{
		return 0;
	}
	for(i=0;szBuf[i];i++);
	return i;
}
int MYAPI MinInt(int a,int b)
{
	if(a<b)
	{
		return a;
	}
	else
	{
		return b;
	}
}
int MYAPI MaxInt(int a,int b)
{
	if(a>b)
	{
		return a;
	}
	else
	{
		return b;
	}
}





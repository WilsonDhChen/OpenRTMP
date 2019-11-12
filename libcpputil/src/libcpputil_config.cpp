
#include "libcpputil_def.h"
#include "libcpputil_config.h"






CObjConfig::CObjConfig()
{

	memset( m_szPathIniFile, 0 , sizeof(m_szPathIniFile)  );
	memset( m_szPathWorkDir, 0 , sizeof(m_szPathWorkDir)  );
	memset( m_szPathLogFile, 0 , sizeof(m_szPathLogFile)  );
	memset( m_szPathLogFileDir, 0 , sizeof(m_szPathLogFileDir)  );
	memset( m_szPathLogFileDefault, 0 , sizeof(m_szPathLogFileDefault)  );
	memset( m_szPathKeyFile, 0 , sizeof(m_szPathKeyFile)  );
	memset( m_szBoottime, 0 , sizeof(m_szBoottime)  );
	memset( m_szAppName, 0 , sizeof(m_szAppName)  );
	memset( m_szCompanyName, 0 , sizeof(m_szCompanyName)  );
	

	m_nLogLevel = _LOG_LEVEL_DETAIL ;
	m_IsLogConsole = TRUE ;
	m_bKeepLogfile = TRUE ;
	m_nMaxLogFileSize = SIZE_1M ;

	m_bInited = FALSE ;
	MtxInit(&m_mtxLog,0);

}
CObjConfig::~CObjConfig()
{
	MtxDestroy( &m_mtxLog );
}
BOOL CObjConfig::Init(int argc,char *argv[],void *hInsWin32)
{
	char szTmpBuf[400];
	BOOL bFoundD = FALSE ;
        int iD = 0 ;
	
	if( m_bInited )
	{
		return TRUE;
	}
    
	if(argc>1 && argv)
	{

        for( iD = 1; iD < argc; iD++ )
        {
            if( strcmpipre( argv[ iD ], "-d" ) == 0 )
            {
                bFoundD = TRUE ;
                break ;
            }
        }
        if( bFoundD )
		{
            strcpyn( m_szPathWorkDir, argv[ iD ] + 2, LEN_MAX_PATH );
			if(m_szPathWorkDir[0]==0)
			{
				goto LABEL_DEFAULT_PATH;
			}
			PathAddSlash( m_szPathWorkDir);
			MakeDirRecur(m_szPathWorkDir, NULL);
		}
		else
		{
LABEL_DEFAULT_PATH:

#if defined(WIN32) || defined(_Linux_)|| defined(__DARWIN_UNIX03)|| defined(_Darwin_) || defined(_EmbedSky_)   

			GetCurrentAppDir(m_szPathWorkDir, LEN_MAX_PATH - 20);
#else
			if( m_szCompanyName[0] == 0 )
			{
				sprintf(m_szPathWorkDir,"/opt/%s/",m_szAppName);
			}
			else
			{
				sprintf(m_szPathWorkDir,"/opt/%s/%s/",m_szCompanyName,m_szAppName);
			}
			
			MakeDirRecur(m_szPathWorkDir, NULL);
#endif

		}
	}
	else
	{
		goto LABEL_DEFAULT_PATH;
	}
	chdir(m_szPathWorkDir);
	strcpyn(m_szPathKeyFile,m_szPathWorkDir,LEN_MAX_PATH);


	sprintf(m_szPathKeyFile,"%s%s.key",m_szPathWorkDir,m_szAppName);


	strcpyn(m_szPathLogFileDefault,m_szPathWorkDir,LEN_MAX_PATH);

	if( m_szPathIniFile[0] == 0 )
	{
		strcpyn(m_szPathIniFile,m_szPathWorkDir,LEN_MAX_PATH);
		sprintf(m_szPathIniFile,"%s%s.ini",m_szPathWorkDir,m_szAppName);

	}

	sprintf(m_szPathLogFileDefault,"%s%s.log",m_szPathWorkDir,m_szAppName);


	static const char RC_INI_VAR_LOG_FILE[]="log_file";
	static const char RC_INI_VAR_MAX_LOGFILE_SIZE[]="log_file_size";
	static const char RC_INI_VAR_LOG_LEVEL[]="log_level";
	static const char RC_INI_VAR_IS_LOG_CONSOLE[]="log_console";

	if(IniReadStr(m_szAppName,RC_INI_VAR_LOG_FILE,m_szPathLogFile,LEN_MAX_PATH,m_szPathIniFile)<1)
	{
		strcpyn(m_szPathLogFile,m_szPathLogFileDefault,LEN_MAX_PATH);
	}
	if( FileNameFromPath(m_szPathLogFile)[0] == 0 )
	{
		strcatn(m_szPathLogFile,m_szAppName,LEN_MAX_PATH);
		strcatn(m_szPathLogFile,".log",LEN_MAX_PATH);
	}
#ifdef WIN32
	if( ! IsHaveDriveLabel(m_szPathLogFile)  )
#else
	if( m_szPathLogFile[0] != '/' &&  m_szPathLogFile[0] != '\\'  )
#endif
	{
		strcpyn(szTmpBuf,m_szPathLogFile,_SIZE_OF(szTmpBuf));
		strcpyn(m_szPathLogFile,m_szPathWorkDir,_SIZE_OF(m_szPathLogFile));
		strcatn(m_szPathLogFile,szTmpBuf,_SIZE_OF(m_szPathLogFile));

	}
	strcpyn( m_szPathLogFileDir, m_szPathLogFile, LEN_MAX_PATH );
	ChopPathFileName(m_szPathLogFileDir);
	PathAddSlash(m_szPathLogFileDir);
	MakeDirRecur(m_szPathLogFileDir, NULL);

	szTmpBuf[0];
	if (IniReadStr(m_szAppName, RC_INI_VAR_LOG_LEVEL, szTmpBuf, _SIZE_OF(szTmpBuf), m_szPathIniFile) < 1)
	{
		m_nLogLevel = _LOG_LEVEL_INFO;
	}
	else
	{
		if (strcmpix(szTmpBuf, "FATAL") == 0 || strcmpix(szTmpBuf, "fat") == 0)
		{
			m_nLogLevel =  _LOG_LEVEL_FATAL;
		}
		else if (strcmpix(szTmpBuf, "ERROR") == 0 || strcmpix(szTmpBuf, "err") == 0 )
		{
			m_nLogLevel = _LOG_LEVEL_ERROR;
		}
		else if (strcmpix(szTmpBuf, "WARNING") == 0 || strcmpix(szTmpBuf, "war") == 0)
		{
			m_nLogLevel = _LOG_LEVEL_WARNING;
		}
		else if (strcmpix(szTmpBuf, "INFO") == 0 || strcmpix(szTmpBuf, "inf") == 0)
		{
			m_nLogLevel = _LOG_LEVEL_INFO;
		}
		else if (strcmpix(szTmpBuf, "DEBUG") == 0 || strcmpix(szTmpBuf, "dbg") == 0)
		{
			m_nLogLevel = _LOG_LEVEL_DEBUG;
		}
		else if (strcmpix(szTmpBuf, "DETAIL") == 0 || strcmpix(szTmpBuf, "det") == 0)
		{
			m_nLogLevel = _LOG_LEVEL_DETAIL;
		}
		else
		{
			m_nLogLevel = atoix(szTmpBuf);
		}
	}

	m_IsLogConsole=IniReadBool(m_szAppName,RC_INI_VAR_IS_LOG_CONSOLE,m_szPathIniFile,TRUE);
	m_nMaxLogFileSize=IniReadInt(m_szAppName,RC_INI_VAR_MAX_LOGFILE_SIZE,m_szPathIniFile,0);


	if( m_nMaxLogFileSize < 10 )
	{
		m_nMaxLogFileSize = SIZE_1M ;
	}
	else
	{
		m_nMaxLogFileSize = m_nMaxLogFileSize * 1024 ;
	}



	DateTimeNowStr(m_szBoottime,FALSE,TRUE);


	m_bInited = TRUE ;
	return TRUE;
}
BOOL CObjConfig::IsHaveDriveLabel(const char *szPath)
{
#if defined(WIN32)

#if defined(WIN32_WCE)
	return FALSE ;
#endif

	if( !szPath || !szPath[0] )
	{
		return FALSE ;
	}
	if(
		((szPath[0] >= 'A' && szPath[0] <= 'Z')
		||(szPath[0] >= 'a' && szPath[0] <= 'z')
		) && szPath[1] == ':'
		)
	{
		return TRUE ;
	}
#endif

	return FALSE ;
}
void CObjConfig::SetAppName(const char *name)
{
	strcpyn( m_szAppName, name, sizeof(m_szAppName) );
}
void CObjConfig::SetCompanyName(const char *name)
{
	strcpyn( m_szCompanyName, name, sizeof(m_szCompanyName) );
}
void CObjConfig::Log(int nLevel,const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	VLog( nLevel, fmt, args );
	va_end(args);

}
void CObjConfig::VLog(int nLevel,const char *fmt, va_list args)
{
	FILE *pFile;
	INT_THREADID  threadId;
	char szTime[24];
	char szLogBak[LEN_MAX_PATH];
	threadId = Thread2GetIdNumber();
	if(nLevel != _LOG_LEVEL_FATAL && nLevel>m_nLogLevel)
	{
		return ;
	}

	MtxLock( &m_mtxLog );


	if(  FileLength(m_szPathLogFile) > m_nMaxLogFileSize )
	{
		if( m_bKeepLogfile )
		{
			DateTimeNowStr(szTime,FALSE,FALSE);
			strcpyn( szLogBak, m_szPathLogFileDir, LEN_MAX_PATH );
			strcatn( szLogBak, m_szAppName, LEN_MAX_PATH );
			strcatn( szLogBak, ".", LEN_MAX_PATH );
			strcatn( szLogBak, szTime, LEN_MAX_PATH );
			strcatn( szLogBak, ".log", LEN_MAX_PATH );
			RemoveFileX(szLogBak);
			FileMoveX(m_szPathLogFile,szLogBak);
		}

		RemoveFileX(m_szPathLogFile);
	}
	DateTimeNowStr(szTime,FALSE,TRUE);

	if(m_IsLogConsole)
	{

		printf("%s,%u,%s,",szTime,(unsigned int)threadId,LogLevelStr(nLevel));
#ifdef WIN32
		vprintf(fmt,args);
#else
		va_list arg;
		va_copy( arg ,args );
		vprintf(fmt,arg);
		va_end(arg);
#endif



	}

	pFile=fopen(m_szPathLogFile,"a");
	if(!pFile)
	{
		MakeDirRecur(m_szPathLogFileDir, NULL);
		pFile=fopen(m_szPathLogFile,"a");
	}
	if(!pFile)
	{
		MtxUnLock( &m_mtxLog );
		return  ;
	}

	fprintf(pFile,"%s,%08u,%s,",szTime,(unsigned int)threadId,LogLevelStr(nLevel));
	vfprintf(pFile,fmt,args);


	/////////////////////////
	fclose(pFile);

	MtxUnLock( &m_mtxLog );

	if( nLevel == _LOG_LEVEL_FATAL )
	{
		while( TRUE )
		{
			SleepMilli(1000);
		}
	} 

	return ;
}
DLL_HANDLE CObjConfig::LoadPlugin( const char * dlPath , const char *szName)
{
    DLL_HANDLE hIns = NULL;
    const char *extName = FileNameExt( dlPath );
	char szDll[100] = {0};
    int osType = OS_Type( );

    if( dlPath == NULL || dlPath[ 0 ] == 0 )
    {
		LogPrint(_LOG_LEVEL_INFO, m_szAppName, "[%s] load plugin empty\n", szName);
        return NULL ;
    }

  //  memset( szDll, 0, sizeof( szDll ) );
  //  snprintf( szDll, sizeof( szDll ), "/opt/%s/", m_szAppName );

    hIns = DllOpen( dlPath, 0 );
    if( hIns == NULL )
    {
        if( extName == NULL || ( *extName ) == 0 )
        {
            char *exts[ ] = { ".dll", ".so", ".dylib", NULL };
            int  i = 0;
            for( i = 0; exts[ i ] != NULL; i++ )
            {
                char szTmp[ 500 ];
                if( osType == _OS_TYPE_WINDOWS || osType == _OS_TYPE_WINCE )
                {
                    strcpyn( szTmp, dlPath, sizeof( szTmp ) );
                    strcatn( szTmp, exts[ i ], sizeof( szTmp ) );
                }
                else
                {
                    strcpy( szTmp, szDll );
                    strcatn( szTmp, FileNameFromPath( dlPath ), sizeof( szTmp ) );
                    strcatn( szTmp, exts[ i ], sizeof( szTmp ) );

                }


				LogPrint(_LOG_LEVEL_INFO, m_szAppName, "tring to load <%s>\n", szTmp);
                hIns = DllOpen( szTmp, 0 );
                if( hIns != NULL )
                {
                    break;
                }
				else
				{
					LogPrint(_LOG_LEVEL_ERROR, m_szAppName, "failed to load <%s>\n", szTmp);
				}


            }

        }
        else
        {
            if( osType != _OS_TYPE_WINDOWS && osType != _OS_TYPE_WINCE )
            {
                char szTmp[ 500 ];
                strcpy( szTmp, szDll );
                strcatn( szTmp, FileNameFromPath( dlPath ), sizeof( szTmp ) );
				LogPrint(_LOG_LEVEL_INFO, m_szAppName, "tring to load <%s>\n", szTmp);
                hIns = DllOpen( szTmp, 0 );
				if (hIns == NULL)
				{
					LogPrint(_LOG_LEVEL_ERROR, m_szAppName, "failed to load <%s>\n", szTmp);
				}
            }

        }

    }

    if( hIns == NULL )
    {
        LogPrint( _LOG_LEVEL_ERROR, m_szAppName, "load plugin error <%s> (%s)\n", dlPath, DllError( ) );
       
    }

    return hIns;
}


































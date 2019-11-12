
#include"adapi_def.h"

static char MoveToEndLine(FILE_HANDLE hFile,char *stopCharList)
{
    char c;
    const char defaultStop[]="\r\n";
    const char *pStop=0; 
    if(!stopCharList||!stopCharList[0])
    {
        pStop=defaultStop;
    }
    else
    {
        pStop=stopCharList;
    }
    while((c=FileGetChar(hFile,0))!=(char)EOF&&CharPosInStr(c,pStop)<0);
	if(c==(char)EOF)
	{
		return (char)EOF;
	}	
	else return 0;
} 

static int  ReadLine(FILE_HANDLE hFile,char *outLine,int size,const char *stopCharList,char  *pisEndLine ,char *pisEOF)
{
    int i=0;
    char c=0;
    const char defaultStop[]="\r\n";
    const char *pStop=0; 
    char isEndLine=0;
    size--;
    if(!stopCharList||!stopCharList[0])
    {
        pStop=defaultStop;
    }
    else
    {
        pStop=stopCharList;
    }
    
    while(i<size&&(c=FileGetChar(hFile,0))!=(char)EOF)
    {
        if(CharPosInStr(c,pStop)>=0)
        {
          isEndLine=1;
          break;
        }
        *(outLine+i)=c;
        i++;
    }
    outLine[i]=0;
    
    if(c==(char)EOF)
	{
		if(pisEOF)
        {
            *pisEOF=1;
        }
		isEndLine=1;
	}
	else
	{
		if(pisEOF)*pisEOF=0;
	}
	if(pisEndLine)*pisEndLine=isEndLine;
    
    /*
    if(!isEndLine)
    {
        while((c=FileGetChar(hFile))!=EOF&&CharPosInStr(c,pStop)<0);
    }
    */
    return i;
}
BOOL MYAPI IniReadBool(const char *section,const char *var,const char * iniFile,int nDefaultValue)
{
	char szValue[40];
	int nRet;
	nRet=IniReadStr(section,var,szValue,40,iniFile);
	if(szValue[0]==0)
	{
		return nDefaultValue;
	}
	return Str2Bool(szValue);
}
double MYAPI IniReadFloat(const char *section,const char *var,const char * iniFile,double nDefaultValue)
{
	char szValue[40];
	int nRet;
	nRet=IniReadStr(section,var,szValue,40,iniFile);
	//strcpyn_number(szValue,szValue,40);
	if(szValue[0]==0)
	{
		return nDefaultValue;
	}
	return atof(szValue);
}
int MYAPI IniReadInt(const char *section,const char *var,const char * iniFile,int nDefaultValue)
{
        char szValue[40]={0};
	int nRet;
	//BOOL bNegative = 0;
	nRet=IniReadStr(section,var,szValue,40,iniFile);
	//strcpyn_number(szValue,szValue,40);
	if(szValue[0]==0)
	{
		return nDefaultValue;
	}
	return atoi(szValue);
}
int MYAPI IniReadIndex(const char *section,int nIndex,char *outValue,int size,const char * iniFile)
{
	char szVar[40];
	snprintf(szVar,39,"%d",nIndex);
	return IniReadStr(section,szVar,outValue,size,iniFile);
}
int MYAPI IniWriteMapValue(const char *section,const char *var,
                          char **pMapStr,int nCount,const int *pMapValue,int nValue,
                          const char * iniFile)
{
    int i = 0 ;
    const char *pszValue = NULL ;
    if( !pMapStr  || !pMapValue || nCount < 1)
    {
        return FALSE;
    }
    for( i = 0; i < nCount ; i++ )
    {
        if( pMapValue[i] == nValue )
        {
           pszValue = pMapStr[i] ;
           break;
        }
    }

    if( !pszValue )
    {
        pszValue =pMapStr [0];
    }
    return IniWrite(section,var,pszValue,iniFile);

}
int MYAPI IniReadMapValue(const char *section,const char *var,
                          char **pMapStr,int nCount,const int *pMapValue,
                          const char * iniFile)
{
    char szTmp[100];
    int nRet =0 ;
    int i = 0 ;
    nRet = IniReadStr(section,var,szTmp,_SIZE_OF(szTmp),iniFile);
    if( nRet < 1 || !pMapStr  || nCount<1)
    {
        return 0 ;
    }
    for( i = 0 ; i < nCount; i ++ )
    {   
            if( strcmpix( pMapStr[i] , szTmp ) == 0 )
            {
                    if( pMapValue )
                    {
                        return pMapValue[i];
                    }
                    else
                    {
			            return i;
                    }
            }
    }
    if( pMapValue )
    {
        return pMapValue[0];
    }
    else
    {
        return 0;
    }
}

int MYAPI IniReadStr(const char *section,const char *var,char *outValue,int size,const char * iniFile)
{
	int nRet;
	nRet=IniRead(section,var,outValue,size,iniFile);
	StrTrimX(outValue);
	if(nRet>=0)
	{
		return strlenx(outValue);
	}
	return nRet;
}


int MYAPI IniRead(const char *section,const char *var,char *outValue,int size,const char * iniFile)
{
    int nRet=INI_ERROR_VARNOTFIND;
    FILE_HANDLE hFile;
    char *pBuf=0;
	int  IsHeap = 0;
    int nBufLen=0;
    int nReaded=0;
    char isEndline=1;
	char isEOF=0;
    char c;
	int i = 0;
	int nLenSec,nLenVar;
	if(outValue)*outValue=0;
    hFile=FileOpenXX(iniFile,OX_RDONLY);
    if(hFile==FILE_HANDLE_INVALID)
    {
        return INI_ERROR_FILEOPEN;
    }
    if(!var||!var[0]||!outValue)goto MYEND;
	nLenSec = strlenx(section);
	nLenVar = strlenx(var);
	nBufLen = MaxInt(nLenSec,nLenVar)+4;
	pBuf = (char *)alloca(nBufLen+4);
	if( !pBuf )
	{
		pBuf=(char *)_MEM_ALLOC(nBufLen+4);
		IsHeap = 1;
	}
	if(pBuf==0) goto MYEND;
    if(section&&section[0])//find section
    {
        
        while(!isEOF)
        {
            while(CharPosInStr(c=FileGetChar(hFile,0)," \t\r\n")>=0)
            {
                if( c==(char)EOF )
                {
                    goto MYEND;
                }
            }
			if ((unsigned char)c == 0xEF ||
				(unsigned char)c == 0xBB ||
				(unsigned char)c == 0xBF) //skip BOM(Byte Order Mark)
			{
				continue;
			}

            if(c!='[')
            {
				if(CharPosInStr(c,"\r\n")<0)
				{
					if(MoveToEndLine(hFile,0)==(char)EOF)
					{
						nRet = INI_ERROR_SECTION_NOTFIND ;
						goto MYEND;
					}
					
				}
                continue;
            }
			pBuf[0] = '[';
            nReaded=ReadLine(hFile,pBuf+1,nLenSec+2,0,&isEndline,&isEOF)+1;

			
            if(nReaded==(nLenSec+2))
            {
                if(pBuf[nReaded-1]!=']'||*pBuf!='[') continue;
                pBuf[nReaded-1]=0;
                if(strcmpix(pBuf+1,section)==0)
                {
                    if(!isEndline){isEOF=(MoveToEndLine(hFile,0)==(char)EOF);}
                    goto START_FIND_VAR;
                   
                }
            }
			
            if(!isEndline){isEOF=(MoveToEndLine(hFile,0)==(char)EOF);}
           
        }
        nRet = INI_ERROR_SECTION_NOTFIND ;
        goto MYEND;
    }
START_FIND_VAR:

 
    
    while(!isEOF)
    {
        while(CharPosInStr(c=FileGetChar(hFile,0)," \t\r\n")>=0);
        if(c=='['&&section&&section[0])
        {
            nRet=-1;
            break;
        }     
        
         
        if(CharPosInStr(c,"\r\n")>=0)continue;            
        *pBuf=c;
        nReaded=ReadLine(hFile,pBuf+1,nLenVar,0,&isEndline,&isEOF)+1;
       
       

       if( nReaded == nLenVar && strcmpix(pBuf,var) == 0 )
       {
             nRet=0;
             while(CharPosInStr(c=FileGetChar(hFile,0)," \t")>=0)nRet++;
             if(c=='=')
             {
                 nRet=0;
                 while(CharPosInStr(c=FileGetChar(hFile,0)," \t")>=0);
                 if(c=='\r'||c=='\n'||c==(char)EOF)goto MYEND;
                 outValue[0]=c;
                 nRet=ReadLine(hFile,outValue+1,size-1,0,0,&isEOF)+1;
                 break;
                 
             }
             else if(nRet>0)
             {
                 if(CharPosInStr(c,"\r\n")<0)
                 {
                     nRet=0;
                     outValue[0]=c;
                     nRet=ReadLine(hFile,outValue+1,size-1,0,0,&isEOF)+1;
                     break;
                    
                 }
             }
             else if(CharPosInStr(c,"\r\n")>=0)
             {
                  nRet=0;
                  break;
                  
             }
             else
             {
                 nRet=INI_ERROR_VARNOTFIND;
                 
             }              
       }

       
	   if(!isEndline){isEOF=(MoveToEndLine(hFile,0)==(char)EOF);}
    }
  
MYEND: 
	if( IsHeap )
	{
		_MEM_FREEX(&pBuf);
	}
    
    FileCloseXX(hFile); 
    return nRet;
}



#define INI_SECTION_EXIT	10
int MYAPI IniWriteIndex(const char *section,int nIndex,const char *szValue,const char * iniFile)
{
	char szVar[40];
	snprintf(szVar,39,"%d",nIndex);
	return IniWrite(section,szVar,szValue,iniFile);
}
int MYAPI IniWriteInt(const char *section,const char *var,int nValue,const char * iniFile)
{
        char szTmp[40];
        szTmp[39] = 0 ;
        snprintf(szTmp,38,"%d",nValue);
        return IniWrite(section,var,szTmp,iniFile);
}
int MYAPI IniWrite(const char *section,const char *var,const char *szValue,const char * iniFile)
{
    int nRet=-1;
    FILE_HANDLE hFile;
    char *pBuf=0;
    char *pBufTmp=0;
	char *pBufBefore=0;
	char *pBufAfter=0;
    int nBufLen=0;
    int nReaded=0;
    char isEndline=1;
	int nAddrStart=-1;
	int nAddrEnd=-1;
	int nAddrSection=-1;
	int nFileLength;
	char isEOF=0;
    char c;
	int nWriteLen;
    int i = 0;
	int nLenSec, nLenVar,nLenValue;
	char szNULL[]="\0";
    if(!var||!var[0])return 0;
	if(szValue==0)szValue=szNULL;
	nLenSec = strlenx(section);
	nLenVar = strlenx(var);
	nLenValue = strlenx(szValue);
	nWriteLen=nLenSec + nLenVar + nLenValue + 10;
    hFile=FileOpenXX2(iniFile,OX_RDWR|OX_CREAT);
    if(hFile==FILE_HANDLE_INVALID)
    {
		return INI_ERROR_FILEOPEN;

    }
	nFileLength=(int)FileSeekXX(hFile,0,SEEK_END);
	FileSeekXX(hFile,0,SEEK_SET);
    if(section&&section[0])//find section
    {
        nBufLen=strlenx(section);
        pBuf=(char *)_MEM_ALLOC(nBufLen+4);
        if(pBuf==0) goto MYEND;
        
        while(!isEOF)
        {
            while(CharPosInStr(c=FileGetChar(hFile,0)," \t\r\n")>=0);
			if(c==(char)EOF)break;
            if(c!='[')
            {
				if(CharPosInStr(c,"\r\n")<0){isEOF=(MoveToEndLine(hFile,0)==(char)EOF);}
				if(isEOF)break;
                continue;
            }
            *pBuf='[';
            nReaded=ReadLine(hFile,pBuf+1,nBufLen+2,0,&isEndline,&isEOF)+1;
            
            
            if(nReaded==(nBufLen+2))
            {
                if(pBuf[nReaded-1]!=']'||*pBuf!='[') continue;
                pBuf[nReaded-1]=0;
                if(strcmpix(pBuf+1,section)==0)
                {
                    if(!isEndline){isEOF=(MoveToEndLine(hFile,0)==(char)EOF);}
                    goto START_FIND_VAR;
                   
                }
            }
            if(!isEndline){isEOF=(MoveToEndLine(hFile,0)==(char)EOF);}
           
        }
		_MEM_FREEX(&pBuf);
        goto DIRECT_WRITE;
    }
START_FIND_VAR:

    _MEM_FREEX(&pBuf);
    
    nBufLen=strlenx(var);
    pBuf=(char *)_MEM_ALLOC(nBufLen+1);
    if(pBuf==0) goto MYEND;
    
    while(1)
    {
		
		nAddrStart=(int)FileSeekXX(hFile,0,SEEK_CUR);
		if(isEOF)break;
        while(CharPosInStr(c=FileGetChar(hFile,0)," \t")>=0);
        if(c=='['&&section&&section[0])
        {
			nAddrSection=nAddrStart;
            nRet=INI_SECTION_EXIT;
            break;
        }     
        
         
        if(CharPosInStr(c,"\r\n")>=0)continue; 
        *pBuf=c;
        nReaded=ReadLine(hFile,pBuf+1,nBufLen,0,&isEndline,&isEOF)+1;
       
       
       if(nReaded==nBufLen)
       {
           if(strcmpix(pBuf,var)==0)
           {
                 nRet=0;
                 while(CharPosInStr(c=FileGetChar(hFile,0)," \t")>=0)nRet++;
                 if(c=='=')
                 {
                     if(MoveToEndLine(hFile,0)!=(char)EOF)
					 {
						while(CharPosInStr(c=FileGetChar(hFile,0),"\r\n \t")>=0);
						
						nAddrEnd=(int)FileSeekXX(hFile,0,SEEK_CUR);
						if(c!=(char)EOF)nAddrEnd--;
					 }
					 else
					 {
						 nAddrEnd=(int)FileSeekXX(hFile,0,SEEK_CUR);
					 }
                     break;                    
                 }
                 else if(nRet>0)
                 {
                     if(MoveToEndLine(hFile,0)!=(char)EOF)
					 {
						while(CharPosInStr(c=FileGetChar(hFile,0),"\r\n \t")>=0);
						nAddrEnd=(int)FileSeekXX(hFile,0,SEEK_CUR);
						if(c!=(char)EOF)nAddrEnd--;
					 }
					 else
					 {
						 nAddrEnd=(int)FileSeekXX(hFile,0,SEEK_CUR);
					 }
					 break;
                 }
                 else if(CharPosInStr(c,"\r\n")>=0)
                 {
					 FileSeekXX(hFile,FileSeekXX(hFile,0,SEEK_CUR)-1,SEEK_SET);
					 if(MoveToEndLine(hFile,0)!=(char)EOF)
					 {
						while(CharPosInStr(c=FileGetChar(hFile,0),"\r\n \t")>=0);
						nAddrEnd=(int)FileSeekXX(hFile,0,SEEK_CUR);
						if(c!=(char)EOF)nAddrEnd--;
					 }
					 else
					 {
						 nAddrEnd=(int)FileSeekXX(hFile,0,SEEK_CUR);
					 }
					 break;       
                 }
                 else
                 {
                     continue; 
                     
                 }              
           }
       }
       
       
       if(!isEndline)isEOF=(MoveToEndLine(hFile,0)==(char)EOF);
    }
	_MEM_FREEX(&pBuf);
	if(isEOF)
	{
		section=0;
		goto DIRECT_WRITE;
	}
	else if(nRet==INI_SECTION_EXIT)
	{
		nAddrEnd=nAddrSection;
	}
	pBuf=(char *)_MEM_ALLOC(nWriteLen);
	pBufBefore=(char *)_MEM_ALLOC(nAddrStart+10);
	if(nAddrEnd>nFileLength)nAddrEnd=nFileLength;
	pBufAfter=(char *)_MEM_ALLOC(nFileLength-nAddrEnd+10);
	if(pBuf==0||pBufBefore==0||pBufAfter==0)
	{
		nRet=0;
		goto MYEND;
	}

	sprintf(pBuf,"%s=%s",var,szValue);

	FileSeekXX(hFile,0,SEEK_SET);
	nAddrStart=FileReadXX(hFile,pBufBefore,nAddrStart);
	pBufBefore[nAddrStart]=0;
	FileSeekXX(hFile,nAddrEnd,SEEK_SET);
	nAddrEnd=FileReadXX(hFile,pBufAfter,nFileLength-nAddrEnd+1);
	pBufAfter[nAddrEnd]=0;
	//printf("%s",pBufBefore);
	//printf("%s",pBuf);
	//printf("%s",pBufAfter);
	FileCloseXX(hFile); 
    hFile=FileOpenXX(iniFile,OX_RDWR|OX_CREAT|OX_TRUNC);
    if(hFile==FILE_HANDLE_INVALID)
    {
		_MEM_FREEX(&pBuf);
		_MEM_FREEX(&pBufBefore);
		_MEM_FREEX(&pBufAfter);
		return 0;
	}
    i = nAddrStart-1;
    for( ; i > 0 && (pBufBefore[i] == '\r' || pBufBefore[i] == '\n'|| pBufBefore[i] == '\t') ; i -- )
    {
        pBufBefore[i] = 0 ;
        nAddrStart--;
    }
    pBufTmp = pBufAfter ;
    while( ( *pBufTmp == '\r' ||  *pBufTmp == '\n'||  *pBufTmp == '\t')  )
    {
        pBufTmp ++ ;
        nAddrEnd--;
    }
    /*
    i = 0 ;
    for( i=0; pBuf[i]; i++ )
    {
        if( pBuf[i] == '\r' ||  pBuf[i] == '\n'||  pBuf[i] == '\t') 
        {
            pBuf[i] = ' ';
        }
    }
    */
	FileWriteXX(hFile,pBufBefore,nAddrStart);
    FileWriteXX(hFile,"\n",1);
        nWriteLen = strlenx(pBuf) ;
        FileWriteXX(hFile,pBuf, nWriteLen );
    FileWriteXX(hFile,"\n",1);
	FileWriteXX(hFile,pBufTmp,nAddrEnd);
        FileTruncateXX(hFile , nAddrStart + 1 + nWriteLen + 1 + nAddrEnd );

  
MYEND: 
	_MEM_FREEX(&pBuf);
	_MEM_FREEX(&pBufBefore);
	_MEM_FREEX(&pBufAfter);
    FileCloseXX(hFile); 
    return nRet;
DIRECT_WRITE:
	_MEM_FREEX(&pBuf);
	pBuf=(char *)_MEM_ALLOC(nWriteLen);
	if(pBuf==0)
	{
		FileCloseXX(hFile);
		return 0;
	}
	nAddrStart=(int)FileSeekXX(hFile,0,SEEK_END);


	FileSeekXX(hFile,--nAddrStart,SEEK_SET);
	while(CharPosInStr(c=FileGetChar(hFile,0)," \t\r\n")>=0)
	{
		FileSeekXX(hFile,--nAddrStart,SEEK_SET);
		if(nAddrStart<=0)break;
	}
	if(nAddrStart>0)
	{
		FileWriteXX(hFile,"\n",1);
	}
	if(section&&section[0])
	{
		sprintf(pBuf,"[%s]\n%s=%s\n",section,var,szValue);
		FileWriteXX(hFile,pBuf,strlenx(pBuf));
	}
	else
	{
		sprintf(pBuf,"%s=%s\n",var,szValue);
		FileWriteXX(hFile,pBuf,strlenx(pBuf));
	}


	_MEM_FREEX(&pBuf);
	FileCloseXX(hFile);
	return 1;
}


BOOL MYAPI IniReadRgbColor(const char *section,const char *var,PRGB_COLOR pRgb,const char * iniFile)
{
    char szTmpBuf[100]={0};
    char szValue[20]={0};
    int nRet;
    char *pszTmp = szTmpBuf;
    if( !pRgb )
    {
        return FALSE;
    }

    memset(pRgb,0,_SIZE_OF(RGB_COLOR));
    nRet = IniReadStr(section,var,szTmpBuf,100,iniFile);
    if( nRet<0 )
    {
        return FALSE;
    }
    strcpyn_stop(szValue,pszTmp,20,",:;");
    nRet = atoix(szValue);
    pRgb->rgbRed = (BYTE)nRet;

    for(; *pszTmp && CharPosInStr((*pszTmp),",:;")<0;pszTmp++);
    if( ! (*pszTmp) )
    {
        goto LABLE_OK;
    }
    pszTmp ++ ;
    strcpyn_stop(szValue,pszTmp,20,",:;");
    nRet = atoix(szValue);
    pRgb->rgbGreen = (BYTE)nRet;

    for(; *pszTmp && CharPosInStr((*pszTmp),",:;")<0;pszTmp++);
    if( ! (*pszTmp) )
    {
        goto LABLE_OK;
    }
    pszTmp ++ ;
    strcpyn_stop(szValue,pszTmp,20,",:;");
    nRet = atoix(szValue);
    pRgb->rgbBlue = (BYTE)nRet;
LABLE_OK:
    return TRUE;

}



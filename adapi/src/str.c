


#include"adapi_def.h"



int MYAPI atoix(const char *szValue)
{
	if(!szValue)return 0;

	return atoi(szValue);
}
INT64 MYAPI atoi64x(const char *szValue)
{
    if (!szValue)return 0;

#if defined(WIN32)
    return _atoi64(szValue);
#else
    return atoll(szValue);
#endif
}
int MYAPI atoin( const char *szValue ,int nCount)
{ 
	char szTmp[100];
	int nSet = 0 ;
	if( nCount < 1 || nCount > 98)
	{
		return atoix(szValue);
	}
	nSet =  nCount + 1 ;
	memset(szTmp,0,_SIZE_OF(szTmp) < nSet ? _SIZE_OF(szTmp): nSet ) ;
	memcpy(szTmp,szValue,nCount);
	return atoix(szTmp);
}
int MYAPI GetHostFromUrl(char *szHost,int Size,const char *szUrl,int *portOut)
{
	int i,j;
	i=strcmpifind(szUrl,"://");
	if(i>=0)
	{
		i+=3;
	}
	else
	{
		i=0;
	}

	j=strcpyn_stop(szHost,szUrl+i,Size,"/\\:");
	if(portOut&&szUrl[j+i]==':')
	{
		*portOut=atoi(szUrl+j+i+1);
	}
	return j;

	StrLower(szHost);
	return 0;
}



static inline int  CharCmpI(char cValue1,char cValue2)
{
    if(cValue1>='A'&&cValue1<='Z')cValue1+=32;
    if(cValue2>='A'&&cValue2<='Z')cValue2+=32;
    return (BYTE)cValue1-(BYTE)cValue2;
}

WCHAR * MYAPI strcpyFromChar(WCHAR *buf ,const char * szA , int nLen)
{
	int i = 0 ;
	if( buf == NULL || szA == NULL  )
	{
		return NULL ;
	}
	*buf = 0 ;
	nLen -- ;
	for( i = 0 ; i < (nLen) && szA[i] != 0 ; i ++ )
	{
		buf[i] = (WCHAR)szA[i];
	}
	buf[i] = 0 ;
	return buf ;
}
char * MYAPI strcpyFromWChar(char *buf ,const WCHAR * szW, int nLen)
{
	int i = 0 ;
	if( buf == NULL || szW == NULL  )
	{
		return NULL ;
	}
	*buf = 0 ;
	nLen -- ;
	for( i = 0 ; i < (nLen) && szW[i] != 0 ; i ++ )
	{
		buf[i] = (char)szW[i];
	}
	buf[i] = 0 ;
	return buf ;
}
char * MYAPI strcpyx(char * szDest,const char *szSource)
{
	char * szFirst=szDest;
	if( szDest == szSource )
	{
		return szDest;
	}
	if(!szDest||!szSource)return 0;
	while ((*szDest++=*szSource++)&&(*szSource));
	*szDest=0;
	return szFirst;
}
int MYAPI strcpyn_number(char * szDest,const char *szSource,int nLen)
{
	int i=0;
	nLen--;
	if(!szDest||!szSource)return 0;
	for(;i<nLen&&szSource[i]&&((szSource[i]>='0'&&szSource[i]<='9')||szSource[i]=='.');i++)
	{
		szDest[i]=szSource[i];
	}
	szDest[i]=0;
	return i;
}

int MYAPI strcmpifind(const char *szSource,const char *szDest)
{
	int i;
	if(!szSource||!szDest||!szDest[0])return -1;
	for(i=0;szSource[i]!=0;i++)
	{
		if(strcmpipre(szSource+i,szDest)==0)
		{
			return i;
		}
	}
	return -1;

}
int MYAPI strcmpifind_from_end(const char *szSource,const char *szDest,
							   const void *pHeadPoint)
{
	if(!szSource||!szDest||!szDest[0]||!pHeadPoint)return -1;
	for(;szSource>=(const char *)pHeadPoint;szSource--)
	{
		if(strcmpipre(szSource,szDest)==0)
		{
			return szSource-(const char *)pHeadPoint;
		}
	}
	return -1;

}
int MYAPI strcmpipre(const char *szSource,const char *szDest)
{
	while(CharCmpI(*szSource,*szDest)==0&&(*szSource))
	{
		szSource++;
		szDest++;

	}

	return *szDest==0?0:CharCmpI(*szSource,*szDest);

}
int MYAPI strncmpix(const char *szSource,const char *szDest,int nSize)
{
	register int i=0;
	if(!szDest||!szSource)return -1;
	while(CharCmpI(*szSource,*szDest)==0&&(*szSource)&&i<nSize)
	{
		szSource++;
		szDest++;
		i++;

	}
	if(i==nSize)
	{
		return 0;
	}
	else if(!(*szDest))
	{
		return 0;
	}
	else
	{
		return CharCmpI(*szSource,*szDest);

	}

}
int MYAPI strcmpix(const char *szSource,const char *szDest)
{
	int nRet;
	if(!szSource||!szDest)return -1;

	for(;(nRet=CharCmpI(*szSource,*szDest))==0&&(*szSource);szSource++,szDest++);
	/*
	nRet=CharCmpI(*szSource,*szDest);
	while(nRet==0&&(*szSource))
	{
	szSource++;
	szDest++;
	nRet=CharCmpI(*szSource,*szDest);
	}
	*/
	return nRet;
}
char MYAPI CharIsInRange(char cValue,char cStart,char cEnd)
{
	if((unsigned char)cValue>=(unsigned char)cStart&&cValue<=(unsigned char)cEnd)
	{
		return cValue;
	}
	return 0;
}
int MYAPI strcmpi_endrange(const char *szSource,
						   const char *szDest,
						   char cStart,
						   char cEnd)
{

	while(*szSource&&
		CharCmpI(*szSource,*szDest)==0&&
		CharIsInRange(*szDest,cStart,cEnd)==0&&
		CharIsInRange(*szSource,cStart,cEnd)==0)
	{
		szSource++;
		szDest++;
	}
	if((*szSource==0&&CharIsInRange(*szDest,cStart,cEnd)!=0)||
		(*szDest==0&&CharIsInRange(*szSource,cStart,cEnd)!=0))
	{
		return CharCmpI(*(--szSource),*(--szDest));
	}
	else
	{
		return CharCmpI(*szSource,*szDest);
	}
}
int MYAPI strlenx(const char *s)
{
	int i=0;
	if(!s)return i;
	for(i=0;s[i]!=0;i++);
	return i;
}
int MYAPI strlenxW(const WCHAR *s)
{
    int i=0;
    if(!s)return i;
    for(i=0;s[i]!=0;i++);
    return i;
}
int MYAPI strlenex(const char *s,const char *szStop)
{
	int i=0;
	if(!s)return i;
	for(i=0;s[i]!=0&&CharPosInStr(s[i],szStop)<0;i++);
	return i;
}
char  * MYAPI strcpyn(char *szDest,const char *szSource,int nSize)
{
	char * szFirst=szDest;
	int n=0;
	int nWCharNum=0;
	if(szSource==0||szDest==0||nSize==0) return szDest;
	while (n<nSize&&*szSource)
	{
		n++;
		*szDest=*szSource;
		if(*(unsigned char *)szDest<0x80)
		{
			nWCharNum=0;
		}
		else
		{
			nWCharNum++;
		}
		szDest++;
		szSource++;
	}


	if(n==nSize)
	{
		if(nWCharNum&&nWCharNum%2==0)
		{
			*(szDest-1)=0;
			*(szDest-2)=0;
		}
		else
		{
			*(szDest-1)=0;
		}
	}
	else
	{
		//if(nWCharNum%2)*(szDest-1)=0;
		*szDest=0;
	}

	return szFirst;
}
int MYAPI strcpyn_stop(char * szDest, const char *szSource,int nSize,const char *szStop)
{
	int i=0;
	int nWCharNum=0;
	if(!szDest||!szSource||!nSize)return 0;
	//*szDest=0;
	while(*(szSource+i)
		&&i<nSize
		&&CharPosInStr(*(szSource+i),szStop)<0)
	{
		*(szDest+i)=*(szSource+i);

		if(*(unsigned char *)(szDest+i)<0x80)
		{
			nWCharNum=0;
		}
		else
		{
			nWCharNum++;
		}
		i++;
	}
	if(i==nSize)
	{
		i--;
		if(nWCharNum&&nWCharNum%2==0)
		{
			*(szDest+i)=0;
			i--;
			*(szDest+i)=0;
		}
		else
		{
			*(szDest+i)=0;
		}

	}
	*(szDest+i)=0;
	return i;
}
int MYAPI strcmpi_stop(const char *szSource,
					   const char *szDest,
					   char *szStop)
{

	while(*szSource&&
		CharCmpI(*szSource,*szDest)==0&&
		CharPosInStr(*szDest,szStop)<0&&
		CharPosInStr(*szSource,szStop)<0)
	{
		szSource++;
		szDest++;
	}
	if((*szSource==0&&CharPosInStr(*szDest,szStop)>=0)||
		(*szDest==0&&CharPosInStr(*szSource,szStop)>=0))
	{
		return CharCmpI(*(--szSource),*(--szDest));
	}
	else
	{
		return CharCmpI(*szSource,*szDest);
	}
}


int MYAPI ReplaceChar(char *src,const char *szCharList,char cValue)
{
	int nRet=0;
	for(;*src;src++)
	{
		if(CharPosInStr(*src,szCharList)>=0)
		{
			*src=cValue;
			nRet++;
		}
	}
	return nRet;
}
void MYAPI RemoveStr(char *szSrc,int iStart,int iEnd)
{
	if(!szSrc)
	{
		return ;
	}
	for(;szSrc[iEnd];iStart++,iEnd++)
	{
		szSrc[iStart] = szSrc[iEnd];
	}
	szSrc[iStart]=0;

}
int MYAPI ReplaceStrToChar(char *szSrc,  const char *szWillReplace,char cValue)
{
	int i,j;
	int nLen;
	if(!szSrc||!szWillReplace)return 0;
	nLen=strlenx(szWillReplace);
	nLen--;
	for(i=0;*(szSrc+i)!=0;i++)
	{
		if(strcmpipre(szSrc+i,szWillReplace)==0)
		{
			j=i+1;
			*(szSrc+i)=(char)cValue;
			for(;*(szSrc+j+nLen)!=0;j++)
			{
				*(szSrc+j)=*(szSrc+j+nLen);
			}
			*(szSrc+j)=*(szSrc+j+nLen);

		}
	}
	return 1;
}

int MYAPI CharPosInStr(char cValue,const char *szStr)
{
	int i=0;
	if(!szStr||!szStr[0])return -1;
	while(*(szStr+i))
	{
		if((unsigned char)cValue==(unsigned char)*(szStr+i))
		{
			return i;
		}
		i++;
	}
	return -1;
}


char*   MYAPI  StrTrimLeft(char *str)
{
	int i=0,j=0,n=0;
    if (str == NULL || str[0] == 0 )
    {
        return str;
    }
	for(;*(str+i)==' '||*(str+i)=='\t'||*(str+i)=='\r'||*(str+i)=='\n';i++,n++);

	while(*(str+j+n))
	{
		*(str+j)=*(str+j+n);
		j++;
	}
	*(str+j)=0;
	return str;
}
char*   MYAPI  StrTrimRight(char *szBuf)
{
	int i,n;
    if (szBuf == NULL || szBuf[0] == 0)
    {
        return szBuf;
    }
	n=strlen(szBuf);
	//    printf("n=%d\n",n);
	for(i=n-1;i>-1&&(*(szBuf+i)==' '||*(szBuf+i)=='\t'||*(szBuf+i)=='\r'||*(szBuf+i)=='\n');i--);
	*(szBuf+i+1)=0;
	return szBuf;

}
char*   MYAPI  StrTrimX(const char *str)
{
	return StrTrimLeft(StrTrimRight((char *)str));
}




int MYAPI strcmpipre_ncmped(const char *szSource,const char *szDest,int *pnCmped)
{
	int nCmped=0;
	while(CharCmpI(*szSource,*szDest)==0&&(*szSource))
	{
		szSource++;
		szDest++;
		nCmped++;

	}
	if(pnCmped)*pnCmped=nCmped;
	return *szDest==0?0:CharCmpI(*szSource,*szDest);
}
int MYAPI strcmppre(const char *szSource,const char *szDest)
{
	while((*szSource)==(*szDest)&&(*szDest))
	{
		szSource++;
		szDest++;
	}
	return *szDest==0?0:(*szSource-*szDest);

}





int MYAPI GetValueFromString(const char *szSource, const char *szVarName,
							 char *szVarValue,int  nVarValueSize,
							 const char *szEqual,const char *szStopSig)
{
	int nCmped;
	int i;
	if(!szSource||!szVarName)return -1;
	if(szVarValue)*szVarValue=0;
	for(i=0;*(szSource+i)!=0;i++)
	{
        if( i > 0 )
        {
            if(  CharPosInStr(szSource[i-1],szEqual) < 0 &&
                   CharPosInStr(szSource[i-1],szStopSig) < 0 && 
                   CharPosInStr(szSource[i-1],"  :;,.?|!\r\n\\/" ) < 0 )
            {
                continue;
            }
        }
		if( strcmpipre_ncmped(szSource+i,szVarName,&nCmped) !=0 )
        {
            continue;
        }
		if(!szEqual||*szEqual==0)
		{
			return strcpyn_stop(szVarValue,szSource+i+nCmped,nVarValueSize,szStopSig);
		}
		else if(*(szSource+i+nCmped)==0)
		{

			return 0;
		}
		else if(CharPosInStr(*(szSource+i+nCmped),szEqual)>=0)
		{
			i++;
			for(;CharPosInStr(*(szSource+i+nCmped)," \t")>=0; i++ );
			return strcpyn_stop(szVarValue,szSource+i+nCmped,nVarValueSize,szStopSig);
		}

	}
	return -1;

}

int MYAPI TruncateStringRight(char *in,int n)
{
	int c;
	if (!in||!in[0])return 0;
	c=strlenx(in);
	*(in+c-n)=0;
	return 1;
}
char* MYAPI StrUpper(const char *szBuf)
{
	char *pTmpBuf=(char *)szBuf;
	if(!pTmpBuf)return 0;
	while(*pTmpBuf)
	{
		if(*pTmpBuf>='a'&&*pTmpBuf<='z')
		{
			*pTmpBuf-=32;
		}
		pTmpBuf++;
	}

	return (char *)szBuf;
}
char* MYAPI StrLower(const char *szBuf)
{
	char *pTmpBuf=(char *)szBuf;
	if(!pTmpBuf)return 0;
	while(*pTmpBuf)
	{
		if(*pTmpBuf>='A'&&*pTmpBuf<='Z')
		{
			*pTmpBuf+=32;
		}
		pTmpBuf++;
	}

	return (char *)szBuf;
}

int MYAPI strcatn(char *src,const char *dest,int lensrc)
{
	//    int i;
	int n;
	if(!src||!dest)return 0;
	n=strlenx(src);
	lensrc-=n;
	strcpyn(src+n,dest,lensrc);
	return 0;
}

int MYAPI strcpyn_visual(char *szDest,int nDestSize,
						 const char *szSource,const char *szStop,
						 int    *pCmped)
{
	int i=0;
	int nCpyed=0;
	*szDest=0;
	nDestSize--;
	for(i=0;(BYTE)*(szSource+i)<=' '&&*(szSource+i);i++);

	while(*(szSource+i)&&
		CharPosInStr(*(szSource+i),szStop)<0&&
		nCpyed<nDestSize)
	{
		if((BYTE)*(szSource+i)>=' ')
		{
			*(szDest+nCpyed)=*(szSource+i);
			nCpyed++;
		}
		i++;

	}
	*(szDest+nCpyed)=0;
	nCpyed--;

	for(;*(szDest+nCpyed)==' '&&nCpyed>0;nCpyed--)

	{
		*(szDest+nCpyed)=0;
	}
	nCpyed++;
	if(pCmped)*pCmped=nCpyed;
	//    printf("n=%d\n",n);
	return i;
}

int MYAPI strcmpipre_skipunvisual(const char *szSource,const char *szDest)
{
	int i,j;
	int nRet;

	for(i=0;(unsigned char)*(szSource+i)<=' '&&*(szSource+i);i++);
	for(j=0;(unsigned char)*(szDest+j)<=' '&&*(szDest+j);j++);

	nRet=CharCmpI(*(szSource+i),*(szDest+j));
	while(nRet==0&&*(szDest+j)!=0)
	{
		i++;
		j++;
		for(;(unsigned char)*(szSource+i)<=' '&&*(szSource+i);i++);
		for(;(unsigned char)*(szDest+j)<=' '&&*(szDest+j);j++);
		nRet=CharCmpI(*(szSource+i),*(szDest+j));
	}
	for(;(unsigned char)*(szDest+j)<=' '&&*(szDest+j);j++);
	return *(szDest+j)==0?0:nRet;

}

void MYAPI InsertStr(char *szSrc,int nLenSrc,int iWhereInsert,const char *szInsertStr)
{
	int nLenInsert=0;
	int i=0;
	nLenInsert=strlenx(szInsertStr);
	if(nLenSrc==-1)
	{
		nLenSrc=strlenx(szSrc);
	}

	for(i=nLenSrc;i>=iWhereInsert;i--)
	{
		szSrc[i+nLenInsert]=szSrc[i];
	}
	for(i=0;i<nLenInsert;i++)
	{
		szSrc[iWhereInsert+i]=szInsertStr[i];
	}



}

BOOL MYAPI Str2Bool(const char *szValue)
{
    if( !  szValue || ! szValue[0] )
    {
        return FALSE ;
    }
    if( strcmpix(szValue,"y") == 0 
        || strcmpix(szValue,"1") == 0 
        || strcmpix(szValue,"yes") == 0 
        || strcmpix(szValue,"true") == 0 
        || strcmpix(szValue,"ok") == 0 )
    {
        return TRUE ;
    }
    return FALSE ;
}




























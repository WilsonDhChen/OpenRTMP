

#include"adapi_def.h"


int MYAPI IsHex(char c)
{
	if((c>='0'&&c<='9')||(c>='A'&&c<='F')||(c>='a'&&c<='f'))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

UINT64  MYAPI HexToUINT64(const char *szDest)
{
	long i=0,j=0;
	INT64 cHex;
	INT64 nRet=0;
    if(!szDest||!szDest[0])return 0;
    if(szDest[0]=='0'&&(szDest[1]=='X'||szDest[1]=='x'))szDest+=2;

    for(i=0;IsHex(szDest[i])&&i<16;i++);
	i--;
	for(j=i;j>=0;j--)
	{
        cHex=0;
		if('0'<=*(szDest+j)&&*(szDest+j)<='9')
		{
			cHex=*(szDest+j)-0x30;
		}
		else if('A'<=*(szDest+j)&&*(szDest+j)<='F')
		{
			cHex=*(szDest+j)-0x37;
		}
		else if('a'<=*(szDest+j)&&*(szDest+j)<='f')
		{
			cHex=*(szDest+j)-0x37-32;
		}
        cHex<<=((i-j)*4);
		nRet|=cHex;
	}
	
	return nRet;
}
UINT8  MYAPI HexToUINT8(const char *szDest)
{
	long i=0,j=0;
	UINT8 cHex;
    UINT8 nRet = 0 ;

    if(!szDest||!szDest[0])return 0;

    for(;szDest[0]=='0';szDest++);
    if(szDest[0]=='X'||szDest[0]=='x')szDest++;
    for(;szDest[0]=='0';szDest++);

    for(i=0;IsHex(szDest[i])&&i<2;i++);
	i--;
	for(j=i;j>=0;j--)
	{
        cHex=0;
		if('0'<=*(szDest+j)&&*(szDest+j)<='9')
		{
			cHex=*(szDest+j)-0x30;
		}
		else if('A'<=*(szDest+j)&&*(szDest+j)<='F')
		{
			cHex=*(szDest+j)-0x37;
		}
		else if('a'<=*(szDest+j)&&*(szDest+j)<='f')
		{
			cHex=*(szDest+j)-0x37-32;
		}
        cHex<<=((i-j)*4);
		nRet|=cHex;
	}
	
	return nRet;
}
UINT16  MYAPI HexToUINT16(const char *szDest)
{
	long i=0,j=0;
	UINT16 cHex;
	UINT16 nRet=0;
    if(!szDest||!szDest[0])return 0;

    for(;szDest[0]=='0';szDest++);
    if(szDest[0]=='X'||szDest[0]=='x')szDest++;
    for(;szDest[0]=='0';szDest++);

    for(i=0;IsHex(szDest[i])&&i<4;i++);
	i--;
	for(j=i;j>=0;j--)
	{
        cHex=0;
		if('0'<=*(szDest+j)&&*(szDest+j)<='9')
		{
			cHex=*(szDest+j)-0x30;
		}
		else if('A'<=*(szDest+j)&&*(szDest+j)<='F')
		{
			cHex=*(szDest+j)-0x37;
		}
		else if('a'<=*(szDest+j)&&*(szDest+j)<='f')
		{
			cHex=*(szDest+j)-0x37-32;
		}
        cHex<<=((i-j)*4);
		nRet|=cHex;
	}
	
	return nRet;
}

UINT32  MYAPI HexToUINT32(const char *szDest)
{
	long i=0,j=0;
	INT32 cHex;
	INT32 nRet=0;
    if(!szDest||!szDest[0])return 0;

    for(;szDest[0]=='0';szDest++);
    if(szDest[0]=='X'||szDest[0]=='x')szDest++;
    for(;szDest[0]=='0';szDest++);

    for(i=0;IsHex(szDest[i])&&i<8;i++);
	i--;
	for(j=i;j>=0;j--)
	{
        cHex=0;
		if('0'<=*(szDest+j)&&*(szDest+j)<='9')
		{
			cHex=*(szDest+j)-0x30;
		}
		else if('A'<=*(szDest+j)&&*(szDest+j)<='F')
		{
			cHex=*(szDest+j)-0x37;
		}
		else if('a'<=*(szDest+j)&&*(szDest+j)<='f')
		{
			cHex=*(szDest+j)-0x37-32;
		}
        cHex<<=((i-j)*4);
		nRet|=cHex;
	}
	
	return nRet;
}
int  MYAPI TransHex2Data(const char * szData,void *vData,int nLen)
{
	BYTE *pByte = (BYTE *)vData;
	char szHexBuf[3];
	int i=0,j=0;
	if(!szData)
	{
		return 0;
	}
	
	if(pByte)
	{
		szHexBuf[2]=0;
		for(i=0;szData[i] && j<nLen ;i++)
		{
			if(szData[i] != '%')
			{
				pByte[j++]=szData[i];
				continue;
			}
			if( !IsHex(szData[i+1]) )
			{
				pByte[j++]=szData[i];

				if( !szData[i+1] )
				{
					break;
				}
				else
				{
					if(j<nLen)pByte[j++]=szData[i+1];
					continue;
				}

			}
			if( !IsHex(szData[i+2]) )
			{
				pByte[j++]=szData[i];
				if(j<nLen)pByte[j++]=szData[i+1];

				if( !szData[i+2] )
				{
					break;
				}
				else
				{
					if(j<nLen)pByte[j++]=szData[i+2];
					continue;
				}
			}
			szHexBuf[0]=szData[i+1];
			szHexBuf[1]=szData[i+2];
			pByte[j++]=(BYTE)HexToUINT8(szHexBuf);
			i+=2;
		}
		i=j;
		if((i+4)<nLen)
		{
			nLen=i+4;
		}
		for(;i<nLen;i++)
		{
			pByte[i]=0;
		}
	}
	else
	{
		for(i=0;szData[i]  ;i++)
		{
			if(szData[i] != '%')
			{
				j++;
				continue;
			}
			if( !IsHex(szData[i+1]) )
			{
				j++;

				if( !szData[i+1] )
				{
					break;
				}
				else
				{
					j++;
					continue;
				}

			}
			if( !IsHex(szData[i+2]) )
			{
				j++;
				j++;

				if( !szData[i+2] )
				{
					break;
				}
				else
				{
					j++;
					continue;
				}
			}

			j++;
			i+=2;
		}
	}


	return j;

}

int MYAPI TransData2Hex(const void *vData ,int nDataLen,
                                char *szCodeOut ,int nOutLen)
{
    unsigned char *pByte;
    int i=0,j=0;
    if(nDataLen==-1)nDataLen=strlenx((char *)vData);
    pByte=(unsigned char *)vData;
    if(!pByte)return 0;

	if(szCodeOut)
	{
		szCodeOut[0]=0;
		nOutLen--;
		for(i=0;i<nDataLen;i++)
		{
			if((pByte[i]>='A' && pByte[i]<='Z')
				||(pByte[i]>='a' && pByte[i]<='z')
				||(pByte[i]>='0' && pByte[i]<='9'))	
			{
				if(j>=nOutLen)break;
				szCodeOut[j]=(char)pByte[i];
				j++;
			}
			else 
			{
				if(j>(nOutLen-3))break;
				sprintf(szCodeOut+j,"%%%02X",pByte[i]);
				j+=3;
			}

		}
		szCodeOut[j]=0;
	}
	else
	{
		for(i=0;i<nDataLen;i++)
		{
			if((pByte[i]>='A' && pByte[i]<='Z')
				||(pByte[i]>='a' && pByte[i]<='z')
				||(pByte[i]>='0' && pByte[i]<='9'))	
			{
				j++;
			}
			else 
			{
				j+=3;
			}
		}
	}

    return j;
}

int MYAPI UnicodeToUTF8(const wchar_t *wszIn, char *szUTF8Out,int vLen)
{
	int i=0,j=0;
	unsigned int c=0;
	BYTE *pszBuf=(BYTE *)szUTF8Out;

	if(!wszIn)
	{
		return 0;
	}


	if(pszBuf)
	{
		vLen--;
		for(j=0;wszIn[j] && i<vLen ;j++)
		{
			c=wszIn[j];
			if (c < 0x80)
			{
				pszBuf[i++]=(BYTE)c;
			}
			else if (c < 0x800)
			{
				pszBuf[i++]=(BYTE) (0xC0 | ( c>>6 ) );
				pszBuf[i++]=(BYTE) (0x80 | (c & 0x3F));
			}
			else if (c < 0x10000)
			{
				pszBuf[i++]=(BYTE) (0xE0 | ( c>>12 ) );
				pszBuf[i++]=(BYTE) (0x80 | ((c>>6) & 0x3F) );
				pszBuf[i++]=(BYTE) (0x80 |  ( c & 0x3F ) );
			}
			else if (c < 0x200000)
			{
				pszBuf[i++]=(BYTE) (0xF0 | (c>>18));
				pszBuf[i++]=(BYTE) (0x80 | ((c>>12) & 0x3F));
				pszBuf[i++]=(BYTE) (0x80 | ((c>>6) & 0x3F ) );
				pszBuf[i++]=(BYTE) (0x80 | (c & 0x3F));
			}
		}
		pszBuf[i]=(BYTE)(0);
	}
	else
	{

		for(j=0;wszIn[j] ;j++)
		{
			c=wszIn[j];
			if (c < 0x80)
			{
				i++;
			}
			else if (c < 0x800)
			{
				i++;
				i++;
			}
			else if (c < 0x10000)
			{
				i++;
				i++;
				i++;
			}
			else if (c < 0x200000)
			{
				i++;
				i++;
				i++;
				i++;
			}
		}
	}

	return i;
}
int MYAPI UTF8ToUnicode(const char *szUtf8, wchar_t * szUnicode,int nLen)
{
	const BYTE *pUtf8 = (BYTE *) szUtf8;
	int cwChars = 0;
	unsigned int cc = 0;
	unsigned int wc = 0;
	char mb =0;


	if(szUnicode)
	{
		szUnicode[0]=0;
	}
	while (  *pUtf8  ) 
	{
		mb = *pUtf8++;
		cc = 0;
		wc = 0;

		while ((cc < 7) && (mb & (1 << (7 - cc)))) 
		{
			cc++;
		}

		if (cc == 1 || cc > 6)  // illegal character combination for UTF-8
		{
			continue;
		}

		if (cc == 0) 
		{
			wc = mb;
		}
		else 
		{
			wc = (mb & ((1 << (7 - cc)) - 1)) << ((cc - 1) * 6);
			while (--cc > 0) 
			{
				if ( !(*pUtf8 ) )    // reached end of the buffer
				{
					if( szUnicode && cwChars>0)
					{
						szUnicode[--cwChars] = 0;
					}
					return cwChars;
				}

				mb = *pUtf8++;
				if (((mb >> 6) & 0x03) != 2)    // not part of multibyte character
				{
					if( szUnicode && cwChars>0)
					{
						szUnicode[--cwChars] = 0;
					}
					return cwChars;
				}
				wc |= (mb & 0x3F) << ((cc - 1) * 6);
			}
		}

		if (wc & 0xFFFF0000)
		{
			wc = L'?';
		}

		if(szUnicode ) 
		{
			if(cwChars<nLen)
			{
				szUnicode[cwChars] = wc;
			}
			else
			{
				if( cwChars>0 )
				{
					szUnicode[--cwChars] = 0;
				}
				return cwChars;

			}
			
		}

		if (wc == L'\0')
		{
			return cwChars;
		}
		else
		{
			cwChars++;
		}
	}


	if(szUnicode)
	{
		if( cwChars>=nLen && nLen>0 )
		{
			cwChars=nLen-1;	
		}
		szUnicode[cwChars] = 0;
	}

	return cwChars;
}

static BOOL IsUrlChar(unsigned char ch)
{
    int i = 0 ;
    static char szNoEscape[]=".$&+,/:;=?@%-";
    for( i = 0 ; szNoEscape[i] ; i ++ )
    {
        if( szNoEscape[i] == ch )
        {
            return TRUE ;
        }
    }
    return FALSE ;
}
int  MYAPI UrlEncode(const char *szUrl,char *szUrlOut,int nLen)
{
    static  char hex[] = "0123456789ABCDEF";
    int i = 0 ;
    int j = 0 ;
    int nLenSrc = 0 ;
    if( !szUrl || !szUrl[0] || ( szUrlOut && nLen < 1 ) )
    {
        return 0 ;
    }
    nLenSrc = strlen(szUrl);
    for( i = nLenSrc - 1 ; szUrl[i] == ' ' ; i --) ;
    nLenSrc = i + 1 ;

    if( szUrlOut )
    {
        szUrlOut[ nLen - 1 ] = 0 ;
        for (i = 0; i < nLenSrc && j < ( nLen-1) ; i++)
        {
            unsigned char ch = (unsigned char)szUrl[i];
            if ( isalnum(ch) || IsUrlChar(ch) )
            {
                szUrlOut[j++] = ch;
            }
            else if (szUrl[i] == ' ')
            {
                szUrlOut[j++] = '+';
            }
            else
            {
                if( j > (nLen-5) )
                {
                    break ;
                }
                szUrlOut[j++] = '%';
                szUrlOut[j++] = hex[ch / 16];
                szUrlOut[j++] = hex[ch % 16];
            }
        }
        szUrlOut[j] = 0 ;
    }
    else
    {
        for (i = 0; i < nLenSrc  ; i++)
        {
            unsigned char ch = (unsigned char)szUrl[i];
            if ( isalnum(ch) || IsUrlChar(ch) )
            {
                j++;
            }
            else if (szUrl[i] == ' ')
            {
                j++;
            }
            else
            {

                j++;
                j++;
                j++;
            }
        }

    }

    return j;
}
BOOL  MYAPI IsTextUTF8(const char* str,int length) 
{ 
    int i; 
    DWORD nBytes=0;//UFT8可用1-6个字节编码,ASCII用一个字节 
    unsigned char chr; 
    BOOL bAllAscii= TRUE; //如果全部都是ASCII, 说明不是UTF-8 
    if( ! str || ! str[0] )
    {
        return TRUE ;
    }
    if( length == -1 )
    {
        length = strlen(str);
    }
    for(i=0;i<length;i++) 
    { 
        chr= *(str+i); 
        if( (chr&0x80) != 0 ) // 判断是否ASCII编码,如果不是,说明有可能是UTF-8,ASCII用7位编码,但用一个字节存,最高位标记为0,o0xxxxxxx 
            bAllAscii= FALSE; 
        if(nBytes==0) //如果不是ASCII码,应该是多字节符,计算字节数 
        { 

            if(chr>=0x80) 
            { 
                if(chr>=0xFC&&chr<=0xFD) 
                    nBytes=6; 
                else if(chr>=0xF8) 
                    nBytes=5; 
                else if(chr>=0xF0) 
                    nBytes=4; 
                else if(chr>=0xE0) 
                    nBytes=3; 
                else if(chr>=0xC0) 
                    nBytes=2; 
                else 
                { 
                    return FALSE; 
                } 
                nBytes--; 
            } 


        } 
        else //多字节符的非首字节,应为 10xxxxxx 
        { 
            if( (chr&0xC0) != 0x80 ) 
            { 

                return FALSE; 
            } 
            nBytes--; 
        } 
    } 



    if( nBytes > 0 ) //违返规则 
    { 
        return FALSE; 
    } 

    if( bAllAscii ) //如果全部都是ASCII, 说明不是UTF-8 
    { 
        return FALSE; 
    } 

    return TRUE; 
}



#include "adapi_def.h"


char MYAPI GetBase64Char(int index)
{
    static const char szBase64Table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    if (index >= 0 && index < 64)
    {
        return szBase64Table[index];
    }
    else
    {
        return '=';
    }
}
signed char MYAPI GetBase64Index(unsigned char ch)
{
    char  index = (char)-1;
    if (ch >= 'A' && ch <= 'Z')
    {
        index = ch - 'A';
    }
    else if (ch >= 'a' && ch <= 'z')
    {
        index = ch - 'a' + 26;
    }
    else if (ch >= '0' && ch <= '9')
    {
        index = ch - '0' + 52;
    }
    else if (ch == '+')
    {
        index = 62;
    }
    else if (ch == '/')
    {
        index = 63;
    }

    return index;
}
int MYAPI Base64Encode(const void * vData, int nDataLen,
                        char * szB64Out, int nB64BufLen)
{
    int nLen=0;
    int nRest=0;
	int i=0;	
	unsigned char Tmp1,Tmp2;
	char szBase64Unit[5]={0,0,0,0,0};
    unsigned char* psrc = (unsigned char*)vData;
    char * p64 = szB64Out;
    
    
    if(!szB64Out||!nB64BufLen)return 0;
    if (nDataLen == -1)nDataLen=strlenx((const char*)vData);
	if (nDataLen == 0) return 0;
    if(nB64BufLen<5)return 0;
    for(i=0;i<nDataLen-3;i+=3)
    {
        szBase64Unit[0]=GetBase64Char((*psrc)>>2);
        Tmp1=*psrc<<6;
        Tmp1=Tmp1>>2;
        Tmp2=*(psrc+1)>>4;

        szBase64Unit[1]=GetBase64Char(Tmp1|Tmp2);

        Tmp1=*(psrc+1)<<4;
        Tmp1=Tmp1>>2;
        Tmp2=(*(psrc+2)>>6);

        szBase64Unit[2]=GetBase64Char(Tmp1|Tmp2);

        Tmp1=*(psrc+2)<<2;
        Tmp1=Tmp1>>2;
        szBase64Unit[3]=GetBase64Char(Tmp1);

		p64[0]=szBase64Unit[0];
		p64[1]=szBase64Unit[1];
		p64[2]=szBase64Unit[2];
		p64[3]=szBase64Unit[3];

        nLen += 4;
        if(nLen>nB64BufLen)return nLen;
        p64  += 4;
        psrc += 3;
    }

    if((nLen+5)>nB64BufLen)return nLen;
    if(i<nDataLen)
    {

        nRest = nDataLen - i;

        if(nRest>0)
        {
            Tmp2=0;
            p64[0]=GetBase64Char((*psrc)>>2);
            Tmp1=*psrc<<6;Tmp1=Tmp1>>2;
            if(nRest>1)Tmp2=*(psrc+1)>>4;
            p64[1]=GetBase64Char(Tmp1|Tmp2);
        }
        if(nRest>1)
        {
            Tmp2=0;
            Tmp1=*(psrc+1)<<4;Tmp1=Tmp1>>2;
            if(nRest>2)Tmp2=(*(psrc+2)>>6);

            p64[2]=GetBase64Char(Tmp1|Tmp2);

        }
        else
        {
            p64[2]='=';
        }
        if(nRest>2)
        {
            Tmp1=*(psrc+2)<<2;
            Tmp1>>=2;
            p64[3]=GetBase64Char(Tmp1);
        }
        else
        {
            p64[3]='=';
        }
        p64 += 4;
        nLen += 4;

    }

    *p64 = '\0';
    return nLen;
}

int MYAPI Base64Encode72PerLine(const void * vData, int nDataLen,
                        char * szB64Out, int nB64BufLen)
{
    int nLen=0;
    int nRest=0;
    char szBase64Unit[5]={0,0,0,0,0};
    unsigned char* psrc = (unsigned char*)vData;
    char * p64 = szB64Out;
    int i=0;
    unsigned char Tmp1,Tmp2;
    if(!szB64Out||!nB64BufLen)return 0;
    if (nDataLen == -1)nDataLen=strlenx((const char*)vData);
	if (nDataLen == 0) return 0;
    if(nB64BufLen<5)return 0;
    for(i=0;i<nDataLen-3;i+=3)
    {
        if(i!=0&&i%54==0)
        {
            if(nB64BufLen-nLen<3)return nLen;
            *p64='\r';
            p64++;
            *p64='\n';
            p64++;
            *p64=0;
            nLen += 2;
        }
        szBase64Unit[0]=GetBase64Char((*psrc)>>2);
        Tmp1=*psrc<<6;
        Tmp1=Tmp1>>2;
        Tmp2=*(psrc+1)>>4;

        szBase64Unit[1]=GetBase64Char(Tmp1|Tmp2);

        Tmp1=*(psrc+1)<<4;
        Tmp1=Tmp1>>2;
        Tmp2=(*(psrc+2)>>6);

        szBase64Unit[2]=GetBase64Char(Tmp1|Tmp2);

        Tmp1=*(psrc+2)<<2;
        Tmp1=Tmp1>>2;
        szBase64Unit[3]=GetBase64Char(Tmp1);

		p64[0]=szBase64Unit[0];
		p64[1]=szBase64Unit[1];
		p64[2]=szBase64Unit[2];
		p64[3]=szBase64Unit[3];

        nLen += 4;
        if(nLen>nB64BufLen)return nLen;
        p64  += 4;
        *p64=0;
        psrc += 3;
    }

    if((nLen+5)>nB64BufLen)return nLen;
    if(i<nDataLen)
    {

        nRest = nDataLen - i;

        if(nRest>0)
        {
            Tmp2=0;
            p64[0]=GetBase64Char((*psrc)>>2);
            Tmp1=*psrc<<6;Tmp1=Tmp1>>2;
            if(nRest>1)Tmp2=*(psrc+1)>>4;
            p64[1]=GetBase64Char(Tmp1|Tmp2);
        }

        if(nRest>1)
        {
            Tmp2=0;
            Tmp1=*(psrc+1)<<4;Tmp1=Tmp1>>2;
            if(nRest>2)Tmp2=(*(psrc+2)>>6);

            p64[2]=GetBase64Char(Tmp1|Tmp2);

        }
        else
        {
            p64[2]='=';
        }

        if(nRest>2)
        {
            Tmp1=*(psrc+2)<<2;
            Tmp1>>=2;
            p64[3]=GetBase64Char(Tmp1);
        }
        else
        {
            p64[3]='=';
        }
        p64 += 4;
        nLen += 4;

    }

    *p64 = '\0';
    return nLen;
}





int Base64Fetch4Code(char **pCode,signed char *Base64Index,char *szStopCharList,char *Code64Ok)
{
    int i;
    for(i=0;i<4;i++){Base64Index[i]=(char)-1;Code64Ok[i]=0;}

    for(i=0;i<4;i++)
    {
        if((**pCode)==0||CharPosInStr((**pCode),szStopCharList)>=0)return i;
        for( ;(**pCode)!=0&&
          CharPosInStr((**pCode),szStopCharList)<0&&
         (Base64Index[i]=GetBase64Index((unsigned char)(**pCode)))<0;(*pCode)++);

        if((**pCode)==0||CharPosInStr((**pCode),szStopCharList)>=0)return i;
        (*pCode)++;
        Code64Ok[i]=(**pCode);
    }
    return i;

}
int Base64Fetch4CodeNoStopList(char **pCode,signed char *Base64Index,char *Code64Ok)
{
    int i;
    for(i=0;i<4;i++){Base64Index[i]=(char)-1;Code64Ok[i]=0;}

    for(i=0;i<4;i++)
    {
        if((**pCode)==0)
		{
			return i;
		}

        for( ;(**pCode)!=0 && ((Base64Index[i])=GetBase64Index(**pCode)) < 0;(*pCode)++);

        if((**pCode)==0)
		{
			return i;
		}
      
        Code64Ok[i]=(**pCode);
		(*pCode)++;
    }
    return i;

}
int MYAPI Base64Decode(const char * szCode, void * vDataOut, int nDataBufLen)
{
    unsigned char Tmp0,Tmp1;
	signed char Base64Index[4];
    char pCode[4];
    int nLen=0;
    int nRest=0;
    int i=0;
    unsigned char *p64Code=(unsigned char *)szCode;
    unsigned char *pData=(unsigned char *)vDataOut;


    while((nRest=Base64Fetch4CodeNoStopList((char **)&p64Code,Base64Index,pCode))==4)
    {
        Tmp0=Base64Index[0];
        Tmp1=Base64Index[1];
        Tmp0=Tmp0<<2;
        Tmp1=Tmp1>>4;
        pData[0]=Tmp0|Tmp1;

        Tmp0=Base64Index[1];
        Tmp1=Base64Index[2];
        Tmp0=Tmp0<<4;
        Tmp1=Tmp1>>2;
        pData[1]=Tmp0|Tmp1;

        Tmp0=Base64Index[2];
        Tmp1=Base64Index[3];
        Tmp0=Tmp0<<6;
        pData[2]=Tmp0|Tmp1;

        pData+=3;
        nLen+=3;
    }

    if((nLen+1)>nDataBufLen)return 0;
   // printf("nRest:%d\n",nRest);
    if(nRest>1)
    {
        Tmp1=0;
        Tmp0=Base64Index[0];
        Tmp1=Base64Index[1];
        Tmp0=Tmp0<<2;
        Tmp1=Tmp1>>4;
        pData[0]=Tmp0|Tmp1;
    //    printf("c:%c\n",pData[0]);
        pData++;
        nLen++;

    }
    if(nRest>2&&pCode[1]!='='&&pCode[2]!='=')
    {
        Tmp1=0;
        Tmp0=Base64Index[1];
        Tmp1=Base64Index[2];

        Tmp0=Tmp0<<4;
        Tmp1=Tmp1>>2;
        pData[0]=Tmp0|Tmp1;
        pData++;
        nLen++;
    }
    if(nRest>3&&pCode[2]!='='&&pCode[3]!='=')
    {
        Tmp1=0;
        Tmp0=Base64Index[2];
        Tmp1=Base64Index[3];
        Tmp0=Tmp0<<6;
        pData[0]=Tmp0|Tmp1;
        pData++;
        nLen++;
    }


    return nLen;
}


int MYAPI Base64DecodeEx(const char * szCode,char *szStopCharList,
                        void * vDataOut, int nDataBufLen)
{
    unsigned char Tmp0,Tmp1;
	signed char Base64Index[4];
    char pCode[4];
    int nLen=0;
    int nRest=0;
    int i=0;
    unsigned char *p64Code=(unsigned char *)szCode;
    unsigned char *pData=(unsigned char *)vDataOut;


    while((nRest=Base64Fetch4Code((char **)&p64Code,Base64Index,szStopCharList,pCode))==4)
    {
        Tmp0=Base64Index[0];
        Tmp1=Base64Index[1];
        Tmp0=Tmp0<<2;
        Tmp1=Tmp1>>4;
        pData[0]=Tmp0|Tmp1;

        Tmp0=Base64Index[1];
        Tmp1=Base64Index[2];
        Tmp0=Tmp0<<4;
        Tmp1=Tmp1>>2;
        pData[1]=Tmp0|Tmp1;

        Tmp0=Base64Index[2];
        Tmp1=Base64Index[3];
        Tmp0=Tmp0<<6;
        pData[2]=Tmp0|Tmp1;

        pData+=3;
        nLen+=3;
    }

    if((nLen+1)>nDataBufLen)return 0;
   // printf("nRest:%d\n",nRest);
    if(nRest>1)
    {
        Tmp1=0;
        Tmp0=Base64Index[0];
        Tmp1=Base64Index[1];
        Tmp0=Tmp0<<2;
        Tmp1=Tmp1>>4;
        pData[0]=Tmp0|Tmp1;
    //    printf("c:%c\n",pData[0]);
        pData++;
        nLen++;

    }
    if(nRest>2&&pCode[1]!='='&&pCode[2]!='=')
    {
        Tmp1=0;
        Tmp0=Base64Index[1];
        Tmp1=Base64Index[2];

        Tmp0=Tmp0<<4;
        Tmp1=Tmp1>>2;
        pData[0]=Tmp0|Tmp1;
        pData++;
        nLen++;
    }
    if(nRest>3&&pCode[2]!='='&&pCode[3]!='=')
    {
        Tmp1=0;
        Tmp0=Base64Index[2];
        Tmp1=Base64Index[3];
        Tmp0=Tmp0<<6;
        pData[0]=Tmp0|Tmp1;
        pData++;
        nLen++;
    }


    return nLen;
}

















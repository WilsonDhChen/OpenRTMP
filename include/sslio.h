
/* this is a SSl API ,it depend on OpenSSL,				*/
/* it auto switch from normal to SSL mode				*/
/* licence GPL											*/
/********************************************************/
/************* windows/wince/unix/linux	*****************/
/**************author : f117cdh**************************/
/**************e-mail: f117cdh@gmail.com ****************/
/***********bugs report: f117cdh@gmail.com **************/
/********************************************************/
#ifndef _SSLIO_H_
#define _SSLIO_H_   1


#ifndef _ADTYPES_H_
#include "adtypes.h"
#endif

#define SSLIO_TYPE_UNKNOWN		-1
#define SSLIO_TYPE_NO			0
#define SSLIO_TYPE_SSLV2		120
#define SSLIO_TYPE_SSLV23		123
#define SSLIO_TYPE_SSLV3		130
#define SSLIO_TYPE_TLSV1		210
#define SSLIO_TYPE_DTLSV1		310
#define SSLIO_TYPE_MAILAUTO		500  // only for mail module use
typedef struct tagSSLIO
{
	void *hRde;
	void *SSLCtx;
	void *hSSL;
	void *X509CertSrv;
	void *SSLMeth;
	char *szSubject;
	char *szIssuer;
}SSLIO,*PSSLIO;

typedef void *HSSLIO;

#ifdef __cplusplus
extern "C"
{
#endif
BOOL MYAPI SSLIO_Supported();
int MYAPI  SSLIO_HandleSize();
void MYAPI SSLIO_HandleInit(HSSLIO pSSLio);
void MYAPI SSLIO_Init();
SOCKET MYAPI SSLIO_SocketHandle(HSSLIO hSSL);
int MYAPI SSLType(const char *szUrl);
BOOL MYAPI SSLIO_ClientStart(HSSLIO hSSL,SOCKET hRde,int nSSLType);
BOOL MYAPI SSLIO_ServerStart(HSSLIO hSSL,SOCKET hRde,int nSSLType,const char *szCertFile,const char *szKeyFile);
int MYAPI SSLIO_Close(HSSLIO pSSLio);
int MYAPI SSLIO_Read(HSSLIO pSSLio,void *buf,int nLen);
int MYAPI SSLIO_ReadStr(HSSLIO pSSLio,char *buf,int nLen);
int MYAPI SSLIO_Write(HSSLIO pSSLio,const void *buf,int nLen);
int MYAPI SSLIO_SendReadStr(HSSLIO pSSLio,const void *vSend ,int nSendSize,void *vRec,int nRecSize);

int MYAPI SSLIO_SetReadTimeout(HSSLIO pSSLio,unsigned int nTime);
int MYAPI SSLIO_SetWriteTimeout(HSSLIO pSSLio,unsigned int nTime);
int MYAPI SSLIO_Shutdown(HSSLIO pSSLio);

int MYAPI SSLIO_ReadLoop(HSSLIO pSSLio,void *buffer,int size);
int MYAPI SSLIO_PkgWrite(HSSLIO pSSLio,const void *buffer,int size);
int MYAPI SSLIO_PkgRead(HSSLIO pSSLio,void *buffer,int size ,int *pnPkgLen);

#ifdef __cplusplus
}
#endif


#endif

















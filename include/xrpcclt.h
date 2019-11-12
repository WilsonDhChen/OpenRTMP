

#ifndef _XRPCCLT_H_
#define _XRPCCLT_H_
	
	
	
#define _PORT_XRPCSVR			9103	
#define _XRPC_HEAD_LEN 32
#define _XRPC_HEAD_ "^xrpchelper~&$%@"

typedef struct tagXRPC_HEAD
{
	char szHead[_XRPC_HEAD_LEN];
	INT32	nCmd;
}XRPC_HEAD , *PXRPC_HEAD;




typedef struct tagXRPC_PAR
{
	char szHead[_XRPC_HEAD_LEN];
	INT32	nDataType;
	INT32   nDataContentType;
	INT32   nDataLen;
	char    szPar[1];
}XRPC_PAR , *PXRPC_PAR;


#define _XRPC_DATA_CONTENT_STR		    1 
#define _XRPC_DATA_CONTENT_BMP		    2 
#define _XRPC_DATA_CONTENT_JPG		    3 

#define _XRPC_DATA_NO		    0 
#define _XRPC_DATA_STRING		1 
#define _XRPC_DATA_FILE		    2 
#define _XRPC_DATA_BMP		    3

#define _XRPC_CMD_GEN_VSCROLL_BMP		1
#define _XRPC_CMD_GEN_HSCROLL_BMP		2


#ifdef __cplusplus
extern "C"   
{
#endif

BOOL MYAPI Xrpc_SendCmd(SOCKET hRde,int nCmd);	
BOOL MYAPI Xrpc_SendPar(SOCKET hRde,const char *szPar,int nDataType,int nDataContentType);
BOOL MYAPI Xrpc_SendDataFile(SOCKET hRde,const char *szFile);
BOOL MYAPI Xrpc_SendData(SOCKET hRde,const void *vData , int nLen);
BOOL MYAPI Xrpc_ReturnDataType(SOCKET hRde,int *pnDataType,int *pnDataContentType,int *pnDataLen);
BOOL MYAPI Xrpc_ReturnDataSaveFile(SOCKET hRde,const char *szFile,int nLen);
BOOL MYAPI Xrpc_ReturnDataCancel(SOCKET hRde);
	
#ifdef __cplusplus
}   
#endif
	
	
	
	
	
	
	
	
	
	
	
#endif






















































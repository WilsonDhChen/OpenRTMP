

#ifndef _SQLBRIDGE_H_
#define _SQLBRIDGE_H_
	

typedef void * HSQLBRG;
typedef void * BRG_RES;
	
#define _DEF_PORT_BRG		9800
#define _DEF_PORT_BRG_SSL	9801

#ifndef _SQLBRIDGE_ERROR_CODE
#define _SQLBRIDGE_ERROR_CODE

#define _BRG_ERR_OK               0
#define _BRG_ERR_FAIL			  -1
#define _BRG_ERR_SQL              -2
#define _BRG_ERR_SOCKET           -3
#define _BRG_ERR_SSL              -4
#define _BRG_ERR_ACT_TIMEOUT      -5
#define _BRG_ERR_INVALID_PAR      -6
#define _BRG_ERR_NO_MEM           -7

#endif
	

#ifndef _SQLBRIDGE_RES_TYPE
#define _SQLBRIDGE_RES_TYPE
#define _BRG_RES_LOCAL           0
#define _BRG_RES_REMOTE          2
#endif

#ifndef _SQLBRIDGE_SQL_TYPE
#define _SQLBRIDGE_SQL_TYPE
#define _SQLBRG_SQLCMD			1
#define _SQLBRG_SQLPROC			2
#endif

typedef UINT64 ULL_SQLBRG ;	
typedef INT64 LL_SQLBRG ;	


typedef struct tagSQLBRG_FIELD
{
	INT32 nDefinedSize;
	INT32 nDataType;
	char szName[1];
}SQLBRG_FIELD,*PSQLBRG_FIELD;

#ifdef __cplusplus
extern "C"   
{
#endif	
void MYAPI SQLBrgInit(HSQLBRG hBrg);
int MYAPI SQLBrgHandleSize();
HSQLBRG MYAPI SQLBrgAlloc();
void MYAPI SQLBrgFree(HSQLBRG hBrg);
void MYAPI SQLBrgShutdown(HSQLBRG hBrg);
LL_SQLBRG MYAPI SQLBrgAffectedRows(HSQLBRG hBrg);
int  MYAPI SQLBrgLastErrno(HSQLBRG hBrg);
const char * MYAPI SQLBrgErrText(HSQLBRG hBrg);


int  MYAPI SQLBrgConnect(HSQLBRG hBrg,
                         const char *szBrgServerAddr,
                         const char *szBrgDriverName,
                         const char *szConnectString,
                         const char *szExtParameter, // for Bridge server Driver use
                         const char *szAgentUrl // for socks4/5 https proxy use
                         );

int  MYAPI SQLBrgQuery(HSQLBRG hBrg, const char *szSQL,int nLenSQL,int nSQLType);
ULL_SQLBRG MYAPI SQLBrgQueryReturnedRows(HSQLBRG hBrg);

BRG_RES MYAPI SQLBrgStoreRes(HSQLBRG hBrg,int nResType); // res at local or remote server
LL_SQLBRG MYAPI SQLBrgNumRows(BRG_RES res);
unsigned int MYAPI SQLBrgNumFields(BRG_RES res);
void MYAPI SQLBrgFreeRes(BRG_RES res);

int MYAPI SQLBrgFiledsCount(BRG_RES res);
SQLBRG_FIELD * SQLBrgFiled(BRG_RES res,int iCol);





#ifdef __cplusplus
}
#endif	
	
	
	
	
	
	
	
	
	
	
#endif
































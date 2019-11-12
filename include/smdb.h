
/* this a simple database engine ,one session only 		*/
/* support one table,one index,the records have a   	*/
/* limit 6000000,but the support data is very large if	*/
/* you open a smdb with option SMDB_STORAGE_BFS it 		*/
/* can support more than one TB .it is very very fast,	*/
/* because it is very very easy.support multithread 	*/
/* access.												*/
/********************************************************/
/************* windows/wince/unix/linux	*****************/
/**************author : f117cdh**************************/
/**************e-mail: f117cdh@gmail.com ****************/
/***********bugs report: f117cdh@gmail.com **************/
/********************************************************/

#if !defined(_SMDB_H_)

#define _SMDB_H_

#if  defined(_WIN32_WCE)
	#include <windows.h>
#elif defined(WIN32)
	#include <windows.h>
#else

#endif

#ifndef _ADTYPES_H_
#include "adtypes.h"
#endif

#if !defined(_SMDB_OPERATE)
#define _SMDB_OPERATE

#define _SMDB_OPERATE_OK			0
#define _SMDB_OPERATE_INVALID_ADDR		-1
#define _SMDB_OPERATE_BUSY		-2
#define _SMDB_OPERATE_ERROR		-3
#define _SMDB_OPERATE_DBWILLCLOSE	-4
#define _SMDB_OPERATE_EXIST		-5
#define _SMDB_OPERATE_NOTEXIST		-6
#define _SMDB_OPERATE_DATA_ERROR		-7
#define _SMDB_OPERATE_DENIED		-8	
#define _SMDB_OPERATE_LOCK_FAIL		-9
#endif
/**************************************/

#if !defined(_INT_IO)
#define _INT_IO
#if defined(WIN32)
	typedef __int64 INT_IO;
	typedef unsigned __int64 UINT_IO;
#else
	typedef long long INT_IO;
	typedef unsigned  long long UINT_IO;
#endif
#endif

#if !defined(_SMDB_OPEN_TYPE)
#define _SMDB_OPEN_TYPE

#define SMDB_OPEN_AUTO	0
#define SMDB_OPEN_CREATE 1
#define SMDB_OPEN_RESTORE 2
#define SMDB_OPEN_READONLY 3
//#define SMDB_OPEN_RESTORE_MF 4

#define SMDB_STORAGE_FILE 0
#define SMDB_STORAGE_BFS 1
#define SMDB_STORAGE_MF 2


#endif


#define SmdbSQLDelete( hMySmdb,szPrimaryKey )  SmdbSQLDeleteL( (hMySmdb) ,(szPrimaryKey),0, TRUE,0 ) 
#define SmdbSQLDeleteCheck( hMySmdb,szPrimaryKey,funcCanDelete )  SmdbSQLDeleteL( (hMySmdb) ,(szPrimaryKey),0, TRUE,(funcCanDelete) ) 		

#define SmdbSQLInsert( hMySmdb, szPrimaryKey, Buf, nBufSize, pDiskAddr ) SmdbSQLInsertL( (hMySmdb),(szPrimaryKey), (Buf), (nBufSize), (pDiskAddr), TRUE ) 
		

#define SmdbSQLGet( hMySmdb,szPrimaryKey, Buf, nBufSize)  SmdbSQLSelectL( (hMySmdb),(szPrimaryKey), (Buf), (nBufSize), (0), (0), TRUE,0 ) 
#define SmdbSQLSelect( hMySmdb,szPrimaryKey, Buf, nBufSize, nOffset, pDiskAddr )  SmdbSQLSelectL( (hMySmdb),(szPrimaryKey), (Buf), (nBufSize), (nOffset), (pDiskAddr), TRUE,0 ) 		
#define SmdbSQLSelectLock( hMySmdb,szPrimaryKey, Buf, nBufSize, funcLock)  SmdbSQLSelectL( (hMySmdb),(szPrimaryKey), (Buf), (nBufSize), (0), (0), TRUE,(funcLock) ) 


#define SmdbIoDelete( hMySmdb,szPrimaryKey )  SmdbSQLDeleteL( (hMySmdb) ,(szPrimaryKey),0, FALSE,0 ) 
		

#define SmdbIoInsert( hMySmdb,szPrimaryKey, Buf, nBufSize, pDiskAddr )   SmdbSQLInsertL( (hMySmdb),(szPrimaryKey), (Buf), (nBufSize), (pDiskAddr), FALSE ) 
		
		
#define SmdbIoSelect( hMySmdb,szPrimaryKey, Buf, nBufSize, nOffset, pDiskAddr )  SmdbSQLSelectL( (hMySmdb),(szPrimaryKey), (Buf), (nBufSize), (nOffset), (pDiskAddr), FALSE,0 ) 
		


typedef BOOL (MYAPI *_SmdbRecordLock)(void *pData,int nLen); // return TRUE ,the data will re-save to smdb
typedef BOOL (MYAPI *_SmdbRecordCanDelete)(void *pData,int nLen); // return TRUE ,the record will be deleted

/**************************************/
#ifdef __cplusplus
extern "C"   
{
#endif

typedef int (MYAPI *_SmdbCompare)(const void *Source,const void *Dest);

void * MYAPI SmdbOpenDatabase(const char *szDBPath,const char *szDbName,
							  int nOpenType,int nStorageType);
BOOL MYAPI SmdbCloseDatabase(void * hMySmdb);
BOOL MYAPI SmdbCloseDatabaseForce(void ** phMySmdb);
BOOL MYAPI SmdbSave(void * hMySmdb);
int MYAPI SmdbMf2File(void * hMySmdb);
int MYAPI SmdbMfFromFile(void * hMySmdb);
void MYAPI SmdbSetUserData(void * hMySmdb ,void *vUserData);
void *MYAPI SmdbGetUserData(void * hMySmdb);


void *MYAPI SmdbSetCompareFunction(void * hMySmdb,void *FucCmp);
int MYAPI SmdbSetTableInfo(void * hMySmdb,
								int nTableSize,
								int nIndexColumnOffset,
								int nIndexColumnSize);
int MYAPI SmdbGetInsertSearchQueue(void * hMySmdb);
int MYAPI SmdbRecordCount(void * hMySmdb);
INT_IO MYAPI SmdbRecordToAddr(void * hMySmdb,int nRecord);

INT_IO MYAPI SmdbFetchAddr(void * hMySmdb);
int MYAPI SmdbFetchGetNum(void * hMySmdb);
int MYAPI SmdbFetchRead(void * hMySmdb,INT_IO nDiskAddr,
						void *Buf,int nLenBuf,
						int *IsDeleted,int nOffset);
int MYAPI SmdbFetchSetNum(void * hMySmdb,int nNumber);


int MYAPI SmdbSQLDeleteL(void * hMySmdb,const void *szPrimaryKey,
                         INT_IO *pAddrdel,BOOL bSafe,void *funcCanDelete);
int MYAPI SmdbSQLInsertL(void * hMySmdb,const void *szPrimaryKey,
						void *Buf, int nBufSize,INT_IO *pDiskAddr,
						BOOL bSafe);
int MYAPI SmdbSQLSelectL(void * hMySmdb,const void *szPrimaryKey,
						void *Buf,int nLenBuf,int nOffset,
						INT_IO *pDiskAddr,BOOL bSafe,void *funcLock);

int MYAPI SmdbIoWaitLock(void * hMySmdb,unsigned int nWaitTime);
void MYAPI SmdbIoUnlock(void * hMySmdb);
INT_IO MYAPI SmdbIoSeek(void * hMySmdb,INT_IO nSeek,int nWhere);
int MYAPI SmdbIoRead(void * hMySmdb,void *Buf,int nLenBuf);
int MYAPI SmdbIoWrite(void * hMySmdb,const void *Buf,int nLenBuf);
IO_HANDLE MYAPI SmdbIoHandle(void * hMySmdb);
const char * MYAPI SmdbErrStr(int nCode);
/*******************************************/
#ifdef __cplusplus
}   
#endif

#endif














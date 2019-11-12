
#ifndef _SQLBRGSRV_H_
#define _SQLBRGSRV_H_




// INT64 row counts,;INT32 fields count ;   1, 2, 3, 4, 5, ..... index   ;   ISQL_FIELD ..............,

typedef void *ISQLConn;

typedef BOOL (MYAPI *_ISQLBrgDriverMatch)(const char *szDriverName);
typedef ISQLConn (MYAPI *_ISQLBrgConnect)(const char *szConnectString,const char *szExtParameter,char *szErrorOut,int nLenError);
typedef void (MYAPI *_ISQLBrgDisconnect)(ISQLConn hConn);
typedef BOOL (MYAPI *_ISQLBrgQuery)(ISQLConn hConn,const char *szSQL,int nLenSQL,int nSQLType,INT64 *pnAffectedRows,INT64 *pnRecordCount);
typedef const char * (MYAPI *_ISQLBrgLastErrorText)(ISQLConn hConn);
typedef int (MYAPI *_ISQLBrgResultFiledsInfo)(ISQLConn hConn,void * pMem,int nLenMem);

typedef BOOL (MYAPI *_ISQLBrgMove)(ISQLConn hConn,INT64 nRow);
typedef BOOL (MYAPI *_ISQLBrgMoveNext)(ISQLConn hConn);
typedef BOOL (MYAPI *_ISQLBrgMovePrevious)(ISQLConn hConn);
typedef BOOL (MYAPI *_ISQLBrgMoveFirst)(ISQLConn hConn);
typedef BOOL (MYAPI *_ISQLBrgMoveLast)(ISQLConn hConn);
typedef int  (MYAPI *_ISQLBrgRowData)(ISQLConn hConn,void * pMem,int nLenMem);

#endif


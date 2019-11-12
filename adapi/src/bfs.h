
#include"adapi.h" 

#if !defined(_BFS_)
#define _BFS_

//#define BFS_DBG_PRINT(fmt ,args...)     printf(fmt,## args)

//#define BFS_BLOCK_SIZE_DEFAULT  1073741824L //1GB
//#define BFS_BLOCK_SIZE_MAX  SIZE_2GB//2147483648 //2GB
#define BFS_ERROR_LEN	600
#define BFS_PATH_LEN   400
#define BFS_FIELNAME_LEN   60


#define BFS_OPENWAY_DIR		1
#define BFS_OPENWAY_NODE	2



#define BFS_DIR_PTR(bfsHandel) ((PBFSDIR)((bfsHandel)->pWayStruct))
#define BFS_NODE_PTR(bfsHandel) ((PBFSNODE)((bfsHandel)->pWayStruct))

typedef unsigned int BFS_BLOCK_NUM_INT;

typedef struct tagBFSDIR
{	
	char szBfsDir[BFS_PATH_LEN];
	int nBfsDirLen;	
}BFSDIR,*PBFSDIR;
typedef struct tagBFSNODE
{	
	char szBfsDir[BFS_PATH_LEN];
	int nBfsDirLen;	
	unsigned int nBlockStart;
	unsigned int nBlockEnd;
}BFSNODE,*PBFSNODE;
/////////////////////////////////
typedef struct tagBFSHANDLE
{
	int nOpenMode;
	int nOpenWay;
	int nOperator;
	FILE_HANDLE hCurHandle;
	unsigned int nCurBlock;
	unsigned int nLastBlock;
	INT_FILE nBlockSize;
	int nCurNode;
	int nNodes;
	char szBfsIni[BFS_PATH_LEN];
	char szError[BFS_ERROR_LEN];
	int nErrorCode;
	void *pWayStruct;

}BFSHANDLE,*PBFSHANDLE;

#endif                             
















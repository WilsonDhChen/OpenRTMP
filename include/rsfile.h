




#ifndef  _RS_FILE_H_
#define  _RS_FILE_H_

#define MAX_FS_FILENAME    240
typedef void * RSFILE_HANDLE;
typedef void * RSFILE_MASTER;
typedef void * RSFILE_DIR;

#define  _RSFILE_ERROR_OK   0
#define  _RSFILE_ERROR_LOCK   -1
#define  _RSFILE_ERROR_SEEK   -2
#define  _RSFILE_ERROR_READ   -3
#define  _RSFILE_ERROR_WRITE   -4
#define  _RSFILE_ERROR_PAR   -5

typedef struct tagRSFILE_NODE
{
    char   szName[MAX_FS_FILENAME];
    UINT64 tLstModify;
    BOOL   bFile;
    
}RSFILE_NODE,*PRSFILE_NODE;

#ifdef __cplusplus
extern "C"   
{
#endif

RSFILE_MASTER  RsFileMasterConnect(const char *szPath,int nOption);
BOOL  RsFileMasterFree(RSFILE_MASTER _Master,int *pErrorCodeOut);

RSFILE_HANDLE  RsFileOpen(const char *szFile,int nMode,RSFILE_MASTER _Master);
BOOL  RsFileClose(RSFILE_HANDLE hFile);
int  RsFileRead(RSFILE_HANDLE hFile,void *buf,int nLen);
int  RsFileWrite(RSFILE_HANDLE hFile,const void *buf,int nLen);
INT_FILE  RsFileSeek(RSFILE_HANDLE hFile,INT_FILE nOffSet,int nSet);

BOOL  RsFileDeleteFile(const char *szFile);
BOOL  RsFileRemoveDir(const char *szPath);

RSFILE_DIR  RsFileOpenDir(const char *szPath,RSFILE_MASTER _Master);
BOOL  RsFileCloseDir(RSFILE_DIR hDir);
BOOL  RsFileReadNodeNext(RSFILE_DIR hDir,RSFILE_NODE *pNode);
BOOL  RsFileReadNodePrev(RSFILE_DIR hDir,RSFILE_NODE *pNode);

#ifdef __cplusplus
}
#endif














































#endif

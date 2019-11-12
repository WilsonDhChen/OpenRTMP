


#ifndef _URLMAP_H_
#define _URLMAP_H_


/************************************************************************/
/*                         urlmap section                               */
/************************************************************************/
typedef struct tagMAP_URL
{
    const char *szNode;
    const char *szPath;
    struct tagMAP_URL *next;
}MAP_URL,*PMAP_URL;



typedef PMAP_URL (MYAPI *_GetMapNode)(const char *szNode,PMAP_URL *pHead);
typedef PMAP_URL (MYAPI *_GetMapNodeEx)(const char *szUrl);
typedef struct tagURLMAP_CTX
{
    _GetMapNode GetMapNode;
    _GetMapNodeEx GetMapNodeEx;
    const char *szDestIP;
}URLMAP_CTX,*PURLMAP_CTX;

typedef int (MYAPI *_UrlMapPlugin)(const char *szClientID,INT32 nFileCmd,const char *szUrl,char *szPath,int  nLen,PURLMAP_CTX ctx);
typedef BOOL (MYAPI *_UrlMapPluginInit)(const char *szIniFile);

/**************************************************************/



#endif























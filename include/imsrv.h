

#ifndef _IMSRV_H_
#define _IMSRV_H_



#ifndef _IMSRV_EXPORT
#ifdef WIN32
#define _IMSRV_EXPORT __declspec(dllimport)
#else
#define _IMSRV_EXPORT
#endif
#endif

typedef enum tagSendType
{
    SendTypeAtOnce,
    SendTypeTimer,

}SendType;


typedef enum tagDropType
{
    DropTypeNone,
	DropTypeAny,
	DropTypeOffline,
    DropTypeTimeout,

}DropType;

typedef enum tagLoginMode
{
    LoginModeTCP,
    LoginModeUDP,

}LoginMode;

typedef struct tagIM_CMD
{
    INT64 nCmdID;
    SendType sendType;
	DropType dropType;
    INT64 tmSend;
    char szID[40];
    int  nCmdName;
    char szCmd[SIZE_1KB];
  
}IM_CMD;


#ifndef _IM_PACKET_
#define _IM_PACKET_

#ifndef     _IM_LOGIN_
    #define  _IM_LOGIN_         201
    #define  _IM_KEEPALIVE_     202
#endif

#define _IM_NOTIFY_TASK_CHANGED "TASKCHANGED"

#define    _IM_FLAG_RequireReply        0
#define    _IM_FLAG_ForceRepeat         1
#define    _IM_FLAG_FreeReply           2

#ifndef     _FlagRequireReply
#define    _FlagRequireReply(x)  ((x).Flags[_IM_FLAG_RequireReply]) 
#define    _PFlagRequireReply(x)  ((x)->Flags[_IM_FLAG_RequireReply]) 
#define    _FlagForceRepeat(x)  ((x).Flags[_IM_FLAG_ForceRepeat]) 
#define    _PFlagForceRepeat(x)  ((x)->Flags[_IM_FLAG_ForceRepeat]) 
#define    _FlagFreeReply(x)  ((x).Flags[_IM_FLAG_FreeReply]) 
#define    _PFlagFreeReply(x)  ((x)->Flags[_IM_FLAG_FreeReply]) 
#endif


#define    _LEN_CLIENT_ID	36
#define    _IM_CMD_HEAD	8
#define    _LEN_CMD_FLAGS	4

#define _IM_HEAD_SESSION_   "$%PLY#&"

#define _IM_RETURN_OK		0
#define _IM_RETURN_ERROR		1

#define _IM_REPLY_MODE_REQUEST		0
#define _IM_REPLY_MODE_REPLY		1
#define _IM_REPLY_MODE_REACHED		2

#pragma pack(1)

#ifdef _SIMPLE_PACKET_
typedef struct tagIM_PACKET
{
	INT64	idCmd;
	char 	szClientID[_LEN_CLIENT_ID];
	INT32	nLenData;
	char    szData[1];
}IM_PACKET,*PIM_PACKET;//max 512Bytes
#else
typedef struct tagIM_PACKET
{
    char 	szHead[_IM_CMD_HEAD];
    char 	szClientID[_LEN_CLIENT_ID];
    INT64	idCmd;
    INT32	nCmdName;
    UINT8	Flags[_LEN_CMD_FLAGS];
    INT32   nReplyMode;
    INT32   nReturnCode;
    INT32	nLenData;
    char    szData[1];
}IM_PACKET,*PIM_PACKET;//max 512Bytes
#endif


#pragma pack()
#endif

typedef void (MYAPI *_IMSrvNotifyFunc)();
typedef void * ( MYAPI *_IMSrvCommandCreate)(const char *szIniFile,const char *szVendor) ;
typedef BOOL (MYAPI *_IMSrvCommandInit)(const char *szConfigFile,const char *szLogFileDir,int *verOut, _IMSrvNotifyFunc func);
typedef void ( MYAPI *_IMSrvCommandClose)(void *handle) ;

#ifdef __cplusplus
extern "C"
{
#endif

void * MYAPI IMSrvCommandCreate(const char *szIniFile,const char *szVendor);
BOOL MYAPI IMSrvCommandInit(const char *szConfigFile,const char *szLogFileDir,int *nVerOut , _IMSrvNotifyFunc func);
void MYAPI IMSrvCommandClose(void *handle) ;


#ifdef __cplusplus
}
#endif



#if defined(_LIBCPPUTIL_NET_H_) &&  defined(__cplusplus)

class _IMSRV_EXPORT  CIMPacketIOBuffer :
    public CObjNetIOBuffer
{
public:
    _CPP_UTIL_DYNAMIC_DECLARE(CIMPacketIOBuffer)  ;
    _CPP_UTIL_CLASSNAME_DECLARE(CIMPacketIOBuffer) ;
    _CPP_UTIL_QUERYOBJ_DECLARE(CIMPacketIOBuffer) ;

    CIMPacketIOBuffer(void);
    virtual ~CIMPacketIOBuffer(void);

    void CopyPacket( const IM_PACKET  *packet );
    void CopyIM_CMD(const IM_CMD *pCmd );
    IM_PACKET *GetPacket();
    void SetClientID(const char *szID);
    void SetIMCommand(int nCmdName,const char *szCmd);
    void SetIMCommandLength(int nLen = -1);
    void SetReplyMode(int nMode);
public:
    char m_szBuf[SIZE_2KB];
    IM_PACKET * m_pPacket;
};



class    IObjImsrvCmd:
    public CObj 
{
public:

    _CPP_UTIL_DYNAMIC(IObjImsrvCmd)  ;
    _CPP_UTIL_CLASSNAME(IObjImsrvCmd) ;
    _CPP_UTIL_QUERYOBJ(CObj) ;
public:

	IObjImsrvCmd(void){};
	virtual ~IObjImsrvCmd(void){};

public:
	virtual BOOL OnLogin(CObjConnContext *conn,const char *szID,const char *szAddr,LoginMode loginMode = LoginModeTCP, char *szData = NULL,int nDataLen = 0 ){return FALSE;}
	virtual void OnOffline(CObjConnContext *conn,const char *szID){ return ;}
    virtual void OnCommandSendFinished(CObjConnContext *conn,const char *szID,INT64 nCmdID,const char *szCmd ,const char *szStatus){ return ;}
    virtual void OnCommandReturned(CObjConnContext *conn,const char *szID,INT64 nCmdID,const char *szRsultMsg){ return ;}

public:
    virtual CObj *GetCommandRecords(const char *szID){ return NULL ;}
    virtual BOOL GetNextCommand(CObj *res,IM_CMD *pCmd){ return FALSE;}
    virtual void ReleaseCommandRes(CObj *res){ return ;}

};

#endif




#endif
















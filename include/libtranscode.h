


#ifndef _libTransCode_h_
#define _libTransCode_h_



typedef int (MYAPI *__TransCodeLogFunc)( void *control,int prio,const char *fmt, va_list ap );
typedef int (MYAPI *__TransCodeSysTimeProgressFunc)(void *control, int is_last_report, TIME_T timer_start, TIME_T cur_time);

#define _TRANSCODE_CMD_BEGIN        0
#define _TRANSCODE_CMD_END          1
#define _TRANSCODE_CMD_PROGRESS     2

typedef int (MYAPI *__TransCodeCallbackFunc)(void *control, int cmd, INT64 seconds);

typedef struct tagTransCodeCtx
{
    INT64   m_duration;

    int     m_nReturnCode;
    int     m_bExit;
    int     *m_pbExit;
    TIME_T  m_tmBegin;
    TIME_T  m_tmEnd;
    void *  m_usrData;
    __TransCodeLogFunc m_log;
    __TransCodeSysTimeProgressFunc m_timeprogress;
    __TransCodeCallbackFunc m_callback;
}TransCodeCtx;



#ifdef __cplusplus
extern "C"
{
#endif

    void StreamTransferInit();

    int StreamTransfer(int argc, char * argv[], TransCodeCtx *control);

    int Onvif_Download(const char *url, const char *localFile, const char *clockRange,
                        __TransCodeCallbackFunc callback,void *  usrData,BOOL *bExit);



#ifdef __cplusplus
}
#endif




#endif









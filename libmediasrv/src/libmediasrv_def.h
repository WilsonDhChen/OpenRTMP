



#ifndef _libmediasrv_def_h_
#define _libmediasrv_def_h_


#ifdef WIN32
#ifdef LIBMEDIASRV_EXPORTS
#undef  _MEDIASRV_EXPORT
#define _MEDIASRV_EXPORT __declspec(dllexport)
#endif
#else

#undef  _MEDIASRV_EXPORT
#define _MEDIASRV_EXPORT 

#endif


#ifdef WIN32
#define _MSWSOCK_
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <direct.h>
#include <tchar.h>
#include <process.h>
#include <rpc.h>
#pragma comment(lib,"rpcrt4.lib")
#else

#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/stat.h>
#define strcmpi strcmpix


#endif


#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#if !defined(_FreeBSD_) && !defined(__DARWIN_UNIX03)
#include <malloc.h>
#endif
#include <time.h>
#include <math.h>

#include "adtypes.h"
#include "adapi.h"
#include "rde.h"
#include "libmalloc2.h"
#include "memdbg.h"


#include "libcpputil.h"
#include "libcpputil_net.h"
#include "libcpputil_net_rtsp.h"
#include "libcpputil_net_pkg.h"
#include "libcpputil_net_http.h"
#include "libcpputil_thread.h"
#include "libcpputil_event.h"
#include "libcpputil_wait.h"
#include "libcpputil_rtmp.h"

#include "libcpputil_config.h"

#define _TAGNAME "libmediasrv"

#define _TAGNAME_HLS "hsl"
#define _TAGNAME_RTSP "rtsp"
#define _TAGNAME_RTMP "rtmp"
#define _TAGNAME_ETS "ets"
#define _TAGNAME_CDN "cdn"
#define _TAGNAME_HTTPD "httpd"

#include "libav_types.h"
#include "adts_decode.h"

#include "map_ext.h"

#define _DEFAULT_QUEUE_TIMEOUT  -1

typedef enum tagH264NaluFlag
{
	H264NaluFlagUndefine,
	H264NaluFlagFirst,
	H264NaluFlagLast,
	H264NaluFlagNoLast,

}H264NaluFlag;

#pragma pack(1)
typedef struct tagRTSP_INTERLEAVED
{
	UINT8  Flag;
	UINT8  Channel;
	UINT16 Length;
}RTSP_INTERLEAVED;
#pragma pack()


typedef enum tagCamCodec
{
	CamCodecUnknown ,
	CamCodecMP4V_ES ,
	CamCodecH263 ,
	CamCodecH264 ,
	CamCodecJPEG ,
	CamCodecAudioAAC ,
	CamCodecAudioAMRNB ,

}CamCodec ;

typedef struct tagSpsPps
{
    unsigned char data[100];
    int len;

}SpsPps;


#define _ETS_PORT 2554
#define _MAX_TRACKS_ 2
#define _MAX_PACKET_ 2048
#define RTSP_PARAM_STRING_MAX 100


#define _SAMPLE_RATE_  90000
#define _SAMPLE_RATE_AUDIO_  8000
#define _RTP_MTU_	1300


#define _DEF_BITRATE_   400000

#define _VIDEO_TRACKID  0
#define _AUDIO_TRACKID  1

#define _VIDEO_RTP_CHANNEL 0
#define _VIDEO_RTPCP_CHANNEL 1
#define _AUDIO_RTP_CHANNEL 2
#define _AUDIO_RTPCP_CHANNEL 3

#define TS_SEND_PATPMT_INTERVAL 250
#define TS_SEND_PCR_INTERVAL 35

static inline UINT32 Scale(unsigned int v,unsigned int Num,unsigned int Den)
{
	if (Den)
		return (UINT32)((INT64)v * Num / Den);
	return 0;
}

#define _RECORD_TYPE_NVR  0
#define _RECORD_TYPE_SESSION  1



#include "libets.h"







#if !defined(_MAX_CONNECTIONS)
#define _MAX_CONNECTIONS  5
#endif

#include "libmediasrv_ctx.h"

#if defined(_RTSPTS_SUPPORT_) || defined(_HLS_SUPPORT_) || defined(_HTTPTS_SUPPORT_) 
#include "libmediasrv_ts.h"
#endif





#include "libmediasrv.h"
#include "libmediasrv_netbuffer.h"



#if defined(_RTMP_SUPPORT_)
#include "rtmp_protocol.h"
#include "libmediasrv_rtmp.h"
#endif





#if defined(_HLS_SUPPORT_)
#include "libmediasrv_hls.h"
#endif



#if defined(_FLVREC_SUPPORT_)
#include "libmediasrv_flv_rec.h"
#endif

#include "libmediasrv_rtpparse.h"



#define _MDSOURCE_THREADS  30






#include "libmediasrv_callback.h"

#include "libmediasrv_settingsrv.h"
#include "libmediasrv_usr.h" 
#include "libmediasrv_udp.h"
#include "libmediasrv_app.h"
#include "libmediasrv_session.h"
#include "libmediasrv_config.h"






#define is_num(c) ((c)>='0' && (c)<='9')

#define Convert2CCtx(ptr) ((CCtx *)((char *)(ptr)-(unsigned long)(&((CCtx *)0)->m_mdItem)))

extern CMediaSrvApp *_MediasrvApp;
CamCodec Str2CamCodec(const char *szStr);
void GetCNAME(char *szBuf,int nLen);
UINT32 GetOStime();
void GetPcr(TIME_T tm, UINT32 &pcr, UINT32 &pcr_ext);

BOOL PullStream(const char *szApp, const char *szSession, const char *url);
BOOL  CDNCallParentNode(CMediasrvCallback *callback,const char *szApp, const char *szSession, const char *szPar, const char *szClientProtocol);
void ParseRTSPTransportLine(const char *szLine , BOOL &bTcp, BOOL &bUdp, int &nPortRTP, int &nPortRTCP);
const char *GetServerInfo();
const char *GetMediaServerUsrAgent();


void CallbackLoadPlugin(const char * dlPath);

BOOL h264_decode_sps__(UINT8 * buf,unsigned int nLen,int &width,int &height)  ;
unsigned char PT(XCodec codec);
BOOL LoadKey(APP_KEY *key, const char *szMachineId, const char *szAppId, const char *szPath);

void LoadGpsPlugin(const char *szPath);
void StartGps();
void StopGps();
int  ReadGpsData(char *szBuf, int nLen);

extern CConfigIni _AppConfig;
extern OBJECTS_COUNT _ObjCount;
extern void *_hIns;
extern CObjNetAsync::NetAsyncType _NetType ;


BOOL X264_GetSpsPps(int width, int height, int framerate, int avclevel, int avcprofile, int videokeyframe_frequency, CObjBuffer &sps, CObjBuffer &pps, CObjBuffer &sei);
typedef void  (*_UdpTsSendFrame)(unsigned char *encodedFrame, int nLen, XCodec codec, unsigned int  tmNow, int bKeyFrame);

extern _UdpTsSendFrame _udpTsSendFrame;
extern _MediaOpenCallback _funcMediaOpenCallback;
extern BOOL _gpsOpened;



#endif





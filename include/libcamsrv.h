

#ifndef _libcamsrv_h_
#define _libcamsrv_h_
	
#include <libcam.h>

#ifdef WIN32
#define  WM_BMP_FRAME_ARRIVED  (WM_USER+5000)
#define  WM_VIDEO_OPEN_FAILED    (WM_USER+5001)
#define  WM_NOTIFY_BEGIN_DETECT_CAMERAS    (WM_USER+5002)
#define  WM_NOTIFY_END_DETECT_CAMERAS    (WM_USER+5003)
#define  WM_NOTIFY_FINISHED_INIT_CAMERAS    (WM_USER+5004)
#define  WM_NOTIFY_CLOSE_CAMERAS    (WM_USER+5005)
#endif

typedef enum tagCamRunMode
{
	CamRunModeCommandLine ,
	CamRunModeWin32GUI ,
	CamRunModeIOSGUI ,
}CamRunMode ;



#ifdef __cplusplus
extern "C"
{
#endif
    
    typedef void (*_NotifyDirectConnectionClosed)( int soc);
	typedef void (*_PostCamFrame)( void *hWnd,LibCamFrame *frame);
	typedef BOOL (*_CanGetNextFile)(char *szPath ,int nLen);
#ifdef _GPS_UTIL_H_
	typedef void (*_GpsDataCallback)(GPS_INFO *gps,const char *szHead);
#endif
	

	void MYAPI SetFuncCanGetNextFile( _CanGetNextFile pfunc);
#ifdef _GPS_UTIL_H_
	void MYAPI SetFuncGpsDataCallback( _GpsDataCallback pfunc);
#endif
	void MYAPI SetCamEnableSound(BOOL bEnable);
	void MYAPI SetCamEnableVideo(BOOL bEnable);
	void MYAPI SetCamNotifyHandle(void *handle);
	BOOL MYAPI StartCamSrv(const char *cmdline);
	BOOL MYAPI StopCamSrv();
	BOOL MYAPI OpenCamera(const char *szCamID);
	
    void MYAPI CamSrvSetAlarm(BOOL bAlarm);
    BOOL MYAPI CamSrvGetAlarm();

	BOOL MYAPI CloseCamera(const char *szCamID);
	BOOL MYAPI IsCameraOpened(const char *szCamID);
	BOOL MYAPI IsCamSrvStarted();
	void * MYAPI GetCameraHandle(const char *szCamID);
	void * MYAPI GetCameraHandleByIndex(int index);
	void  MYAPI BindPreviewHandle2Cam(void *camHandle, void *hWnd );
	void  MYAPI BindPreviewHandle2Cam2(void *camHandle, void *hWnd ,_PostCamFrame funcPost, int width , int height,CamPixelFormat fmt);
	void  MYAPI CamSrvTakePicture(void *camHandle,const char *szFile);
	int MYAPI CamSrvGetBindPort();
	void MYAPI CamSrvSetRunMode(CamRunMode mode);
	void MYAPI CamSrvSetMaxCams(int MaxCams);

	BOOL  MYAPI SetCamName(void *camHandle, const char *name);
	BOOL  MYAPI IsCamRecording(void *camHandle);
	BOOL  MYAPI StartCamRecording(void *camHandle);
	BOOL  MYAPI StopCamRecording(void *camHandle);
    BOOL  MYAPI CamSrvSetCtl(void *camHandle, int id , void *value);
	int   MYAPI CamSrvDirectSendVideoFrame(void *camHandle, LibCamFrame *frame, TIME_T tm);
	int   MYAPI   CamSrvFrameConvert(const LibCamFrame *fromImg, LibCamFrame *toImg);


    void MYAPI CamSrvSetQuality(LibCamQuality  quality);
	void MYAPI SetVideoCodec(const char *codec);
	void MYAPI SetRecordPath( const char *path, const char *szBlockDev);
	void MYAPI SetCamPreviewRect( const LibCamRect *rt);
	void MYAPI SetNetworkUseSecondaryChannel(BOOL bEnable);

	void MYAPI CamSrvSuspendRecord(void *camHandle,BOOL bSuspend);
	BOOL  MYAPI IsCamRecordSuspend(void *camHandle);
	void MYAPI CamSrvSuspend(BOOL bSuspend, BOOL bCloseCam);
	void MYAPI CamSrvFalabaacParemeter(float quality, float band_width, int speed_level ,int sample_rate );

	void MYAPI CamSrvSetVideoSize(int width,int height);
	void MYAPI CamSrvSetDeviceID(const char *szDevId);
	void MYAPI CamSrvDebugFps(BOOL bEnable);
#ifdef _GPS_UTIL_H_
	TIME_T MYAPI CamSrvGetGpsInfo(GPS_INFO *gpsInfo);
	BOOL   MYAPI CamSrvStartGps(const char *dev, const char *bandrate);
	void   MYAPI CamSrvStopGps();
	BOOL   MYAPI CamSrvIsGpsRunning();
#endif
	/* for android use*/
	void MYAPI SetDirectConnectionNotify(_NotifyDirectConnectionClosed notify);
	BOOL MYAPI AddDirectConnection(SOCKET soc);
	void MYAPI SetCameraPort(int index ,int port);

	/* end */

	typedef BOOL ( MYAPI *_CamSrvAuthentication)(const char *szAddr,const char *szUser,const char *szPwd,const char *szURL,const char *szTimeBegin);
	BOOL MYAPI SetCamSrvAuthenticationCallback( _CamSrvAuthentication func);

	typedef BOOL ( MYAPI *_CamSrvDisconnected)(const char *szAddr,const char *szUser,const char *szPwd,const char *szURL,const char *szTimeBegin,const char *szTimeEnd,int totalSeconds);
	BOOL MYAPI SetCamSrvDisconnectedCallback( _CamSrvDisconnected func);

	typedef BOOL ( MYAPI *_CamSrvError)(int ErrorCode ,const char *szMsg);
	BOOL MYAPI SetCamSrvErrorCallback( _CamSrvError func);

	typedef BOOL ( MYAPI *_CamSrvCameraDetected)(int index ,const char *szName,const char *szCamID);
	BOOL MYAPI SetCamSrvCameraDetectedCallback( _CamSrvCameraDetected func);

	typedef BOOL ( MYAPI *_CamSrvCameraStatus)(int index ,const char *szName,const char *szCamID,int status);
	BOOL MYAPI SetCamSrvCameraStatusCallback( _CamSrvCameraStatus func);

#ifdef __cplusplus
}
#endif	
	
	
	
	
	
	
	
	
	
#endif






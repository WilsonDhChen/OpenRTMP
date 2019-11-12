

#if !defined(_libonvif_h_)
#define _libonvif_h_



#if defined(_NO_CPP_ONVIF_EXPORT)
#define _CPP_ONVIF_EXPORT
#elif !defined(_LIBONVIF_EXPORTS)
#ifdef WIN32
#ifdef _MSC_VER
#define _CPP_ONVIF_EXPORT __declspec(dllimport)
#else
#define _CPP_ONVIF_EXPORT
#endif
#else
#define _CPP_ONVIF_EXPORT
#endif
#endif


#ifdef __cplusplus



#include "libcpputil.h"
#include "libcpputil_var.h"

class _CPP_ONVIF_EXPORT COnvifDeviceInfo
{
public:
    COnvifDeviceInfo();
    virtual ~COnvifDeviceInfo();
    COnvifDeviceInfo(const COnvifDeviceInfo &_this);
    COnvifDeviceInfo & operator = (const COnvifDeviceInfo &_this);
    bool operator == (const COnvifDeviceInfo& _this) const;
    bool operator != (const COnvifDeviceInfo& _this) const;

    CRefString m_epAddress;
    CRefString m_types;
    CRefString m_ip;
    CRefString m_serviceAddress;
    CRefString m_serviceAddress2;
    CRefString m_metadataVersion;
    CRefString m_scopes;
};

class _CPP_ONVIF_EXPORT COnvifFimwareInfo
{
public:
    COnvifFimwareInfo();
    virtual ~COnvifFimwareInfo();
    COnvifFimwareInfo(const COnvifFimwareInfo &_this);
    COnvifFimwareInfo & operator = (const COnvifFimwareInfo &_this);

    CRefString m_brand;
    CRefString m_model;
    CRefString m_sn;
    CRefString m_version;
    CRefString m_id;
};



class  _CPP_ONVIF_EXPORT  COnvifProfile :
    public CObj
{

public:
    _CPP_UTIL_DYNAMIC_DECLARE(COnvifProfile);
    _CPP_UTIL_CLASSNAME_DECLARE(COnvifProfile);
    _CPP_UTIL_QUERYOBJ_DECLARE(COnvifProfile);
public:
    void * operator new(size_t cb);
    void operator delete(void* p);
public:
    COnvifProfile();
    virtual ~COnvifProfile();
public:
    CRefString m_token;
    CRefString m_tokenPTZ;
    CRefString m_name;

    int m_width;
    int m_height;
    bool  m_fixed;
};

class  _CPP_ONVIF_EXPORT  COnvifRecordingInformation :
    public CObj
{

public:
    _CPP_UTIL_DYNAMIC_DECLARE(COnvifRecordingInformation);
    _CPP_UTIL_CLASSNAME_DECLARE(COnvifRecordingInformation);
    _CPP_UTIL_QUERYOBJ_DECLARE(COnvifRecordingInformation);
public:
    void * operator new(size_t cb);
    void operator delete(void* p);
public:
    static CRefString Systime2RangeClock(SYSTEMTIME*tm);
    COnvifRecordingInformation();
    virtual ~COnvifRecordingInformation();
public:
    CRefString m_token;
    CRefString m_status;
    CRefString m_content;
    SYSTEMTIME m_tmBegin;
    SYSTEMTIME m_tmEnd;
    

    //source
    CRefString m_SourceId;
    CRefString m_Name;
    CRefString m_Location;
    CRefString m_Description;
    CRefString m_Address;
};


class _CPP_ONVIF_EXPORT COnvifDevice 
{
public:
    COnvifDevice();
    virtual ~COnvifDevice();
    int GetDeviceInformation();
    int GetServices();
    int GetProfiles();
    int GetRecordingTokens();
    CRefString GetStreamUri(const char *profileToken);
    COnvifRecordingInformation * GetRecordingInformation(const char *recordingToken);  // need releasobj
    CRefString GetReplayStreamUri(const char *recordingToken);


    CRefString XAddrMedia();
    CRefString XAddrEvents();
    CRefString XAddrPTZ();
    CRefString XAddrImaging();
    CRefString XAddrDeviceIO();
    CRefString XAddrAnalytics();
    CRefString XAddrRecording();
    CRefString XAddrSearchRecording();
    CRefString XAddrReplay();


    

public:
    COnvifDeviceInfo m_info;
    COnvifFimwareInfo m_fimwareInfo;

    CRefString m_usr;
    CRefString m_pwd;
    bool  m_needAuth;

    CObjArray m_profiles;    //COnvifProfile
    CObjVarArray m_recordingTokens; // CObjVarRef string
private:
    void *m_onvif;
};


class _CPP_ONVIF_EXPORT COnvifDiscover 
{

public:
    COnvifDiscover();
    virtual ~COnvifDiscover();
    BOOL Setup(const char *bindAddr = NULL, BOOL bIp6 = FALSE);
    BOOL Start();
    void Stop();
    void SetScanInterval(int seconds = -1);
    BOOL SendScan();
public:
    virtual void OnFoundDevice(COnvifDeviceInfo &info);

private:
    void *m_data;
};

#endif


#ifdef __cplusplus
extern "C"
{
#endif
    
    #if defined(WIN32) || defined(_WIN32_WCE)
    #define MYAPI WINAPI
    #else
    #define MYAPI
    #endif   

    typedef void *ONVIF_Discover;
    typedef void *ONVIF_DevInfo;
    typedef void *ONVIF_Dev;
    typedef void *ONVIF_RecInfo;



    typedef void  (MYAPI *_OnvifDiscoverCallback)(void * usrdata, ONVIF_DevInfo devinfo);

     ONVIF_Discover MYAPI Onvif_DiscoverOpen(const char *bindAddr, int scanInterval,
        _OnvifDiscoverCallback callback, void *usrdata);
     BOOL MYAPI Onvif_DiscoverStart(ONVIF_Discover handle);
     BOOL MYAPI Onvif_DiscoverStop(ONVIF_Discover handle);
     void MYAPI Onvif_DiscoverClose(ONVIF_Discover handle);

     void MYAPI Onvif_DevInfoClose(ONVIF_DevInfo handle);
     const char * MYAPI Onvif_DevInfo_epAddress(ONVIF_DevInfo handle);
     const char * MYAPI Onvif_DevInfo_types(ONVIF_DevInfo handle);
     const char * MYAPI Onvif_DevInfo_ip(ONVIF_DevInfo handle);
     const char * MYAPI Onvif_DevInfo_serviceAddress(ONVIF_DevInfo handle);
     const char * MYAPI Onvif_DevInfo_serviceAddress2(ONVIF_DevInfo handle);
     const char * MYAPI Onvif_DevInfo_metadataVersion(ONVIF_DevInfo handle);
     const char * MYAPI Onvif_DevInfo_scopes(ONVIF_DevInfo handle);


     ONVIF_Dev  MYAPI Onvif_DeviceCreate(const char *url, const char *usr, const char *pwd);
     ONVIF_Dev  MYAPI Onvif_DeviceCreateByInfo(ONVIF_DevInfo devinfo);
     void       MYAPI Onvif_DeviceClose(ONVIF_Dev handle);
     int        MYAPI Onvif_DeviceInit(ONVIF_Dev handle);  //return HTTP status
     void       MYAPI Onvif_DeviceSetPwd(ONVIF_Dev handle,const char *use,const char *pwd);
     int        MYAPI Onvif_DeviceGetProfiles(ONVIF_Dev handle,int *count);  //return HTTP status
     const char * MYAPI Onvif_DeviceGetProfileToken(ONVIF_Dev handle, int index);
     int        MYAPI Onvif_DeviceGetPlayUrl(ONVIF_Dev handle, const char *profileToken,
                                char *urlOut,int nLen);

     int        MYAPI Onvif_DeviceGetRecordingTokens(ONVIF_Dev handle, int *count);  //return HTTP status
     const char * MYAPI Onvif_DeviceGetRecordingToken(ONVIF_Dev handle, int index);
     int        MYAPI Onvif_DeviceGetReplayUrl(ONVIF_Dev handle, const char *recordingToken,
         char *urlOut, int nLen);
     BOOL     MYAPI Onvif_DeviceGetRecordingTime(ONVIF_Dev handle, const char *recordingToken, 
         SYSTEMTIME *tmBegin, SYSTEMTIME *tmEnd);
     void     MYAPI Onvif_Systime2RangeClock(SYSTEMTIME *tm, char *urlOut, int nLen);


    







#ifdef __cplusplus
}
#endif









#endif

























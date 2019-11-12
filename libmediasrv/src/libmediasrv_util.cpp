

#include "libmediasrv_def.h"




const char *GetServerInfo()
{
	static char _szServerInfo[] = "f117cdh@gmail.com Media Sever 1.0.0.0";
	return _szServerInfo ;
}
const char *GetMediaServerUsrAgent()
{
	static char _szLibRtspDefUserAgent[] = "LibVLC/2.0.5 (LIVE555 Streaming Media v2012.09.13)";
	return _szLibRtspDefUserAgent;
}
void ParseRTSPTransportLine(const char *szLine , BOOL &bTcp, BOOL &bUdp, int &nPortRTP, int &nPortRTCP)
{
	char szPosts[80];
	bTcp = ( strcmpifind(szLine,"RTP/AVP/TCP") >= 0 ) ;
	bUdp = ( strcmpifind(szLine,"RTP/AVP;") >= 0 ) ;
	if( !bUdp )
	{
		bUdp = ( strcmpifind(szLine,"RTP/AVP/UDP;") >= 0 ) ;
	}
	GetValueFromString(szLine,"client_port",szPosts,sizeof(szPosts),"=",";,: \r\n");

	int isplit = strcmpifind(szPosts,"-");
	if( isplit > 0 )
	{
		szPosts[isplit] = 0 ;
		nPortRTP = atoi(szPosts);
		nPortRTCP = atoi(szPosts + isplit + 1);
	}
	return ;
}

CamCodec Str2CamCodec(const char *szStr)
{
	if( strcmpi( szStr ,"m4v") == 0 || 
		strcmpi( szStr ,"mp4") == 0 ||
		strcmpi( szStr ,"mpeg4") == 0|| 
		strcmpi( szStr ,"mp4v-es") == 0)
	{
		return CamCodecMP4V_ES ;
	}
	else if( strcmpi( szStr ,"h263") == 0 ||
		strcmpi( szStr ,"h263p") == 0  )
	{
		return CamCodecH263 ;
	}
	else if( strcmpi( szStr ,"h264") == 0 )
	{
		return CamCodecH264 ;
	}
	else
	{
		return CamCodecUnknown ;
	}
}
void GetPcr(TIME_T tm, UINT32 &pcr, UINT32 &pcr_ext)
{
    if (tm >= 10)
    {
        tm -= 10;
    }
    pcr = (27000 * tm) / 300;
    pcr_ext = (27000 * tm) % 300;
}
UINT32 GetOStime()
{
#ifdef WIN32
	return timeGetTime();
#elif defined(_IOS_)
	static uint64_t _tickStart = 0 ;
	static mach_timebase_info_data_t _tickInfo;
	uint64_t nanosecs = 0 ;
	uint64_t elapsed = 0 ;
	if ( _tickStart == 0 )
	{
		_tickStart = mach_absolute_time() ;
		if (mach_timebase_info (&_tickInfo) != KERN_SUCCESS) 
		{
			printf ("mach_timebase_info failed\n");
		}
	}
	elapsed =  mach_absolute_time() - _tickStart ;

	nanosecs = elapsed * _tickInfo.numer / _tickInfo.denom;
	int millisecs = nanosecs / 1000000;

	return millisecs;
//#elif defined(__DARWIN_UNIX03) || defined(_Darwin_)
#else
	static TIME_T tupTimeBoot=0;
	TIME_T tupTime = 0;
	struct timeval tv = {0,0};
	if( tupTimeBoot == 0 )
	{
		tupTime = 0;
		gettimeofday(&tv,0);
		tupTimeBoot = tv.tv_sec - tupTime ;
		return (tupTime*1000 + (tv.tv_usec/1000));
	}
	else
	{
		gettimeofday(&tv,0);
		tupTime = tv.tv_sec - tupTimeBoot;
		if( tupTime < 0 )
		{
			tupTime = 0 ;
			tupTimeBoot = tv.tv_sec ;
		}
		return (tupTime*1000 + (tv.tv_usec/1000));
	}
#endif
}

void GetCNAME(char *szBuf,int nLen)
{
	strcpyn(szBuf,"f117cdh@gmail.com",nLen);
}


unsigned char PT(XCodec codec)
{
	switch ( codec )
	{
	case XCodecAudioAAC:
		return 97 ;
	case XCodecAudioAMRNB:
		return 101 ;
	case XCodecMP4V_ES:
		return 96 ;
	case XCodecH263:
		return 96 ;
	case XCodecH264:
		return 96 ;
	case XCodecJPEG:
		return 96 ;
	default:
		return 96 ;
	}
}




















 

#ifdef WIN32
#define _MSWSOCK_
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <direct.h>
#include <tchar.h>
#include <process.h>
#else

#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>

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

#include "adtypes.h"
#include "adapi.h"
#include "rde.h"
#include "libmalloc2.h"
#include "memdbg.h"


#include "adts_decode.h"



CADTSHeader::CADTSHeader()
{
	Clear();
}
CADTSHeader::~CADTSHeader()
{

}
void CADTSHeader::Clear()
{
	sample_rate = 0;
	frame_dur = 0;
	syncword = 0;
	id  = ADTSIdUnknown;
	layer = -1 ;
	profile = ADTSProfileUnknown;
	sampling_frequency_index = ADTSSampleRateUnknown ;
	frame_length = 0 ;
	adts_buffer_fullness = 0 ;
	channels = 0 ;
}
BOOL CADTSHeader::IsValid()
{
	return ( sampling_frequency_index != ADTSSampleRateUnknown ) ;
}
int  CADTSHeader::SampleRate()
{
	return ADTSSampleRate2SampleRate(sampling_frequency_index) ;
}
BOOL CADTSHeader::DecodeADTS(const UINT8 *buf , int nLength)
{
	if( nLength < 7 )
	{
		return FALSE ;
	}
	syncword = (buf[0] << 4) | ( buf[1] >> 4 ) ;

	if( syncword !=  0xFFF )
	{
		return FALSE ;
	}

	id = (ADTSId)((UINT8)(( buf[1] >> 3 ) & 0x01)) ;
	layer = ((UINT8)(( buf[1] >> 1 ) & 0x03)) ;

	profile = (ADTSProfile)((UINT8)( buf[2] >> 6 ) ) ;

	sampling_frequency_index = (ADTSSampleRate)((UINT8)(( buf[2] >> 2 ) & 0x0F)) ;

	sample_rate = CADTSHeader::ADTSSampleRate2SampleRate(sampling_frequency_index);

	channels = ((UINT8)(( buf[2] << 1 ) & 0x04) | ( buf[3] >> 6 )  ) ;

	frame_length = (buf[3]  &  0x03);
	frame_length <<= 8 ;
	frame_length |= buf[4] ;
	frame_length <<= 3 ;
	frame_length |= (buf[5] >> 5 );

	adts_buffer_fullness = ( buf[5]  & 0x1F );
	adts_buffer_fullness <<= 6 ;
	adts_buffer_fullness |= (buf[6] >> 2 );

    frame_dur = 1024.0*1000.0 / SampleRate() / channels;
	return TRUE ;
}
int CADTSHeader::EncodeADTS(UINT8 *buf , int nLength)
{
	if( nLength < 7 )
	{
		return 0 ;
	}
	buf[0] = (UINT8)( syncword >> 4 );
	buf[1] = ( syncword & 0x0F ) ;
	buf[1] <<= 4 ;
	buf[1] |= ( ( id  << 3 ) & 0x08 ) ;
	buf[1] |= 0x01 ;

	buf[2] = ( profile << 6 ) ;
	buf[2] |= ( ( sampling_frequency_index & 0x0F ) << 2 ) ;
	buf[2] |= ( ( ((UINT8)channels) & 0x07 ) >> 2 ) ;
	
	buf[3] = ( ( ((UINT8)channels) & 0x03 ) << 6 ) ;
	buf[3] |= ( ( frame_length >> 11 ) &  0x03 ) ;

	buf[4] = ( ( frame_length & 0x07f8 ) >> 3 ) ;
	buf[5] = ( ( frame_length & 0x07 ) << 5 ) ; 

	buf[5] |= ( ( adts_buffer_fullness & 0x07c0 ) >> 6 ) ; 

	buf[6] = ( ( adts_buffer_fullness & 0x3F ) << 2 ) ; 

	return 7 ;
}
int CADTSHeader::GetAACConfigSampleRateIndex()
{
	return GetAACConfigSampleRateIndex( ADTSSampleRate2SampleRate(sampling_frequency_index) );
}
int CADTSHeader::GetAACConfigData(UINT8 *data)
{
	return GetAACConfigData( ADTSProfile2AACConfig(profile) ,ADTSSampleRate2SampleRate( sampling_frequency_index), channels ,data );
}
int CADTSHeader::GetAACRTPConfig(char *szBuf)
{
    UINT8 audioConfig[2] = {0};
    UINT8 const audioObjectType = ADTSProfile2AACConfig(profile) ;
    audioConfig[0] = (audioObjectType<<3) | (sampling_frequency_index>>1);
    audioConfig[1] = (sampling_frequency_index<<7) | (channels<<3);
    sprintf(szBuf , "%02X%02x", audioConfig[0], audioConfig[1]);
    
    return 4 ;
}
int CADTSHeader::GetAACConfigSampleRateIndex(unsigned int sampleRate)
{


	if (92017 <= sampleRate) return 0;
	if (75132 <= sampleRate) return 1;
	if (55426 <= sampleRate) return 2;
	if (46009 <= sampleRate) return 3;
	if (37566 <= sampleRate) return 4;
	if (27713 <= sampleRate) return 5;
	if (23004 <= sampleRate) return 6;
	if (18783 <= sampleRate) return 7;
	if (13856 <= sampleRate) return 8;
	if (11502 <= sampleRate) return 9;
	if (9391 <= sampleRate) return 10;


	return 11;
}
unsigned int CADTSHeader::ADTSSampleRate2SampleRate(ADTSSampleRate index)
{
	unsigned int sampleRates[13] = { 96000  , 88200 , 64000 , 48000 ,44100 ,32000 ,24000 , 22050 , 16000 , 12000 , 11025 , 8000 , 7350 };

	if( index < 0 || index > 12 )
	{
		return 0 ;
	}

	return sampleRates[ index ];
}
int CADTSHeader::ADTSProfile2AACConfig(ADTSProfile profile)
{
	if( profile == ADTSProfileMain )
	{
		return 1 ;
	}
	else if( profile == ADTSProfileLowComplexity )
	{
		return 2 ;
	}
	else if( profile == ADTSProfileScalableSamplingRate)
	{
		return 3 ;
	}
	else
	{
		return 0 ;
	}
}
ADTSProfile CADTSHeader::AACConfig2ADTSProfile(int profile )
{
	if( profile == 1 )
	{
		return ADTSProfileMain ;
	}
	else if( profile == 2 )
	{
		return ADTSProfileLowComplexity ;
	}
	else if( profile == 3 )
	{
		return ADTSProfileScalableSamplingRate ;
	}
	else
	{
		return ADTSProfileUnknown ;
	}
}
int CADTSHeader::GetAACConfigData(int profile, int samplerate, int channels , UINT8 *data)
{
	if( data == NULL )
	{
		return 0 ;
	}

	samplerate = GetAACConfigSampleRateIndex( samplerate );

	data[0] = 0xAF ;
	data[1] = 0x00 ;

	data[2] = (profile << 3 ) ;
	data[2] |= ( samplerate >> 1 ) ;

	data[3] = ( samplerate & 0x01 );
	data[3] <<= 4 ;
	data[3] |= (UINT8) channels ;
	data[3] <<= 3 ;


	return 4 ;
}
void CADTSHeader::DecodeAACConfigData(  UINT8 *data )
{
	if( data == NULL )
	{
		return  ;
	}
	UINT8 profile1 = 0 ;
	UINT8 samplerate_index1 = 0 ;
	UINT8 channels1 = 0 ;
	ADTSProfile profileTmp ;
	profile1 = ( data[0] >> 3  ) ;

	samplerate_index1 = (( data[0] & 0x07 ) << 1 ) ;
	samplerate_index1 |= ( data[1] >> 7 );

	channels1 = (( data[1] & 0x78 ) >> 3);

	profileTmp = AACConfig2ADTSProfile(profile1) ;
	if( profileTmp == ADTSProfileUnknown )
	{
		return ;
	}
	profile = profileTmp ;
	sampling_frequency_index = (ADTSSampleRate)samplerate_index1 ;
	channels = channels1 ;


	syncword =  0xFFF;
	id = ADTSIdMpeg4 ;
	layer = 0 ;
	adts_buffer_fullness = 0x7FF ;

	frame_dur = 1024.0*1000.0 / SampleRate();
}


































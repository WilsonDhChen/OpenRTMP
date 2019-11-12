

#ifndef _adts_decode_h_
#define _adts_decode_h_

#include "libav_types.h"

class CADTSHeader : public 
	ADTSHeader
{
public:
	static int GetAACConfigSampleRateIndex(unsigned int sampleRate);
	static unsigned int ADTSSampleRate2SampleRate(ADTSSampleRate index);
	static int ADTSProfile2AACConfig(ADTSProfile profile);
	static ADTSProfile AACConfig2ADTSProfile(int profile );
	static int GetAACConfigData(int profile, int samplerate, int channels , UINT8 *data);
public:
	CADTSHeader();
	virtual ~CADTSHeader();
	

	void Clear();
	BOOL IsValid();
	int  SampleRate();

	void DecodeAACConfigData(  UINT8 *data );
	BOOL DecodeADTS(const UINT8 *buf , int nLength);
	int EncodeADTS(UINT8 *buf , int nLength);

	int GetAACConfigSampleRateIndex();
	int GetAACConfigData(UINT8 *data);
    int GetAACRTPConfig(char *szBuf);
};



#endif




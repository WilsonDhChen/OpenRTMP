

#include "libmediasrv_def.h"


#if defined(_USE_X264)
#include"stdint.h"

#ifdef __cplusplus
extern "C"{
#endif
#include "x264.h"
#ifdef __cplusplus
}
#endif

static BOOL FindStartCode3(unsigned char *Buf)
{
	if (Buf[0] != 0 || Buf[1] != 0 || Buf[2] != 1)
	{
		return FALSE; //判断是否为0x000001,如果是返回1
	}
	else
	{
		return TRUE;
	}
}

static BOOL FindStartCode4(unsigned char *Buf)
{
	if (Buf[0] != 0 || Buf[1] != 0 || Buf[2] != 0 || Buf[3] != 1)
	{
		return FALSE;//判断是否为0x00000001,如果是返回1
	}
	else
	{
		return TRUE;
	}
}

BOOL X264_GetSpsPps(int width, int height, int framerate, int avclevel, int avcprofile, int videokeyframe_frequency , CObjBuffer &sps, CObjBuffer &pps, CObjBuffer &sei)
{
	x264_t*handle;
	x264_param_t param;

	x264_nal_t*nal_t = NULL;
	int i_nal = 0 ;
	int size = 0 ;
	int i = 0;

	x264_param_default(&param);

	param.i_log_level = X264_LOG_NONE;
	param.b_repeat_headers = 0;//每个关键帧前是否放SPS,PPS
	param.b_annexb = 1;//0前四字节为nal_size,1以00000001开始
	//	param.i_keyint_min=25;
	//	param.i_keyint_max=25;
	param.i_width = width;
	param.i_height = height;
	param.i_level_idc = avclevel;
	param.i_fps_num = 1;
	param.i_fps_den = framerate;


	//x264_param_apply_profile(&param, "baseline");
	handle = x264_encoder_open(&param);
	

	if ( handle == NULL )
	{
		return FALSE;
	}
	int prefixLen = 0;

	size = x264_encoder_headers(handle, &nal_t, &i_nal);

	for (i = 0; i < i_nal; i++)
	{
		if (NALU_TYPE_SPS == nal_t[i].i_type)
		{
			prefixLen = 0;
			if (FindStartCode4(nal_t[i].p_payload))
			{
				prefixLen = 4;
			}
			else if (FindStartCode3(nal_t[i].p_payload))
			{
				prefixLen = 3;
			}

			sps.Drop();
			sps.Write(nal_t[i].p_payload + prefixLen, nal_t[i].i_payload - prefixLen, 256);
			
		}
		else if (NALU_TYPE_PPS == nal_t[i].i_type)
		{
			prefixLen = 0;
			if (FindStartCode4(nal_t[i].p_payload))
			{
				prefixLen = 4;
			}
			else if (FindStartCode3(nal_t[i].p_payload))
			{
				prefixLen = 3;
			}

			pps.Drop();
			pps.Write(nal_t[i].p_payload + prefixLen, nal_t[i].i_payload - prefixLen, 256);
		}


	}


	x264_encoder_close(handle);

	return TRUE;
}


#endif

























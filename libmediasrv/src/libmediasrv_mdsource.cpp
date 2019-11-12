

#include "libmediasrv_def.h"

#if !defined(_MDSOURCE_MT)



unsigned int CMdSource::SendETS_AAC(UINT8 *buf, int nLen, INT64 tick, UINT64 iAudioFrameQueue)
{


	m_bHaveAudio= TRUE ;


	unsigned int nBytes = 0;
	TIME_T tmNow = OSTickCount();

	CETS2SharedMemVar sharedVar(this, tick, 0, buf, nLen, XCodecAudioAAC);








#if defined(_RTMP_SUPPORT_)
	CETS2RTMPVar rtmpVar(this, tick, 0, RTMPPacketType_Audio);
#endif



	CMtxAutoLock lock(&m_mtxList);
	CObj * const pItemHead = &m_clientList;
	CObj *pItem = NULL;


	if (!m_adtsHeader.IsValid())
	{
		if (!m_adtsHeader.DecodeADTS(buf, nLen))
		{
			return 0;
		}


		m_adtsHeader.GetAACConfigData(m_szAACConfig);

		GenAACConfigStr();

		m_audioType = XCodecAudioAAC;
	}


	pItem = pItemHead->m_pNextObj;

	while (pItem != NULL && pItem != pItemHead)
	{
		CCtx *ctxTmp = Convert2CCtx(pItem);
		pItem = pItem->m_pNextObj;

		switch (ctxTmp->m_typeCtx)
		{

		case CtxTypeRTSP:
		{// CtxTypeRTSP begin
#if defined(_RTSP_SUPPORT_)
			CRTSPNetContext *ctx = (CRTSPNetContext *)(ctxTmp);
			if (ctx->m_status != RtspStatusPlaying)
			{
				continue;
			}
			if (!rtspVar.m_bInitedBuffer)
			{
				if (!rtspVar.ETSCreateAACIoBuffers(buf, nLen))
				{
					continue;
				}
				rtspVar.m_bInitedBuffer = TRUE;
			}

			CObj * const pItemHead = &rtspVar.m_ioList;
			CObjNetIOBufferSharedMemory *pItemTmp = NULL;
			CObjNetIOBufferSharedMemory *iobuf = NULL;
			CObj *pItem = NULL;

			pItem = pItemHead->m_pNextObj;

			while (pItem != NULL && pItem != pItemHead)
			{
				pItemTmp = (CObjNetIOBufferSharedMemory *)pItem;
				pItem = pItem->m_pNextObj;

				iobuf = pItemTmp->Copy();

				if (!ctx->Send(iobuf))
				{
					iobuf->ReleaseObj();
					iobuf = NULL;
				}
				else
				{
					nBytes += pItemTmp->m_nDataLen;
				}

			}
#endif
			break;
		}// CtxTypeRTSP end
		case CtxTypeRTMP:
		{// CtxTypeRTMP begin
#if defined(_RTMP_SUPPORT_)
			CRTMPNetSession *ctx = (CRTMPNetSession *)(ctxTmp);
			CObjNetIOBufferSharedMemory *iobuf = NULL;
			if (!ctx->m_bPlaying)
			{
				continue;
			}

			if (iAudioFrameQueue <= ctx->m_iAudioFrameQueue)
			{
				continue;
			}
			ctx->m_iAudioFrameQueue = iAudioFrameQueue;

#if defined(_AUDIO_WAIT_VIDEO)
			if (m_bHaveVideo &&   !ctx->m_bSendPpsSps)
			{
				continue;
			}
#endif

			if (!rtmpVar.m_bInitedBuffer)
			{
				rtmpVar.m_bInitedBuffer = TRUE;
				if (!rtmpVar.ETSCreateAACIoBuffers(buf, nLen))
				{
					continue;
				}

			}





            if (ctx->m_verAudio != 1)
			{

				iobuf = rtmpVar.CopyAACHeader();
				if (iobuf == NULL)
				{
					continue;
				}
				if (!ctx->Send(iobuf))
				{
					iobuf->ReleaseObj();
					iobuf = NULL;

					continue;
				}
                ctx->m_verAudio = 1;
				iobuf = NULL;


			}



			CObj * const pItemHead = &rtmpVar.m_ioList;
			CObjNetIOBufferSharedMemory *pItemTmp = NULL;

			CObj *pItem = NULL;

			pItem = pItemHead->m_pNextObj;

			while (pItem != NULL && pItem != pItemHead)
			{
				pItemTmp = (CObjNetIOBufferSharedMemory *)pItem;
				pItem = pItem->m_pNextObj;

				iobuf = pItemTmp->Copy();

				if (!ctx->Send(iobuf))
				{
					iobuf->ReleaseObj();
					iobuf = NULL;

				}
				else
				{
					nBytes += pItemTmp->m_nDataLen;
				}

			}
			ctx->Sync();
#endif
			break;
		}// CtxTypeRTMP end
		case CtxTypeHLS:
		{// CtxTypeHLS begin
#if defined(_HLS_SUPPORT_)
			CHLSCtx *ctx = (CHLSCtx *)(ctxTmp);
			if (ctx->m_bClosed)
			{
				continue;
			}
            ctx->SetAudioValid();
			if (!sharedVar.Init())
			{
				continue;
			}
			CObjNetIOBufferSharedMemoryHLS *ioBuf = CObjNetIOBufferSharedMemoryHLS::CopyShared((UINT8 *)(sharedVar.m_sharedMem), nLen, tick, 0, XCodecAudioAAC);
			if (ioBuf == NULL)
			{
				LogPrint(_LOG_LEVEL_ERROR, _TAGNAME, "%s %d, CObjNetIOBufferSharedMemoryHLS::Copy failed, HLS will be closed\n", __FUNCTION__, __LINE__);

				continue;
			}
			if (!ctx->Send(ioBuf))
			{
				ioBuf->ReleaseObj();
				LogPrint(_LOG_LEVEL_ERROR, _TAGNAME, "%s %d, CHLSCtx::Send failed, HLS will be closed\n", __FUNCTION__, __LINE__);
				continue;
			}

#endif
			break;
		}// CtxTypeHLS end
		case CtxTypeFLV:
		{// CtxTypeFLV begin
#if defined(_FLVREC_SUPPORT_)

			CFlvCtx *ctx = (CFlvCtx *)(ctxTmp);
			if (ctx->m_bClosed || ctx->m_bError )
			{
				continue;
			}
			
			flvVar.ReleaseIoBuffers();
			do
			{
				if (!flvVar.m_bInitedBuffer)
				{
					flvVar.m_bInitedBuffer = TRUE;
					if (!flvVar.ETSCreateAACIoBuffers(buf, nLen, TRUE))
					{
						break;
					}
				}

				if (m_flvAACHeaderTag == NULL)
				{
					break;
				}

				if (ctx->m_flvHeader.p == NULL)
				{
					if (m_flvHeader == NULL)
					{
						if (!flvVar.CreateFlvHeader())
						{
							break;
						}
					}
					ctx->m_flvHeader.Attach(m_flvHeader->Copy());
					if (ctx->m_flvHeader.p == NULL)
					{
						break;
					}

				}

                if (ctx->m_verAudio != 1)
                {

                    ctx->m_flvAacHeader.Attach(flvVar.CopyAacHeaderTag(0));
                    if (ctx->m_flvAacHeader.p == NULL)
                    {
                        break;
                    }
                    ctx->m_verAudio = 1;
                    ctx->m_bSendFlvAacHeader = FALSE;
                    
                }
	


				CObj * const pItemHead = &flvVar.m_ioList;
				CObjNetIOBufferSharedMemory *pItemTmp = NULL;
				CObj *pItem = NULL;

				CFlvEvent *flvEv = ctx->CreateFlvEvent();
				int i = 0;
				CObjNetIOBufferSharedMemory *iobuf = NULL;

				if (flvEv == NULL)
				{
					break;
				}
				flvEv->m_codec = XCodecAudioAAC;
				flvEv->m_timestamp_abs = tick;
				flvEv->m_timestamp = flvVar.m_timestamp;


				pItem = pItemHead->m_pNextObj;

				while (pItem != NULL && pItem != pItemHead && i < _MAX_FLV_IOBUFS)
				{
					pItemTmp = (CObjNetIOBufferSharedMemory *)pItem;
					pItem = pItem->m_pNextObj;

					if (i == 0)
					{
						iobuf = pItemTmp->CopyNoShare();
					}
					else
					{
						iobuf = pItemTmp->Copy();
					}

					if (iobuf == NULL)
					{
						break;
					}
					flvEv->m_ioBufs[i] = iobuf;
					i++;
				}

				if (!ctx->Send(flvEv))
				{
					flvEv->ReleaseObj();
				}

			} while (0);

			flvVar.ReleaseIoBuffers();
			
			

#endif
			break;
		}// CtxTypeFLV end

		default:
		{
			break;
		}

		}


	}// loop end







	return nBytes;

}






unsigned int CMdSource::SendETS_H264(UINT8 *buf, int nLen, INT64 tick, int cts, int flagsKeyFrame, UINT64 iVideoFrameQueue)
{
	m_bHaveVideo = TRUE;




	m_videoType = XCodecH264;
	unsigned int nBytes = 0;
	TIME_T tmNow = OSTickCount();

	CETS2SharedMemVar sharedVar(this, tick, cts, buf, nLen, XCodecH264);




#if defined(_RTSPTS_SUPPORT_) || defined(_HTTPTS_SUPPORT_) 
	CETS2TSVar tsVar(this, cts, tick);
#endif



#if defined(_RTMP_SUPPORT_)
	CETS2RTMPVar rtmpVar(this, tick, cts, RTMPPacketType_Video);
#endif
#if defined(_RTSP_SUPPORT_)
	CETS2RTSPVar rtspVar(this, tick, cts, m_videoType);
#endif

	CMtxAutoLock lock(&m_mtxList);
	CObj * const pItemHead = &m_clientList;
	CObj *pItem = NULL;

	pItem = pItemHead->m_pNextObj;

	while (pItem != NULL && pItem != pItemHead)
	{
		CCtx *ctxTmp = Convert2CCtx(pItem);
		pItem = pItem->m_pNextObj;

		switch (ctxTmp->m_typeCtx)
		{

		case CtxTypeRTSP:
		{// CtxTypeRTSP begin
#if defined(_RTSP_SUPPORT_)
			CRTSPNetContext *ctx = (CRTSPNetContext *)(ctxTmp);
			if (ctx->m_status != RtspStatusPlaying)
			{
				continue;
			}
			if (!rtspVar.m_bInitedBuffer)
			{
				if (!rtspVar.ETSCreateH264IoBuffers(buf, nLen))
				{
					continue;
				}
				rtspVar.m_bInitedBuffer = TRUE;
			}

			CObj * const pItemHead = &rtspVar.m_ioList;
			CObjNetIOBufferSharedMemoryRTSP *pItemTmp = NULL;
			CObjNetIOBufferSharedMemoryRTSP *iobuf = NULL;
			CObj *pItem = NULL;

			pItem = pItemHead->m_pNextObj;

			while (pItem != NULL && pItem != pItemHead)
			{
				pItemTmp = (CObjNetIOBufferSharedMemoryRTSP *)pItem;
				pItem = pItem->m_pNextObj;

				iobuf = pItemTmp->Copy();
				ctx->m_tracks[_VIDEO_TRACKID].m_OctetCount += iobuf->m_OctetCount;
				if (!ctx->Send(iobuf))
				{
					iobuf->ReleaseObj();
					iobuf = NULL;
				}
				else
				{
					nBytes += pItemTmp->m_nDataLen;
				}

			}

#endif
			break;
		}// CtxTypeRTSP end
		case CtxTypeRTMP:
		{// CtxTypeRTMP begin
#if defined(_RTMP_SUPPORT_)
			CRTMPNetSession *ctx = (CRTMPNetSession *)(ctxTmp);
			CObjNetIOBufferSharedMemory *iobuf = NULL;
			if (!ctx->m_bPlaying)
			{
				continue;
			}

			if (iVideoFrameQueue <= ctx->m_iVideoFrameQueue)
			{
				continue;
			}
			ctx->m_iVideoFrameQueue = iVideoFrameQueue;

			if (!rtmpVar.m_bInitedBuffer)
			{
				rtmpVar.m_bInitedBuffer = TRUE;
				if (!rtmpVar.ETSCreateH264IoBuffers(buf, nLen))
				{
					continue;
				}
			}

			if (m_rtmpSpsPpsBuf == NULL)
			{
				continue;
			}






			if (!ctx->m_bSendCodecInfo)
			{
				if (m_rtmpMetaDataBuf == NULL)
				{
					if (!rtmpVar.CreateRtmpH264MetaData())
					{
						ctx->Sync();
						continue;
					}
				}
				iobuf = m_rtmpMetaDataBuf->Copy();
				if (iobuf == NULL)
				{
					continue;
				}

				if (!ctx->Send(iobuf))
				{
					iobuf->ReleaseObj();
					iobuf = NULL;

					continue;
				}
				ctx->m_bSendCodecInfo = TRUE;
				iobuf = NULL;



			}

            if (ctx->m_verVideo != 1)
			{

				if (!rtmpVar.m_bKeyFrame)
				{
					continue;
				}

				iobuf = rtmpVar.CopySpsPps();
                if (iobuf == NULL)
                {
                    continue;
                }
				if (!ctx->Send(iobuf))
				{
					iobuf->ReleaseObj();
					iobuf = NULL;

					continue;
				}

                ctx->m_verVideo = 1;
				iobuf = NULL;


			}




			if (rtmpVar.m_header_ioBuf != NULL)
			{
				iobuf = rtmpVar.m_header_ioBuf->Copy();
				if (!ctx->Send(iobuf))
				{

					iobuf->ReleaseObj();
					iobuf = NULL;

				}
				else
				{
					nBytes += rtmpVar.m_header_ioBuf->m_nDataLen;
				}

			}

			CObj * const pItemHead = &rtmpVar.m_ioList;
			CObjNetIOBufferSharedMemory *pItemTmp = NULL;

			CObj *pItem = NULL;

			pItem = pItemHead->m_pNextObj;

			while (pItem != NULL && pItem != pItemHead)
			{
				pItemTmp = (CObjNetIOBufferSharedMemory *)pItem;
				pItem = pItem->m_pNextObj;

				iobuf = pItemTmp->Copy();

				if (!ctx->Send(iobuf))
				{
					iobuf->ReleaseObj();
					iobuf = NULL;

				}
				else
				{
					nBytes += pItemTmp->m_nDataLen;
				}


			}
			ctx->Sync();

#endif
			break;
		}// CtxTypeRTMP end
		case  CtxTypeHLS:
		{// CtxTypeHLS begin
#if defined(_HLS_SUPPORT_)
			CHLSCtx *ctx = (CHLSCtx *)(ctxTmp);
			if (ctx->m_bClosed)
			{
				continue;
			}
            ctx->SetVideoValid();
			if (!sharedVar.Init())
			{
				continue;
			}
			CObjNetIOBufferSharedMemoryHLS *ioBuf = CObjNetIOBufferSharedMemoryHLS::CopyShared((UINT8 *)(sharedVar.m_sharedMem), nLen, tick, cts, XCodecH264);
			if (ioBuf == NULL)
			{
				LogPrint(_LOG_LEVEL_ERROR, _TAGNAME, "%s %d, CObjNetIOBufferSharedMemoryHLS::CopyShared failed, HLS will be closed\n", __FUNCTION__, __LINE__);

				continue;
			}
            ioBuf->m_keyFlag = flagsKeyFrame;
            
			if (!ctx->Send(ioBuf))
			{
				ioBuf->ReleaseObj();
				LogPrint(_LOG_LEVEL_ERROR, _TAGNAME, "%s %d, CHLSCtx::Send failed, HLS will be closed\n", __FUNCTION__, __LINE__);
				continue;
			}

#endif
			break;
		}// CtxTypeHLS end
		case  CtxTypeFLV:
		{// CtxTypeFLV begin
#if defined(_FLVREC_SUPPORT_)
			CFlvCtx *ctx = (CFlvCtx *)(ctxTmp);
			if (ctx->m_bClosed || ctx->m_bError )
			{
				continue;
			}

			flvVar.ReleaseIoBuffers();
			do
			{
				if (!flvVar.m_bInitedBuffer)
				{
					flvVar.m_bInitedBuffer = TRUE;
					if (!flvVar.ETSCreateH264IoBuffers(buf, nLen, TRUE))
					{
						break;
					}
				}

				if (m_flvSpsPpsTag == NULL)
				{
					break;
				}

				if (ctx->m_flvHeader.p == NULL)
				{
					if (m_flvHeader == NULL)
					{
						if (!flvVar.CreateFlvHeader())
						{
							break;
						}
					}
					ctx->m_flvHeader.Attach(m_flvHeader->Copy());
					if (ctx->m_flvHeader.p == NULL)
					{
						break;
					}

				}
                if (ctx->m_verVideo != 1)
                {
            
                    if (!flvVar.m_bKeyFrame)
                    {
                        break;
                    }

                    ctx->m_flvSpsPpsHeader.Attach(flvVar.CopySpsPpsTag(0));

                    if (ctx->m_flvSpsPpsHeader.p == NULL)
                    {
                        break;
                    }
                    ctx->m_bSendFlvSpsPpsHeader = FALSE;
                    ctx->m_verVideo = 1;
                 
                }

	


				CObj * const pItemHead = &flvVar.m_ioList;
				CObjNetIOBufferSharedMemory *pItemTmp = NULL;
				CObj *pItem = NULL;


				CFlvEvent *flvEv = ctx->CreateFlvEvent();
				int i = 0;
				CObjNetIOBufferSharedMemory *iobuf = NULL;

				if (flvEv == NULL)
				{
					break;
				}
				flvEv->m_codec = XCodecH264;
				flvEv->m_bKeyFrame = flvVar.m_bKeyFrame;
				flvEv->m_timestamp_abs = tick;
				flvEv->m_timestamp = flvVar.m_timestamp;


				pItem = pItemHead->m_pNextObj;

				while (pItem != NULL && pItem != pItemHead && i < _MAX_FLV_IOBUFS)
				{
					pItemTmp = (CObjNetIOBufferSharedMemory *)pItem;
					pItem = pItem->m_pNextObj;

					if (i == 0)
					{
						iobuf = pItemTmp->CopyNoShare();
					}
					else
					{
						iobuf = pItemTmp->Copy();
					}

					if (iobuf == NULL)
					{
						break;
					}
					flvEv->m_ioBufs[i] = iobuf;
					i++;


				}

				if (!ctx->Send(flvEv))
				{
					flvEv->ReleaseObj();
				}
			} while (0);
			flvVar.ReleaseIoBuffers();

			

#endif
			break;
		}// CtxTypeFLV end
		default:
		{
			break;
		}

		}


	}// loop end







	return nBytes;
}




























#endif
















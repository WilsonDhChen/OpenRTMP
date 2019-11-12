
#ifndef _libmediasrv_ts_h_
#define _libmediasrv_ts_h_



#if defined(_RTSPTS_SUPPORT_) || defined(_HLS_SUPPORT_) || defined(_HTTPTS_SUPPORT_) 




class CTSIOCtx :
	public CTSCtx
{
public:
	CTSIOCtx();
	virtual ~CTSIOCtx();
	virtual CObjNetIOBuffer *AllocTSIOBuffer();

	void InitTSConnection(CObj &ioList);
	int  SendPATandPMT( int count,CObj &ioList);
    BOOL CheckPATPMT( TIME_T tmNow, CObj &ioList);
    BOOL CheckPCR(BOOL &bSendPCR, TIME_T tmNow);

public:
	BOOL   m_bHaveInitPatPmt;
	TIME_T m_tmLastSendPATPMT ;
	TIME_T m_tmLastSendPCR;
	TIME_T m_firstTSFrameTime;

    int    m_ts_patpmt_interval;
    BOOL   m_bAudioValid;
    BOOL   m_bVideoValid;
	
};

class CMdSource;
class CETS2TSVar
{
public:
	CETS2TSVar();
	CETS2TSVar(CMdSource *mdsrc, int  cts, INT64 tick);
	void SetPar(CMdSource *mdsrc, int  cts, INT64 tick);
	virtual ~CETS2TSVar();
public:
	BOOL m_bSendPCR;
	UINT32 m_timestamp;
	CObj m_ioList;
	BOOL m_bInitedBuffer;
	int  m_cts;
	UINT32 m_pts;
	UINT32 m_dts;
	UINT32 m_pcr;
    UINT32 m_pcr_ext;
	int    m_got_dts;
};

#endif















#endif








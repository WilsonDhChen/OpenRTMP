

#ifndef _libmediasrv_netbuffer_h_
#define _libmediasrv_netbuffer_h_


typedef enum tagRtspTransport
{
	RtspTransportNone ,
	RtspTransportTCP ,
	RtspTransportUDP ,
}RtspTransport;
typedef enum tagIoBuffer
{
	IoBufferUnknown ,
	IoBufferOPTIONS ,
	IoBufferClose ,
	IoBufferSETUP ,
	IoBufferDESCRIBE ,
	IoBufferPLAY ,
	IoBufferTEARDOWN ,
	IoBufferPAUSE ,
	IoBufferJOIN ,
	IoBufferGET_PARAMETER ,
	IoBufferSET_PARAMETER ,
	IoBufferBad ,
	IoBufferNotFound ,
	IoBufferNotSupported ,
	IoBufferUnsupportedTransport ,
	IoBufferRTP ,
	IoBufferRTSP ,
	IoBufferRTCP ,
	IoBufferCONNECTIONS,
}IoBuffer;



class   CRTSPNetIOBuffer :
	public CObjNetIOBuffer
{
public:
	static CRTSPNetIOBuffer * CreateObj(  CRTSPNetIOBuffer *src = NULL , int size = 0 );
	_CPP_UTIL_CLASSNAME_DECLARE(CRTSPNetIOBuffer) ;
	_CPP_UTIL_QUERYOBJ_DECLARE(CObjNetIOBuffer) ;

	CRTSPNetIOBuffer( CRTSPNetIOBuffer *src = NULL , int size = 0 );
	virtual ~CRTSPNetIOBuffer();

public:
	IoBuffer m_bufType ;
	
private:
	void *m_BufShared;

};




#endif




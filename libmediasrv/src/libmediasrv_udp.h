


#ifndef _libcamsrv_udp_h_
#define _libcamsrv_udp_h_


#if defined(__WIN32__) || defined(_WIN32)
#define TTL_TYPE int
#else
#define TTL_TYPE u_int8_t
#endif

class CRtspUDP :
	public CObj ,
	public CObjThread
{
	_CPP_UTIL_DYNAMIC(CRtspUDP)  ;
	_CPP_UTIL_CLASSNAME(CRtspUDP)  ;
	_CPP_UTIL_QUERYOBJ(CObj)  ;
public:
	CRtspUDP();
	~CRtspUDP();
public:
	BOOL Open();
	void Close();
	BOOL SendRTP(CObjNetIOBuffer *pBuf,INT_IP addr ,int port);
	BOOL SendRTCP(CObjNetIOBuffer *pBuf,INT_IP addr ,int port);
	int  GetPortRTP();
	int  GetPortRTCP();
private:
	SOCKET m_socRTP;
	SOCKET m_socRTCP;

	int m_portRTP;
	int m_portRTCP;
	BOOL m_bSetTTL;
};



#endif




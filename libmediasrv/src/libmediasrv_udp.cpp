
#include "libmediasrv_def.h"

int _portRTP = 0 ;
int _portRTCP = 0 ;

CRtspUDP::CRtspUDP()
{
	m_socRTP = INVALID_RDE ;
	m_socRTCP = INVALID_RDE ;
	m_portRTP = 0 ;
	m_portRTCP = 0 ;
	m_bSetTTL = FALSE ;
}
CRtspUDP::~CRtspUDP()
{
	RDE_Close( m_socRTP );
	RDE_Close( m_socRTCP );
	m_socRTP = INVALID_RDE ;
	m_socRTCP = INVALID_RDE ;
}
void CRtspUDP::Close()
{
	if( m_socRTP != INVALID_RDE )
	{
		RDE_Close( m_socRTP );
	}
	if( m_socRTCP != INVALID_RDE )
	{
		RDE_Close( m_socRTCP );
	}	
	m_socRTP = INVALID_RDE ;
	m_socRTCP = INVALID_RDE ;
}
BOOL CRtspUDP::Open()
{
	m_socRTP = RDEU_Open("",_portRTP);
	if( m_socRTP == INVALID_RDE  )
	{
		return FALSE ;
	}
	m_socRTCP = RDEU_Open("",_portRTCP);
	if( m_socRTCP == INVALID_RDE  )
	{
		RDE_Close( m_socRTP );
		m_socRTP = INVALID_RDE ;
		return FALSE ;
	}

	RDE_GetBindName( m_socRTP , NULL , &m_portRTP ,NULL);
	RDE_GetBindName( m_socRTCP , NULL , &m_portRTCP ,NULL);


	return TRUE ;
}
int  CRtspUDP::GetPortRTP()
{
	return  m_portRTP ;
}
int  CRtspUDP::GetPortRTCP()
{
	return  m_portRTCP ;
}
BOOL CRtspUDP::SendRTP(CObjNetIOBuffer *ioBuf,INT_IP addr ,int port)
{
	if( m_socRTP == INVALID_RDE || ioBuf == NULL )
	{
		return FALSE ;
	}


	if( m_bSetTTL )
	{
		TTL_TYPE ttl = (TTL_TYPE)255;
		if (setsockopt(m_socRTP, IPPROTO_IP, IP_MULTICAST_TTL,
			(const char*)&ttl, sizeof ttl) < 0) 
		{
			return FALSE ;
		}
	}


	RDEU_WriteEx(m_socRTP,ioBuf->m_pBuf,ioBuf->m_nDataLen,addr,port);
	ioBuf->ReleaseObj();
	return TRUE ;
}
BOOL CRtspUDP::SendRTCP(CObjNetIOBuffer *ioBuf,INT_IP addr ,int port)
{
	if( m_socRTCP == INVALID_RDE || ioBuf == NULL  )
	{
		return FALSE ;
	}

	if( m_bSetTTL )
	{
		TTL_TYPE ttl = (TTL_TYPE)255;
		if (setsockopt(m_socRTP, IPPROTO_IP, IP_MULTICAST_TTL,
			(const char*)&ttl, sizeof ttl) < 0) 
		{
			return FALSE ;
		}
	}


	RDEU_WriteEx(m_socRTCP,ioBuf->m_pBuf,ioBuf->m_nDataLen,addr,port);
	ioBuf->ReleaseObj();
	return TRUE ;
}
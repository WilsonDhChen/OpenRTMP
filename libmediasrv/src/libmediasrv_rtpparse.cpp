


#include "libmediasrv_def.h"




CRtpParse::CRtpParse( void * buf)
{
	m_buf = (unsigned char *)buf ;
}
void CRtpParse::Reset(void * buf)
{
	m_buf = (unsigned char *)buf ;
}
void CRtpParse::Clear()
{
	memset( m_buf , 0 , 12 ) ;
}
void CRtpParse::V(unsigned char v)
{
	unsigned char v1 = m_buf[0]&0x3F ;
	v = v<< 6 ;
	m_buf[0] = v|v1;

}
unsigned char CRtpParse::V()
{
	unsigned char v = 0 ;
	v = m_buf[0]& 0xC0 ;
	v = v >> 6 ;
	return v ;
}
void CRtpParse::P(unsigned char p)
{
	unsigned char p1 = m_buf[0]&0xDF ;
	p &= 0x01 ;
	p = p << 5 ;
	m_buf[0] = p|p1;
}
unsigned char CRtpParse::P()
{
	unsigned char p = 0 ;
	p = m_buf[0]& 0x20 ;
	p = p >> 5 ;
	return p ;
}
void CRtpParse::X(unsigned char x)
{
	unsigned char x1 = m_buf[0]&0xEF ;
	x &= 0x01 ;
	x = x << 4 ;
	m_buf[0] = x|x1;
}
unsigned char CRtpParse::X()
{
	unsigned char x = 0 ;
	x = m_buf[0]& 0x10 ;
	x = x >> 4 ;
	return x ;
}
void CRtpParse::CC(unsigned char cc)
{
	cc &= 0x0F ;
	m_buf[0]|= cc ;
}
unsigned char CRtpParse::CC()
{
	return (m_buf[0]&0x0F) ;
}
void CRtpParse::M(unsigned char m)
{
	unsigned char m1 = m_buf[1]&0x7F ;
	m = m << 7 ;
	m_buf[1] = m|m1;
}
unsigned char CRtpParse::M()
{
	return (m_buf[0] >> 7 ) ;
}
void CRtpParse::PT(unsigned char pt)
{
	pt &= 0x7F ;
	m_buf[1] |= pt ;
}
unsigned char CRtpParse::PT()
{
	return ( m_buf[1]&0x7F ) ;
}
void CRtpParse::SequenceNumber( UINT16 sq )
{
	sq = htons( sq );
	memcpy( m_buf+2 , &sq , 2 ) ;
}
UINT16  CRtpParse::SequenceNumber()
{
	UINT16 sq = 0 ;
	memcpy( &sq , m_buf+2  , 2 ) ;

	return ntohs(sq);
}
void CRtpParse::Timestamp( UINT32 ts )
{
	ts = htonl(ts) ;
	memcpy( m_buf+4 , &ts , 4 ) ;
}
UINT32 CRtpParse::Timestamp()
{
	UINT32 ts = 0 ;
	memcpy( &ts , m_buf+4  , 4 ) ;

	return ntohl(ts);
}
void CRtpParse::SSRC( UINT32 ssrc )
{
	ssrc = htonl(ssrc) ;
	memcpy( m_buf+8 , &ssrc, 4 ) ;
}
UINT32  CRtpParse::SSRC()
{
	UINT32 ssrc = 0 ;
	memcpy( &ssrc , m_buf+8  , 4 ) ;

	return ntohl(ssrc);
}
void CRtpParse::CSRC( unsigned char index , UINT32 csrc )
{
	if( index > 0x0F )
	{
		return ;
	}
	csrc = htonl(csrc) ;
	memcpy( m_buf+12+(index*4) , &csrc, 4 ) ;
}
UINT32  CRtpParse::CSRC( unsigned char index )
{
	if( index > 0x0F )
	{
		return (UINT32)-1;
	}
	UINT32 csrc = 0 ;
	memcpy( &csrc , m_buf+12+(index*4)  , 4 ) ;

	return ntohl(csrc);
}
unsigned char *CRtpParse::stream()
{
	unsigned char cc = CC();

	return ( m_buf+12+(cc*4) );
}
int CRtpParse::HeadSize()
{
	return (CC()*4+12);
}
//////////////////////////////////////////////////////////////////////
CRtcpParse::CRtcpParse( void * buf)
{
	m_buf = (unsigned char *)buf ;
}
CRtcpParse::~CRtcpParse()
{

}
void CRtcpParse::Reset(void * buf)
{
	m_buf = (unsigned char *)buf ;
}
void CRtcpParse::V(unsigned char v)
{
	unsigned char v1 = m_buf[0]&0x3F ;
	v = v<< 6 ;
	m_buf[0] = v|v1;
}
unsigned char CRtcpParse::V()
{
	unsigned char v = 0 ;
	v = m_buf[0]& 0xC0 ;
	v = v >> 6 ;
	return v ;
}

void CRtcpParse::P(unsigned char p)
{
	unsigned char p1 = m_buf[0]&0xDF ;
	p &= 0x01 ;
	p = p << 5 ;
	m_buf[0] = p|p1;
}
unsigned char CRtcpParse::P()
{
	unsigned char p = 0 ;
	p = m_buf[0]& 0x20 ;
	p = p >> 5 ;
	return p ;
}

void CRtcpParse::RC_SC(unsigned char rc)
{
	rc &= 0x1F ;
	m_buf[0]|= rc ;
}
unsigned char CRtcpParse::RC_SC()
{
	return (m_buf[0]&0x1F) ;
}


void CRtcpParse::PT(unsigned char pt)
{
	m_buf[1] = pt ;
}
unsigned char CRtcpParse::PT()
{
	return m_buf[1] ;
}

void CRtcpParse::Length( UINT16 len )
{
	len = htons(len);
	memcpy( m_buf+2,&len , 2 );

}
UINT16 CRtcpParse::Length()
{
	UINT16 len ;
	memcpy( &len ,m_buf+2 , 2 );

	return ntohs(len);
}
void CRtcpParse::writeUINT32(int index ,UINT32 val)
{
	val = htonl( val ) ;
	memcpy( m_buf + (index)*4 ,&val ,4 ) ;

}
UINT32 CRtcpParse::readUINT32(int index )
{
	UINT32 val;
	memcpy( &val , m_buf + (index)*4  ,4 ) ;
	return ntohl( val ) ;
}
////////////////////////////////////////////////////////////////////////
CH264NaluParse::CH264NaluParse( unsigned char * buf , int nLen  )
{
	m_pBuf = buf ;
	m_nBufLen = nLen;
	m_iSeak = 0 ;
}
CH264NaluParse::~CH264NaluParse()
{
}
BOOL CH264NaluParse::FindStartCode3 (unsigned char *Buf)
{
	if(Buf[0]!=0 || Buf[1]!=0 || Buf[2] !=1) 
	{
		return FALSE; //判断是否为0x000001,如果是返回1
	}
	else 
	{
		return TRUE;
	}
}

BOOL CH264NaluParse::FindStartCode4 (unsigned char *Buf)
{
	if(Buf[0]!=0 || Buf[1]!=0 || Buf[2] !=0 || Buf[3] !=1) 
	{
		return FALSE;//判断是否为0x00000001,如果是返回1
	}
	else 
	{
		return TRUE;
	}
}
BOOL CH264NaluParse::GetNextNalu(NALU_T *nalu, BOOL *pbEnd )
{


	BOOL bStart3 = FALSE ;
	BOOL bStart4 = FALSE ;
	BOOL bFoundNextStartCode = FALSE ;
	int  iSeekBegin = 0 ;
	int  nRemain = 0 ;

	nRemain = m_nBufLen - m_iSeak ;
	if( pbEnd != NULL )
	{
		*pbEnd = FALSE ;
	}

	if( nRemain < 4 )
	{
		if( pbEnd != NULL )
		{
			*pbEnd = TRUE ;
		}
		return FALSE ;
	}
	if (nRemain > 4)
	{
		bStart4 = FindStartCode4(m_pBuf+m_iSeak);
	}
	
	if( !bStart4 )
	{
		if ( nRemain > 3)
		{
			bStart3 = FindStartCode3(m_pBuf+m_iSeak);
		}
		
		if( !bStart3 )
		{
			return FALSE ;
		}
		else
		{
			nalu->startcodeprefix_len = 3 ;
			m_iSeak += 3 ;
		}

	}
	else
	{
		nalu->startcodeprefix_len = 4 ;
		m_iSeak += 4 ;
	}
	iSeekBegin = m_iSeak ;
	nRemain = m_nBufLen - m_iSeak ;

	if (nRemain < 1)
	{
		return FALSE ;
	}

	nalu->buf = m_pBuf + iSeekBegin ;
	nalu->forbidden_bit = nalu->buf[0] & 0x80; //1 bit
	nalu->nal_reference_idc = nalu->buf[0] & 0x60; // 2 bit
	nalu->nal_unit_type = (NaluType)((nalu->buf[0]) & 0x1f);// 5 bit
	nalu->lost_packets = 0 ;


	bStart3 = FALSE ;
	bStart4 = FALSE ;
	bFoundNextStartCode = FALSE ;

	while ( !bFoundNextStartCode )
	{
		if( m_iSeak >= m_nBufLen )
		{
			nalu->len = m_nBufLen - iSeekBegin;  
			if( pbEnd != NULL )
			{
				*pbEnd = TRUE ;
			}
			return ( nalu->len > 0 );
		}

		m_iSeak ++ ;
		nRemain = m_nBufLen - m_iSeak ;

		if (nRemain >= 4)
		{
			bStart4 = FindStartCode4(m_pBuf+m_iSeak);
		}
		
		if( !bStart4 )
		{
			if (nRemain >= 3)
			{
				bStart3 = FindStartCode3(m_pBuf+m_iSeak);
			}
			
		}

		bFoundNextStartCode = ( bStart4 || bStart3 );
	}
	nalu->len = m_iSeak - iSeekBegin;  

	return ( nalu->len > 0 );
}
////////////////////////////////////////////////////////////////////////
void * CH264NaluItem:: operator new( size_t cb )
{
	return malloc2(cb);
}
void CH264NaluItem:: operator delete( void* p )
{
	free2(p);
}
CH264NaluItem::CH264NaluItem( )
{
	memset( &m_nalu , 0 , sizeof(m_nalu) );

	
}
CH264NaluItem::~CH264NaluItem()
{

}
void CH264NaluItem::CopyNalu(NALU_T *nalu)
{
	memcpy( &m_nalu , nalu , sizeof(m_nalu) );
}
const NALU_T *CH264NaluItem::nalu()
{
	return &m_nalu;
}


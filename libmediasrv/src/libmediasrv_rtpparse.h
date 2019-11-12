

#ifndef _libcamsrv_rtpparse_h_
#define _libcamsrv_rtpparse_h_



class CRtpParse
{
public:
	CRtpParse( void * buf = NULL);
	void Reset(void * buf);
	void V(unsigned char v);
	unsigned char V();

	void P(unsigned char p);
	unsigned char P();

	void X(unsigned char x);
	unsigned char X();


	void CC(unsigned char cc);
	unsigned char CC();

	void M(unsigned char m);
	unsigned char M();

	void PT(unsigned char pt);
	unsigned char PT();

	void SequenceNumber( UINT16 sq );
	UINT16 SequenceNumber();

	void Timestamp( UINT32 ts );
	UINT32  Timestamp();

	void SSRC( UINT32 ssrc );
	UINT32  SSRC();

	void CSRC( unsigned char index , UINT32 csrc );
	UINT32  CSRC( unsigned char index );

	void Clear();

	int HeadSize();
	unsigned char *stream();
private:
	unsigned char *m_buf;
};

class CRtcpParse
{
public:
	typedef enum tagRtcpPtType
	{
		RtcpPtTypeSR = 200 ,
		RtcpPtTypeRR = 201 ,
		RtcpPtTypeSDES = 202 ,
		RtcpPtTypeBYE = 203 , 

	}RtcpPtType;
public:
	CRtcpParse( void * buf = NULL);
	virtual ~CRtcpParse();
	void Reset(void * buf);
	void V(unsigned char v);
	unsigned char V();

	void P(unsigned char p);
	unsigned char P();

	void RC_SC(unsigned char x);
	unsigned char RC_SC();


	void PT(unsigned char pt);
	unsigned char PT();

	void Length( UINT16 sq );
	UINT16 Length();

	void   writeUINT32(int index ,UINT32 val);
	UINT32 readUINT32(int index );

private:
	unsigned char *m_buf;
};


class   CH264NaluParse :
	public CObj
{

public:

	_CPP_UTIL_CLASSNAME(CH264NaluParse) ;
	_CPP_UTIL_QUERYOBJ(CObj) ;

	CH264NaluParse( unsigned char * buf , int nLen );
	virtual ~CH264NaluParse();

	BOOL GetNextNalu(NALU_T *nalu , BOOL *pbEnd = NULL );
	static BOOL FindStartCode3 (unsigned char *Buf);
	static BOOL FindStartCode4 (unsigned char *Buf);

private:
	unsigned char *m_pBuf;
	int m_nBufLen;
	int m_iSeak;
};

class   CH264NaluItem :
	public CObj
{
public:
	void * operator new( size_t cb );
	void operator delete( void* p );
public:
	_CPP_UTIL_DYNAMIC(CH264NaluItem)  ;
	_CPP_UTIL_CLASSNAME(CH264NaluItem) ;
	_CPP_UTIL_QUERYOBJ(CObj) ;

	CH264NaluItem( );
	virtual ~CH264NaluItem();
	void CopyNalu(NALU_T *nalu);
	const NALU_T *nalu();

private:
	NALU_T m_nalu;
};






#endif




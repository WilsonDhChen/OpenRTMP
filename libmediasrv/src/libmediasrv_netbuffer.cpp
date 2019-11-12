

#include "libmediasrv_def.h"


////////////////////////////////////////////
_CPP_UTIL_CLASSNAME_IMP(CRTSPNetIOBuffer)
_CPP_UTIL_QUERYOBJ_IMP(CRTSPNetIOBuffer,CObjNetIOBuffer)
CRTSPNetIOBuffer * CRTSPNetIOBuffer::CreateObj( CRTSPNetIOBuffer *src , int size  )
{
		CRTSPNetIOBuffer *pObj = NULL ; 
		pObj = new CRTSPNetIOBuffer(  src  , size) ; 
		if( pObj == NULL ) 
		{ 
			return NULL ; 
		} 
		pObj->m_bAlloced = TRUE ; 
		pObj->m_nObjSize = sizeof(CRTSPNetIOBuffer) ; 
		pObj->AddObjRef(); 
		return pObj ; 
}
CRTSPNetIOBuffer::CRTSPNetIOBuffer( CRTSPNetIOBuffer *src ,  int size  )
{
	if( src == NULL )
	{
		if( size == 0 )
		{
			size = SIZE_8KB ;
		}
		m_BufShared = SharedMemCreate( AlignLength(size , SIZE_4KB) );
		m_pBuf = (char *)m_BufShared ;
		m_nBufLen = SharedMemSize(m_BufShared) -1 ;
		m_pBuf[m_nBufLen] = 0 ;
		m_bufType = IoBufferUnknown ;

		memset(m_pBuf , 0 , 20 ) ;
	}
	else
	{
		m_BufShared = src->m_BufShared ;
		m_nDataLen = src->m_nDataLen ;
		m_nLenTrans = src->m_nLenTrans ;
		SharedMemAddRef( m_BufShared );
		m_pBuf = (char *)m_BufShared ;
		m_nBufLen = SharedMemSize(m_BufShared) -1 ;
		m_pBuf[m_nBufLen] = 0 ;
		
		m_bufType = IoBufferUnknown ;

	}

}
CRTSPNetIOBuffer::~CRTSPNetIOBuffer()
{
	SharedMemRelease( m_BufShared );
	m_BufShared = NULL ;


}



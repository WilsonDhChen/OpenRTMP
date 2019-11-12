
#include"rde_def.h"


#define DATA_SUM 254



void MYAPI PKG_Fill(void  *pBuf,int nLenPkg)
{
#ifdef _DYN_PKG_
	int nTmp = 0 ;
#endif
	PPKG_HEAD pHead=(PPKG_HEAD)pBuf;
	if(!pHead)
	{
		return;
	}


#ifdef _DYN_PKG_

	
#ifdef WIN32
	srand(GetTickCount());
#else
	srand(time(NULL)+ (INT64)pBuf );
#endif

	nTmp = (DATA_SUM - 134) ;
	if( nTmp == 0 )
	{
		nTmp = 3 ;
	}
	pHead->Marker[0]  = rand() % nTmp ;

#ifdef WIN32
	srand(GetTickCount()+(INT64)pBuf);
#else
	srand(time(NULL)+ (INT64)pBuf + (INT64)pBuf );
#endif

	nTmp = (( DATA_SUM - pHead->Marker[0] - rand()% 66 ) ) ;
	if( nTmp == 0 )
	{
		nTmp = 3 ;
	}

	pHead->Marker[1]  = rand() % nTmp;


#ifdef WIN32
	srand(GetTickCount()+(INT64)pBuf + 5);
#else
	srand(time(NULL)+ (INT64)pBuf + (INT64)pBuf + 6);
#endif

	nTmp = ( DATA_SUM - pHead->Marker[0] - pHead->Marker[1] - rand()% 15 ) ;
	if( nTmp == 0 )
	{
		nTmp = 3 ;
	}

	pHead->Marker[2]  = rand() %  nTmp;

	pHead->Marker[3]  = DATA_SUM - pHead->Marker[0] - pHead->Marker[1] - pHead->Marker[2] ;

	

#else
	pHead->Marker[0] = _PKG_0 ;
	pHead->Marker[1] = _PKG_1 ;
	pHead->Marker[2] = _PKG_2 ;
	pHead->Marker[3] = _PKG_3 ;
#endif

	pHead->nLenPkg=htonl(nLenPkg);
}
int MYAPI IsPkgHead(void  *pBuf)
{
	PPKG_HEAD pHead=(PPKG_HEAD)pBuf;
	if(!pHead)
	{
		return 0;
	}
#ifdef _DYN_PKG_
	if(  ( pHead->Marker[0] + pHead->Marker[1] + pHead->Marker[2] + pHead->Marker[3] ) != DATA_SUM  )
	{
		return 0 ;
	}
#else
	if( pHead->Marker[0] != _PKG_0 ) return 0 ;
	if( pHead->Marker[1] != _PKG_1 ) return 0 ;
	if( pHead->Marker[2] != _PKG_2 ) return 0 ;
	if( pHead->Marker[3] != _PKG_3 ) return 0 ;
#endif

	if( ntohl(pHead->nLenPkg)<0)
	{
		return 0;
	}
	return 1;
}
int MYAPI PKG_GetPkgLen(void  *pBuf)
{
	PPKG_HEAD pHead=(PPKG_HEAD)pBuf;
	if(!pHead)return 0;
	return ntohl(pHead->nLenPkg);
}
int MYAPI PKG_GetHeadLen()
{
	return sizeof(PKG_HEAD);
}
void *MYAPI PKG_Alloc(int size)
{
	return _MEM_ALLOC(size);
}
void MYAPI PKG_Free(void *p)
{
	_MEM_FREE_(p);
}
int MYAPI  PKG_Write(SOCKET h_rde,const void *buffer,int size)
{
	PKG_HEAD PkgHead;
	int nWritten;
	if(size==-1)
	{
		size=strlenx((char *)buffer)+1;
	}
	PKG_Fill(&PkgHead,size);
	if(RDE_Write(h_rde,&PkgHead,sizeof(PkgHead))!=sizeof(PkgHead))
	{
		return PKG_ERROR_SOCKET;
	}
    if( size == 0 )
    {
        return 0 ;
    }
	nWritten=RDE_Write(h_rde,buffer,size);
	if(nWritten!=size)
    {
        return PKG_ERROR_SOCKET;
    }
	return nWritten;
}
int MYAPI  PKG_Read(SOCKET h_rde,void *buffer,int size ,int *pnPkgLen)
{
	PKG_HEAD PkgHead;
	int nReaded;
	int lenPkg = 0 ;
	memset(&PkgHead,0,sizeof(PkgHead));
	nReaded=RDE_ReadLoop(h_rde,&PkgHead,sizeof(PkgHead));
	if(nReaded!=sizeof(PkgHead))
    {
        return PKG_ERROR_SOCKET;
    }

	if(!IsPkgHead(&PkgHead))
    {
        return PKG_ERROR_PKGHEAD;
    }
	lenPkg = PKG_GetPkgLen(&PkgHead);
	if( !buffer || lenPkg > size )
	{
		if( pnPkgLen )
		{
			*pnPkgLen = lenPkg;
		}
		return PKG_ERROR_READBUF_SMALL;
	}

	nReaded=RDE_ReadLoop(h_rde,buffer,lenPkg);
	if(nReaded!=lenPkg)
    {
        return PKG_ERROR_SOCKET;
    }
	return nReaded;
}
int MYAPI  PKG_ReadPkgLen(SOCKET h_rde)
{
	PKG_HEAD PkgHead;
	int nReaded;
	memset(&PkgHead,0,sizeof(PkgHead));
	nReaded=RDE_Read(h_rde,&PkgHead,sizeof(PkgHead));
	if(nReaded!=sizeof(PkgHead))return PKG_ERROR_SOCKET;
	if(!IsPkgHead(&PkgHead))return PKG_ERROR_PKGHEAD;
	
	return PKG_GetPkgLen(&PkgHead);
}
int MYAPI  PKG_ReadAlloc(SOCKET h_rde,void **pBuffer)
{
	PKG_HEAD PkgHead;
	int nReaded;
	char *buffer;
	int size;
	int lenPkg = 0 ;
	if(!pBuffer)return 0;
	*pBuffer=0;
	memset(&PkgHead,0,sizeof(PkgHead));
	nReaded=RDE_Read(h_rde,&PkgHead,sizeof(PkgHead));
	if(nReaded!=sizeof(PkgHead))return PKG_ERROR_SOCKET;

	if(!IsPkgHead(&PkgHead))return PKG_ERROR_PKGHEAD;

	lenPkg = PKG_GetPkgLen(&PkgHead);

	size=lenPkg+1;
	buffer=(char *)PKG_Alloc(size);
	if(!buffer)return PKG_ERROR_READBUF_SMALL;
	

	nReaded=RDE_ReadLoop(h_rde,buffer,size);
	buffer[lenPkg]=0;
	if(nReaded!=lenPkg)
	{
		PKG_Free(buffer);
		*pBuffer=0;
		return PKG_ERROR_SOCKET;
	}
	*pBuffer=buffer;
	return nReaded;
}


















































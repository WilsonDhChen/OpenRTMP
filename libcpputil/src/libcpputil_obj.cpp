

#include "libcpputil_def.h"





_CPP_UTIL_DYNAMIC_IMP(CObj)
_CPP_UTIL_CLASSNAME_IMP(CObj)

void * CObj::operator new( size_t cb )
{


     return malloc(cb) ;
}
void CObj::operator delete( void* p )
{


    free(p);
}
#if defined(_DEBUG_OBJ_REF_)
INT32 _nCObjUtil = 0 ;
#endif
CObj::CObj()
{
#if defined(_DEBUG_OBJ_REF_)
    MtxInterlockedInc(&_nCObjUtil) ;
#endif
    m_nObjSize = 0 ;
    m_nListCount = 0 ;
    m_nRef = 0 ;
    m_bAlloced = FALSE;
    m_pPreObj = this;
    m_pNextObj = this;
}
CObj::~CObj()
{
#if defined(_DEBUG_OBJ_REF_)
    MtxInterlockedDec(&_nCObjUtil) ;
#endif
}
int  CObj::ListItemsCount() const
{
    return m_nListCount ;
}
CObj::CObj( const CObj &src)
{
    m_pPreObj = this;
    m_pNextObj = this;
    m_nListCount = 0 ;
    *this = src ;

}
CObj & CObj::operator =(  const CObj & src )
{
    if( ! src.ListIsEmpty() )
    {
        CObj *item = NULL ;
        m_pPreObj  = this ;
        m_pNextObj = this ;
        m_nListCount = 0 ;
        while( ( item = src.ListRemoveHead() ) != NULL )
        {
            ListAddTail(item);
            m_nListCount ++ ;

        }

    }



    return *this;
}
void CObj::ListInsert(CObj *newItem,CObj *prev, CObj *next)
{
    _ListAdd(newItem,prev,next,this);
}
void  CObj::_ListAdd(CObj *newItem,CObj *prev, CObj *next,CObj *head)
{
    next->m_pPreObj = newItem;
    newItem->m_pNextObj = next;
    newItem->m_pPreObj = prev;
    prev->m_pNextObj = newItem;

    if( head != NULL )
    {
        //MtxInterlockedInc(&(head->m_nListCount));
        head->m_nListCount ++ ;
    }

}
void CObj::_ListDel(CObj *prev, CObj *next,CObj *head)
{
    next->m_pPreObj = prev;
    prev->m_pNextObj = next;

    if( head != NULL )
    {
        //MtxInterlockedDec(&(head->m_nListCount));
        head->m_nListCount -- ;
    }
}
void CObj::ListAddHead(CObj *newEntry)
{
    _ListAdd(newEntry, this, this->m_pNextObj, this);
}
void CObj::ListAddTail(CObj *newEntry)
{

    _ListAdd(newEntry, this->m_pPreObj, this, this);
}
BOOL CObj::ListDel(CObj *oldEntry) const
{
    if( oldEntry == NULL || oldEntry->ListIsEmpty() || m_nListCount < 1 )
    {
        return FALSE;
    }
    _ListDel(oldEntry->m_pPreObj, oldEntry->m_pNextObj, (CObj *)this);
    oldEntry->m_pNextObj = oldEntry ;
    oldEntry->m_pPreObj = oldEntry ;
    return TRUE ;
}
void CObj::ListMoveHead(CObj *EntryMv)
{
    if( EntryMv == NULL || EntryMv->ListIsEmpty() )
    {
        return  ;
    }
    /*
    if( EntryMv->m_pPreObj != NULL && EntryMv->m_pNextObj != NULL )
    {
        _ListDel(EntryMv->m_pPreObj, EntryMv->m_pNextObj, this);
    }
    */

    //_ListDel(EntryMv->m_pPreObj, EntryMv->m_pNextObj, NULL);
    //_ListAdd(EntryMv, this, this->m_pNextObj, NULL);

    CObj *next = NULL ,*prev = NULL;

    prev = EntryMv->m_pPreObj ;
    next = EntryMv->m_pNextObj ;

    prev->m_pNextObj = next;
    next->m_pPreObj = prev ;


    next = m_pNextObj ;
    m_pNextObj = EntryMv;
    EntryMv->m_pNextObj = next;
    EntryMv->m_pPreObj = this;
    next->m_pPreObj = EntryMv;
}
void CObj::ListMoveTail(CObj *EntryMv)
{

    if( EntryMv == NULL || EntryMv->ListIsEmpty() )
    {
        return  ;
    }

    /*
    if( EntryMv->m_pPreObj != NULL && EntryMv->m_pNextObj != NULL )
    {
        _ListDel(EntryMv->m_pPreObj, EntryMv->m_pNextObj, this);
    }
    ListAddTail(EntryMv);
    */
    //_ListDel(EntryMv->m_pPreObj, EntryMv->m_pNextObj, NULL);
    //_ListAdd(EntryMv, this->m_pPreObj, this, NULL);
    //ListDel(EntryMv);
    //ListAddTail(EntryMv);

    CObj *next = NULL ,*prev = NULL ;

    prev = EntryMv->m_pPreObj ;
    next = EntryMv->m_pNextObj ;

    prev->m_pNextObj = next;
    next->m_pPreObj = prev ;


    prev = m_pPreObj ;
    m_pPreObj = EntryMv;
    EntryMv->m_pNextObj = this;
    EntryMv->m_pPreObj = prev;
    prev->m_pNextObj = EntryMv;

}
BOOL CObj::ListIsEmpty() const
{
    return ( ( ( m_pNextObj == this ) && (m_pPreObj == this) ) || m_pNextObj == NULL || m_pPreObj == NULL ) ;
}
CObj * CObj::ListRemoveTail()
{
    if( ListIsEmpty() || m_nListCount < 1 )
    {
        return NULL ;
    }
    CObj *obj = m_pPreObj ;
    ListDel( obj ) ;

    return  obj ;
}
CObj * CObj::ListRemoveHead() const
{
    if( ListIsEmpty()  || m_nListCount < 1 )
    {
        return NULL ;
    }
    CObj *obj = m_pNextObj ;
    ListDel( obj ) ;

    return  obj ;

}
void * CObj::QueryObj(const char *szObjName)
{
    if( szObjName == NULL )
    {
        return NULL ;
    }

    if( strcmp( szObjName, GetObjName() ) == 0 )
    {
        return this;
    }
    else
    {
        return NULL ;
    }
}

long CObj::AddObjRef()
{
    if( this == NULL )
    {
        return 0;
    }
    if( ! m_bAlloced )
    {
        return 1 ;
    }
    return MtxInterlockedInc(&m_nRef);
}
long CObj::ReleaseObj()
{

    if( ! m_bAlloced )
    {
        return 1 ;
    }
    int lResult = MtxInterlockedDec(&m_nRef);

	if( lResult < 0 )
	{
		LogPrint( _LOG_LEVEL_FATAL, _TAGNAME, "OBJ:0X%X %d System bug  m_nRef <= 0, wait for debugger\n"  , this ,lResult );

		while( TRUE )
		{
			SleepMilli(1000);
		}

		return 0 ;
	}

    if (lResult == 0)
    {
        delete this;
    }
    return lResult;
}
void LibCppDumpCObjList(CObj *pItemHead)
{
    if( pItemHead == NULL )
    {
        return ;
    }
    CObj *pItemTmp = NULL;
    CObj *pItem = NULL;
    int i = 0 ;

    pItem = pItemHead->m_pNextObj ;

    printf("--------------%p-------------\n",pItemHead);
    while( pItem != NULL && pItem != pItemHead )
    {

        pItemTmp = pItem ;
        pItem = pItem->m_pNextObj ;
        printf("\t%d = %p\n",i,pItemTmp)  ;
        SleepMilli(2000);

        i ++ ;

    }
     printf("--------------------------------------\n");

}
/************************************************************************/
/*                                                                      */
/************************************************************************/

_CPP_UTIL_DYNAMIC_IMP(CUtilList)
CUtilList::CUtilList()
{

    m_nObjSize = 0 ;
    m_bAlloced = FALSE ;
    ListInit(&m_listHead);
    m_nListCount = 0 ;

}
CUtilList::CUtilList(const CUtilList &src)
{
    ListInit(&m_listHead);
    m_nListCount = 0 ;
    *this = src ;
}

CUtilList::~CUtilList()
{

}
void * CUtilList::operator new( size_t cb )
{
    return malloc(cb) ;
}
void CUtilList::operator delete( void* p )
{
    free(p);
}
CUtilList & CUtilList::operator =(  const CUtilList & src )
{
    if( ! src.ListIsEmpty() )
    {
        LIST_ITEM *item = NULL ;
        ListInit(&m_listHead);
        m_nListCount = 0 ;
        while( ( item = src.ListRemoveHead() ) != NULL )
        {
            ListAddTail(item);
            m_nListCount ++ ;

        }

      //  m_nListCount = src.m_nListCount ;
    }


    return *this;
}
int  CUtilList::ListItemsCount()
{
    return m_nListCount ;
}
void CUtilList::ListInsert(LIST_ITEM *newItem,LIST_ITEM *prev, LIST_ITEM *next)
{
    _ListAdd(newItem,prev,next,this);
}
void  CUtilList::_ListAdd(LIST_ITEM *newItem,LIST_ITEM *prev, LIST_ITEM *next,CUtilList *head)
{
    next->prev = newItem;
    newItem->next = next;
    newItem->prev = prev;
    prev->next = newItem;

    if( head != NULL )
    {
        head->m_nListCount ++ ;
    }

}
void CUtilList::_ListDel(LIST_ITEM *prev, LIST_ITEM *next,CUtilList *head)
{
    next->prev = prev;
    prev->next = next;

    if( head != NULL )
    {
        head->m_nListCount -- ;
    }
}
void CUtilList::ListAddHead(LIST_ITEM *newEntry)
{
    _ListAdd(newEntry, &m_listHead, m_listHead.next, this);
}
void CUtilList::ListAddTail(LIST_ITEM *newEntry)
{

    _ListAdd(newEntry, m_listHead.prev, &m_listHead, this);
}
void CUtilList::ListDel(LIST_ITEM *oldEntry) const
{
    if( oldEntry == NULL || oldEntry == oldEntry->next )
    {
        return ;
    }
    _ListDel(oldEntry->prev, oldEntry->next, (CUtilList *)this);
    oldEntry->next = oldEntry ;
    oldEntry->prev = oldEntry ;
}
void CUtilList::ListMoveHead(LIST_ITEM *EntryMv)
{
    if(EntryMv == NULL || EntryMv == EntryMv->next )
    {
        return ;
    }
    ListDel(EntryMv);
    ListAddHead(EntryMv);
}
void CUtilList::ListMoveTail(LIST_ITEM *EntryMv)
{
    if(EntryMv == NULL || EntryMv == EntryMv->next )
    {
        return ;
    }
    ListDel(EntryMv);
    ListAddTail(EntryMv);
}
BOOL CUtilList::ListIsEmpty() const
{
    return ( m_listHead.next == &m_listHead );
}
LIST_ITEM * CUtilList::ListRemoveTail()
{
    if( ListIsEmpty() )
    {
        return NULL ;
    }
    LIST_ITEM *obj = m_listHead.prev ;
    ListDel( obj ) ;

    return  obj ;
}
LIST_ITEM * CUtilList::ListRemoveHead()  const
{
    if( ListIsEmpty() )
    {
        return NULL ;
    }
    LIST_ITEM *obj = m_listHead.next ;
    ListDel( obj ) ;

    return  obj ;

}
int CUtilList::AddObjRef()
{
    if( ! m_bAlloced )
    {
        return 1 ;
    }
    return MtxInterlockedInc(&m_nRef);
}
int CUtilList::ReleaseObj()
{
    if( this == NULL )
    {
        return 0;
    }
    if( ! m_bAlloced )
    {
        return 1 ;
    }
    if( m_nRef <= 0 )
    {
        // delete this;
        return 0 ;
    }
    INT32 lResult = MtxInterlockedDec(&m_nRef);
    if (lResult == 0)
    {
        delete this;
    }
    return lResult;
}


/////////////////////////////////////////////////////////////////////

#define SAFETAIL (sizeof(void *)*32)
CObjBuffer::CObjBuffer()
{
	Data = NULL ;
	WritePos = 0 ;
	ReadPos = 0 ;
	Allocated = 0 ;
	bMalloc2 = FALSE ;
	SafeTail = SAFETAIL ;
}
CObjBuffer::~CObjBuffer()
{
	if( Data != NULL )
	{
		if( bMalloc2 )
		{
			free2( Data );
		}
		else
		{
			MemoryRelease( Data );
		}
		
		Data = NULL ;
	}
}
void CObjBuffer::Clear()
{
	if( bMalloc2 )
	{
		free2( Data );
	}
	else
	{
		MemoryRelease( Data );
	}
	Data = NULL;
	Allocated = 0;
	ReadPos = 0;
	WritePos = 0;
	
}
void CObjBuffer::Drop()
{
	//drop previous packets
	WritePos = 0;
	ReadPos = 0;
}
BOOL CObjBuffer::Alloc(int Size, int Align)
{
	UINT8* Data1 = NULL ;
	 

	if( bMalloc2 )
	{
		Align--;
		Size = (Size + SafeTail + Align) & ~Align;
		Data1 = (UINT8*)realloc2(Data,Size);
		if ( Data1 == NULL )
		{
			printf("ERROR CObjBuffer::Alloc %d failed\n",Size);
			return FALSE;
		}
		Data = Data1;

	}
	else
	{
		Align--;
		Size = (Size + SafeTail + Align) & ~Align;
		Data1 = (UINT8*)MemoryRealloc(Data,Size);
		if ( Data1 == NULL )
		{
			printf("ERROR CObjBuffer::Alloc %d failed\n",Size);
			return FALSE;
		}
		Data = Data1;

	}
	

	
	

	
	Allocated = Size  - SafeTail ;
	return TRUE;
}
BOOL CObjBuffer::Write( const void* Ptr, int Length, int Align)
{
	//append new data to buffer
	int WritePos1 = WritePos + (int)Length; //buffer!
	if (WritePos1 > Allocated && !Alloc(WritePos1,Align))
	{
		return FALSE;
	}
	if (Ptr)
	{
		memcpy( Data +  WritePos,Ptr,Length);
	}
	WritePos = WritePos1;
	return TRUE;
}
BOOL CObjBuffer::Read( const unsigned char** Ptr, int Length)
{
	if (WritePos < ReadPos + (int)Length) //buffer!
	{
		return FALSE;
	}

	*Ptr = Data + ReadPos;
	ReadPos += (int)Length; //buffer!
	return TRUE;
}
void CObjBuffer::Pack(int Length)
{
	int Skip = ReadPos + (int)Length; //buffer!
	if (WritePos > Skip)
	{
		WritePos -= Skip;
		if (Skip)
		{
			memmove(Data,Data+Skip,WritePos); // move end part to the beginning
		}
	}
	else
		WritePos = 0;
	ReadPos = 0;
}
int  CObjBuffer::DataLength()
{
	return ( WritePos - ReadPos );
}


/////////////////////////////////////////////////////////////////////////////////
_CPP_UTIL_DYNAMIC_IMP(CObjArray)
_CPP_UTIL_CLASSNAME_IMP(CObjArray)
_CPP_UTIL_QUERYOBJ_IMP(CObjArray, CObj)

CObjArray::CObjArray()
{

}
CObjArray::~CObjArray()
{
    Clear();
}
CObjArray::CObjArray(const CObjArray& _this)
{
    *this = _this;
}
CObjArray& CObjArray::operator = (const CObjArray& _this)
{
    Clear();

    const  int nCount = _this.m_arry.WritePos / sizeof(CObj *);

    for (int i = 0; i < nCount; i++)
    {

        CObj *var = NULL;
        memcpy(&var, _this.m_arry.Data + i * sizeof(CObj *), sizeof(CObj *));

        Add(var);
    }

    return *this;
}
void CObjArray::Add(CObj *varRef)
{
    varRef->AddObjRef();
    m_arry.Write(&varRef, sizeof(CObj *), sizeof(CObj *) * 10);
}
int  CObjArray::GetCount()
{
    return m_arry.WritePos / sizeof(CObj *);
}
void CObjArray::Clear()
{
    int  i = 0;

    const unsigned char *data = NULL;
    CObj *varRef = NULL;
    m_arry.ReadPos = 0;
    while (m_arry.Read(&data, sizeof(CObj *)))
    {
        memcpy(&varRef, data, sizeof(CObj *));
        if (varRef != NULL)
        {
            //printf("%p=%s\n", varRef, (const char *)varRef->m_var);
            varRef->ReleaseObj();

        }

    }

    m_arry.Drop();

}
CObj *CObjArray::GetAt(int i)
{
    int pos = i * sizeof(CObj *);

    if (pos > (m_arry.WritePos - sizeof(CObj *)))
    {
        return NULL;
    }

    CObj *varRef = NULL;

    memcpy(&varRef, m_arry.Data + pos, sizeof(CObj *));

    return varRef;
}

/***********************************************************/

CObjMemIO::CObjMemIO(const void *bytes , int nLen)
{
	m_nSeekPos = 0 ;
	m_buf = (unsigned char *)bytes ;
	m_nBufLen = nLen ;
	if( m_nBufLen == -1 )
	{
		m_nBufLen = strlen( (char *)bytes ) ;
	}

}
CObjMemIO::~CObjMemIO()
{

}

int CObjMemIO::Write(const void *data , int nLen)
{
	int nCpy = 0 ;
	if( nLen == -1 )
	{
		nLen = strlen((const char *) data );
	}

	nCpy = MinInt( nLen , (m_nBufLen-m_nSeekPos));

	memcpy( m_buf + m_nSeekPos ,  data , nCpy ) ;
	m_nSeekPos += nCpy ;
	return nCpy ;
}

static char  readChar(CObjMemIO *_this,BOOL *IsFileEnd)
{
	int nRet;
	char c = 0 ;
	if(IsFileEnd)
	{
		*IsFileEnd=0;
	}

	if( _this->m_nSeekPos < _this->m_nBufLen )
	{
		c = (char)_this->m_buf[ _this->m_nSeekPos ] ;
		_this->m_nSeekPos ++ ;
		return c ;
	}
	else
	{
		return (char)EOF ;
	}

}
int CObjMemIO::ReadLine(void *data , int size)
{
	int i = 0;
	char c = 0;
	static const char defaultStop[]="\r\n";
	char isEndLine = 0;
	char *outLine = (char *)data;
	size--;


	while(i<size&&(c=readChar(this,0))!=(char)EOF)
	{
		if(CharPosInStr(c,defaultStop)>=0)
		{
			isEndLine=1;
			break;
		}
		*(outLine+i)=c;
		i++;
	}
	outLine[i]=0;

	if(c==(char)EOF)
	{

		isEndLine=1;
	}
	else
	{

	}


	return i ;
}
BOOL CObjMemIO::IsEof()
{
	if( m_nSeekPos >= m_nBufLen )
	{
		return TRUE ;
	}
	else
	{
		return FALSE ;
	}
}

/***********************************************************/


void MYAPI DbgPrintfObjRef()
{
#if defined(_DEBUG_OBJ_REF_)

#else
    printf("---------_DEBUG_OBJ_REF_ not defined---------------\n");
#endif
}









#include "libcpputil_def.h"
#include "libcpputil_var.h"

static char szNULL[2] = { 0, 0 };

CObjVar::CObjVar()
{
	m_type = ObjVarTypeUnknown ;
	memset( &m_value , 0 , sizeof(m_value) );
}
CObjVar::~CObjVar()
{
	Clear();
}
void * CObjVar::operator new( size_t cb )
{
	

	return malloc2_align(cb , 0 , 4 );


}
void CObjVar::operator delete( void* p )
{

	free2(p);

}
CObjVar::CObjVar(const CObjVar& _this)
{
	m_type = ObjVarTypeUnknown ;
	*this = _this ;
}
CObjVar::CObjVar(const char *str)
{
	m_type = ObjVarTypeUnknown ;
	*this = str ; 

}
CObjVar::CObjVar(const double fv)
{
	m_type = ObjVarTypeUnknown ;
	*this = fv ; 
}
CObjVar::CObjVar(const int uv)
{
	m_type = ObjVarTypeUnknown ;
	*this = uv ;
}
CObjVar::CObjVar(const bool b)
{
	m_type = ObjVarTypeUnknown ;
	*this = b ;
}
CObjVar::CObjVar(CObj *obj)
{
	m_type = ObjVarTypeUnknown ;
	*this = obj ;
}
CObjVar::CObjVar(void *ptr)
{
	m_type = ObjVarTypePtr ;
	m_value.ptr = ptr ;
}
CObjVar& CObjVar::operator = (const CObjVar& _this)
{
	if( this == &_this )
	{
		return *this ;
	}
	Clear();
	m_type = _this.m_type ;
	memcpy( & m_value , &_this.m_value , sizeof(m_value) );

	if( m_type == ObjVarTypeObj )
	{
		if( m_value.obj != NULL )
		{
			m_value.obj->AddObjRef();
		}
		

	}
	else if( m_type == ObjVarTypeString )
	{
		if( _this.m_value.str.str != NULL )
		{
			m_value.str.str = (char *)malloc2_align(m_value.str.len+1 , 0, 32 );
			memcpy( m_value.str.str , _this.m_value.str.str , m_value.str.len );
			m_value.str.str[m_value.str.len] = 0 ;
		}
		else
		{
			m_value.str.str = NULL ;
			m_value.str.len = 0 ;
		}

	}
	else if( m_type == ObjVarTypeBinary )
	{
		if( _this.m_value.bin.data != NULL )
		{
			m_value.bin.data = malloc2_align(m_value.bin.len ,0,  32 );
			memcpy( m_value.bin.data , _this.m_value.bin.data , m_value.bin.len );
		}
		else
		{
			m_value.bin.data = NULL;
			m_value.bin.len = 0 ;
		}

	}
	return *this ;
}
BOOL CObjVar::IsEmpty() const
{
	return ( m_type == ObjVarTypeUnknown ) ;
}
BOOL CObjVar::IsNull() const
{
    return ( m_type == ObjVarTypeNULL);
}
void CObjVar::Trim()
{
    if (m_type == ObjVarTypeString)
    {
        if (m_value.str.str != NULL)
        {
            StrTrimLeft(StrTrimRight(m_value.str.str));
            m_value.str.len = strlen(m_value.str.str);
        }
        
    }
}
BOOL CObjVar::SetString( const char *str , int nLen )
{
	if( str == NULL )
	{
		Clear();
		m_type = ObjVarTypeString ;
		m_value.str.str = NULL ;
		m_value.str.len = 0 ;
		return FALSE ;
	}
	if( nLen == -1 )
	{
		nLen = strlen( str ) ;
	}
	Clear();
	m_type = ObjVarTypeString ;
	m_value.str.len  = nLen  ;
	m_value.str.str = (char *)malloc2_align(m_value.str.len+1 , 0, 32 );
	memcpy( m_value.str.str , str , m_value.str.len );
    if (m_value.str.len > 0)
    {
        if (m_value.str.str[m_value.str.len - 1] == 0)
        {
            m_value.str.len--;
        }
    }
	m_value.str.str[m_value.str.len] = 0 ;
	return TRUE ;
}
CObjVar& CObjVar::operator = (const char *str)
{
	Clear();

	m_type = ObjVarTypeString ;

	if( str == NULL )
	{
		m_value.str.str = NULL ;
		m_value.str.len = 0 ;
	}
	else
	{
		m_value.str.len  = strlen(str) ;
		m_value.str.str = (char *)malloc2_align(m_value.str.len+1 , 0, 32 );
		memcpy( m_value.str.str , str , m_value.str.len );
		m_value.str.str[m_value.str.len] = 0 ;
	}

	return *this ;
}
CObjVar& CObjVar::operator = (const double fv)
{
	Clear();
	m_type = ObjVarTypeDouble ;
	m_value.db = fv ;
	return *this ;
}
CObjVar& CObjVar::operator = (const  bool b)
{
	Clear();
	m_type = ObjVarTypeBool ;
	m_value.b = b ;
	return *this ;
}
CObjVar& CObjVar::operator = (const  int uv)
{

	Clear();
	m_type = ObjVarTypeInt32 ;
	m_value.int_32 = uv ;
	return *this ;
}
CObjVar& CObjVar::operator = ( CObj *obj)
{
	if( obj != NULL )
	{
		obj->AddObjRef();
	}

	Clear();
	m_type = ObjVarTypeObj ;
	m_value.obj = obj ;
	return *this ;
}
BOOL CObjVar::operator == (const  int uv) const
{
	switch ( m_type )
	{
	case ObjVarTypeInt32:
		{
			return ( m_value.int_32 == uv );
		}
	case ObjVarTypeInt64:
		{
			return ( m_value.int_64 == uv );
		}
	case ObjVarTypeDouble:
		{
			return ( m_value.db == uv );
		}
	}
	return FALSE ;	
}
BOOL CObjVar::operator != (const  int uv) const
{
		return !( (*this) == uv );
}
BOOL CObjVar::operator == (const CObjVar& _this) const
{
	if( this == &_this )
	{
		return TRUE ;
	}
	if( m_type != _this.m_type )
	{
		return FALSE ;
	}
	switch ( m_type )
	{
	case ObjVarTypeString:
		{
			if( m_value.str.len != _this.m_value.str.len )
			{
				return FALSE ;
			}
			if( m_value.str.len == 0 )
			{
				return TRUE ;
			}
			return ( strcmp(m_value.str.str , _this.m_value.str.str) == 0 );

			break ;
		}
	case ObjVarTypeBinary:
		{
			if( m_value.bin.len != _this.m_value.bin.len )
			{
				return FALSE ;
			}
			if( m_value.bin.len == 0 )
			{
				return TRUE ;
			}
			return ( memcmp(m_value.bin.data , _this.m_value.bin.data ,m_value.bin.len ) == 0 );
			break ;
		}
	case ObjVarTypeInt32:
		{
			return ( m_value.int_32 == _this.m_value.int_32 );
		}
	case ObjVarTypeInt64:
		{
			return ( m_value.int_64 == _this.m_value.int_64 );
		}
	case ObjVarTypeDouble:
		{
			return ( m_value.db == _this.m_value.db );
		}
	case ObjVarTypePtr:
		{
			return ( m_value.ptr == _this.m_value.ptr );
		}
	case ObjVarTypeObj:
		{
			return ( m_value.obj == _this.m_value.obj );
		}
	case ObjVarTypeBool:
		{
			return ( m_value.b == _this.m_value.b );
		}
	}
	return FALSE ;
}
BOOL CObjVar::operator != (const CObjVar& _this) const
{
	return !( (*this) == _this );
}

void CObjVar::SetNull()
{
	Clear();
	m_type = ObjVarTypeNULL ;

}

void CObjVar::Clear()
{
	if( m_type == ObjVarTypeObj )
	{
		if( m_value.obj != NULL )
		{
			m_value.obj->ReleaseObj();
		}


	}
	else if( m_type == ObjVarTypeBinary )
	{
		if( m_value.bin.data != NULL )
		{
			free2( m_value.bin.data );
		}


	}
	else if( m_type == ObjVarTypeString )
	{
		if( m_value.str.str != NULL )
		{
			free2( m_value.str.str );
		}

	}
	m_type = ObjVarTypeNULL ;
	memset( &m_value , 0 , sizeof(m_value) );
}
int  CObjVar::DataLength() const
{
	
	switch ( m_type )
	{
	case ObjVarTypeString:
		{
			return  m_value.str.len ;
		}
	case ObjVarTypeBinary:
		{
			return m_value.bin.len ;
		}
	case ObjVarTypeInt32:
		{
			return sizeof( m_value.int_32 ) ;
		}
	case ObjVarTypeInt64:
		{
			return sizeof( m_value.int_64 );
		}
	case ObjVarTypeDouble:
		{
			return sizeof( m_value.db );
		}
	case ObjVarTypePtr:
		{
			return sizeof( m_value.ptr );
		}
	case ObjVarTypeObj:
		{
			return sizeof( m_value.obj );
		}
    case ObjVarTypeBool:
        {
            return sizeof( m_value.b );
        }
	}
	return 0 ;
}
const void* CObjVar::Data() const
{
	switch ( m_type )
	{
	case ObjVarTypeString:
		{
			return  m_value.str.str ;
		}
	case ObjVarTypeBinary:
		{
			return m_value.bin.data ;
		}
	case ObjVarTypeInt32:
		{
			return &( m_value.int_32 ) ;
		}
	case ObjVarTypeInt64:
		{
			return &( m_value.int_64 );
		}
	case ObjVarTypeDouble:
		{
			return &( m_value.db );
		}
	case ObjVarTypePtr:
		{
			return &( m_value.ptr );
		}
	case ObjVarTypeObj:
		{
			return &( m_value.obj );
		}
	default:
		return NULL ;
	}
}
INT64   CObjVar::toInt64()
{
    switch (m_type)
    {
    case ObjVarTypeString:
    {
        if (m_value.str.str == NULL)
        {
            return 0;
        }
        else
        {
            return atoi64x(m_value.str.str);
        }
    }
    case ObjVarTypePtr:
    {
        return ((INT64)m_value.ptr);
    }
    case ObjVarTypeInt32:
    {
        return (m_value.int_32);
    }
    case ObjVarTypeInt64:
    {
        return (m_value.int_64);
    }
    case ObjVarTypeDouble:
    {
        return (m_value.db);
    }
    case ObjVarTypeBool:
    {
        return (m_value.b);
    }
    default:
        return 0;
    }
}
CObjVar::operator int() const
{
	switch ( m_type )
	{
	case ObjVarTypeString:
		{
			if( m_value.str.str == NULL  )
			{
				return 0 ;
			}
			else
			{
				return atoi(m_value.str.str );
			}
		}
	case ObjVarTypePtr:
		{
			return (int)( (INT64)m_value.ptr ) ;
		}
	case ObjVarTypeInt32:
		{
			return (int)( m_value.int_32 ) ;
		}
	case ObjVarTypeInt64:
		{
			return (int)( m_value.int_64 );
		}
	case ObjVarTypeDouble:
		{
			return (int)( m_value.db );
		}
	case ObjVarTypeBool:
		{
			return (int)( m_value.b );
		}
	default:
		return 0 ;
	}
}
CObjVar::operator double() const
{
	switch ( m_type )
	{
	case ObjVarTypeString:
		{
			if( m_value.str.str == NULL  )
			{
				return 0 ;
			}
			else
			{
				return atof(m_value.str.str );
			}
		}
	case ObjVarTypeInt32:
		{
			return (double)( m_value.int_32 ) ;
		}
	case ObjVarTypeInt64:
		{
			return (double)( m_value.int_64 );
		}
	case ObjVarTypeDouble:
		{
			return (double)( m_value.db );
		}
	case ObjVarTypeBool:
		{
			return (double)( m_value.b );
		}
	default:
		return 0 ;
	}
}

CObjVar::operator CObj *() const
{
	switch ( m_type )
	{
	case ObjVarTypeObj:
		{
			return m_value.obj ;
		}
	default:
		return NULL ;
	}
}
CObjVar::operator const char *() const
{
	
	if( m_type != ObjVarTypeString  )
	{
		return szNULL ;
	}
	if( m_value.str.str == NULL )
	{
		return szNULL ;
	}

	return m_value.str.str ;
}
int  CObjVar::StringLength( ) const
{
    if( m_type != ObjVarTypeString )
    {
        return 0;
    }
    if( m_value.str.str == NULL )
    {
        return 0;
    }

    return m_value.str.len;
}
/////////////////////////////////////////////////////////////////////////////////
_CPP_UTIL_DYNAMIC_IMP(CObjVarRef)
_CPP_UTIL_CLASSNAME_IMP(CObjVarRef)
_CPP_UTIL_QUERYOBJ_IMP( CObjVarRef , CObj)



/////////////////////////////////////////////////////////////////////////////////
_CPP_UTIL_DYNAMIC_IMP(CObjVarArray)
_CPP_UTIL_CLASSNAME_IMP(CObjVarArray)
_CPP_UTIL_QUERYOBJ_IMP(CObjVarArray, CObj)

CObjVarArray::CObjVarArray()
{

}
CObjVarArray::~CObjVarArray()
{
    Clear();
}
CObjVarArray::CObjVarArray(const CObjVarArray& _this)
{
    *this = _this;
}
CObjVarArray& CObjVarArray::operator = (const CObjVarArray& _this)
{
    Clear();

    const  int nCount = _this.m_arry.WritePos / sizeof(CObjVarRef *);

    for (int i = 0; i < nCount ; i++)
    {

        CObjVarRef *varRef = NULL;
        memcpy(&varRef, _this.m_arry.Data + i * sizeof(CObjVarRef *), sizeof(CObjVarRef *));

        Add(varRef );
    }

    return *this;
}
void CObjVarArray::Add(CObjVarRef *varRef) 
{
    varRef->AddObjRef();
    m_arry.Write(&varRef, sizeof(CObjVarRef *), sizeof(CObjVarRef *) * 10);
}
int  CObjVarArray::GetCount()   
{
    return m_arry.WritePos / sizeof(CObjVarRef *);
}
void CObjVarArray::Clear()
{
    int  i = 0;

    const unsigned char *data = NULL;
    CObjVarRef *varRef = NULL;
    m_arry.ReadPos = 0;
    while (m_arry.Read(&data, sizeof(CObjVarRef *)))
    {
        memcpy(&varRef, data, sizeof(CObjVarRef *));
        if (varRef != NULL)
        {
            //printf("%p=%s\n", varRef, (const char *)varRef->m_var);
            varRef->ReleaseObj();

        }

    }

    m_arry.Drop();

}
BOOL CObjVarArray::IsExist(const char *str, BOOL ignoreCase )
{
    int  i = 0;

    const unsigned char *data = NULL;
    CObjVarRef *varRef = NULL;
    m_arry.ReadPos = 0;
    while (m_arry.Read(&data, sizeof(CObjVarRef *)))
    {
        memcpy(&varRef, data, sizeof(CObjVarRef *));
        if (varRef != NULL)
        {

            if (ignoreCase)
            {
                if (strcmpix(str, varRef->m_var) == 0)
                {
                    return TRUE;
                }
            }
            else
            {
                if (strcmp(str, varRef->m_var) == 0)
                {
                    return TRUE;
                }
            }
        }

    }

    return FALSE;
}
CObjVarRef *CObjVarArray::GetAt(int i)
{
    int pos = i * sizeof(CObjVarRef *);

    if (pos > (m_arry.WritePos - sizeof(CObjVarRef *)))
    {
        return NULL;
    }

    CObjVarRef *varRef = NULL;

    memcpy(&varRef, m_arry.Data + pos , sizeof(CObjVarRef *));

    return varRef;
}
int  CObjVarArray::SplitByChars(const char *str, const char *splitChars, BOOL skipEmpty)
{
    const char *pre = str;
    const char *cur = str;
    int lenSplit = 1;
    int count = 0;
    BOOL bFound = FALSE;
    CObjVarRef *strRef = NULL;

    if (splitChars == NULL || splitChars[0] == 0)
    {
        return 0;
    }


    for (; *cur != 0;)
    {
        if (CharPosInStr(*cur, splitChars) >= 0)
        {
            bFound = TRUE;
            strRef = CObjVarRef::CreateObj();
            strRef->m_var.SetString(pre, cur - pre);

            if (skipEmpty)
            {
                strRef->m_var.Trim();
                if (strRef->m_var.DataLength() > 0)
                {
                    Add(strRef);
                    count++;
                }
            }
            else
            {
                Add(strRef);
                count++;
            }





            strRef->ReleaseObj();
            strRef = NULL;

            cur += lenSplit;
            pre = cur;


        }
        else
        {
            cur++;
        }
    }

    if (bFound)
    {
        strRef = CObjVarRef::CreateObj();
        strRef->m_var.SetString(pre, cur - pre);
        if (skipEmpty)
        {
            strRef->m_var.Trim();
            if (strRef->m_var.DataLength() > 0)
            {
                Add(strRef);
                count++;
            }
        }
        else
        {
            Add(strRef);
            count++;
        }


        strRef->ReleaseObj();
        strRef = NULL;

    }


    return count;
}
int  CObjVarArray::Split(const char *str, const char *split, BOOL skipEmpty)
{
    const char *pre = str;
    const char *cur = str;
    int lenSplit = 0;
    int count = 0;
    BOOL bFound = FALSE;
    CObjVarRef *strRef = NULL;

    if (split == NULL || split[0] == 0 )
    {
        return 0;
    }

    lenSplit = strlen(split);

    for (; *cur != 0; )
    {
        if (strcmpipre(cur, split) == 0)
        {
            bFound = TRUE;
            strRef = CObjVarRef::CreateObj();
            strRef->m_var.SetString(pre, cur - pre);

            if (skipEmpty)
            {
                strRef->m_var.Trim();
                if (strRef->m_var.DataLength() > 0)
                {
                    Add(strRef);
                    count++;
                }
            }
            else
            {
                Add(strRef);
                count++;
            }
            




            strRef->ReleaseObj();
            strRef = NULL;

            cur += lenSplit;
            pre = cur;
            

        }
        else
        {
            cur++;
        }
    }

    if ( bFound )
    {
        strRef = CObjVarRef::CreateObj();
        strRef->m_var.SetString(pre, cur - pre);
        if (skipEmpty)
        {
            strRef->m_var.Trim();
            if (strRef->m_var.DataLength() > 0)
            {
                Add(strRef);
                count++;
            }
        }
        else
        {
            Add(strRef);
            count++;
        }


        strRef->ReleaseObj();
        strRef = NULL;

    }


    return count;
}


////////////////////////////////////////////////////////////////////////////////////////



CRefString::CRefString(const char *szKey )
{

    m_szString = NULL;
    *this = szKey;

}
CRefString::CRefString(const CRefString &_this)
{
    m_szString = NULL;
    *this = _this;
    return;
}
CRefString::~CRefString()
{

    MemoryRelease(m_szString);

}
int CRefString::Length()
{
    return  MemoryStringLen(m_szString);
}
void CRefString::SetString(const char *str, int nLen )
{
    if (m_szString != NULL)
    {
        MemoryRelease(m_szString);
        m_szString = NULL;
    }
    if (str == NULL)
    {
        m_szString = MemoryEmptyString();
    }
    else
    {
        m_szString = MemoryString(str, nLen, 16);
    }
}
CRefString & CRefString::operator = (const char *szKey)
{
    if (m_szString != NULL)
    {
        MemoryRelease(m_szString);
        m_szString = NULL;
    }
    if (szKey == NULL)
    {
        m_szString = MemoryEmptyString();
    }
    else
    {
        m_szString = MemoryString(szKey, -1, 16);
    }

    return *this;
}
CRefString & CRefString::operator = (const CRefString &_this)
{

    if (this == &_this)
    {
        return *this;
    }

    MemoryAddRef(_this.m_szString);
    MemoryRelease(m_szString);

    m_szString = _this.m_szString;

    return *this;
}
bool CRefString::operator != (const  CRefString &_this) const
{
    return (strcmp(m_szString, _this.m_szString) != 0);
}
bool CRefString::operator == (const  CRefString &_this) const
{
    return (strcmp(m_szString, _this.m_szString) == 0);
}
bool CRefString::operator > (const  CRefString &_this) const
{
    return (strcmp(m_szString, _this.m_szString) > 0);
}
bool CRefString::operator < (const  CRefString &_this) const
{
    return (strcmp(m_szString, _this.m_szString) < 0);
}
bool CRefString::operator >= (const  CRefString &_this) const
{
    return (strcmp(m_szString, _this.m_szString) >= 0);
}
bool CRefString::operator <= (const  CRefString &_this) const
{
    return (strcmp(m_szString, _this.m_szString) <= 0);
}
CRefString::operator const char *()
{
    return m_szString;
}














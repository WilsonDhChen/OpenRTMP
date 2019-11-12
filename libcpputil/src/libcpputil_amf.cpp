

#include "libcpputil_def.h"
#include "libcpputil_amf.h"

static char _szNULL[2] = { 0 , 0 };

static const unsigned short __endian_test = 0x0001;
#define is_little_endian (*((const unsigned char*)&__endian_test))
#define _ALIGN_ENC 1024
#define RTMP_LOGDEBUG _LOG_LEVEL_DEBUG
#define RTMP_LOGERROR _LOG_LEVEL_ERROR
#define LOG_TAG "amf"
#define RTMP_DEFAULT_CHUNKSIZE	128

typedef enum tagAMF3DataType
{ AMF3_UNDEFINED = 0, AMF3_NULL, AMF3_FALSE, AMF3_TRUE,
AMF3_INTEGER, AMF3_DOUBLE, AMF3_STRING, AMF3_XML_DOC, AMF3_DATE,
AMF3_ARRAY, AMF3_OBJECT, AMF3_XML, AMF3_BYTE_ARRAY
} AMF3DataType;




#define  AMF_DecodeInt16  (CAMFObject::DecodeInt16)
#define  AMF_DecodeInt24  (CAMFObject::DecodeInt24)
#define  AMF_DecodeInt32  (CAMFObject::DecodeInt32)
#define  AMF_DecodeInt32LE  (CAMFObject::DecodeInt32LE)
/* Data is Big-Endian */





class  AMF3ClassDef
{
public:
	AMF3ClassDef()
	{

		cd_externalizable = 0 ;
		cd_dynamic = 0 ;
		cd_num = 0 ;
		cd_props.SafeTail = sizeof(void *);
	}
	~AMF3ClassDef()
	{

		int i = 0 ;
		for( i = 0 ; i < cd_num ; i ++ )
		{
			CObjVar *prop = NULL ;
			memcpy( &prop , cd_props.Data + ( i * sizeof(CObjVar *) )  , sizeof(CObjVar *)  );
			delete prop ;
		}
	}
	AMF3ClassDef(const AMF3ClassDef &_this)
	{
		*this = _this ;
	}
	AMF3ClassDef& operator = (const AMF3ClassDef& _this)
	{
		if( this == & _this )
		{
			return *this ;
		}

		int i = 0 ;
		for( i = 0 ; i < _this.cd_num ; i ++ )
		{
			CObjVar *prop = NULL ;
			memcpy( &prop , _this.cd_props.Data + ( i * sizeof(CObjVar *) )  , sizeof(CObjVar *)  );

			AddProp( new CObjVar(*prop) );
		}


		cd_name = _this.cd_name ;
		cd_externalizable = _this.cd_externalizable ;
		cd_dynamic = _this.cd_dynamic ;
		cd_num = _this.cd_num ;


		return *this ;

	}
	BOOL AddProp(CObjVar *prop)
	{
		if( prop == NULL )
		{
			return FALSE ;
		}

		BOOL bOk = cd_props.Write( &prop , sizeof(CObjVar *)  , sizeof(CObjVar *) * 16);

		if( bOk )
		{
			cd_num = cd_props.DataLength()/(sizeof(CObjVar *)) ;
		}

		return bOk ;
	}
	CObjVar *GetProp(int index)
	{


		if( index < 0 || index >= cd_num )
		{
			return NULL ;
		}
		CObjVar *prop = NULL ;

		memcpy( &prop , cd_props.Data + ( index * sizeof(CObjVar *) )  , sizeof(CObjVar *)  );
		return prop ;
	}
public:
	CObjVar cd_name;
	char cd_externalizable;
	char cd_dynamic;
	int cd_num;
private:
	CObjBuffer cd_props;
} ;


typedef CAMFObject * CAMFObjectPtr;

class CAMFParseCtx :
	public CObj
{

#define AMF_MAX_OBJS 500
#define AMF_MAX_CLS 200

public:
	_CPP_UTIL_DYNAMIC(CAMFParseCtx)  ;
	_CPP_UTIL_CLASSNAME(CAMFParseCtx) ;
	_CPP_UTIL_QUERYOBJ(CObj) ;
public:
	void * operator new( size_t cb )
	{


		return malloc2_align(cb , 0 , 8 );


	}
	void operator delete( void* p )
	{


		free2(p);


	}
	CAMFParseCtx()
	{
		m_nObjsCount = 0 ;
		m_nClsCount = 0 ;
	}
	~CAMFParseCtx()
	{
		int  i = 0 ;
		for ( i = 0 ; i < m_nObjsCount ; i ++ )
		{
			m_objs [ i ] ->ReleaseObj() ;
		}

	}



	BOOL AddObj( CAMFObject * obj )
	{
		if( m_nObjsCount >= (AMF_MAX_OBJS - 1) )
		{
			return FALSE ;
		}
		m_objs [ m_nObjsCount ] = obj ;
		obj->AddObjRef() ;
		m_nObjsCount ++ ;

		return TRUE ;;
	}

	BOOL AddClass( AMF3ClassDef &cls )
	{
		if( m_nClsCount >= (AMF_MAX_CLS - 1) )
		{
			return FALSE ;
		}
		m_cls [ m_nClsCount ] = cls ;
		m_nClsCount ++ ;

		return TRUE ;;
	}

	CAMFObject *GetObj(int index)
	{
		if( index >= m_nObjsCount || index < 0 )
		{
			return NULL ;
		}

		return m_objs[ index ] ;

	}

	AMF3ClassDef *GetClass(int index)
	{
		if( index >= m_nClsCount || index < 0 )
		{
			return NULL ;
		}

		return & ( m_cls[ index ] ) ;

	}

public:
	CAMFObjectPtr m_objs[AMF_MAX_OBJS];
	AMF3ClassDef  m_cls[AMF_MAX_CLS];

	int  m_nObjsCount;
	int  m_nClsCount;


};






static void AMF_DecodeShortString(const UINT8 *data, CObjVar &str )
{
	UINT16 len = AMF_DecodeInt16(data);
	if( len < 1 )
	{
		str.SetString( NULL , 0 );
		return ;
	}
	else
	{
		str.SetString( (const char *)data + 2 , len );
		return ;
	}
}
static void AMF_DecodeLongString(const UINT8 *data, CObjVar &str )
{
	UINT32 len = AMF_DecodeInt32(data);
	if( len < 1 )
	{
		str.SetString( NULL , 0 );
		return ;
	}
	else
	{
		str.SetString( (const char *)data + 2 , len );
		return ;
	}
}

static int AMF_DecodeBoolean(const UINT8 *data)
{
	return *data != 0;
}
static double AMF_DecodeNumber(const UINT8 *data)
{
	double dVal = 0 ;

	if( !is_little_endian )
	{
		memcpy(&dVal, data, 8);
	}
	else
	{
		unsigned char *ci, *co;
		ci = (unsigned char *)data;
		co = (unsigned char *)&dVal;
		co[0] = ci[7];
		co[1] = ci[6];
		co[2] = ci[5];
		co[3] = ci[4];
		co[4] = ci[3];
		co[5] = ci[2];
		co[6] = ci[1];
		co[7] = ci[0];
	}
	return dVal;
}
#define AMF3_INTEGER_MAX	268435455
#define AMF3_INTEGER_MIN	-268435456

static int AMF3ReadInteger(const UINT8 *data, INT32 *valp)
{
	int i = 0;
	INT32 val = 0;

	while (i <= 2)
	{				/* handle first 3 bytes */
		if (data[i] & 0x80)
		{			/* byte used */
			val <<= 7;		/* shift up */
			val |= (data[i] & 0x7f);	/* add bits */
			i++;
		}
		else
		{
			break;
		}
	}

	if (i > 2)
	{				/* use 4th byte, all 8bits */
		val <<= 8;
		val |= data[3];

		/* range check */
		if (val > AMF3_INTEGER_MAX)
			val -= (1 << 29);
	}
	else
	{				/* use 7bits of last unparsed byte (0xxxxxxx) */
		val <<= 7;
		val |= data[i];
	}

	*valp = val;

	return i > 2 ? 4 : i + 1;
}
static int AMF3ReadString(const UINT8 *data, CObjVar &str )
{
	INT32 ref = 0;
	int len;


	len = AMF3ReadInteger(data, &ref);
	data += len;

	if ((ref & 0x1) == 0)
	{				/* reference: 0xxx */
		UINT32 refIndex = (ref >> 1);
		LogPrint(RTMP_LOGDEBUG, LOG_TAG,
			"%s, string reference, index: %d, not supported, ignoring!\n",
			__FUNCTION__, refIndex);
		return len;
	}
	else
	{
		UINT32 nSize = (ref >> 1);

		str.SetString(  (char *)data , nSize ) ;
		return len + nSize;
	}
	return len;
}

static int AMF_Decode(CAMFObject *obj, const UINT8 *pBuffer, int nSize, int bDecodeName, CAMFParseCtx *ctx);
static int AMFProp_Decode(CAMFObjectProperty *prop, const UINT8 *pBuffer, int nSize, int bDecodeName, CAMFParseCtx *ctx);
static int AMF3_Decode(CAMFObject *obj, const UINT8 *pBuffer, int nSize, int bAMFData,int *refObjectIndex , CAMFParseCtx *ctx);
static int AMF3Prop_Decode(CAMFObjectProperty *prop, const UINT8 *pBuffer, int nSize,int bDecodeName, CAMFParseCtx *ctx);
static int AMF3_DecodeObject(CAMFObject *obj, const UINT8 *pBuffer, int nSize, int bAMFData,
							 int *refObjectIndex , CAMFParseCtx *ctx);
static int AMF3_DecodeArray(CAMFObject *obj, const UINT8 *pBuffer, int nSize, int bAMFData,
							int *refObjectIndex , CAMFParseCtx *ctx);
static int AMF_DecodeArray(CAMFObject *obj, const UINT8 *pBuffer, int nSize,
						   int nArrayLen, int bDecodeName , CAMFParseCtx *ctx);

static int AMF3Prop_Decode(CAMFObjectProperty *prop, const UINT8 *pBuffer, int nSize,int bDecodeName, CAMFParseCtx *ctx)
{
	int nOriginalSize = nSize;
	AMF3DataType type =  AMF3_UNDEFINED ;


	if (nSize == 0 || !pBuffer)
	{
		LogPrint(RTMP_LOGDEBUG,LOG_TAG ,"empty buffer/no buffer pointer!\n");
		return -1;
	}

	/* decode name */
	if (bDecodeName)
	{

		int nRes = AMF3ReadString(pBuffer, prop->m_name);

		if ( prop->m_name.DataLength() <= 0 )
		{
			return nRes;
		}
		pBuffer += nRes;
		nSize -= nRes;
	}

	/* decode */
	type = (AMF3DataType) (*pBuffer) ;
	pBuffer ++ ;
	nSize--;

	switch (type)
	{
	case AMF3_UNDEFINED:
	case AMF3_NULL:
		prop->m_type = AMFTYPE_NULL;
		break;
	case AMF3_FALSE:
		prop->m_type = AMFTYPE_BOOLEAN;
		prop->m_value = false;
		break;
	case AMF3_TRUE:
		prop->m_type = AMFTYPE_BOOLEAN;
		prop->m_value = true;
		break;
	case AMF3_INTEGER:
		{
			INT32 res = 0;
			int len = AMF3ReadInteger(pBuffer, &res);
			prop->m_value = res;
			prop->m_type = AMFTYPE_NUMBER;
			nSize -= len;
			break;
		}
	case AMF3_DOUBLE:
		{
			if (nSize < 8)
			{
				return -1;
			}
			prop->m_value = AMF_DecodeNumber(pBuffer);
			prop->m_type = AMFTYPE_NUMBER;
			nSize -= 8;
			break;
		}
	case AMF3_STRING:
	case AMF3_XML_DOC:
	case AMF3_XML:
		{
			int len = AMF3ReadString(pBuffer, prop->m_value);
			prop->m_type = AMFTYPE_STRING;
			nSize -= len;
			break;
		}
	case AMF3_DATE:
		{
			INT32 res = 0;
			int len = AMF3ReadInteger(pBuffer, &res);

			nSize -= len;
			pBuffer += len;

			if ((res & 0x1) == 0)
			{			/* reference */
				UINT32 nIndex = (res >> 1);
				LogPrint(RTMP_LOGDEBUG, LOG_TAG , "AMF3_DATE reference: %d, not supported!\n", nIndex);
			}
			else
			{
				if ( nSize < 8 )
				{
					return -1;
				}

				prop->m_value = AMF_DecodeNumber(pBuffer);
				nSize -= 8;
				prop->m_type = AMFTYPE_NUMBER;
			}
			break;
		}
	case AMF3_OBJECT:
		{
			CAMFObject *obj = CAMFObject::CreateObj();
			if( obj == NULL )
			{
				return -1;
			}

			int refObjectIndex = -1 ;

			int nRes = AMF3_DecodeObject( obj , pBuffer, nSize, FALSE , &refObjectIndex , ctx);
			if (nRes == -1)
			{
				obj->ReleaseObj();
				return -1;
			}
			if( refObjectIndex >= 0 && ctx != NULL )
			{
				CAMFObject *objRef = ctx->GetObj( refObjectIndex ) ;
				if( objRef != NULL )
				{
					obj->ReleaseObj();
					obj = objRef ;
					obj->AddObjRef() ;

				}

			}
			nSize -= nRes;
			prop->m_value = (CObj *)obj ;
			prop->m_type = AMFTYPE_OBJECT;
			obj->ReleaseObj();
			break;
		}
	case AMF3_ARRAY:
		{
			CAMFObject *obj = CAMFObject::CreateObj();
			if( obj == NULL )
			{
				return -1;
			}

			int refObjectIndex = -1 ;

			int nRes = AMF3_DecodeArray(obj , pBuffer, nSize, FALSE , &refObjectIndex , ctx);
			if (nRes == -1)
			{
				obj->ReleaseObj();
				return -1;
			}
			if( refObjectIndex >= 0 && ctx != NULL )
			{
				CAMFObject *objRef = ctx->GetObj( refObjectIndex ) ;
				if( objRef != NULL )
				{
					obj->ReleaseObj();
					obj = objRef ;
					obj->AddObjRef() ;

				}

			}
			nSize -= nRes;
			prop->m_value = (CObj *)obj ;
			prop->m_type = AMFTYPE_ARRAY;
			obj->ReleaseObj();
			break;
		}
	default:
		{
			return -1 ;
		}

	}

	return nOriginalSize - nSize;
}

static int AMF3_Decode(CAMFObject *obj, const UINT8 *pBuffer, int nSize, int bAMFData,
					   int *refObjectIndex , CAMFParseCtx *ctx)
{
	UINT8 AmfType = *pBuffer ;

	switch (AmfType)
	{
	case AMF3_OBJECT:
		{
			return AMF3_DecodeObject( obj , pBuffer , nSize , bAMFData,
				refObjectIndex , ctx );
			break;
		}
	case AMF3_ARRAY:
		{
			return AMF3_DecodeArray( obj , pBuffer , nSize , bAMFData,
				refObjectIndex , ctx );
			break;
		}
	default:
		{

			LogPrint(RTMP_LOGERROR, LOG_TAG ,"AMF3 type 0X%02X not support!\n", AmfType );
			break;
		}
	}

	return -1;

}

static int AMF3_DecodeArray(CAMFObject *obj, const UINT8 *pBuffer, int nSize, int bAMFData,
							int *refObjectIndex , CAMFParseCtx *ctx)
{
	int nOriginalSize = nSize;
	INT32 ref;
	int len;




	if (bAMFData)
	{
		pBuffer++;
		nSize--;
	}

	ref = 0;




	len = AMF3ReadInteger(pBuffer, &ref);
	pBuffer += len;
	nSize -= len;



	if ((ref & 1) == 0)
	{				/* object reference, 0xxx */
		UINT32 objectIndex = (ref >> 1);

		if( refObjectIndex != NULL )
		{
			*refObjectIndex = (int) objectIndex ;
		}

		LogPrint(RTMP_LOGDEBUG,LOG_TAG, "Object reference, index: %d\n", objectIndex);
	}
	else				/* object instance */
	{
		INT32 nArrayLen = (ref >> 1);

		int bError = FALSE;

		CAMFObjectProperty *prop = NULL ;

		if( pBuffer[0] == 1 )
		{
			pBuffer ++ ;
			nSize -- ;
		}

		if( ctx != NULL )
		{
			ctx->AddObj( obj );
		}


		while (nArrayLen > 0)
		{

			int nRes;
			nArrayLen--;

			if( prop == NULL )
			{
				prop = CAMFObjectProperty::CreateObj() ;
				if( prop == NULL )
				{
					return -1 ;
				}
			}
			nRes = AMF3Prop_Decode( prop, pBuffer, nSize, FALSE, ctx);
			if (nRes == -1)
			{
				bError = TRUE;
			}
			else
			{
				nSize -= nRes;
				pBuffer += nRes;
				obj->AddProp( prop );
				prop = NULL ;
			}
		}

		if( prop != NULL )
		{
			prop->ReleaseObj() ;
		}



		if (bError)
		{
			return -1;
		}






	}



	return nOriginalSize - nSize;
}



static int AMF3_DecodeObject(CAMFObject *obj, const UINT8 *pBuffer, int nSize, int bAMFData,
							 int *refObjectIndex , CAMFParseCtx *ctx)
{
	int nOriginalSize = nSize;
	INT32 ref;
	int len;




	if (bAMFData)
	{
		pBuffer++;
		nSize--;
	}

	ref = 0;
	len = AMF3ReadInteger(pBuffer, &ref);
	pBuffer += len;
	nSize -= len;



	if ((ref & 1) == 0)
	{				/* object reference, 0xxx */
		UINT32 objectIndex = (ref >> 1);

		if( refObjectIndex != NULL )
		{
			*refObjectIndex = (int) objectIndex ;
		}

		LogPrint(RTMP_LOGDEBUG,LOG_TAG, "Object reference, index: %d\n", objectIndex);
	}
	else				/* object instance */
	{
		INT32 classRef = (ref >> 1);

		AMF3ClassDef cd ;
		AMF3ClassDef *pcd = NULL ;



		if ((classRef & 0x1) == 0)
		{			/* class reference */
			UINT32 classIndex = (classRef >> 1);
			LogPrint(RTMP_LOGDEBUG,LOG_TAG, "Class reference: %d\n", classIndex);

			if( ctx != NULL )
			{
				pcd = ctx->GetClass( classIndex );
			}

			if( pcd == NULL )
			{
				pcd = &cd ;
			}
		}
		else
		{
			pcd = &cd ;
			INT32 classExtRef = (classRef >> 1);
			int i;

			pcd->cd_externalizable = (classExtRef & 0x1) == 1;
			pcd->cd_dynamic = ((classExtRef >> 1) & 0x1) == 1;

			pcd->cd_num = classExtRef >> 2;

			/* class name */

			len = AMF3ReadString(pBuffer, pcd->cd_name);
			nSize -= len;
			pBuffer += len;

			/*std::string str = className; */

			LogPrint(RTMP_LOGDEBUG,LOG_TAG,
				"Class name: %s, externalizable: %d, dynamic: %d, classMembers: %d\n",
				(const char *)pcd->cd_name, pcd->cd_externalizable, pcd->cd_dynamic,
				pcd->cd_num);

			for (i = 0; i < pcd->cd_num; i++)
			{
				CObjVar *memberName = new CObjVar ;
				if( memberName == NULL )
				{
					return -1 ;
				}
				len = AMF3ReadString(pBuffer, (*memberName) );
				LogPrint(RTMP_LOGDEBUG,LOG_TAG, "Member: %s\n", (const char *)(*memberName) );
				pcd->AddProp( memberName );
				nSize -= len;
				pBuffer += len;
			}

			if( ctx != NULL )
			{
				ctx->AddClass( cd );
			}
		}

		if( ctx != NULL )
		{
			ctx->AddObj( obj );
		}


		/* add as referencable object */

		if ( pcd->cd_externalizable)
		{
			int nRes;

			CAMFObjectProperty  *prop = CAMFObjectProperty::CreateObj();
			if( prop == NULL )
			{
				return -1;
			}



			LogPrint(RTMP_LOGDEBUG,LOG_TAG, "Externalizable, TODO check\n");

			nRes = AMF3Prop_Decode(prop, pBuffer, nSize, FALSE, ctx);
			if (nRes == -1)
			{
				LogPrint(RTMP_LOGDEBUG,LOG_TAG, "%s, failed to decode AMF3 property!\n",
					__FUNCTION__);
			}
			else
			{
				nSize -= nRes;
				pBuffer += nRes;
			}

			prop->m_name = "DEFAULT_ATTRIBUTE" ;
			obj->AddProp( prop);
		}
		else
		{
			int nRes, i;
			for (i = 0; i <  pcd->cd_num; i++)	/* non-dynamic */
			{
				CAMFObjectProperty  *prop = CAMFObjectProperty::CreateObj();
				if( prop == NULL )
				{
					return -1;
				}

				nRes = AMF3Prop_Decode(prop, pBuffer, nSize, FALSE, ctx);
				if (nRes == -1)
				{
					LogPrint(RTMP_LOGDEBUG,LOG_TAG, "%s, failed to decode AMF3 property!\n",
						__FUNCTION__);
				}



				CObjVar *name =  pcd->GetProp( i );

				if( name != NULL )
				{
					prop->m_name =  *name ;
				}

				obj->AddProp( prop);
				prop = NULL ;

				pBuffer += nRes;
				nSize -= nRes;
			}
			if ( pcd->cd_dynamic)
			{
				int len = 0;

				do
				{
					CAMFObjectProperty  *prop = CAMFObjectProperty::CreateObj();
					if( prop == NULL )
					{
						return -1;
					}

					nRes = AMF3Prop_Decode(prop, pBuffer, nSize, TRUE, ctx);
					len = prop->m_name.DataLength();

					if( len > 0 )
					{
						obj->AddProp( prop );
					}
					else
					{
						prop->ReleaseObj();
					}
					prop = NULL ;


					pBuffer += nRes;
					nSize -= nRes;


				}
				while (len > 0);
			}
		}
		LogPrint(RTMP_LOGDEBUG,LOG_TAG, "class object!\n");
	}
	return nOriginalSize - nSize;


}
static int AMF_DecodeArray(CAMFObject *obj, const UINT8 *pBuffer, int nSize,
						   int nArrayLen, int bDecodeName , CAMFParseCtx *ctx)
{
	int nOriginalSize = nSize;
	int bError = FALSE;

	CAMFObjectProperty *prop = NULL ;

	while (nArrayLen > 0)
	{

		int nRes;
		nArrayLen--;

		if( prop == NULL )
		{
			prop = CAMFObjectProperty::CreateObj() ;
			if( prop == NULL )
			{
				return -1 ;
			}
		}
		nRes = AMFProp_Decode( prop, pBuffer, nSize, bDecodeName, ctx);
		if (nRes == -1)
		{
			bError = TRUE;
		}
		else
		{
			nSize -= nRes;
			pBuffer += nRes;
			obj->AddProp( prop );
			prop = NULL ;
		}
	}

	if( prop != NULL )
	{
		prop->ReleaseObj() ;
	}


	if (bError)
	{
		return -1;
	}


	return nOriginalSize - nSize;
}


static int AMFProp_Decode(CAMFObjectProperty *prop, const UINT8 *pBuffer, int nSize,
						  int bDecodeName, CAMFParseCtx *ctx)
{
	int nOriginalSize = nSize;
	int nRes = - 1;
	if (nSize == 0 || !pBuffer)
	{
		LogPrint( RTMP_LOGDEBUG, LOG_TAG ,"%s: Empty buffer/no buffer pointer!\n", __FUNCTION__);
		return -1;
	}


	if (bDecodeName && nSize < 4)
	{				
		/* at least name (length + at least 1 byte) and 1 byte of data */
		LogPrint(RTMP_LOGDEBUG, LOG_TAG ,
			"%s: Not enough data for decoding with name, less than 4 bytes!\n",
			__FUNCTION__);
		return -1;
	}

	if (bDecodeName)
	{
		unsigned short nNameSize = AMF_DecodeInt16(pBuffer);
		if (nNameSize > nSize - 2)
		{
			LogPrint(RTMP_LOGDEBUG, LOG_TAG ,
				"%s: Name size out of range: namesize (%d) > len (%d) - 2\n",
				__FUNCTION__, nNameSize, nSize);
			return -1;
		}

		AMF_DecodeShortString(pBuffer, prop->m_name);
		nSize -= 2 + nNameSize;
		pBuffer += 2 + nNameSize;
	}

	if (nSize == 0)
	{
		return -1;
	}
	nSize--;

	prop->m_type =  (RtmpAmfType)(*pBuffer);
	pBuffer++ ;
	switch (prop->m_type)
	{
	case AMFTYPE_STRING:
		{
			unsigned short nStringSize = AMF_DecodeInt16(pBuffer);

			if (nSize < (long)nStringSize + 2)
			{
				return -1;
			}
			AMF_DecodeShortString(pBuffer, prop->m_value);
			nSize -= (2 + nStringSize);
			break ;
		}
	case AMFTYPE_NUMBER:
		{
			if ( nSize < 8 )
			{
				return -1;
			}
			prop->m_value = AMF_DecodeNumber(pBuffer);
			nSize -= 8;
			break ;
		}
	case AMFTYPE_OBJECT:
		{
			CAMFObject *obj = CAMFObject::CreateObj();
			if( obj == NULL )
			{
				return -1;
			}
			int nRes = AMF_Decode(obj, pBuffer, nSize, TRUE, ctx);
			if (nRes == -1)
			{
				obj->ReleaseObj();
				return -1;
			}
			nSize -= nRes;
			prop->m_value = (CObj *)obj ;
			obj->ReleaseObj();
			break ;
		}
	case AMFTYPE_BOOLEAN:
		{
			if (nSize < 1)
			{
				return -1;
			}
			prop->m_value = (bool)AMF_DecodeBoolean(pBuffer);
			nSize--;
			break ;
		}
	case AMFTYPE_END_OF_OBJECT:
		{
			return -1;
			break;
		}
	case AMFTYPE_NULL:
	case AMFTYPE_UNDEFINED:
		{
			prop->m_type = AMFTYPE_NULL ;
			prop->m_value.SetNull();
			break ;
		}
	case AMFTYPE_MIXED_ARRAY:
		{
			nSize -= 4;

			CAMFObject *obj = CAMFObject::CreateObj();
			if( obj == NULL )
			{
				return -1;
			}
			/* next comes the rest, mixed array has a final 0x000009 mark and names, so its an object */
			nRes = AMF_Decode( obj , pBuffer + 4, nSize, TRUE, ctx);
			if (nRes == -1)
			{
				obj->ReleaseObj();
				return -1;
			}
			nSize -= nRes;
			prop->m_value = (CObj *)obj ;
			prop->m_type = AMFTYPE_OBJECT ;
			obj->ReleaseObj();
			break ;
		}
	case AMFTYPE_DATE:
		{
			if ( nSize < 10 )
			{
				return -1;
			}

			prop->m_value = AMF_DecodeNumber(pBuffer);
			prop->m_UTCOffset = AMF_DecodeInt16(pBuffer + 8);

			nSize -= 10;
			break;
		}
	case AMFTYPE_LONG_STRING:
		{
			unsigned int nStringSize = AMF_DecodeInt32(pBuffer);
			if (nSize < nStringSize + 4)
			{
				return -1;
			}
			AMF_DecodeLongString(pBuffer, prop->m_value);
			nSize -= (4 + nStringSize);
			prop->m_type = AMFTYPE_STRING;
			break;
		}
	case AMFTYPE_ARRAY:
		{
			unsigned int nArrayLen = AMF_DecodeInt32(pBuffer);
			nSize -= 4;


			CAMFObject *obj = CAMFObject::CreateObj();
			if( obj == NULL )
			{
				return -1;
			}

			nRes = AMF_DecodeArray( obj , pBuffer + 4, nSize,
				nArrayLen, FALSE, ctx);

			if (nRes == -1)
			{
				obj->ReleaseObj();
				return -1;
			}
			nSize -= nRes;
			prop->m_value = (CObj *)obj ;
			prop->m_type = AMFTYPE_OBJECT;
			obj->ReleaseObj();
			break ;
		}
	case AMFTYPE_AMF3_OBJECT:
		{
			UINT8 AmfType = *pBuffer;

			if( AmfType == AMF3_ARRAY || AmfType == AMFTYPE_OBJECT )
			{
				CAMFObject *obj = CAMFObject::CreateObj( );
				if( obj == NULL )
				{
					return -1;
				}

				int refObjectIndex = -1;


				int nRes = AMF3_Decode( obj, pBuffer, nSize, TRUE, &refObjectIndex, ctx );
				if( nRes == -1 )
				{
					obj->ReleaseObj( );
					return -1;
				}

				if( refObjectIndex >= 0 && ctx != NULL )
				{
					CAMFObject *objRef = ctx->GetObj( refObjectIndex );
					if( objRef != NULL )
					{
						obj->ReleaseObj( );
						obj = objRef;
						obj->AddObjRef( );

					}

				}

				nSize -= nRes;
				prop->m_value = ( CObj * )obj;
				if( AmfType == AMF3_ARRAY )
				{
					prop->m_type = AMFTYPE_ARRAY;
				}
				else
				{
					prop->m_type = AMFTYPE_OBJECT;
				}

				obj->ReleaseObj( );
			}
			else
			{

				nRes = AMF3Prop_Decode( prop, pBuffer, nSize, FALSE, ctx  );
				if( nRes == -1 )
				{
					return -1;
				}
				nSize -= nRes;
			}


			break ;
		}
	default:
		{
			return -1;
		}

	}
	return nOriginalSize - nSize;
}
static int AMF_Decode(CAMFObject *obj, const UINT8 *pBuffer, int nSize, int bDecodeName, CAMFParseCtx *ctx)
{
	int nOriginalSize = nSize ;
	int bError = FALSE;		/* if there is an error while decoding - try to at least find the end mark AMF_OBJECT_END */
	CAMFObjectProperty *prop = NULL ;

	while ( nSize > 0 )
	{
		int nRes;

		if ( nSize >=3 && AMF_DecodeInt24(pBuffer) == AMFTYPE_END_OF_OBJECT)
		{
			nSize -= 3;
			bError = FALSE;
			break;
		}

		if ( bError )
		{
			//RTMP_Log(RTMP_LOGERROR,
			//	"DECODING ERROR, IGNORING BYTES UNTIL NEXT KNOWN PATTERN!");
			nSize--;
			pBuffer++;
			continue;
		}

		if( prop == NULL )
		{
			prop = CAMFObjectProperty::CreateObj();
			if( prop == NULL )
			{
				bError = TRUE;
				continue;
			}
		}
		nRes = AMFProp_Decode(prop, pBuffer, nSize, bDecodeName, ctx);
		if (nRes == -1)
		{
			bError = TRUE;
		}
		else
		{
			nSize -= nRes;
			pBuffer += nRes;

			/*
			if( prop->m_name.DataLength() < 1 &&  prop->m_value.DataLength() < 1 )
			{
			prop->ReleaseObj() ;
			prop = NULL ;

			}
			else
			*/
			{
				if( obj->AddProp( prop ) )
				{
					prop = NULL ;
				}
			}

		}
	}


	if( prop != NULL )
	{
		prop->ReleaseObj();
	}

	if ( bError )
	{
		return -1;
	}


	return nOriginalSize - nSize;
}
////////////////////////////////////////////////////////////////
CAMFBuffer::CAMFBuffer()
{
	m_chunksize = RTMP_DEFAULT_CHUNKSIZE ;
	m_amfsize = 0 ;
	SafeTail = 16;
	m_channelId = 0x03 ;
	bMalloc2 = TRUE;
}
CAMFBuffer::~CAMFBuffer()
{

}
BOOL CAMFBuffer::SetWritePos(int pos, int Align)
{
    if (Align == -1)
    {
        Align = _ALIGN_ENC;
    }
    if (pos <= WritePos)
    {
        WritePos = pos;
        if (ReadPos > WritePos)
        {
            ReadPos = WritePos;
        }
        return TRUE;
    }
    else
    {
        BOOL bOk = Alloc(pos, Align);

        if (bOk)
        {
            WritePos = pos;
            ReadPos = 0;
        }

        return bOk;
    }
}
BOOL CAMFBuffer::SetReadPos(int pos, int Align)
{
	if(Align == -1 )
	{
		Align = _ALIGN_ENC ;
	}
	if( pos <= WritePos )
	{
		ReadPos = pos ;
		return TRUE ;
	}
	else
	{
		BOOL bOk = Alloc( pos , Align );

		if( bOk )
		{
			ReadPos = pos ;
			WritePos = pos ;
		}

		return bOk ;
	}
}
BOOL CAMFBuffer::Write( const void* Ptr, int Length, int Align)
{
	const UINT8 *buf = ( UINT8 * ) Ptr ;
	int Totol = 0 ;
	int nCpyed = 0 ;

	while( Totol < Length )
	{
		if( m_amfsize > 0 )
		{
			if( m_amfsize % m_chunksize == 0 )
			{
				UINT8 tmp = 0xC0 | m_channelId ;
				CObjBuffer::Write( &tmp , 1 ,  Align );
			}
		}
		nCpyed = MinInt( Length-Totol ,  m_chunksize - (m_amfsize % m_chunksize)  );
		CObjBuffer::Write( buf+Totol , nCpyed ,  Align );
		Totol += nCpyed ;
		m_amfsize += nCpyed ;

	}

	return TRUE ;
}
/////////////////////////////////////////////////////////////////
_CPP_UTIL_DYNAMIC_IMP(CAMFObjectProperty)
_CPP_UTIL_CLASSNAME_IMP(CAMFObjectProperty)
_CPP_UTIL_QUERYOBJ_IMP(CAMFObjectProperty ,CObj)
CAMFObjectProperty::CAMFObjectProperty()
{
	m_type = AMFTYPE_INVALID ;
	m_UTCOffset = 0 ;
}
CAMFObjectProperty::~CAMFObjectProperty()
{

}
CAMFObject *CAMFObjectProperty::GetAMFObject()
{
	CObj *obj = m_value ;

	if( obj == NULL )
	{
		return NULL ;
	}

	return _CPP_UTIL_DYNCAST(obj,CAMFObject);

}
void CAMFObjectProperty::Encode(CAMFBuffer &buf)
{
	UINT8 tmp = 0 ;
	int nameLen = m_name.DataLength();
	if( m_value.m_type != CObjVar::ObjVarTypeNULL && m_name.m_type == CObjVar::ObjVarTypeString  && nameLen > 0 )
	{
		tmp = nameLen >> 8 ;
		buf.Write( &tmp , 1 , _ALIGN_ENC );
		tmp = nameLen & 0xff ;
		buf.Write( &tmp , 1 , _ALIGN_ENC );
		buf.Write( m_name.Data() , nameLen , _ALIGN_ENC );
	}

	CAMFObject *obj = GetAMFObject();
	if( obj != NULL )
	{
		obj->Encode( buf );
		return ;
	}


	switch (m_value.m_type)
	{
	case CObjVar::ObjVarTypeInt32:
	case CObjVar::ObjVarTypeInt64:
	case CObjVar::ObjVarTypeDouble:
		CAMFObject::EncodeNumber( buf, (double) m_value );
		break;

	case CObjVar::ObjVarTypeBool:
		CAMFObject::EncodeBoolean( buf, (int)m_value );
		break;

	case CObjVar::ObjVarTypeString:
		CAMFObject::EncodeString(buf , m_value );
		break;

	case CObjVar::ObjVarTypeNULL:
		tmp = AMFTYPE_NULL ;
		buf.Write( &tmp , 1 , _ALIGN_ENC );
		break;
	default :
		tmp = AMFTYPE_NULL ;
		buf.Write( &tmp , 1 , _ALIGN_ENC );
		break;
	};




}
void CAMFObjectProperty::Dump()
{
	BOOL bUnknownType = FALSE ;
	if( m_name.m_type == CObjVar::ObjVarTypeNULL )
	{
		LogPrint( _LOG_LEVEL_DEBUG, LOG_TAG, "name is NULL\n" );
	}
	else if( m_name.m_type != CObjVar::ObjVarTypeUnknown )
	{
		LogPrint( _LOG_LEVEL_DEBUG, LOG_TAG, "name = %s\n", ( const char* )m_name );
	}


	switch ( m_value.m_type )
	{
	case CObjVar::ObjVarTypeString:
		{
			LogPrint( _LOG_LEVEL_DEBUG, LOG_TAG, "value = %s(string)\n", ( const char* )m_value );
			break ;
		}
	case CObjVar::ObjVarTypeInt32:
	case CObjVar::ObjVarTypeInt64:
		{
			LogPrint( _LOG_LEVEL_DEBUG, LOG_TAG, "value = %d(integer)\n", ( int )m_value );
			break ;
		}
	case CObjVar::ObjVarTypeDouble:
		{
			LogPrint( _LOG_LEVEL_DEBUG, LOG_TAG, "value = %f(double)\n", ( double )m_value );
			break ;
		}
	case CObjVar::ObjVarTypeBool:
		{
			LogPrint( _LOG_LEVEL_DEBUG, LOG_TAG, "value = %d(bool)\n", ( int )m_value );
			break ;
		}
	case CObjVar::ObjVarTypeNULL:
		{
			LogPrint( _LOG_LEVEL_DEBUG, LOG_TAG, "value = NULL\n" );
			break ;
		}
	default:
		{
			bUnknownType = TRUE ;;
			break ;
		}
	}

	if( bUnknownType )
	{
		CAMFObject *obj = GetAMFObject() ;
		if( obj != NULL )
		{
			if( m_type == AMFTYPE_OBJECT )
			{
				LogPrint( _LOG_LEVEL_DEBUG, LOG_TAG, "value is Object\n" );
			}	
			else if( m_type == AMFTYPE_ARRAY )
			{
				LogPrint( _LOG_LEVEL_DEBUG, LOG_TAG, "value is Array\n" );
			}
			obj->Dump();

		}
		else
		{
			LogPrint( _LOG_LEVEL_DEBUG, LOG_TAG, "value is unknown type\n" );
		}
	}


	LogPrint( _LOG_LEVEL_DEBUG, LOG_TAG, "-----------------------------------------------\n" );
}
/////////////////////////////////////////////////////////////////
_CPP_UTIL_DYNAMIC_IMP(CAMFObject)
_CPP_UTIL_CLASSNAME_IMP(CAMFObject)
_CPP_UTIL_QUERYOBJ_IMP(CAMFObject ,CObj)
CAMFObject::CAMFObject()
{
	MtxInit( &m_mtx , 0 );
	m_count = 0 ;
}
CAMFObject::~CAMFObject()
{
	MtxLock( &m_mtx );

	int i = 0 ;
	for( i = 0 ; i < m_count ; i ++ )
	{
		CAMFObjectProperty *prop = NULL ;
		memcpy( &prop , m_props.Data + ( i * sizeof(CAMFObjectProperty *) )  , sizeof(CAMFObjectProperty *)  );
		prop->ReleaseObj();
	}


	MtxUnLock( &m_mtx  );

	MtxDestroy( &m_mtx );
}
UINT8 CAMFObject::HeadSize2HeadType( int size )
{
	switch ( size )
	{
	case 1:
		{
			return 3 ;
		}
	case 4:
		{
			return 2 ;
		}
	case 8:
		{
			return 1 ;
		}
	case 12:
		{
			return 0 ;
		}
	}

	return -1;
}

void CAMFObject::Encode(CAMFBuffer &buf)
{
	EncodeObjectBegin( buf );


	MtxLock( &m_mtx );
	int i = 0 ;
	for( i = 0 ; i < m_count ; i ++ )
	{
		CAMFObjectProperty *prop = NULL ;
		memcpy( &prop , m_props.Data + ( i * sizeof(CAMFObjectProperty *) )  , sizeof(CAMFObjectProperty *)  );
		prop->Encode(buf);
	}
	MtxUnLock( &m_mtx  );

	EncodeObjectEnd( buf );

}
CObjVar *CAMFObject::SearchVar(const char *name)
{
	CObjVar *var = NULL ;

	MtxLock( &m_mtx );

	int i = 0 ;
	for( i = 0 ; i < m_count ; i ++ )
	{
		CAMFObjectProperty *prop = NULL ;
		memcpy( &prop , m_props.Data + ( i * sizeof(CAMFObjectProperty *) )  , sizeof(CAMFObjectProperty *)  );

		if( strcmpix( prop->m_name , name ) == 0 )
		{
			var = &  (prop->m_value) ;
			break ;		
		}
		else
		{
			CAMFObject *obj = prop->GetAMFObject();
			if( obj != NULL )
			{
				var = obj->SearchVar( name );
				if( var != NULL )
				{
					break ;
				}
			}
		}
	}

	MtxUnLock( &m_mtx  );

	return var ;

}
void CAMFObject::Dump()
{
	MtxLock( &m_mtx );

	LogPrint( _LOG_LEVEL_DEBUG, LOG_TAG, "*************Object begin (%d Objects) %p******************\n", m_count, this );
	int i = 0 ;
	for( i = 0 ; i < m_count ; i ++ )
	{
		CAMFObjectProperty *prop = NULL ;
		memcpy( &prop , m_props.Data + ( i * sizeof(CAMFObjectProperty *) )  , sizeof(CAMFObjectProperty *)  );
		prop->Dump(); 	
	}
	LogPrint( _LOG_LEVEL_DEBUG, LOG_TAG, "*************Object end (%d Objects) %p******************\n", m_count, this );

	MtxUnLock( &m_mtx  );
}
CAMFObject *CAMFObject::CreateFromAMFBuffer(const UINT8 *buf , int nLen)
{
	CAMFObject *obj = CAMFObject::CreateObj();
	if( obj == NULL )
	{
		return NULL ;
	}
	CAMFParseCtx * ctx = CAMFParseCtx::CreateObj() ;
	AMF_Decode( obj , buf , nLen , 0  , ctx );
	ctx->ReleaseObj();
	return obj ;
}
void CAMFObject::EncodeInt8(CAMFBuffer &buf, INT8 nVal)
{
	buf.Write( &nVal , 1 , _ALIGN_ENC );
}
int CAMFObject::EncodeInt8(UINT8 *buf, INT8 nVal)
{
	buf[0] = nVal ;
	return 1 ;
}
void CAMFObject::EncodeInt16(CAMFBuffer &buf, INT16 nVal)
{

	UINT8 output[2];
	output[1] = nVal & 0xff;
	output[0] = nVal >> 8;

	buf.Write( output , 2 , _ALIGN_ENC );

}
int CAMFObject::EncodeInt16(UINT8 *output, INT16 nVal)
{
	output[1] = nVal & 0xff;
	output[0] = nVal >> 8;

	return 2 ;

}
void CAMFObject::EncodeInt24(CAMFBuffer &buf, int nVal)
{
	UINT8 output[3];
	output[2] = nVal & 0xff;
	output[1] = nVal >> 8;
	output[0] = nVal >> 16;

	buf.Write( output , 3 , _ALIGN_ENC );
}
int CAMFObject::EncodeInt24(UINT8 *output, int nVal)
{
	output[2] = nVal & 0xff;
	output[1] = nVal >> 8;
	output[0] = nVal >> 16;

	return 3 ;
}
void CAMFObject::EncodeInt32(CAMFBuffer &buf, INT32 nVal)
{
	UINT8 output[4];
	output[3] = nVal & 0xff;
	output[2] = nVal >> 8;
	output[1] = nVal >> 16;
	output[0] = nVal >> 24;

	buf.Write( output , 4 , _ALIGN_ENC );
}
void CAMFObject::EncodeInt32LE(CAMFBuffer &buf, INT32 nVal)
{
	UINT8 output[4];
	output[0]=nVal;
	nVal>>=8;
	output[1]=nVal;
	nVal>>=8;
	output[2]=nVal;
	nVal>>=8;
	output[3]=nVal;
	buf.Write( output , 4 , _ALIGN_ENC );
}
int CAMFObject::EncodeInt32(UINT8 *output, INT32 nVal)
{
	output[3] = nVal & 0xff;
	output[2] = nVal >> 8;
	output[1] = nVal >> 16;
	output[0] = nVal >> 24;
	return 4 ;
}
int CAMFObject::EncodeInt32LE(UINT8 *output,int nVal)
{
	output[0]=nVal;
	nVal>>=8;
	output[1]=nVal;
	nVal>>=8;
	output[2]=nVal;
	nVal>>=8;
	output[3]=nVal;
	return 4;
}

INT16 CAMFObject::DecodeInt16(const UINT8 *data)
{
	unsigned char *c = (unsigned char *) data;
	unsigned short val;
	val = (c[0] << 8) | c[1];
	return val;
}
INT32 CAMFObject::DecodeInt24(const UINT8 *data)
{
	unsigned char *c = (unsigned char *) data;
	INT32 val;
	val = (c[0] << 16) | (c[1] << 8) | c[2];
	return val;
}

UINT32 CAMFObject::DecodeInt32(const UINT8 *data)
{
	unsigned char *c = (unsigned char *)data;
	UINT32 val;
	val = (c[0] << 24) | (c[1] << 16) | (c[2] << 8) | c[3];
	return val;
}
UINT32 CAMFObject::DecodeInt32LE(const UINT8 *data)
{
	unsigned char *c = (unsigned char *)data;
	UINT32 val;

	val = (c[3] << 24) | (c[2] << 16) | (c[1] << 8) | c[0];
	return val;
}
void CAMFObject::EncodeString(CAMFBuffer &buf, const CObjVar &bv)
{
	UINT8 type = 0 ;
	if ( bv.DataLength() < 65536)
	{
		type = AMFTYPE_STRING;
		buf.Write( &type , 1 , _ALIGN_ENC );
		EncodeInt16( buf, (INT16)bv.DataLength() );

	}
	else
	{
		type = AMFTYPE_LONG_STRING;
		buf.Write( &type , 1 , _ALIGN_ENC );
		EncodeInt32( buf, (INT32)bv.DataLength() );
	}

	buf.Write( bv.Data() , bv.DataLength() , _ALIGN_ENC );
}
void CAMFObject::EncodeString(CAMFBuffer &buf, const  char * bv)
{
	int nLen = 0 ;
	int len_str = 0 ;
	if( bv != NULL )
	{
		len_str = strlen(bv);
	}
	else
	{
		bv = _szNULL ;
	}

	if ( len_str < 65536)
	{
		EncodeInt8( buf , AMFTYPE_STRING);
		EncodeInt16( buf, (INT16)len_str );

	}
	else
	{
		EncodeInt8( buf , AMFTYPE_LONG_STRING);
		EncodeInt32( buf, (INT32)len_str );
	}

	buf.Write( bv , len_str , _ALIGN_ENC );

	return ;
}
int CAMFObject::EncodeString(UINT8 *buf, const CObjVar &bv)
{
	int nLen = 0 ;
	if ( bv.DataLength() < 65536)
	{
		nLen += EncodeInt8( buf , AMFTYPE_STRING);
		nLen += EncodeInt16( buf, (INT16)bv.DataLength() );

	}
	else
	{
		nLen += EncodeInt8( buf , AMFTYPE_LONG_STRING);
		nLen += EncodeInt32( buf, (INT32)bv.DataLength() );
	}

	memcpy( buf  + nLen,  bv.Data() , bv.DataLength() );
	return ( nLen + bv.DataLength() );
}
int CAMFObject::EncodeString(UINT8 *buf, const char *bv)
{
	int nLen = 0 ;
	int len_str = 0 ;
	if( bv != NULL )
	{
		len_str = strlen(bv);
	}
	else
	{
		bv = _szNULL ;
	}

	if ( len_str < 65536)
	{
		nLen += EncodeInt8( buf , AMFTYPE_STRING);
		nLen += EncodeInt16( buf, (INT16)len_str );

	}
	else
	{
		nLen += EncodeInt8( buf , AMFTYPE_LONG_STRING);
		nLen += EncodeInt32( buf, (INT32)len_str );
	}

	memcpy( buf  + nLen,  bv , len_str );
	return ( nLen + len_str );
}
void CAMFObject::EncodeNumber(CAMFBuffer &buf, double dVal)
{
	UINT8 type = AMFTYPE_NUMBER ;

	buf.Write( &type , 1 , _ALIGN_ENC );
	if( !is_little_endian )
	{
		buf.Write( &dVal , 8 ,  _ALIGN_ENC );
	}
	else
	{
		unsigned char *ci;
		ci = (unsigned char *)&dVal;
		UINT8 co[8];
		co[0] = ci[7];
		co[1] = ci[6];
		co[2] = ci[5];
		co[3] = ci[4];
		co[4] = ci[3];
		co[5] = ci[2];
		co[6] = ci[1];
		co[7] = ci[0];

		buf.Write( co , 8 ,  _ALIGN_ENC );
	}
}
void CAMFObject::EncodeObjectBegin(CAMFBuffer &buf)
{
	EncodeInt8( buf , AMFTYPE_OBJECT );
}
void CAMFObject::EncodeObjectEnd(CAMFBuffer &buf)
{
	EncodeInt24( buf , AMFTYPE_END_OF_OBJECT );
}
int CAMFObject::EncodeObjectBegin(UINT8 *buf)
{
	buf[0] =  AMFTYPE_OBJECT ;
	return 1;
}
int CAMFObject::EncodeObjectEnd(UINT8 *buf)
{
	return EncodeInt24( buf , AMFTYPE_END_OF_OBJECT );
}
int CAMFObject::EncodeNumber(UINT8 *buf, double dVal)
{

	buf[0]  = AMFTYPE_NUMBER ;
	if( !is_little_endian )
	{
		memcpy( buf + 1 , &dVal , 8   ) ;
	}
	else
	{
		unsigned char *ci;
		ci = (unsigned char *)&dVal;
		UINT8 *co = buf + 1 ;
		co[0] = ci[7];
		co[1] = ci[6];
		co[2] = ci[5];
		co[3] = ci[4];
		co[4] = ci[3];
		co[5] = ci[2];
		co[6] = ci[1];
		co[7] = ci[0];


	}

	return 9 ;
}
void CAMFObject::EncodeBoolean(CAMFBuffer &buf, int bVal)
{
	UINT8 tmp = AMFTYPE_BOOLEAN ;
	buf.Write( &tmp , 1 , _ALIGN_ENC );

	tmp = bVal ? 0x01 : 0x00;

	buf.Write( &tmp , 1 , _ALIGN_ENC );
}
int CAMFObject::EncodeBoolean(UINT8 *buf, int bVal)
{
	buf[0] = AMFTYPE_BOOLEAN ;
	buf[1] = bVal ? 0x01 : 0x00;

	return 2 ;
}
void CAMFObject::EncodeNull(CAMFBuffer &buf)
{
	UINT8 tmp = AMFTYPE_NULL ;
	buf.Write( &tmp , 1 , _ALIGN_ENC );
}
int CAMFObject::EncodeNull(UINT8 *buf)
{
	buf[0] = AMFTYPE_NULL ;

	return 1 ;
}
void CAMFObject::EncodeNamedString(CAMFBuffer &buf, const CObjVar &strName, const CObjVar &strValue)
{
	EncodeInt16( buf ,(INT16)strName.DataLength() );
	buf.Write( strName.Data() , strName.DataLength() , _ALIGN_ENC );

	EncodeString( buf , strValue );
}
/*
void CAMFObject::EncodeObjectNameString(CAMFBuffer &buf, const char *strName)
{
int name_len = 0 ;
if( strName != NULL )
{
name_len = strlen( strName ) ;
}
else
{
strName = _szNULL ;
}
EncodeInt16( buf ,(INT16)name_len );
buf.Write( strName , name_len , _ALIGN_ENC );

}
*/
void CAMFObject::EncodeNamedString(CAMFBuffer &buf, const char *strName, const char *strValue)
{
	int name_len = 0 ;
	if( strName != NULL )
	{
		name_len = strlen( strName ) ;
	}
	else
	{
		strName = _szNULL ;
	}
	EncodeInt16( buf ,(INT16)name_len );
	buf.Write( strName , name_len , _ALIGN_ENC );
	EncodeString( buf , strValue );

}
int CAMFObject::EncodeNamedString(UINT8 *buf, const char *strName, const char *strValue)
{
	int nLen = 0 ;
	int name_len = 0 ;
	if( strName != NULL )
	{
		name_len = strlen( strName ) ;
	}
	else
	{

		strName = _szNULL ;
	}
	nLen += EncodeInt16( buf ,(INT16)name_len );
	memcpy( buf + nLen , strName , name_len );
	nLen += name_len ;
	nLen += EncodeString( buf , strValue );

	return nLen ;
}
int CAMFObject::EncodeNamedNumber(UINT8 *buf, const char *strName, double dVal)
{
	int nLen = 0 ;
	int name_len = 0 ;
	if( strName != NULL )
	{
		name_len = strlen( strName ) ;
	}
	else
	{

		strName = _szNULL ;
	}
	nLen += EncodeInt16( buf ,(INT16)name_len );
	memcpy( buf + nLen , strName , name_len );
	nLen += name_len ;
	nLen += EncodeNumber( buf , dVal );

	return nLen ;
}
int CAMFObject::EncodeNamedBoolean(UINT8 *buf, const char *strName, int bVal)
{
	int nLen = 0 ;
	int name_len = 0 ;
	if( strName != NULL )
	{
		name_len = strlen( strName ) ;
	}
	else
	{

		strName = _szNULL ;
	}
	nLen += EncodeInt16( buf ,(INT16)name_len );
	memcpy( buf + nLen , strName , name_len );
	nLen += name_len ;
	nLen += EncodeBoolean( buf , bVal );

	return nLen ;
}
void CAMFObject::EncodeNamedNumber(CAMFBuffer &buf, const CObjVar &strName, double dVal)
{
	EncodeInt16( buf ,(INT16)strName.DataLength() );
	buf.Write( strName.Data() , strName.DataLength() , _ALIGN_ENC );

	EncodeNumber( buf , dVal );
}
void CAMFObject::EncodeNamedNumber(CAMFBuffer &buf, const char *strName, double dVal)
{
	int name_len = 0 ;
	if( strName != NULL )
	{
		name_len = strlen( strName ) ;
	}
	else
	{

		strName = _szNULL ;
	}
	EncodeInt16( buf ,(INT16)name_len );
	buf.Write( strName , name_len , _ALIGN_ENC );
	EncodeNumber( buf , dVal );

}
void CAMFObject::EncodeNamedBoolean(CAMFBuffer &buf, const CObjVar &strName, int bVal)
{
	EncodeInt16( buf ,(INT16)strName.DataLength() );
	buf.Write( strName.Data() , strName.DataLength() , _ALIGN_ENC );

	EncodeBoolean( buf, bVal );
}
void CAMFObject::EncodeNamedBoolean(CAMFBuffer &buf, const char *strName, int bVal)
{
	int name_len = 0 ;
	if( strName != NULL )
	{
		name_len = strlen( strName ) ;
	}
	else
	{

		strName = _szNULL ;
	}
	EncodeInt16( buf ,(INT16)name_len );
	buf.Write( strName , name_len , _ALIGN_ENC );
	EncodeBoolean( buf, bVal );

}
void CAMFObject::EncodeNamedNull(CAMFBuffer &buf, const CObjVar &strName)
{
	EncodeInt16( buf ,(INT16)strName.DataLength() );
	buf.Write( strName.Data() , strName.DataLength() , _ALIGN_ENC );

	EncodeNull( buf );
}
void CAMFObject::EncodeNamedNull(CAMFBuffer &buf, const char *strName)
{
	int name_len = 0 ;
	if( strName != NULL )
	{
		name_len = (int)strlen( strName ) ;
	}
	else
	{

		strName = _szNULL ;
	}
	EncodeInt16( buf ,(INT16)name_len );
	buf.Write( strName , name_len , _ALIGN_ENC );
	EncodeNull( buf );

}
int  CAMFObject::PropCount()
{
	return m_count ;
}
BOOL CAMFObject::AddProp(CAMFObjectProperty *prop)
{
	if( prop == NULL )
	{
		return FALSE ;
	}
	CMtxAutoLock lock(  &m_mtx );


	BOOL bOk = m_props.Write( &prop , sizeof(CAMFObjectProperty *)  , sizeof(CAMFObjectProperty *) * 16);

	if( bOk )
	{
		m_count = m_props.DataLength()/(sizeof(CAMFObjectProperty *)) ;
	}

	return bOk ;
}
CAMFObjectProperty *CAMFObject::GetProp(int index)
{
	CMtxAutoLock lock(  &m_mtx );

	if( index < 0 || index >= m_count )
	{
		return NULL ;
	}
	CAMFObjectProperty *prop = NULL ;

	memcpy( &prop , m_props.Data + ( index * sizeof(CAMFObjectProperty *) )  , sizeof(CAMFObjectProperty *)  );
	return prop ;
}

















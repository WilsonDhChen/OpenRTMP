
#include"adapi_def.h"


typedef struct tagBUF2ITEM
{
	LIST_ITEM item;
	int nBlockSize;
	int iDataStart;
	int nDataLen;
	unsigned char *data;

}BUF2ITEM,*PBUF2ITEM;

typedef struct tagMEMORY_BUFFER2
{

	MTX_OBJ mtx;
	int nTotalSize;
    int nPosition;
	int nBlockSize;
	int nSeekLength;
	int nMaxFreeBlocks;
	PBUF2ITEM curBlock;
	BOOL bShareData ;

	LIST_ITEM lstFree;
	int nFreeItems;
	LIST_ITEM lstBuf;
	int nUsedItems;

}MEMORY_BUFFER2;


//#define  CheckPosition(x)

#ifndef CheckPosition
static void CheckPosition( MEMORY_BUFFER2 *obj )
{
	int tmp = 0; 
	if( obj != NULL && obj->curBlock  != NULL )
	{
		tmp = obj->nPosition % obj->nBlockSize ;
		if(obj->curBlock->iDataStart != obj->nBlockSize && tmp != obj->curBlock->iDataStart)
		{
			tmp = tmp - 1 ;
			tmp = tmp + 1 ;
		}
	}
	else
	{
		tmp = tmp - 1 ;
		tmp = tmp + 1 ;
	}

}
#endif

MemoryBuffer2Ref * MYAPI  MemoryBuffer2AllocRef()
{
	MemoryBuffer2Ref * ref = (MemoryBuffer2Ref *)malloc( sizeof(MemoryBuffer2Ref) );

	if( ref != NULL )
	{
		memset( ref , 0 , sizeof(MemoryBuffer2Ref) );

	}
	return ref ;
}
void MYAPI  MemoryBuffer2FreeRef(MemoryBuffer2Ref *ref)
{
	MemoryBuffer2Ref *tmp = NULL ;
	while( ref != NULL )
	{
		tmp = ref ;
		ref = ref->Next ;

		MemoryRelease( tmp->Data );
		free( tmp );

	}
}

static  PBUF2ITEM  MemoryBuffer2AllocItem(MEMORY_BUFFER2 *obj,int blocksize)
{
	PBUF2ITEM item = NULL ;

	if( !obj->bShareData  )
	{
		item = (PBUF2ITEM)malloc(sizeof(BUF2ITEM)+blocksize+4);
		if( item == NULL )
		{
			return NULL ;
		}
		ListInit( &(item->item) );
		item->nBlockSize = blocksize ;
		item->iDataStart = 0 ;
		item->nDataLen = 0 ;
		item->data = ((unsigned char *)item)+sizeof(BUF2ITEM) ;
		item->data[blocksize] = 0 ;
	}
	else
	{
		item = (PBUF2ITEM)malloc(sizeof(BUF2ITEM));
		if( item == NULL )
		{
			return NULL ;
		}
		ListInit( &(item->item) );
		item->nBlockSize = blocksize ;
		item->iDataStart = 0 ;
		item->nDataLen = 0 ;
		item->data = MemoryCreate( blocksize + 4, NULL );
		if( item->data == NULL )
		{
			free( item );
			return NULL ;
		}
		item->data[blocksize] = 0 ;
	}



	return item ;
}
static  void  MemoryBuffer2FreeItem( MEMORY_BUFFER2 *obj, PBUF2ITEM item)
{
	if( item != NULL )
	{
		if( !obj->bShareData  )
		{
			free(item);
		}
		else
		{
			MemoryRelease( item->data ) ;
			free(item);
		}

	}
}
static  void  MemoryBuffer2PutItem2Free(MEMORY_BUFFER2 *obj  , PBUF2ITEM item)
{
	if( item == NULL || obj == NULL  )
	{
		return ;
	}
	if( obj->bShareData )
	{
		MemoryBuffer2FreeItem( obj, item ) ;
		return ;
	}
	if( obj->nFreeItems > obj->nMaxFreeBlocks )
	{
		MemoryBuffer2FreeItem(obj,item);
		return ;
	}
	ListAddTail( &(item->item) , &(obj->lstFree));
	obj->nFreeItems ++ ;
}
static  void  MemoryBuffer2PutItem2Used(MEMORY_BUFFER2 *obj , PBUF2ITEM item)
{
	if( item == NULL || obj == NULL  )
	{
		return ;
	}
	ListAddTail( &(item->item) , &(obj->lstBuf));
	obj->nUsedItems ++ ;
}
static  PBUF2ITEM  MemoryBuffer2GetFreeItem( MEMORY_BUFFER2 *obj )
{
	PBUF2ITEM item = NULL ;
	if( ListIsEmpty( & (obj->lstFree) ) )
	{
		return NULL ;
	}
	item = (PBUF2ITEM)obj->lstFree.next ;
	ListDel( &(item->item) );
	ListInit( &(item->item) );
	item->iDataStart = 0 ;
	item->nDataLen = 0 ;
	obj->nFreeItems -- ;
	return item ;
}
static  PBUF2ITEM  MemoryBuffer2GetUsedItem( MEMORY_BUFFER2 *obj )
{
	PBUF2ITEM item = NULL ;
	if( ListIsEmpty( & (obj->lstBuf) ) )
	{
		return NULL ;
	}
	item = (PBUF2ITEM)obj->lstBuf.next ;
	ListDel( &(item->item) );
	ListInit( &(item->item) );
	item->iDataStart = 0 ;
	item->nDataLen = 0 ;
	obj->nUsedItems --  ;
	return item ;
}
void MYAPI MemoryBuffer2Close( MemoryBuffer2Handle handle )
{
	 MEMORY_BUFFER2 *obj =(MEMORY_BUFFER2 *) handle ;
	 PBUF2ITEM item = NULL ;

	 if( obj == NULL )
	 {
		 return ;
	 }

	 MtxLock(&(obj->mtx));

	 item = MemoryBuffer2GetUsedItem( obj );
	 while ( item != NULL )
	 {
		 MemoryBuffer2FreeItem( obj, item );
		 item = MemoryBuffer2GetUsedItem( obj );
	 }

	 item = MemoryBuffer2GetFreeItem( obj );
	 while ( item != NULL )
	 {
		 MemoryBuffer2FreeItem(obj, item );
		 item = MemoryBuffer2GetFreeItem( obj );
	 }
	 MtxUnLock(&(obj->mtx));

	 MtxDestroy( & (obj->mtx) );

	 free(obj);

}
int  MYAPI MemoryBuffer2Length( MemoryBuffer2Handle handle  )
{
	int nTotalSize = 0 ;
	MEMORY_BUFFER2 *obj =(MEMORY_BUFFER2 *) handle ;
	if ( obj == NULL )
	{
		return 0 ;
	}
	MtxLock(&(obj->mtx));
	nTotalSize = (int)obj->nTotalSize  ;
	MtxUnLock(&(obj->mtx));

	return nTotalSize;
}
int MYAPI MemoryBuffer2ReadRef( MemoryBuffer2Handle handle ,MemoryBuffer2Ref **refPtr ,int nSize  ,MemoryBuffer2Ref **refTail )
{
	MEMORY_BUFFER2 *obj =(MEMORY_BUFFER2 *) handle ;
	PBUF2ITEM item = NULL ;
	PBUF2ITEM itemHead = NULL ;
	MemoryBuffer2Ref *refHead= NULL ;
	MemoryBuffer2Ref *ref= NULL ; 
	int nPostion = 0 ;
	
	int nCpyed = 0 ;
	int nCpy = 0 ;
	if ( obj == NULL || !obj->bShareData )
	{
		return  0;
	}
	if( refPtr == NULL || nSize < 1 || ( obj->nTotalSize - obj->nPosition ) < 1 )
	{
		return 0 ;
	}
	nPostion = obj->nPosition ;

	MtxLock(&(obj->mtx));
	itemHead = (PBUF2ITEM)&(obj->lstBuf) ;
	

	if( obj->curBlock == NULL )
	{
		obj->curBlock = (PBUF2ITEM) ( obj->lstBuf.next ) ;
		obj->nPosition = 0 ;
	}


	while( nCpyed < nSize && ( obj->nTotalSize - obj->nPosition ) > 0 )
	{

		item = obj->curBlock ;

		nCpy = MinInt( nSize - nCpyed , item->nDataLen - item->iDataStart);
		if( nCpy > 0  )
		{
			MemoryBuffer2Ref *refCur = NULL ; 
			refCur = MemoryBuffer2AllocRef();
			if( refCur == NULL )
			{
				break ;
			}
			refCur->Data = item->data ;
			MemoryAddRef( refCur->Data );
			refCur->Begin = item->iDataStart ;
			refCur->Length = nCpy ;
			refCur->DataLen = item->nDataLen ;

			if( refHead == NULL )
			{
				refHead = ref = refCur ;
			}
			else
			{
				ref->Next = refCur ;
				ref = refCur ;
			}

			item->iDataStart += nCpy ;
			nCpyed += nCpy ;
			obj->nPosition += nCpy ;

			CheckPosition(obj);



		}


		if( item->iDataStart < item->nDataLen )
		{
			break ;
		}
		else
		{
			if( ( obj->nTotalSize - obj->nPosition ) < 1 )
			{
				break ;
			}
			obj->curBlock->iDataStart = 0 ;
			obj->curBlock = (PBUF2ITEM) (obj->curBlock->item.next) ;
			obj->curBlock->iDataStart = 0 ;

			if( obj->nPosition >= ( obj->nSeekLength + obj->nBlockSize )  )
			{

				item = (PBUF2ITEM)obj->lstBuf.next ;
				if( item == obj->curBlock )
				{
					break ;
				}


				if( item != obj->curBlock )
				{
					ListDel( &(item->item) );
					obj->nPosition  -= item->nDataLen  ;
					obj->nTotalSize -= item->nDataLen  ;
					CheckPosition(obj);
					MemoryBuffer2PutItem2Free(obj,item);

				}


			}

		}

	} 

	while( obj->nPosition >= ( obj->nSeekLength + obj->nBlockSize )  )
	{

		item = (PBUF2ITEM)obj->lstBuf.next ;

		if( item == obj->curBlock )
		{
			break ;
		}

		ListDel( &(item->item) );

		obj->nPosition  -= item->nDataLen ;
		obj->nTotalSize -= item->nDataLen  ;

		CheckPosition(obj);
		MemoryBuffer2PutItem2Free(obj,item);

	}


	*refPtr = refHead ;
	if( refTail != NULL )
	{
		*refTail = ref ;
	}


	
	CheckPosition(obj);

	MtxUnLock(&(obj->mtx));

#ifndef CheckPosition
	if( nSize != nCpyed )
	{
		CheckPosition(NULL);
	}
#endif
	return nCpyed ;
}

int MYAPI MemoryBuffer2Read( MemoryBuffer2Handle handle ,void *buf,int nSize)
{
	MEMORY_BUFFER2 *obj =(MEMORY_BUFFER2 *) handle ;
	PBUF2ITEM item = NULL ;
	PBUF2ITEM itemHead = NULL ;
	unsigned char *dataOut = (unsigned char *)buf ;
	int nCpyed = 0 ;
	int nCpy = 0 ;
	if ( obj == NULL )
	{
		return  -1;
	}
	if( buf == NULL || nSize < 1 || ( obj->nTotalSize - obj->nPosition ) < 1 )
	{
		return 0 ;
	}

	MtxLock(&(obj->mtx));
	itemHead = (PBUF2ITEM)&(obj->lstBuf) ;
	

	if( obj->curBlock == NULL )
	{
		obj->curBlock = (PBUF2ITEM) ( obj->lstBuf.next ) ;
		obj->nPosition = 0 ;
	}
	

	while( nCpyed < nSize && ( obj->nTotalSize - obj->nPosition ) > 0 )
	{

		item = obj->curBlock ;

		nCpy = MinInt( nSize - nCpyed , item->nDataLen - item->iDataStart);
		if( nCpy > 0  )
		{
			memcpy( dataOut + nCpyed, item->data+item->iDataStart,nCpy);

			item->iDataStart += nCpy ;
			nCpyed += nCpy ;
			obj->nPosition += nCpy ;
			
		}


		if( item->iDataStart < item->nDataLen )
		{
			break ;
		}
		else
		{
			if( ( obj->nTotalSize - obj->nPosition ) < 1 )
			{
				break ;
			}
			obj->curBlock->iDataStart = 0 ;
			obj->curBlock = (PBUF2ITEM) (obj->curBlock->item.next) ;
			obj->curBlock->iDataStart = 0 ;

			if( obj->nPosition >= ( obj->nSeekLength + obj->nBlockSize )  )
			{

				item = (PBUF2ITEM)obj->lstBuf.next ;
				if( item == obj->curBlock )
				{
					break ;
				}


				if( item != obj->curBlock )
				{
					ListDel( &(item->item) );
					obj->nPosition  -= item->nDataLen  ;
					obj->nTotalSize -= item->nDataLen  ;
					CheckPosition(obj);
					MemoryBuffer2PutItem2Free(obj,item);

				}


			}

		}

	} 

	while( obj->nPosition >= ( obj->nSeekLength + obj->nBlockSize )  )
	{

		item = (PBUF2ITEM)obj->lstBuf.next ;

		if( item == obj->curBlock )
		{
			break ;
		}

		ListDel( &(item->item) );

		obj->nPosition  -= item->nDataLen ;
		obj->nTotalSize -= item->nDataLen  ;

		CheckPosition(obj);
		MemoryBuffer2PutItem2Free(obj,item);
	}

	CheckPosition(obj);
	MtxUnLock(&(obj->mtx));

	return nCpyed ;
}
int MYAPI MemoryBuffer2Seek(MemoryBuffer2Handle handle ,int pos,int origin )
{
	MEMORY_BUFFER2 *obj =(MEMORY_BUFFER2 *) handle ;
	int nCurPos = 0 ;
	PBUF2ITEM item = NULL ;
	int pos1 = 0 ;
	if ( obj == NULL )
	{
		return  -1;
	}


	MtxLock(&(obj->mtx));

	if( obj->nTotalSize < 1 )
	{
		MtxUnLock(&(obj->mtx));
		return 0 ;
	}

	if( origin == SEEK_SET )
	{
		if( pos == obj->nPosition )
		{
			MtxUnLock(&(obj->mtx));
			return pos ; 
		}
		else
		{
			origin = SEEK_CUR ;
			pos = pos - obj->nPosition ;
		}

	}

	if( origin == SEEK_CUR )
	{
		if( pos == 0 )
		{
			pos = obj->nPosition ;
			MtxUnLock(&(obj->mtx));
			return pos ; 
		}
		else
		{
			if( obj->curBlock != NULL  )
			{
				pos1 = obj->curBlock->iDataStart + pos ;
				if( pos1 >= 0 && pos1 <= obj->curBlock->nDataLen )
				{
					obj->curBlock->iDataStart = pos1 ;
					obj->nPosition += pos ;
					pos = obj->nPosition;
					MtxUnLock(&(obj->mtx));
					return pos ;
				}

			}
			
			pos += obj->nPosition ;
		}
		
	}
	else if( origin == SEEK_END )
	{
		pos += obj->nTotalSize ;
	}


	if( pos <= 0 )
	{
		pos = 0 ;
		obj->curBlock = NULL ;
                //CheckPosition( obj );
	}
	else if( pos >= obj->nTotalSize )
	{
		pos = obj->nTotalSize ;
		obj->curBlock = (PBUF2ITEM)(obj->lstBuf.prev);
		obj->curBlock->iDataStart = obj->curBlock->nDataLen ;
                //CheckPosition( obj );
	}
	else
	{
		nCurPos = 0 ;
		item = (PBUF2ITEM)obj->lstBuf.next ;


		while( (nCurPos + item->nDataLen) < pos  )
		{
			item->iDataStart = 0 ;
			nCurPos += item->nDataLen ;

			item = (PBUF2ITEM) ( item->item.next ) ;
		}
		obj->curBlock = item ;
		obj->curBlock->iDataStart = pos - nCurPos ;
	}


	obj->nPosition = pos ;
	CheckPosition(obj);
	MtxUnLock(&(obj->mtx));
	return pos ;
}
int MYAPI MemoryBuffer2Write(MemoryBuffer2Handle handle ,const void *buf,int nSize)
{
	MEMORY_BUFFER2 *obj =(MEMORY_BUFFER2 *) handle ;
	PBUF2ITEM item = NULL ;
	const unsigned char *dataIn = (const unsigned char *)buf ;
	int nCpyed = 0 ;
	int nCpy = 0 ;
	if ( obj == NULL )
	{
		return  -1;
	}
	if( buf == NULL || nSize < 1 )
	{
		return 0 ;
	}

	MtxLock(&(obj->mtx));

	if( !ListIsEmpty( & (obj->lstBuf) ) )
	{
		item = (PBUF2ITEM)obj->lstBuf.prev ;
		nCpy = MinInt( nSize - nCpyed , item->nBlockSize - item->nDataLen);
		if( nCpy > 0 )
		{
			memcpy( item->data + item->nDataLen, dataIn + nCpyed ,nCpy);
			nCpyed += nCpy ;
			item->nDataLen += nCpy ;
			obj->nTotalSize += nCpy ;
		}
	}


	while( nCpyed < nSize )
	{
		item = MemoryBuffer2GetFreeItem(obj);
		if( item == NULL )
		{
			item = MemoryBuffer2AllocItem(obj,obj->nBlockSize);
		}
		if( item == NULL )
		{  
			break ;
		}
		nCpy = MinInt( nSize - nCpyed , item->nBlockSize - item->nDataLen);
		memcpy( item->data + item->nDataLen, dataIn + nCpyed ,nCpy);
		nCpyed += nCpy ;
		item->nDataLen += nCpy ;
		obj->nTotalSize += nCpy ;		

		MemoryBuffer2PutItem2Used(obj,item);
		

	}
	CheckPosition(obj);
	MtxUnLock(&(obj->mtx));
	return nCpyed ;
}
void MYAPI MemoryBuffer2Reset(MemoryBuffer2Handle handle)
{
	MEMORY_BUFFER2 *obj =(MEMORY_BUFFER2 *) handle ;
	PBUF2ITEM item = NULL ;
	if ( obj == NULL )
	{
		return  ;
	}
	MtxLock(&(obj->mtx));
	item = MemoryBuffer2GetUsedItem( obj );
	while ( item != NULL )
	{
		MemoryBuffer2FreeItem(obj, item );
		item = MemoryBuffer2GetUsedItem( obj );
	}
	obj->nTotalSize = 0 ;
    obj->nPosition = 0 ;
    obj->curBlock = NULL ;
	MtxUnLock(&(obj->mtx));
	return  ;
}
MemoryBuffer2Handle MYAPI MemoryBuffer2Open(int blocksize)
{
	return MemoryBuffer2Open2( blocksize, 0, 10, FALSE ) ;
}
MemoryBuffer2Handle MYAPI MemoryBuffer2Open2(int blocksize,int nSeekLength, int nMaxFreeBlocks,  BOOL bShareData )
{
	MEMORY_BUFFER2 *obj = NULL ;

	obj = (MEMORY_BUFFER2 *)malloc(sizeof(MEMORY_BUFFER2));
	if( obj == NULL )
	{
		return NULL ;
	}
	memset( obj , 0 , sizeof(MEMORY_BUFFER2));
	MtxInit(&(obj->mtx),0);
	obj->nBlockSize = blocksize ;
	obj->nMaxFreeBlocks = nMaxFreeBlocks ;
	obj->bShareData = bShareData ;
	if( obj->nBlockSize < 1 )
	{
		obj->nBlockSize = 1024 ;
	}
	obj->nTotalSize = 0 ;
	obj->nSeekLength = nSeekLength ;
	ListInit( &(obj->lstBuf));
	ListInit( &(obj->lstFree));

	return obj ;
}









#include "adapi_def.h"




void MYAPI ListInit(LIST_ITEM *newItem)
{
    if( !newItem )
    {
        return ;
    }
    newItem->next = newItem;
    newItem->prev = newItem;
}
void MYAPI  _ListAdd(LIST_ITEM *newItem,LIST_ITEM *prev, LIST_ITEM *next)
{
    next->prev = newItem;
    newItem->next = next;
    newItem->prev = prev;
    prev->next = newItem;
}

void MYAPI ListAddHead(LIST_ITEM *newItem, LIST_ITEM *head)
{
    _ListAdd(newItem, head, head->next);
}


void MYAPI ListAddTail(LIST_ITEM *newItem, LIST_ITEM *head)
{
    _ListAdd(newItem, head->prev, head);
}


void MYAPI _ListDel(LIST_ITEM *prev, LIST_ITEM *next)
{
    next->prev = prev;
    prev->next = next;
}

void MYAPI ListDel(LIST_ITEM *entry)
{
    _ListDel(entry->prev, entry->next);
    entry->next = (LIST_ITEM *) 0;
    entry->prev = (LIST_ITEM *) 0;
}


void MYAPI ListMoveHead(LIST_ITEM *listMv, LIST_ITEM *head)
{
    _ListDel(listMv->prev, listMv->next);
    ListAddHead(listMv, head);
}


void MYAPI ListMoveTail(LIST_ITEM *listMv,LIST_ITEM *head)
{
    _ListDel(listMv->prev, listMv->next);
    ListAddTail(listMv, head);
}

BOOL MYAPI ListIsEmpty(LIST_ITEM *head)
{
    return (head->next == head);
}

/////////////////////////////////////////////////////////////////

void MYAPI TxtItemFreeList(LIST_ITEM *  pHead)
{

    TXT_ITEM *itemFree = NULL ;
    LIST_ITEM *pItemTmp = NULL ;
    LIST_ITEM *pItem = NULL;
    if( pHead == NULL )
    {
        return ;
    }
    pItem = pHead->next;
    while( pItem != NULL && pItem != pHead )
    {
        pItemTmp = pItem ;
        pItem = pItem->next ;

        itemFree = _Convert2TxtItem_(pItemTmp);
        _MEM_FREE_(itemFree);
    }
    ListInit( pHead );
}
void MYAPI TxtItemFree( LIST_ITEM *pItemTmp )
{

    TXT_ITEM *itemFree = NULL ;

    if( pItemTmp == NULL )
    {
        return ;
    }

    itemFree = _Convert2TxtItem_(pItemTmp);
    _MEM_FREE_(itemFree);

    return ;

}
TXT_ITEM * MYAPI TxtItemAlloc(char *szhead ,int nLen)
{
    TXT_ITEM *head = NULL ;
    if( szhead == NULL || szhead[0] == 0 )
    {
        return NULL ;
    }
    if( nLen == -1 )
    {
        nLen = strlen( szhead );
    }
    head = (TXT_ITEM *)_MEM_ALLOC( sizeof(TXT_ITEM) + nLen + 2 );
    if( head == NULL )
    {
        return NULL ;
    }
    ListInit( &(head->item) );
    head->nLen = nLen ;
    head->szTxt = ((char *)head) + sizeof(TXT_ITEM)  ;
    head->szTxt[nLen] = 0 ;
    head->szTxt[nLen+1] = 0 ;
    memcpy( head->szTxt , szhead , nLen ) ;
    return head ;
}













































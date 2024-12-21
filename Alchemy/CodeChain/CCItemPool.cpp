//	CCItemPool.cpp
//
//	Implements CCItemPool template class

#include "PreComp.h"

#define SEGMENT_SIZE					4096	//	Size of each backbone segment
#define BACKBONE_SIZE					1024	//	Size of the backbone

#pragma warning (disable : 4660)

template class CCItemPool<CCInteger>;
template class CCItemPool<CCDouble>;
template class CCItemPool<CCString>;
template class CCItemPool<CCLinkedList>;
template class CCItemPool<CCPrimitive>;
template class CCItemPool<CCNil>;
template class CCItemPool<CCTrue>;
template class CCItemPool<CCSymbolTable>;
template class CCItemPool<CCLambda>;
template class CCItemPool<CCAtomTable>;
template class CCItemPool<CCVector>;
template class CCItemPool<CCExternal>;

template <class ItemClass>
CCItemPool<ItemClass>::CCItemPool (void)

//	CCItemPool constructor

	{
	}

template <class ItemClass>
CCItemPool<ItemClass>::~CCItemPool (void)

//	CCItemPool destructor

	{
	CleanUp();
	}

template <class ItemClass>
void CCItemPool<ItemClass>::CleanUp (void)

//	CleanUp
//
//	Free up allocation

	{
	if (m_pBackbone)
		{
		for (int i = 0; i < m_iBackboneSize; i++)
			delete [] m_pBackbone[i];

		delete [] m_pBackbone;
		m_pBackbone = NULL;
		}
	}

template <class ItemClass>
ICCItem *CCItemPool<ItemClass>::CreateItem (void)

//	CreateItem
//
//	Creates an item in the pool

	{
	CSmartLock Lock(m_cs);

	int i;
	ICCItem *pItem;

	//	If we've got no backbone, allocate it now

	if (m_pBackbone == NULL)
		{
		m_pBackbone = new ItemClass *[BACKBONE_SIZE];
		if (m_pBackbone == NULL)
			{
			::kernelDebugLogPattern("CCodeChain: Out of memory creating item.");
			throw CException(ERR_MEMORY);
			}

		m_iBackboneSize = 0;
		}

	//	If we've got no more free items, allocate another segment
	//	of backbone

	if (m_pFreeList == NULL)
		{
		ItemClass *pSegment;

		if (m_iBackboneSize == BACKBONE_SIZE)
			{
			::kernelDebugLogPattern("CCodeChain: Backbone filled up.");
			throw CException(ERR_MEMORY);
			}

		pSegment = new ItemClass[SEGMENT_SIZE];
		if (pSegment == NULL)
			{
			::kernelDebugLogPattern("CCodeChain: Out of memory creating segment.");
			throw CException(ERR_MEMORY);
			}

		m_pBackbone[m_iBackboneSize] = pSegment;
		m_iBackboneSize++;

		//	Add all the entries to the free list

		for (i = 0; i < SEGMENT_SIZE; i++)
			{
			ICCItem *pItem = dynamic_cast<ICCItem *>(&pSegment[i]);

			pItem->SetNextFree(m_pFreeList);
			m_pFreeList = pItem;
			}
		}

	//	Return the next free item

	pItem = m_pFreeList;
	m_pFreeList = pItem->GetNextFree();

	pItem->ResetItem();
	m_iCount++;
	return pItem;
	}

template <class ItemClass>
void CCItemPool<ItemClass>::DestroyItem (ICCItem *pItem)

//	DestroyItem
//
//	Destroys an item in the pool

	{
	CSmartLock Lock(m_cs);

#ifdef DEBUG
	ItemClass *pClass = dynamic_cast<ItemClass *>(pItem);
	ASSERT(pClass);
#endif

	//	Add the item back to the free list

	pItem->SetNextFree(m_pFreeList);
	m_pFreeList = pItem;
	m_iCount--;
	}

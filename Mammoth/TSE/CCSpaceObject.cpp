//	CCSpaceObject.cpp
//
//	CCSpaceObject class
//	Copyright (c) 2014 by Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

CCItemPool<CCSpaceObject> CCSpaceObject::m_ObjectPool;



CCSpaceObject::CCSpaceObject (CSpaceObject *pObj) : ICCAtom(NULL)

//	CCSpaceObject constructor

	{

	m_pObj = pObj;
	m_dwID = pObj->GetID();
	m_ivalid = g_ivalid;

	}

void CCSpaceObject::Resolve(void)

//	Resolve
//
//	Resolves the object pointer

	{
	if (m_dwID != OBJID_NULL)
		{
		CSystem* pSystem = g_pUniverse->GetCurrentSystem();
		if (pSystem)
			m_pObj = pSystem->FindObject(m_dwID);
		else
			m_pObj = NULL;
		}
	else
		{
		m_pObj = NULL;
		}
	}

ICCItem *CCSpaceObject::Clone(CCodeChain *pCC)

//	Clone
//
//	Returns a new item with a single ref-count

	{

	ICCItem *pResult;
	CCSpaceObject *pClone;

	pResult = m_ObjectPool.CreateItem();
	if (pResult->IsError())
		return pResult;

	pClone = dynamic_cast<CCSpaceObject*>(pResult);
	pClone->Reset();
	pClone->m_dwID = this->m_dwID;
	pClone->m_pObj = this->m_pObj;
	pClone->m_ivalid = this->m_ivalid;

	return pClone;
	}

void CCSpaceObject::Reset(void)

//	Reset
//
//	Reset the internal variables

	{
	m_dwID = OBJID_NULL;
	m_pObj = NULL;
	m_ivalid = -1;
	}


ICCItem* CCSpaceObject::CreateItem(DWORD dwID)

//	CreateItem
//
//	Creates a SpaceObject item

	{
	ICCItem* pItem;
	CCSpaceObject* pObject;

	pItem = m_ObjectPool.CreateItem();
	if (pItem->IsError())
		return pItem;

	pObject = dynamic_cast<CCSpaceObject*>(pItem);
	pObject->Reset();
	pObject->m_dwID = dwID;

	return pObject->Reference();
	}

ICCItem* CCSpaceObject::CreateItem(CSpaceObject &pObj)

//	CreateItem
//
//	Creates a SpaceObject item

	{
	ICCItem* pItem;
	CCSpaceObject* pObject;

	pItem = m_ObjectPool.CreateItem();
	if (pItem->IsError())
		return pItem;

	pObject = dynamic_cast<CCSpaceObject*>(pItem);
	pObject->Reset();
	pObject->m_dwID = pObj.GetID();
	pObject->m_pObj = &pObj;
	pObject->m_ivalid = g_ivalid;

	return pObject->Reference();
	}

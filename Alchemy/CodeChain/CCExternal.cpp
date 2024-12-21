//	CCExternal.cpp
//
//	Implements CCExternal class

#include "PreComp.h"

CCExternal::CCExternal (void) : ICCAtom(NULL),
	m_iValue(0),
	m_pObject(NULL)

	//	CCInteger constructor

	{
	}

ICCItem *CCExternal::Clone (CCodeChain *pCC)

//	Clone
//
//	Returns a new item with a single ref-count

	{
	ICCItem *pResult;
	CCExternal *pClone;

	pResult = pCC->CreateExternal();
	if (pResult->IsError())
		return pResult;

	pClone = (CCExternal *)pResult;
	pClone->CloneItem(this);
	pClone->m_iValue = this->m_iValue;
	pClone->m_pObject = this->m_pObject;

	return pClone;
	}

void CCExternal::DestroyItem (void)

//	DestroyItem
//
//	Destroys the item

	{
	CCodeChain::DestroyExternal(this);
	}

CString CCExternal::Print (DWORD dwFlags) const

//	Print
//
//	Returns a text representation of this item

	{
	//	If this is an error code, translate it

	if (IsError())
		{
		switch (m_iValue)
			{
			case CCRESULT_NOTFOUND:
				return strPatternSubst(LITERAL("[%d] Item not found."), m_iValue);

			case CCRESULT_CANCEL:
				return strPatternSubst(LITERAL("[%d] Operation canceled."), m_iValue);

			case CCRESULT_DISKERROR:
				return strPatternSubst(LITERAL("[%d] Disk error."), m_iValue);

			default:
				return strPatternSubst(LITERAL("[%d] Unknown error."), m_iValue);
			}
		}

	//	Otherwise, just print the integer value

	else
		return strFromInt(m_iValue);
	}

void CCExternal::Reset (void)

//	Reset
//
//	Reset to initial conditions

	{
	ASSERT(m_dwRefCount == 0);
	m_iValue = 0;
	m_pObject = NULL;
	}


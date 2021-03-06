//	CDockScreenStack.cpp
//
//	CDockScreenStack class
//	Copyright (c) 2018 Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

const SDockFrame CDockScreenStack::m_NullFrame;

ICCItemPtr CDockScreenStack::AsCCItem (const SDockFrame &Frame)

//	AsCCItem
//
//	Encodes a frame as an item.

	{
	DWORD dwRootUNID = (Frame.pResolvedRoot ? Frame.pResolvedRoot->GetUNID() : 0);
    CString sScreen = Frame.sResolvedScreen;

    ICCItemPtr pResult(ICCItem::SymbolTable);

	pResult->SetIntegerAt(CONSTLIT("type"), dwRootUNID);
	if (Frame.pResolvedRoot)
		pResult->SetStringAt(CONSTLIT("typeEntity"), Frame.pResolvedRoot->GetEntityName());

	if (sScreen.IsBlank())
		{
        pResult->SetIntegerAt(CONSTLIT("screen"), dwRootUNID);
		pResult->SetIntegerAt(CONSTLIT("screenType"), dwRootUNID);
		}
	else
		{
		bool bNotUNID;
		DWORD dwScreen = strToInt(sScreen, 0, &bNotUNID);
		if (bNotUNID)
			{
			pResult->SetStringAt(CONSTLIT("screen"), sScreen);
			pResult->SetStringAt(CONSTLIT("screenName"), sScreen);
			}
		else
			{
			pResult->SetIntegerAt(CONSTLIT("screen"), dwScreen);
			pResult->SetIntegerAt(CONSTLIT("screenType"), dwScreen);
			}
		}

    pResult->SetStringAt(CONSTLIT("pane"), Frame.sPane);
    if (Frame.pStoredData)
        pResult->SetAt(CONSTLIT("data"), Frame.pStoredData);

    return pResult;
	}

void CDockScreenStack::DeleteAll (void)

//	DeleteAll
//
//	Delete all frames

	{
	while (!IsEmpty())
		Pop();
	}

const SDockFrame &CDockScreenStack::GetCurrent (void) const

//	GetCurrent
//
//	Returns the top of the stack

	{
	if (IsEmpty())
		return m_NullFrame;

	int iTop = m_Stack.GetCount() - 1;
	return m_Stack[iTop];
	}

const CString &CDockScreenStack::GetDisplayData (const CString &sID)

//	GetDisplayData
//
//	Returns opaque data stored by displays

	{
	if (IsEmpty())
		return NULL_STR;

	SDockFrame &Frame = m_Stack[m_Stack.GetCount() - 1];
	CString *pValue = Frame.DisplayData.GetAt(sID);
	if (pValue == NULL)
		return NULL_STR;

	return *pValue;
	}

void CDockScreenStack::Push (const SDockFrame &Frame)

//	Push
//
//	Push a frame

	{
	//	Whenever we add a frame, we clear the return data block of the top frame
	//	so that we can get return data.

	if (!IsEmpty())
		m_Stack[m_Stack.GetCount() - 1].pReturnData.Delete();

	//	Add

	m_Stack.Insert(Frame);
	}

void CDockScreenStack::Pop (void)

//	Pop
//
//	Pop a frame

	{
	if (!IsEmpty())
		{
		int iTop = m_Stack.GetCount() - 1;
		m_Stack.Delete(iTop);
		}
	}

void CDockScreenStack::ResolveCurrent (const SDockFrame &ResolvedFrame)

//	SetCurrent
//
//	Sets the current frame (and optionally returns the old frame).
//
//	NOTE: Caller is responsible for discarding pInitialData if it get the previous
//	frame.

	{
	if (!IsEmpty())
		{
		int iTop = m_Stack.GetCount() - 1;

		m_Stack[iTop].pResolvedRoot = ResolvedFrame.pResolvedRoot;
		m_Stack[iTop].sResolvedScreen = ResolvedFrame.sResolvedScreen;
		m_Stack[iTop].sCurrentTab = ResolvedFrame.sCurrentTab;
		}
	}

void CDockScreenStack::SetBackgroundDesc (const SDockScreenBackgroundDesc &BackgroundDesc)

//	SetBackgroundDesc
//
//	Sets the current background descriptor

	{
	if (!IsEmpty())
		{
		int iTop = m_Stack.GetCount() - 1;
		m_Stack[iTop].BackgroundDesc = BackgroundDesc;
		}
	}

void CDockScreenStack::SetCurrent (const SDockFrame &NewFrame, SDockFrame *retPrevFrame)

//	SetCurrent
//
//	Sets the current frame (and optionally returns the old frame).
//
//	NOTE: Caller is responsible for discarding pInitialData if it get the previous
//	frame.

	{
	if (!IsEmpty())
		{
		int iTop = m_Stack.GetCount() - 1;

		if (retPrevFrame)
			*retPrevFrame = m_Stack[iTop];

		m_Stack[iTop] = NewFrame;
		}
	}

void CDockScreenStack::SetCurrentPane (const CString &sPane)

//	SetCurrentPane
//
//	Sets the current pane.

	{
	if (!IsEmpty())
		{
		int iTop = m_Stack.GetCount() - 1;
		m_Stack[iTop].sPane = sPane;
		}
	}

void CDockScreenStack::SetDisplayData (const CString &sID, const CString &sData)

//	SetDisplayData
//
//	Sets opaque data for a display

	{
	if (IsEmpty())
		return;

	SDockFrame &Frame = m_Stack[m_Stack.GetCount() - 1];
	Frame.DisplayData.SetAt(sID, sData);
	}

void CDockScreenStack::SetLocation (CSpaceObject *pLocation)

//	SetLocation
//
//	Sets the location for all frames. This is called, e.g., when we change ships.

	{
	int i;

	for (i = 0; i < m_Stack.GetCount(); i++)
		m_Stack[i].pLocation = pLocation;
	}


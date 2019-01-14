//	CGFont.cpp
//
//	Implements CGFont
//	Copyright (c) 2019 Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

CGFont::CGFont (void) :
		m_hFont(NULL),
		m_cxWidth(-1),
		m_cyHeight(-1),
		m_fBold(false),
		m_fItalic(false),
		m_fUseCellHeight(false),
		m_iHeight(20)

//	CGFont constructor

	{
	}

CGFont::~CGFont (void)

//	CGFont destructor

	{
	FlushFont();
	}

CGFont::CGFont (const CGFont &NewFont) :
		m_hFont(NULL),
		m_fBold(NewFont.m_fBold),
		m_fItalic(NewFont.m_fItalic),
		m_iHeight(NewFont.m_iHeight),
		m_sTypeface(NewFont.m_sTypeface)

//	CGFont copy constructor

	{
	}

HFONT CGFont::GetFont (void)

//	GetFont
//
//	Returns a font to use in an HDC. Note that the font will not
//	be valid if the CGFont object is destroyed or if any of the
//	properties of the font are changed.

	{
	//	If no font has been created yet, create it now

	if (m_hFont == NULL)
		{
		m_hFont = CreateFont((m_fUseCellHeight ? m_iHeight : -m_iHeight),	
				0,
				0, 0,
				(m_fBold ? FW_BOLD : FW_NORMAL),
				m_fItalic,
				FALSE,
				FALSE,
				ANSI_CHARSET,
				OUT_DEFAULT_PRECIS,
				CLIP_EMBEDDED,
				DEFAULT_QUALITY,
				VARIABLE_PITCH | FF_SWISS,
				m_sTypeface.GetASCIIZPointer());
		}

	return m_hFont;
	}

void CGFont::FlushFont (void)

//	FlushFont
//
//	Destroys the cached font

	{
	if (m_hFont)
		{
		DeleteObject(m_hFont);
		m_hFont = NULL;

		//	Reset the heights so that we recompute them the
		//	next time that we are asked for them.

		m_cxWidth = -1;
		m_cyHeight = -1;
		}
	}

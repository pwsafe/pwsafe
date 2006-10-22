// ControlExtns.cpp
//

#include "stdafx.h"
#include "ControlExtns.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const COLORREF crefInFocus = (RGB(222,255,222));  // Light green
const COLORREF crefNoFocus = (RGB(255,255,255));  // White
const COLORREF crefBlack = (RGB(0,0,0));          // Black

/////////////////////////////////////////////////////////////////////////////
// CEditExtn

CEditExtn::CEditExtn()
{
	brInFocus.CreateSolidBrush(crefInFocus);
	brNoFocus.CreateSolidBrush(crefNoFocus);
}

CEditExtn::~CEditExtn()
{
}

BEGIN_MESSAGE_MAP(CEditExtn, CEdit)
	//{{AFX_MSG_MAP(CEditExtn)
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_CTLCOLOR_REFLECT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditExtn message handlers

void CEditExtn::OnSetFocus(CWnd* pOldWnd)
{
	m_bIsFocused = TRUE;
	CEdit::OnSetFocus(pOldWnd);
	Invalidate(TRUE);
}

void CEditExtn::OnKillFocus(CWnd* pNewWnd)
{
	m_bIsFocused = FALSE;
	CEdit::OnKillFocus(pNewWnd);
	Invalidate(TRUE);
}

HBRUSH CEditExtn::CtlColor(CDC* pDC, UINT /*nCtlColor*/)
{
	if (!this->IsWindowEnabled())
		return NULL;

	pDC->SetTextColor(crefBlack);
	if (m_bIsFocused == TRUE) {
		pDC->SetBkColor(crefInFocus);
		return brInFocus;
	} else {
		pDC->SetBkColor(crefNoFocus);
		return brNoFocus;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CListBoxExtn

CListBoxExtn::CListBoxExtn()
{
	brInFocus.CreateSolidBrush(crefInFocus);
	brNoFocus.CreateSolidBrush(crefNoFocus);
}

CListBoxExtn::~CListBoxExtn()
{
}

BEGIN_MESSAGE_MAP(CListBoxExtn, CListBox)
	//{{AFX_MSG_MAP(CListBoxExtn)
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_CTLCOLOR_REFLECT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CListBoxExtn message handlers

void CListBoxExtn::OnSetFocus(CWnd* pOldWnd)
{
	m_bIsFocused = TRUE;
	CListBox::OnSetFocus(pOldWnd);
	Invalidate(TRUE);
}

void CListBoxExtn::OnKillFocus(CWnd* pNewWnd)
{
	m_bIsFocused = FALSE;
	CListBox::OnKillFocus(pNewWnd);
	Invalidate(TRUE);
}

HBRUSH CListBoxExtn::CtlColor(CDC* pDC, UINT /* nCtlColor */)
{
	if (!this->IsWindowEnabled())
		return NULL;

	if (m_bIsFocused == TRUE) {
		pDC->SetBkColor(crefInFocus);
		return brInFocus;
	} else {
		pDC->SetBkColor(crefNoFocus);
		return brNoFocus;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CComboBoxExtn

BEGIN_MESSAGE_MAP(CComboBoxExtn, CComboBox)
	//{{AFX_MSG_MAP(CComboBoxExtn)
	ON_WM_CTLCOLOR()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CComboBoxExt message handlers

HBRUSH CComboBoxExtn::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	if (nCtlColor == CTLCOLOR_EDIT) {
		// Extended Edit control
		if (m_edit.GetSafeHwnd() == NULL)
			m_edit.SubclassWindow(pWnd->GetSafeHwnd());
	}
	else if (nCtlColor == CTLCOLOR_LISTBOX) {
		// Extended ListBox control
		if (m_listbox.GetSafeHwnd() == NULL)
			m_listbox.SubclassWindow(pWnd->GetSafeHwnd());
	}

	return CComboBox::OnCtlColor(pDC, pWnd, nCtlColor);
}

void CComboBoxExtn::OnDestroy()
{
	if (m_edit.GetSafeHwnd() != NULL)
		m_edit.UnsubclassWindow();

	if (m_listbox.GetSafeHwnd() != NULL)
		m_listbox.UnsubclassWindow();

	CComboBox::OnDestroy();
}

void CComboBoxExtn::ChangeColour()
{
	m_edit.ChangeColour();
	m_listbox.ChangeColour();
}

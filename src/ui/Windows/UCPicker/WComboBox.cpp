
#include "StdAfx.h"

#include "WComboBox.h"

IMPLEMENT_DYNCREATE(CWComboBox, CComboBox)

CWComboBox::CWComboBox(): CComboBox()
{
  m_clrHilight = GetSysColor(COLOR_HIGHLIGHT);
  m_clrNormalText = GetSysColor(COLOR_WINDOWTEXT);
  m_clrHilightText = GetSysColor(COLOR_HIGHLIGHTTEXT);
  m_clrBkgnd = RGB(255, 255, 255);
}

CWComboBox::~CWComboBox()
{
}

BEGIN_MESSAGE_MAP(CWComboBox, CComboBox)
END_MESSAGE_MAP()

void CWComboBox::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
  CDC *pDC = CDC::FromHandle(lpDIS->hDC);
  CRect rect;

  // draw the colored rectangle portion
  rect.CopyRect(&lpDIS->rcItem);

  pDC->SetBkMode(TRANSPARENT);

  if (lpDIS->itemState & ODS_SELECTED) {
    pDC->FillSolidRect(rect, m_clrHilight);
    pDC->SetTextColor(m_clrHilightText);
  } else {
    pDC->FillSolidRect(rect, m_clrBkgnd);
    pDC->SetTextColor(m_clrNormalText);
  }

  if ((int)(lpDIS->itemID) >= 0) {
    CString strText;
    GetLBText(lpDIS->itemID, strText);
    pDC->DrawText(strText, rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
  }
}

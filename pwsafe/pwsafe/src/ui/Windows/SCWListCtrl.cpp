/*
* Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "stdafx.h"

#include "SCWListCtrl.h"
#include "DboxMain.h" // For TIMER_FIND

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CSCWListCtrl::CSCWListCtrl()
{
  m_crWindowText = ::GetSysColor(COLOR_WINDOWTEXT);
}

CSCWListCtrl::~CSCWListCtrl()
{
}

BEGIN_MESSAGE_MAP(CSCWListCtrl, CListCtrl)
  //{{AFX_MSG_MAP(CSCWListCtrl)
  ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CSCWListCtrl::OnCustomDraw(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  NMLVCUSTOMDRAW *pLVCD = (NMLVCUSTOMDRAW *)pNotifyStruct;

  *pLResult = CDRF_DODEFAULT;

  static CDC *pDC = NULL;

  switch (pLVCD->nmcd.dwDrawStage) {
    case CDDS_PREPAINT:
      // PrePaint
      pDC = CDC::FromHandle(pLVCD->nmcd.hdc);
      *pLResult = CDRF_NOTIFYITEMDRAW;
      break;

    case CDDS_ITEMPREPAINT:
      // Item PrePaint
      *pLResult |= CDRF_NOTIFYSUBITEMDRAW;
      break;

    case CDDS_ITEMPREPAINT | CDDS_SUBITEM:
      // Sub-item PrePaint
      if (pLVCD->iSubItem == 0) {
        CRect rect;
        GetSubItemRect(pLVCD->nmcd.dwItemSpec, pLVCD->iSubItem, LVIR_BOUNDS, rect);
        if (rect.top < 0) {
          *pLResult = CDRF_SKIPDEFAULT;
          break;
        }
        CRect rect1;
        GetSubItemRect(pLVCD->nmcd.dwItemSpec, 1, LVIR_BOUNDS, rect1);
        rect.right = rect1.left;
        rect.DeflateRect(2, 2);

        CString str = GetItemText(pLVCD->nmcd.dwItemSpec, pLVCD->iSubItem);

        pDC->SetTextColor(((pLVCD->nmcd.lItemlParam & REDTEXT) == REDTEXT) ? RGB(255, 0, 0) : m_crWindowText);

        int iFormat = (pLVCD->nmcd.lItemlParam & 0x0F);
        UINT nFormat = DT_VCENTER | DT_SINGLELINE;
        if (iFormat == LVCFMT_RIGHT)
          nFormat |= DT_RIGHT;
        else if (iFormat == LVCFMT_CENTER)
          nFormat |= DT_CENTER;
        pDC->DrawText(str, &rect, nFormat);
        *pLResult = CDRF_SKIPDEFAULT;
      } else {
        pLVCD->clrText = m_crWindowText;
        *pLResult |= CDRF_NOTIFYPOSTPAINT;
      }
      break;

    /*
    case CDDS_PREERASE:
    case CDDS_POSTERASE:
    case CDDS_ITEMPREERASE:
    case CDDS_ITEMPOSTERASE:
    case CDDS_ITEMPOSTPAINT:
    case CDDS_ITEMPOSTPAINT | CDDS_SUBITEM:
    case CDDS_POSTPAINT:
    */
    default:
      break;
  }
}

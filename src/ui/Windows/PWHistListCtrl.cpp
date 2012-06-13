/*
* Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "stdafx.h"

#include "PWHistListCtrl.h"

#include "Fonts.h"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CPWHistListCtrl, CListCtrl)
  //{{AFX_MSG_MAP(CPWHistListCtrl)
  ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CPWHistListCtrl::OnCustomDraw(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  NMLVCUSTOMDRAW *pNMLVCUSTOMDRAW = (NMLVCUSTOMDRAW *)pNotifyStruct;

  *pLResult = CDRF_DODEFAULT;

  static bool bchanged_subitem_font(false);
  static CFont *pCurrentFont = NULL;
  static CFont *pPasswordFont = NULL;
  static CDC *pDC = NULL;

  switch (pNMLVCUSTOMDRAW->nmcd.dwDrawStage) {
    case CDDS_PREPAINT:
      // PrePaint
      bchanged_subitem_font = false;
      pCurrentFont = Fonts::GetInstance()->GetCurrentFont();
      pPasswordFont = Fonts::GetInstance()->GetPasswordFont();
      pDC = CDC::FromHandle(pNMLVCUSTOMDRAW->nmcd.hdc);
      *pLResult = CDRF_NOTIFYITEMDRAW;
      break;

    case CDDS_ITEMPREPAINT:
      // Item PrePaint
      *pLResult |= CDRF_NOTIFYSUBITEMDRAW;
      break;

    case CDDS_ITEMPREPAINT | CDDS_SUBITEM:
      // Sub-item PrePaint
      if (pNMLVCUSTOMDRAW->iSubItem == 1) {
        bchanged_subitem_font = true;
        pDC->SelectObject(pPasswordFont);
        *pLResult |= (CDRF_NOTIFYPOSTPAINT | CDRF_NEWFONT);
      }
      break;

    case CDDS_ITEMPOSTPAINT | CDDS_SUBITEM:
      // Sub-item PostPaint - restore old font if any
      if (bchanged_subitem_font) {
        bchanged_subitem_font = false;
        pDC->SelectObject(pCurrentFont);
        *pLResult |= CDRF_NEWFONT;
      }
      break;

    /*
    case CDDS_PREERASE:
    case CDDS_POSTERASE:
    case CDDS_ITEMPREERASE:
    case CDDS_ITEMPOSTERASE:
    case CDDS_ITEMPOSTPAINT:
    case CDDS_POSTPAINT:
    */
    default:
      break;
  }
}

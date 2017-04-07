/*
* Copyright (c) 2003-2017 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "stdafx.h"
#include "ExpPswdLC.h"
#include "PWTreeCtrl.h"

#include "resource3.h"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CExpPswdLC::CExpPswdLC()
  : m_pToolTipCtrl(NULL), m_LastToolTipRow(-1), m_pwchTip(NULL)
{
}

CExpPswdLC::~CExpPswdLC()
{
  delete m_pwchTip;
  delete m_pToolTipCtrl;
}

BEGIN_MESSAGE_MAP(CExpPswdLC, CListCtrl)
  //{{AFX_MSG_MAP(CExpPswdLC)
  ON_WM_MOUSEMOVE()
  ON_NOTIFY_EX(TTN_NEEDTEXT, 0, OnToolTipText)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CExpPswdLC::PreSubclassWindow()
{
  CListCtrl::PreSubclassWindow();

  // Disable the CToolTipCtrl of CListCtrl so it won't disturb our own tooltip-ctrl
  GetToolTips()->Activate(FALSE);

  // Enable our own tooltip-ctrl and make it show tooltip even if not having focus
  m_pToolTipCtrl = new CToolTipCtrl;
  if (!m_pToolTipCtrl->Create(this, TTS_ALWAYSTIP | TTS_NOPREFIX)) {
    delete m_pToolTipCtrl;
    m_pToolTipCtrl = NULL;
  } else {
    EnableToolTips(TRUE);
    int iTime = m_pToolTipCtrl->GetDelayTime(TTDT_AUTOPOP);
    m_pToolTipCtrl->SetDelayTime(TTDT_AUTOPOP, iTime * 4);
    m_pToolTipCtrl->SetMaxTipWidth(250);
    m_pToolTipCtrl->Activate(TRUE);
  }
}

BOOL CExpPswdLC::PreTranslateMessage(MSG *pMsg)
{
  if (m_pToolTipCtrl != NULL)
    m_pToolTipCtrl->RelayEvent(pMsg);

  return CListCtrl::PreTranslateMessage(pMsg);
}

void CExpPswdLC::OnMouseMove(UINT nFlags, CPoint point)
{
  CPoint pt(GetMessagePos());
  ScreenToClient(&pt);

  // Find the subitem
  LVHITTESTINFO hitinfo = {0};
  hitinfo.flags = nFlags;
  hitinfo.pt = pt;
  SubItemHitTest(&hitinfo);

  if (m_LastToolTipRow != hitinfo.iItem) {
    // Mouse moved over a new cell
    m_LastToolTipRow = hitinfo.iItem;

    // Remove the old tooltip (if available)
    if (m_pToolTipCtrl->GetToolCount() > 0) {
      m_pToolTipCtrl->DelTool(this);
      m_pToolTipCtrl->Activate(FALSE);
    }

    // Not using CToolTipCtrl::AddTool() because it redirects the messages to CListCtrl parent
    TOOLINFO ti = {0};
    ti.cbSize = sizeof(TOOLINFO);
    ti.uFlags = TTF_IDISHWND;    // Indicate that uId is handle to a control
    ti.uId = (UINT_PTR)m_hWnd;   // Handle to the control
    ti.hwnd = m_hWnd;            // Handle to window to receive the tooltip-messages
    ti.hinst = AfxGetInstanceHandle();
    ti.lpszText = LPSTR_TEXTCALLBACK;
    m_pToolTipCtrl->SendMessage(TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);
    m_pToolTipCtrl->Activate(TRUE);
  }

  CListCtrl::OnMouseMove(nFlags, point);
}

BOOL CExpPswdLC::OnToolTipText(UINT /*id*/, NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  UINT_PTR nID = pNotifyStruct->idFrom;
  *pLResult = 0;

  // check if this is the automatic tooltip of the control
  if (nID == 0) 
    return TRUE;  // do not allow display of automatic tooltip,
                  // or our tooltip will disappear

  TOOLTIPTEXTW *pTTTW = (TOOLTIPTEXTW *)pNotifyStruct;

  CString cs_tooltip;

  // get the mouse position
  CPoint pt(GetMessagePos());
  ScreenToClient(&pt);  // convert the point's coords to be relative to this control

  // see if the point falls onto a list item
  LVHITTESTINFO lvhti = {0};
  lvhti.pt = pt;
  
  SubItemHitTest(&lvhti);
  
  // nFlags is 0 if the SubItemHitTest fails
  // Therefore, 0 & <anything> will equal false
  if (lvhti.flags & LVHT_ONITEM) {
    LVITEM lv= {0};
    lv.iItem = lvhti.iItem;
    lv.mask = LVIF_IMAGE;
    GetItem(&lv);
    UINT uimsg(0);
    switch (lv.iImage) {
      case CPWTreeCtrl::NORMAL:
        uimsg = IDS_EXP_NORMAL;
        break;
      case CPWTreeCtrl::WARNEXPIRED_NORMAL:
        uimsg = IDS_EXP_NORMAL_WARN;
        break;
      case CPWTreeCtrl::EXPIRED_NORMAL:
        uimsg = IDS_EXP_NORMAL_EXP;
        break;
      case CPWTreeCtrl::ALIASBASE:
        uimsg = IDS_EXP_ABASE;
        break;
      case CPWTreeCtrl::WARNEXPIRED_ALIASBASE:
        uimsg = IDS_EXP_ABASE_WARN;
        break;
      case CPWTreeCtrl::EXPIRED_ALIASBASE:
        uimsg = IDS_EXP_ABASE_EXP;
        break;
      case CPWTreeCtrl::SHORTCUTBASE:
        uimsg = IDS_EXP_SBASE;
        break;
      case CPWTreeCtrl::WARNEXPIRED_SHORTCUTBASE:
        uimsg = IDS_EXP_SBASE_WARN;
        break;
      case CPWTreeCtrl::EXPIRED_SHORTCUTBASE:
        uimsg = IDS_EXP_SBASE_EXP;
        break;
      default:
        ASSERT(0);
        break;
    }
    if (uimsg > 0)
      cs_tooltip.LoadString(uimsg);
    else
      return FALSE;  // no tooltip
  } else {
    return FALSE;  // no tooltip
  }

  wcsncpy_s(pTTTW->szText, _countof(pTTTW->szText),
             cs_tooltip, _TRUNCATE);

  return TRUE;   // we found a tool tip,
}

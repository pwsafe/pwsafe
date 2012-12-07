/*
* Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "stdafx.h"
#include "SHCTListCtrl.h"
#include "OptionsShortcuts.h"

#include <algorithm>

#include "resource2.h"
#include "resource3.h"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CSHCTListCtrl::CSHCTListCtrl()
: m_pParent(NULL), m_pHotKey(NULL), m_pwchTip(NULL),
  m_bHotKeyActive(false)
{
  m_pHotKey = new CSHCTHotKey;
  m_crWindowText = ::GetSysColor(COLOR_WINDOWTEXT);
  m_crRedText    = RGB(168, 0, 0);
}

CSHCTListCtrl::~CSHCTListCtrl()
{
  m_pHotKey->DestroyWindow();
  delete m_pHotKey;
  delete m_pwchTip;
}

BEGIN_MESSAGE_MAP(CSHCTListCtrl, CListCtrl)
  //{{AFX_MSG_MAP(CSHCTListCtrl)
  ON_WM_LBUTTONDOWN()
  ON_WM_RBUTTONDOWN()
  ON_WM_HSCROLL()
  ON_WM_VSCROLL()
  ON_WM_MOUSEWHEEL()

  ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
  ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipText)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CSHCTListCtrl::Init(COptionsShortcuts *pParent)
{
  if (m_pHotKey->GetSafeHwnd() == NULL) {
    CRect itemrect(0, 0, 0, 0);
    m_pHotKey->Create(0, itemrect, this, IDC_SHORTCUTHOTKEY);
    m_pHotKey->ModifyStyle(WS_BORDER, 0, 0);
    // Would like to change the default font (e.g. smaller and not bold) but it gets ignored
  }
  m_pHotKey->SetMyParent(this);
  m_pParent = pParent;
  EnableToolTips(TRUE);
}

void CSHCTListCtrl::OnLButtonDown(UINT , CPoint point)
{
  MapMenuShortcutsIter iter;
  CRect subitemrect;
  int iSubItem = -1;

  SaveHotKey();

  LVHITTESTINFO lvhti;
  lvhti.pt = point;
  SubItemHitTest(&lvhti);

  m_item = lvhti.iItem;
  iSubItem = lvhti.iSubItem;
  if (m_item < 0 || iSubItem != SHCT_SHORTCUTKEYS)
    return;

  if (m_pParent != NULL)
    m_pParent->ClearWarning();

  // GetSubItemRect for the first column gives the total width
  // Therefore need to get it from GetColmnWidth
  const int iColWidth = GetColumnWidth(0);
  GetSubItemRect(m_item, SHCT_SHORTCUTKEYS, LVIR_BOUNDS, subitemrect);
  subitemrect.left += 2;
  subitemrect.right = iColWidth - (iColWidth < 4 ? 0 : 2);
  subitemrect.top += 2;
  subitemrect.bottom -= 2;
  m_pHotKey->MoveWindow(&subitemrect);

  m_id = (UINT)LOWORD(GetItemData(m_item));
  if (m_pParent->GetMapMenuShortcutsIter(m_id, iter)) {
    WORD vModifiers = iter->second.cModifier;
    m_pHotKey->SetHotKey(iter->second.siVirtKey, vModifiers);
  }
  m_pHotKey->EnableWindow(TRUE);
  m_pHotKey->ShowWindow(SW_SHOW);
  m_pHotKey->BringWindowToTop();
  m_pHotKey->SetFocus();
  m_bHotKeyActive = true;

  if (m_item >= 0)
    SetItemState(m_item, 0, LVIS_SELECTED | LVIS_DROPHILITED);

  UpdateWindow();
}

void CSHCTListCtrl::OnRButtonDown(UINT , CPoint point)
{
  CMenu PopupMenu;
  MapMenuShortcutsIter iter;
  CString str;
  CPoint pt;
  int iSubItem = -1;

  SaveHotKey();

  LVHITTESTINFO lvhti;
  lvhti.pt = point;
  SubItemHitTest(&lvhti);

  m_item = lvhti.iItem;
  iSubItem = lvhti.iSubItem;

  if (m_item < 0 || iSubItem < 0) {
    // CListCtrl::OnRButtonDown(nFlags, point);
    goto exit;
  }

  m_id = (UINT)LOWORD(GetItemData(m_item));
  if (!m_pParent->GetMapMenuShortcutsIter(m_id, iter)) {
     goto exit;
  }

  PopupMenu.LoadMenu(IDR_POPRESETSHORTCUT);
  CMenu* pContextMenu = PopupMenu.GetSubMenu(0);
  if (iter->second.siVirtKey == 0)
    pContextMenu->RemoveMenu(ID_MENUITEM_REMOVESHORTCUT, MF_BYCOMMAND);

  if (iter->second.siVirtKey   == iter->second.siDefVirtKey &&
      iter->second.cModifier  == iter->second.cDefModifier)
    pContextMenu->RemoveMenu(ID_MENUITEM_RESETSHORTCUT, MF_BYCOMMAND);

  if (pContextMenu->GetMenuItemCount() == 0)
    goto exit;

  pt = point;
  ClientToScreen(&pt);

  int nID = pContextMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD,
                                         pt.x, pt.y, this);

  if (nID == ID_MENUITEM_REMOVESHORTCUT) {
    iter->second.siVirtKey = 0;
    iter->second.cModifier = 0;
    str = L"";
    goto update;
  }

  if (nID != ID_MENUITEM_RESETSHORTCUT)
    goto exit;

  iter->second.siVirtKey = iter->second.siDefVirtKey;
  iter->second.cModifier = iter->second.cDefModifier;

  str = CMenuShortcut::FormatShortcut(iter);

update:
  SetItemText(m_item, SHCT_SHORTCUTKEYS, str);
  RedrawItems(m_item, m_item);
  UpdateWindow();

exit:
  if (m_pParent != NULL)
    m_pParent->ClearWarning();

  if (m_item >= 0)
    SetItemState(m_item, SHCT_SHORTCUTKEYS, LVIS_SELECTED | LVIS_DROPHILITED);
}

void CSHCTListCtrl::SaveHotKey()
{
  if (m_bHotKeyActive) {
    WORD wVK, wMod;
    m_pHotKey->GetHotKey(wVK, wMod);
    OnHotKeyKillFocus(wVK, wMod);
  }
}

void CSHCTListCtrl::OnHotKeyKillFocus(const WORD wVirtualKeyCode, 
                                      const WORD wModifiers)
{
  m_pHotKey->EnableWindow(FALSE);
  m_pHotKey->ShowWindow(SW_HIDE);

  if (m_pParent != NULL) {
    m_pParent->OnHotKeyKillFocus(m_item, m_id, 
                                 wVirtualKeyCode, wModifiers);
  }

  m_bHotKeyActive = false;
}

void CSHCTListCtrl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
  SaveHotKey();
  CListCtrl::OnHScroll(nSBCode, nPos, pScrollBar);
  UpdateWindow();
}

void CSHCTListCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
  SaveHotKey();
  CListCtrl::OnVScroll(nSBCode, nPos, pScrollBar);
  UpdateWindow();
}

BOOL CSHCTListCtrl::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
  SaveHotKey();
  BOOL brc = CListCtrl::OnMouseWheel(nFlags, zDelta, pt);
  UpdateWindow();
  return brc;
}

void CSHCTListCtrl::OnCustomDraw(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW *>(pNotifyStruct);

  *pLResult = CDRF_DODEFAULT;
  const int iSubItem = pLVCD->iSubItem;
  const UINT id = (UINT)LOWORD(pLVCD->nmcd.lItemlParam);
  MapMenuShortcutsIter iter;

  switch(pLVCD->nmcd.dwDrawStage) {
    case CDDS_PREPAINT:
      *pLResult = CDRF_NOTIFYITEMDRAW;
      break;
    case CDDS_ITEMPREPAINT:
      *pLResult = CDRF_NOTIFYSUBITEMDRAW;
      break;
    case CDDS_ITEMPREPAINT | CDDS_SUBITEM:
      pLVCD->clrText = m_crWindowText;
      pLVCD->clrTextBk = GetTextBkColor();
      switch (iSubItem) {
        case SHCT_MENUITEMTEXT:
          if ( m_pParent->GetMapMenuShortcutsIter(id, iter) && 
              ( (iter->second.siVirtKey != iter->second.siDefVirtKey) || (iter->second.cModifier != iter->second.cDefModifier) ))
            pLVCD->clrText = m_crRedText;
          break;
        case SHCT_SHORTCUTKEYS:
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}

INT_PTR CSHCTListCtrl::OnToolHitTest(CPoint point, TOOLINFO *pTI) const
{
  LVHITTESTINFO lvhti;
  lvhti.pt = point;

  int item = ListView_SubItemHitTest(this->m_hWnd, &lvhti);
  if (item < 0)
    return -1;

  int nSubItem = lvhti.iSubItem;

  // nFlags is 0 if the SubItemHitTest fails
  // Therefore, 0 & <anything> will equal false
  if (lvhti.flags & LVHT_ONITEMLABEL) {
    // get the client (area occupied by this control
    RECT rcClient;
    GetClientRect(&rcClient);

    // fill in the TOOLINFO structure
    pTI->hwnd = m_hWnd;
    pTI->uId = (UINT) (nSubItem + 1);
    pTI->lpszText = LPSTR_TEXTCALLBACK;
    pTI->rect = rcClient;

    return pTI->uId;  // By returning a unique value per listItem,
              // we ensure that when the mouse moves over another
              // list item, the tooltip will change
  } else {
    //Otherwise, we aren't interested, so let the message propagate
    return -1;
  }
}

BOOL CSHCTListCtrl::OnToolTipText(UINT /*id*/, NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  UINT_PTR nID = pNotifyStruct->idFrom;

  // check if this is the automatic tooltip of the control
  if (nID == 0) 
    return TRUE;  // do not allow display of automatic tooltip,
                  // or our tooltip will disappear

  TOOLTIPTEXTW *pTTTW = (TOOLTIPTEXTW *)pNotifyStruct;

  *pLResult = 0;

  // get the mouse position
  const MSG* pMessage;
  pMessage = GetCurrentMessage();
  ASSERT(pMessage);
  CPoint pt;
  pt = pMessage->pt;    // get the point from the message
  ScreenToClient(&pt);  // convert the point's coords to be relative to this control

  // see if the point falls onto a list item
  LVHITTESTINFO lvhti;
  lvhti.pt = pt;
  
  SubItemHitTest(&lvhti);
  int nSubItem = lvhti.iSubItem;

  // nFlags is 0 if the SubItemHitTest fails
  // Therefore, 0 & <anything> will equal false
  if (lvhti.flags & LVHT_ONITEMLABEL) {
    // If it did fall on a list item,
    // and it was also hit one of the
    // item specific subitems we wish to show tooltips for
    
    switch (nSubItem) {
      case SHCT_MENUITEMTEXT:
        nID = IDS_SHCT_TOOLTIP0;
        break;
      case SHCT_SHORTCUTKEYS:
        nID = IDS_SHCT_TOOLTIP1;
        break;
      default:
        return FALSE;
    }
    // If there was a CString associated with the list item,
    // copy it's text (up to 80 characters worth, limitation 
    // of the TOOLTIPTEXT structure) into the TOOLTIPTEXT 
    // structure's szText member

    CString cs_TipText(MAKEINTRESOURCE(nID));

    delete m_pwchTip;

    m_pwchTip = new WCHAR[cs_TipText.GetLength() + 1];
    wcsncpy_s(m_pwchTip, cs_TipText.GetLength() + 1,
                cs_TipText, _TRUNCATE);
    pTTTW->lpszText = (LPWSTR)m_pwchTip;

    return TRUE;   // we found a tool tip,
  }
  
  return FALSE;  // we didn't handle the message, let the 
                 // framework continue propagating the message
}

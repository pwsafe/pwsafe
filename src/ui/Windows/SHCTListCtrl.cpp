/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "stdafx.h"
#include "SHCTListCtrl.h"
#include "SHCTHotKey.h"
#include "OptionsShortcuts.h"

#include "HKModifiers.h"

#include <algorithm>

#include "resource2.h"
#include "resource3.h"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CSHCTListCtrlX::CSHCTListCtrlX()
: m_pParent(NULL), m_pHotKey(NULL), m_bHotKeyActive(false)
{
  m_pHotKey = new CSHCTHotKey;
  m_crWindowText = ::GetSysColor(COLOR_WINDOWTEXT);
  m_crRedText    = RGB(168, 0, 0);
}

CSHCTListCtrlX::~CSHCTListCtrlX()
{
  m_pHotKey->DestroyWindow();
  delete m_pHotKey;
}

BEGIN_MESSAGE_MAP(CSHCTListCtrlX, CGridListCtrl)
  //{{AFX_MSG_MAP(CSHCTListCtrlX)
  ON_WM_LBUTTONDOWN()
  ON_WM_RBUTTONDOWN()
  ON_WM_HSCROLL()
  ON_WM_VSCROLL()
  ON_WM_MOUSEWHEEL()

  ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CSHCTListCtrlX::Init(COptionsShortcuts *pParent)
{
  if (m_pHotKey->GetSafeHwnd() == NULL) {
    CRect itemrect(0, 0, 0, 0);
    m_pHotKey->Create(0, itemrect, this, IDC_SHORTCUTHOTKEY);
    m_pHotKey->ModifyStyle(WS_BORDER, 0, 0);
    // Would like to change the default font (e.g. smaller and not bold) but it gets ignored
  }
  m_pHotKey->SetMyParent(dynamic_cast<CSHCTListCtrl *>(this));
  m_pParent = pParent;
}

void CSHCTListCtrlX::OnLButtonDown(UINT , CPoint point)
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
    WORD vHKModifiers = ConvertModifersPWS2MFC(iter->second.cPWSModifier);
    m_pHotKey->SetHotKey(iter->second.siVirtKey, vHKModifiers);
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

void CSHCTListCtrlX::OnRButtonDown(UINT , CPoint point)
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
      iter->second.cPWSModifier  == iter->second.cDefPWSModifier)
    pContextMenu->RemoveMenu(ID_MENUITEM_RESETSHORTCUT, MF_BYCOMMAND);

  if (pContextMenu->GetMenuItemCount() == 0)
    goto exit;

  pt = point;
  ClientToScreen(&pt);

  int nID = pContextMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD,
                                         pt.x, pt.y, this);

  if (nID == ID_MENUITEM_REMOVESHORTCUT) {
    iter->second.siVirtKey = 0;
    iter->second.cPWSModifier = 0;
    str = L"";
    goto update;
  }

  if (nID != ID_MENUITEM_RESETSHORTCUT)
    goto exit;

  iter->second.siVirtKey = iter->second.siDefVirtKey;
  iter->second.cPWSModifier = iter->second.cDefPWSModifier;

  str = CMenuShortcut::FormatShortcut(iter);

update:
  SetItemText(m_item, SHCT_SHORTCUTKEYS, str);
  RedrawItems(m_item, m_item);
  UpdateWindow();

exit:
  if (m_item >= 0)
    SetItemState(m_item, SHCT_SHORTCUTKEYS, LVIS_SELECTED | LVIS_DROPHILITED);
}

void CSHCTListCtrlX::SaveHotKey()
{
  if (m_bHotKeyActive) {
    WORD wVirtualKeyCode, wHKModifiers, wPWSModifiers;
    m_pHotKey->GetHotKey(wVirtualKeyCode, wHKModifiers);

    // Translate from CHotKeyCtrl to PWS modifiers
    wPWSModifiers = ConvertModifersMFC2PWS(wHKModifiers);
    OnMenuShortcutKillFocus(wVirtualKeyCode, wPWSModifiers);
  }
}

void CSHCTListCtrlX::OnMenuShortcutKillFocus(const WORD wVirtualKeyCode,
                                             const WORD wPWSModifiers)
{
  m_pHotKey->EnableWindow(FALSE);
  m_pHotKey->ShowWindow(SW_HIDE);

  if (m_pParent != NULL) {
    m_pParent->OnMenuShortcutKillFocus(m_item, m_id, 
                                 wVirtualKeyCode, wPWSModifiers);
  }

  m_bHotKeyActive = false;
}

void CSHCTListCtrlX::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
  SaveHotKey();
  CGridListCtrl::OnHScroll(nSBCode, nPos, pScrollBar);
  UpdateWindow();
}

void CSHCTListCtrlX::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
  SaveHotKey();
  CGridListCtrl::OnVScroll(nSBCode, nPos, pScrollBar);
  UpdateWindow();
}

BOOL CSHCTListCtrlX::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
  SaveHotKey();
  BOOL brc = CGridListCtrl::OnMouseWheel(nFlags, zDelta, pt);
  UpdateWindow();
  return brc;
}

void CSHCTListCtrlX::OnCustomDraw(NMHDR *pNotifyStruct, LRESULT *pLResult)
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
              ((iter->second.siVirtKey != iter->second.siDefVirtKey) ||
               (iter->second.cPWSModifier != iter->second.cDefPWSModifier)))
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

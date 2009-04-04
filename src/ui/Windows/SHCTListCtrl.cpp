/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
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
: m_pParent(NULL), m_pHotKey(NULL), m_pchTip(NULL), m_pwchTip(NULL),
  m_bChanged(false), m_bHotKeyActive(false)
{
  m_pHotKey = new CSHCTHotKey;
  m_crWindowText = ::GetSysColor(COLOR_WINDOWTEXT);
  m_crRedText    = RGB(168, 0, 0);
}

CSHCTListCtrl::~CSHCTListCtrl()
{
  m_pHotKey->DestroyWindow();
  delete m_pHotKey;
  delete m_pchTip;
  delete m_pwchTip;
}

BEGIN_MESSAGE_MAP(CSHCTListCtrl, CListCtrl)
  //{{AFX_MSG_MAP(CSHCTListCtrl)
  ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipText)
  ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipText)
  ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
  ON_WM_LBUTTONDOWN()
  ON_WM_RBUTTONDOWN()
  ON_WM_HSCROLL()
  ON_WM_VSCROLL()
  ON_WM_MOUSEWHEEL()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CSHCTListCtrl::Init(COptionsShortcuts *pParent)
{
  m_pHotKey->SetMyParent(this);
  m_pParent = pParent;
  EnableToolTips(TRUE);
}

void CSHCTListCtrl::OnLButtonDown(UINT /* nFlags*/ , CPoint point)
{
  MapMenuShortcutsIter iter;
  MapKeyNameIDConstIter citer;
  CRect subitemrect;
  int iSubItem = -1;

  SaveHotKey();

  m_item = -1;

  LVHITTESTINFO lvhti;
  lvhti.pt = point;
  SubItemHitTest(&lvhti);

  if (lvhti.flags & LVHT_ONITEM) {
    m_item = lvhti.iItem;
    iSubItem = lvhti.iSubItem;
  }

  if (m_item < 0 || iSubItem < 1) {
    return; //CListCtrl::OnLButtonDown(nFlags, point);
  } else {
    if (m_pParent != NULL)
      m_pParent->ClearWarning();

    GetSubItemRect(m_item, 1, LVIR_BOUNDS, subitemrect);
    subitemrect.top -= 1;
    subitemrect.left += 1;
    if (m_pHotKey->GetSafeHwnd() == NULL) {
      m_pHotKey->Create(0, subitemrect, this, IDC_SHORTCUTHOTKEY);
      m_pHotKey->ModifyStyle(WS_BORDER, 0, 0);
    } else {
      m_pHotKey->MoveWindow(&subitemrect);
    }

    m_id = (UINT)LOWORD(GetItemData(m_item));
    iter = m_pParent->GetMapMenuShortcutsIter(m_id);

    WORD vModifiers = (iter->second.bCtrl ? HOTKEYF_CONTROL : 0) |
                      (iter->second.bAlt ? HOTKEYF_ALT : 0) |
                      (iter->second.bShift ? HOTKEYF_SHIFT : 0);
    m_pHotKey->SetHotKey(iter->second.cVirtKey, vModifiers);

    m_pHotKey->EnableWindow(TRUE);
    m_pHotKey->ShowWindow(SW_SHOW);
    m_pHotKey->BringWindowToTop();
    m_pHotKey->SetFocus();
    m_bHotKeyActive = true;
  }

  if (m_item >= 0)
    SetItemState(m_item, 0, LVIS_SELECTED | LVIS_DROPHILITED);

  UpdateWindow();
}

void CSHCTListCtrl::OnRButtonDown(UINT nFlags, CPoint point)
{
  CMenu PopupMenu;
  MapMenuShortcutsIter iter;
  MapKeyNameIDConstIter citer;
  CString str;
  CPoint pt;
  int iSubItem = -1;

  SaveHotKey();

  m_item = -1;

  LVHITTESTINFO lvhti;
  lvhti.pt = point;
  SubItemHitTest(&lvhti);

  if (lvhti.flags & LVHT_ONITEM) {
    m_item = lvhti.iItem;
    iSubItem = lvhti.iSubItem;
  }

  if (m_item < 0 || iSubItem < 0) {
    CListCtrl::OnRButtonDown(nFlags, point);
    goto exit;
  }

  m_id = (UINT)LOWORD(GetItemData(m_item));

  iter = m_pParent->GetMapMenuShortcutsIter(m_id);
  citer = m_pParent->GetMapKeyNameIDConstIter(iter->second.cVirtKey);

  PopupMenu.LoadMenu(IDR_POPRESETSHORTCUT);
  CMenu* pContextMenu = PopupMenu.GetSubMenu(0);
  if (iter->second.cVirtKey == 0)
    pContextMenu->RemoveMenu(ID_MENUITEM_REMOVESHORTCUT, MF_BYCOMMAND);

  if (iter->second.cVirtKey == iter->second.cdefVirtKey &&
      iter->second.bCtrl    == iter->second.bdefCtrl &&
      iter->second.bAlt     == iter->second.bdefAlt &&
      iter->second.bShift   == iter->second.bdefShift)
    pContextMenu->RemoveMenu(ID_MENUITEM_RESETSHORTCUT, MF_BYCOMMAND);

  if (pContextMenu->GetMenuItemCount() == 0)
    goto exit;

  pt = point;
  ClientToScreen(&pt);

  int nID = pContextMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD,
                                         pt.x, pt.y, this);

  if (nID == ID_MENUITEM_REMOVESHORTCUT) {
    iter->second.bCtrl = false;
    iter->second.bAlt = false;
    iter->second.bShift = false;
    iter->second.cVirtKey = (unsigned char)0;
    str = _T("");
    goto update;
  }

  if (nID != ID_MENUITEM_RESETSHORTCUT)
    goto exit;

  iter->second.bCtrl = iter->second.bdefCtrl;
  iter->second.bAlt = iter->second.bdefAlt;
  iter->second.bShift = iter->second.bdefShift;
  iter->second.cVirtKey = iter->second.cdefVirtKey;

  if (iter->second.cVirtKey != 0) {
    citer = m_pParent->GetMapKeyNameIDConstIter(iter->second.cVirtKey);
    str.Format(_T("%s%s%s%s"),
               iter->second.bCtrl  ? _T("Ctrl + ")  : _T(""),
               iter->second.bAlt   ? _T("Alt + ")   : _T(""),
               iter->second.bShift ? _T("Shift + ") : _T(""),
               citer->second);
  } else {
    str = _T("");
  }

update:
  SetItemText(m_item, 1, str);
  RedrawItems(m_item, m_item);
  UpdateWindow();
  m_bChanged = true;

exit:
  if (m_pParent != NULL)
    m_pParent->ClearWarning();

  if (m_item >= 0)
    SetItemState(m_item, 0, LVIS_SELECTED | LVIS_DROPHILITED);
}

void CSHCTListCtrl::SaveHotKey()
{
  if (m_bHotKeyActive) {
    WORD vVK, vMod;
    m_pHotKey->GetHotKey(vVK, vMod);
    OnHotKeyKillFocus(vVK, vMod);
  }
}

void CSHCTListCtrl::OnHotKeyKillFocus(const WORD vVirtualKeyCode, 
                                      const WORD vModifiers)
{
  m_pHotKey->EnableWindow(FALSE);
  m_pHotKey->ShowWindow(SW_HIDE);

  if (m_pParent != NULL) {
    m_pParent->OnHotKeyKillFocus(m_item, m_id, 
                                 vVirtualKeyCode, vModifiers);
  }

  m_bHotKeyActive = false;
}

void CSHCTListCtrl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
  SaveHotKey();
  CListCtrl::OnHScroll(nSBCode, nPos, pScrollBar);
  //UpdateWindow();
}

void CSHCTListCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
  SaveHotKey();
  CListCtrl::OnVScroll(nSBCode, nPos, pScrollBar);
  //UpdateWindow();
}

BOOL CSHCTListCtrl::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
  SaveHotKey();
  BOOL brc = CListCtrl::OnMouseWheel(nFlags, zDelta, pt);
  //UpdateWindow();
  return brc;
}

void CSHCTListCtrl::OnCustomDraw(NMHDR* pNotifyStruct, LRESULT* pResult)
{
  NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW *>(pNotifyStruct);

  *pResult = CDRF_DODEFAULT;
  const int iSubItem = pLVCD->iSubItem;
  const UINT id = (UINT)LOWORD(pLVCD->nmcd.lItemlParam);
  MapMenuShortcutsIter iter;

  switch(pLVCD->nmcd.dwDrawStage) {
    case CDDS_PREPAINT:
      *pResult = CDRF_NOTIFYITEMDRAW;
      break;
    case CDDS_ITEMPREPAINT:
      *pResult = CDRF_NOTIFYSUBITEMDRAW;
      break;
    case CDDS_ITEMPREPAINT | CDDS_SUBITEM:
      pLVCD->clrText = m_crWindowText;
      pLVCD->clrTextBk = GetTextBkColor();
      switch (iSubItem) {
        case SHCT_MENUITEMTEXT:
          iter = m_pParent->GetMapMenuShortcutsIter(id);
          if (iter->second.cVirtKey != iter->second.cdefVirtKey ||
              iter->second.bCtrl    != iter->second.bdefCtrl ||
              iter->second.bAlt     != iter->second.bdefAlt ||
              iter->second.bShift   != iter->second.bdefShift)
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

BOOL CSHCTListCtrl::OnToolTipText(UINT /*id*/, NMHDR * pNMHDR, LRESULT * pResult)
{
  UINT nID = pNMHDR->idFrom;

  // check if this is the automatic tooltip of the control
  if (nID == 0) 
    return TRUE;  // do not allow display of automatic tooltip,
                  // or our tooltip will disappear

  // handle both ANSI and UNICODE versions of the message
  TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
  TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;

  *pResult = 0;

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

#define LONG_TOOLTIPS

#ifdef LONG_TOOLTIPS

#ifdef _UNICODE
  if (pNMHDR->code == TTN_NEEDTEXTA) {
    delete m_pchTip;

    m_pchTip = new char[cs_TipText.GetLength() + 1];
#if (_MSC_VER >= 1400)
    size_t num_converted;
    wcstombs_s(&num_converted, m_pchTip, cs_TipText.GetLength() + 1, cs_TipText,
               cs_TipText.GetLength() + 1);
#else
    wcstombs(m_pchTip, cs_TipText, cs_TipText.GetLength() + 1);
#endif
    pTTTA->lpszText = (LPSTR)m_pchTip;
  } else {
    delete m_pwchTip;

    m_pwchTip = new WCHAR[cs_TipText.GetLength() + 1];
    lstrcpyn(m_pwchTip, cs_TipText, cs_TipText.GetLength() + 1);
    pTTTW->lpszText = (LPWSTR)m_pwchTip;
  }

#else // Unicode

  if (pNMHDR->code == TTN_NEEDTEXTA) {
    delete m_pchTip;

    m_pchTip = new char[cs_TipText.GetLength() + 1];
    lstrcpyn(m_pchTip, cs_TipText, cs_TipText.GetLength()+1);
    pTTTA->lpszText = (LPSTR)m_pchTip;
  } else {
    delete m_pwchTip;

    m_pwchTip = new WCHAR[cs_TipText.GetLength() + 1];
#if _MSC_VER >= 1400
    size_t numconverted;
    mbstowcs_s(&numconverted, m_pwchTip, cs_TipText.GetLength() + 1, cs_TipText, 
               cs_TipText.GetLength() + 1);
#else
    mbstowcs(m_pwchTip, cs_TipText, cs_TipText.GetLength() + 1);
#endif
    pTTTW->lpszText = m_pwchTip;
  }

#endif // UNICODE/ASCII

#else // Short Tooltips!

#ifdef _UNICODE
    if (pNMHDR->code == TTN_NEEDTEXTA) {
      int n = WideCharToMultiByte(CP_ACP, 0, cs_TipText, -1,
                                  pTTTA->szText,
                                  sizeof(pTTTA->szText)/sizeof(pTTTA->szText[0]),
                                  NULL, NULL);
      if (n > 0)
        pTTTA->szText[n - 1] = 0;
    } else
#if _MSC_VER >= 1400
      _tcsncpy_s(pTTTW->szText, (sizeof(pTTTW->szText)/sizeof(pTTTW->szText[0])),
      cs_TipText, _TRUNCATE);
#else
      _tcsncpy(pTTTW->szText, cs_TipText, (sizeof(pTTTW->szText)/sizeof(pTTTW->szText[0])));
#endif

#else  // UNICODE

    if (pNMHDR->code == TTN_NEEDTEXTA)
#if _MSC_VER >= 1400
      _tcsncpy_s(pTTTA->szText, (sizeof(pTTTA->szText)/sizeof(pTTTA->szText[0])),
                 cs_TipText, _TRUNCATE);
#else
      _tcsncpy(pTTTA->szText, cs_TipText, (sizeof(pTTTA->szText)/sizeof(pTTTA->szText[0])));
#endif
    else {
      int n = MultiByteToWideChar(CP_ACP, 0, cs_TipText, -1, pTTTW->szText,
                                  sizeof(pTTTW->szText)/sizeof(pTTTW->szText[0]));
      if (n > 0)
        pTTTW->szText[n - 1] = 0;
    }
#endif // UNICODE/ASCII
#endif // Long/short tooltips

    return TRUE;   // we found a tool tip,
  }
  
  return FALSE;  // we didn't handle the message, let the 
                 // framework continue propagating the message
}

/*
* Copyright (c) 2003-2017 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// ShortcutConflictDlg.cpp : implementation file
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "ShortcutConflictDlg.h"
#include "HKModifiers.h"

#include "core/StringX.h"

#include "resource.h"

// CSHCTConflictListCtrl CListCtrl

BEGIN_MESSAGE_MAP(CSHCTConflictListCtrl, CListCtrl)
  //{{AFX_MSG_MAP(CSHCTConflictListCtrl)
  ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CSHCTConflictListCtrl::OnCustomDraw(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  NMLVCUSTOMDRAW *pLVCD = reinterpret_cast<NMLVCUSTOMDRAW *>(pNotifyStruct);

  *pLResult = CDRF_DODEFAULT;
  const int iSubItem = pLVCD->iSubItem;
  static int iColour;

  switch (pLVCD->nmcd.dwDrawStage) {
  case CDDS_PREPAINT:
    *pLResult = CDRF_NOTIFYITEMDRAW;
    break;
  case CDDS_ITEMPREPAINT:
    iColour = pLVCD->clrText;
    *pLResult = CDRF_NOTIFYSUBITEMDRAW;
    break;
  case (CDDS_ITEMPREPAINT | CDDS_SUBITEM):
    pLVCD->clrText = iSubItem == 0 ? RGB(168, 0, 0) : iColour;
    break;
  default:
    break;
  }
}

// CShortcutConflictDlg dialog

IMPLEMENT_DYNAMIC(CShortcutConflictDlg, CPWDialog)

CShortcutConflictDlg::CShortcutConflictDlg(CWnd *pParent, std::vector<st_Conflicts> vConflicts)
	: CPWDialog(CShortcutConflictDlg::IDD, pParent), m_vConflicts(vConflicts),
  m_bKBSortAscending(true), m_iKBSortedColumn(0)
{
}

CShortcutConflictDlg::~CShortcutConflictDlg()
{
}

void CShortcutConflictDlg::DoDataExchange(CDataExchange* pDX)
{
  CPWDialog::DoDataExchange(pDX);

  DDX_Control(pDX, IDC_SHORTCUTCONFLICTLIST, m_LCConflicts);
}

BEGIN_MESSAGE_MAP(CShortcutConflictDlg, CPWDialog)
  ON_NOTIFY(HDN_ITEMCLICK, IDC_SHORTCUTLIST_HEADER, OnColumnClick)
END_MESSAGE_MAP()

// CShortcutConflictDlg message handlers

BOOL CShortcutConflictDlg::OnInitDialog()
{
  CPWDialog::OnInitDialog();

  // Set CListCtrl Header Control ID
  CHeaderCtrl *pHeader = m_LCConflicts.GetHeaderCtrl();
  pHeader->SetDlgCtrlID(IDC_SHORTCUTLIST_HEADER);

  const CString CS_CTRLP(MAKEINTRESOURCE(IDS_CTRLP));
  const CString CS_ALTP(MAKEINTRESOURCE(IDS_ALTP));
  const CString CS_SHIFTP(MAKEINTRESOURCE(IDS_SHIFTP));

  CString cs_header;
  cs_header.LoadString(IDS_COL_SHORTCUT);
  m_LCConflicts.InsertColumn(0, cs_header);
  cs_header.LoadString(IDS_COL_MENUITEM);
  m_LCConflicts.InsertColumn(1, cs_header);
  cs_header.LoadString(IDS_COL_ENTRY);
  m_LCConflicts.InsertColumn(2, cs_header);

  int iItem(0);
  for (auto iter = m_vConflicts.begin(); iter != m_vConflicts.end(); iter++) {
    WORD wVirtualKeyCode = iter->iShortcut & 0xff;
    WORD wPWSModifiers = iter->iShortcut >> 16;

    CString cs_ShortcutValue = CHotKeyCtrl::GetKeyName(wVirtualKeyCode, wPWSModifiers & PWS_HOTKEYF_EXT);
    if (cs_ShortcutValue.GetLength() == 1)
      cs_ShortcutValue.MakeUpper();
    if ((wPWSModifiers & PWS_HOTKEYF_SHIFT) == PWS_HOTKEYF_SHIFT)
      cs_ShortcutValue = CS_SHIFTP + cs_ShortcutValue;
    if ((wPWSModifiers & PWS_HOTKEYF_CONTROL) == PWS_HOTKEYF_CONTROL)
      cs_ShortcutValue = CS_CTRLP + cs_ShortcutValue;
    if ((wPWSModifiers & PWS_HOTKEYF_ALT) == PWS_HOTKEYF_ALT)
      cs_ShortcutValue = CS_ALTP + cs_ShortcutValue;

    CString cs_entry;
    cs_entry.Format(L"\xab%s\xbb \xab%s\xbb \xab%s\xbb",
                      static_cast<LPCWSTR>(iter->cs_Group),
                      static_cast<LPCWSTR>(iter->cs_Title),
                      static_cast<LPCWSTR>(iter->cs_User));
    iItem = m_LCConflicts.InsertItem(iItem, cs_ShortcutValue);
    m_LCConflicts.SetItemText(iItem, 1, iter->cs_Menu);
    m_LCConflicts.SetItemText(iItem, 2, cs_entry);
    m_LCConflicts.SetItemData(iItem, (DWORD_PTR)(iter - m_vConflicts.begin()));
  }

  m_LCConflicts.SetRedraw(FALSE);

  for (int i = 0; i < 2; i++) {
    m_LCConflicts.SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
    int header_width = m_LCConflicts.GetColumnWidth(i);
    m_LCConflicts.SetColumnWidth(i, LVSCW_AUTOSIZE);
    int data_width = m_LCConflicts.GetColumnWidth(i);
    if (header_width > data_width)
      m_LCConflicts.SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
  }

  m_LCConflicts.SetColumnWidth(2, LVSCW_AUTOSIZE_USEHEADER);
  m_LCConflicts.SetRedraw(TRUE);
  m_LCConflicts.Invalidate();

  return TRUE;
}

void CShortcutConflictDlg::OnColumnClick(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  NMHEADER *pNMHeaderCtrl = (NMHEADER *)pNotifyStruct;

  // Get column number to CItemData value
  int iKBSortColumn = pNMHeaderCtrl->iItem;

  if (m_iKBSortedColumn == iKBSortColumn) {
    m_bKBSortAscending = !m_bKBSortAscending;
  } else {
    m_iKBSortedColumn = iKBSortColumn;
    m_bKBSortAscending = true;
  }

  m_LCConflicts.SortItems(CKBSHCompareFunc, (LPARAM)this);

  HDITEM hdi;
  hdi.mask = HDI_FORMAT;

  CHeaderCtrl *pHDRCtrl;

  pHDRCtrl = m_LCConflicts.GetHeaderCtrl();
  pHDRCtrl->GetItem(iKBSortColumn, &hdi);
  // Turn off all arrows
  hdi.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
  // Turn on the correct arrow
  hdi.fmt |= ((m_bKBSortAscending == TRUE) ? HDF_SORTUP : HDF_SORTDOWN);
  pHDRCtrl->SetItem(iKBSortColumn, &hdi);

  *pLResult = TRUE; // Say we have done all processing on return
}

/*
* Compare function used by m_LCConflicts.SortItems()
* "The comparison function must return a negative value if the first item should precede
* the second, a positive value if the first item should follow the second, or zero if
* the two items are equivalent."
*/
int CALLBACK CShortcutConflictDlg::CKBSHCompareFunc(LPARAM lParam1, LPARAM lParam2,
                                                    LPARAM lParamSort)
{
  // m_bSortAscending to determine the direction of the sort (duh)

  CShortcutConflictDlg *self = (CShortcutConflictDlg *)lParamSort;
  int iResult(0);
  const int nSortColumn = self->m_iKBSortedColumn;
  StringX sxLHS(L""), sxRHS(L"");


  if (nSortColumn == 0) {
    int32 iLHS_Shortcut = self->m_vConflicts[lParam1].iShortcut;
    int32 iRHS_Shortcut = self->m_vConflicts[lParam1].iShortcut;
    WORD wLHS_VirtualKeyCode, wRHS_VirtualKeyCode, wPWSModifiers;
    wLHS_VirtualKeyCode = iLHS_Shortcut & 0xff;
    wPWSModifiers = WORD(iLHS_Shortcut >> 16);

    WORD wLHS_HKModifiers = ConvertModifersPWS2MFC(wPWSModifiers);

    wRHS_VirtualKeyCode = iRHS_Shortcut & 0xff;
    wPWSModifiers = WORD(iRHS_Shortcut >> 16);

    WORD wRHS_HKModifiers = ConvertModifersPWS2MFC(wPWSModifiers);

    if (wLHS_HKModifiers != wRHS_HKModifiers)
      iResult = wRHS_HKModifiers < wLHS_HKModifiers ? -1 : 1;
    else
      iResult = wRHS_VirtualKeyCode < wLHS_VirtualKeyCode ? 1 : -1;

  } else {
    switch (nSortColumn) {
    case 1:  // Menu
      sxLHS = self->m_vConflicts[lParam1].cs_Menu;
      sxRHS = self->m_vConflicts[lParam2].cs_Menu;
      break;
    case 2: // Entry - sort via group/user/title
      sxLHS = self->m_vConflicts[lParam1].cs_Group;
      sxRHS = self->m_vConflicts[lParam2].cs_Group;
      iResult = CompareNoCase(sxLHS, sxRHS);

      if (iResult == 0) {
        sxLHS = self->m_vConflicts[lParam1].cs_Title;
        sxRHS = self->m_vConflicts[lParam2].cs_Title;
      }
      iResult = CompareNoCase(sxLHS, sxRHS);

      if (iResult == 0) {
        sxLHS = self->m_vConflicts[lParam1].cs_User;
        sxRHS = self->m_vConflicts[lParam2].cs_User;
      }
      iResult = CompareNoCase(sxLHS, sxRHS);
      break;
    }

    iResult = CompareNoCase(sxLHS, sxRHS);
  }

  if (!self->m_bKBSortAscending && iResult != 0) {
    iResult *= -1;
  }
  return iResult;
}

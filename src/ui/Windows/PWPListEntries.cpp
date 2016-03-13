/*
* Copyright (c) 2003-2016 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// PWPListEntries.cpp : implementation file
//

#include "stdafx.h"
#include "PWPListEntries.h"
#include "SecString.h"

#include "resource.h"
#include "resource3.h"

// CPWPListEntries dialog

IMPLEMENT_DYNAMIC(CPWPListEntries, CPWDialog)

CPWPListEntries::CPWPListEntries(CWnd* pParent, StringX sxPolicyName, 
  std::vector<st_GroupTitleUser> *pventries)
	: CPWDialog(CPWPListEntries::IDD, pParent), m_sxPolicyName(sxPolicyName),
  m_pventries(pventries), m_iSortedColumn(0),  m_bSortAscending(FALSE)
{
}

CPWPListEntries::~CPWPListEntries()
{
}

void CPWPListEntries::DoDataExchange(CDataExchange* pDX)
{
  CPWDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_NPWP_LIST_ENTRIES, m_PolicyEntries);
}

BEGIN_MESSAGE_MAP(CPWPListEntries, CPWDialog)
  ON_NOTIFY(HDN_ITEMCLICKW, 0, OnHeaderClicked)
END_MESSAGE_MAP()

BOOL CPWPListEntries::OnInitDialog()
{
  CPWDialog::OnInitDialog();

  CString csTitle;
  GetWindowText(csTitle);
  csTitle += m_sxPolicyName.c_str();
  SetWindowText(csTitle);

  // Looks nicer with grid lines
  DWORD dwExtendedStyle = m_PolicyEntries.GetExtendedStyle();
  dwExtendedStyle |= LVS_EX_GRIDLINES;
  m_PolicyEntries.SetExtendedStyle(dwExtendedStyle);

  // Add columns
  CString cs_text;
  cs_text.LoadString(IDS_GROUP);
  m_PolicyEntries.InsertColumn(0, cs_text);
  cs_text.LoadString(IDS_TITLE);
  m_PolicyEntries.InsertColumn(1, cs_text);
  cs_text.LoadString(IDS_USERNAME);
  m_PolicyEntries.InsertColumn(1, cs_text);

  // Add entries
  int nPos = 0;
  for (size_t i = 0; i < m_pventries->size(); i++) {
    st_GroupTitleUser st = m_pventries->at(i);
    nPos = m_PolicyEntries.InsertItem(nPos, st.group.c_str());
    m_PolicyEntries.SetItemText(nPos, 1, st.title.c_str());
    m_PolicyEntries.SetItemText(nPos, 2, st.user.c_str());
    m_PolicyEntries.SetItemData(nPos, i);
    nPos++;
  }

  // Resize columns
  m_PolicyEntries.SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);
  m_PolicyEntries.SetColumnWidth(1, LVSCW_AUTOSIZE_USEHEADER);
  m_PolicyEntries.SetColumnWidth(2, LVSCW_AUTOSIZE_USEHEADER);

  return FALSE;
}

void CPWPListEntries::OnHeaderClicked(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  HD_NOTIFY *phdn = (HD_NOTIFY *)pNotifyStruct;

  if (phdn->iButton == 0) {
    // User clicked on header using left mouse button
    if (phdn->iItem == m_iSortedColumn)
      m_bSortAscending = !m_bSortAscending;
    else
      m_bSortAscending = TRUE;

    m_iSortedColumn = phdn->iItem;
    m_PolicyEntries.SortItems(CompareFunc, (LPARAM)this);

    // Note: WINVER defines the minimum system level for which this is program compiled and
    // NOT the level of system it is running on!
    // In this case, these values are defined in Windows XP and later and supported
    // by V6 of comctl32.dll (supplied with Windows XP) and later.
    // They should be ignored by earlier levels of this dll or .....
    //     we can check the dll version (code available on request)!

#if (WINVER < 0x0501)  // These are already defined for WinXP and later
#define HDF_SORTUP 0x0400
#define HDF_SORTDOWN 0x0200
#endif
    HDITEM HeaderItem;
    HeaderItem.mask = HDI_FORMAT;
    m_PolicyEntries.GetHeaderCtrl()->GetItem(m_iSortedColumn, &HeaderItem);
    // Turn off all arrows
    HeaderItem.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
    // Turn on the correct arrow
    HeaderItem.fmt |= ((m_bSortAscending == TRUE) ? HDF_SORTUP : HDF_SORTDOWN);
    m_PolicyEntries.GetHeaderCtrl()->SetItem(m_iSortedColumn, &HeaderItem);
  }

  *pLResult = 0;
}

int CALLBACK CPWPListEntries::CompareFunc(LPARAM lParam1, LPARAM lParam2,
                                          LPARAM closure)
{
  CPWPListEntries *self = (CPWPListEntries *)closure;
  int nSortColumn = self->m_iSortedColumn;
  const st_GroupTitleUser pLHS = self->m_pventries->at(lParam1);
  const st_GroupTitleUser pRHS = self->m_pventries->at(lParam2);

  int iResult(0);
  switch (nSortColumn) {
  case 0:
    iResult = CompareNoCase(pLHS.group, pRHS.group);
    break;
  case 1:
    iResult = CompareNoCase(pLHS.title, pRHS.title);
    break;
  case 2:
    iResult = CompareNoCase(pLHS.user, pRHS.user);
    break;
  default:
    ASSERT(FALSE);
  }

  if (!self->m_bSortAscending && iResult != 0)
    iResult *= -1;

  return iResult;
}

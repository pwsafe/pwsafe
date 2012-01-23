/*
* Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "stdafx.h"

#include "CWListCtrl.h"
#include "DboxMain.h" // For TIMER_FIND

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CCWListCtrl::CCWListCtrl()
  : m_FindTimerID(0), m_csFind(L"")
{
}

CCWListCtrl::~CCWListCtrl()
{
}

BEGIN_MESSAGE_MAP(CCWListCtrl, CListCtrl)
  //{{AFX_MSG_MAP(CCWListCtrl)
  ON_WM_TIMER()
  ON_MESSAGE(WM_CHAR, OnCharItemlist)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

LRESULT CCWListCtrl::OnCharItemlist(WPARAM wParam, LPARAM /* lParam */)
{
  bool bFirst;

  if (m_FindTimerID != 0) {
    KillTimer(TIMER_FIND);
    m_csFind += (wchar_t)wParam;
    bFirst = false;
  } else {
    m_csFind = (wchar_t)wParam;
    bFirst = true;
  }

  if (!FindNext(m_csFind, 0) && !bFirst) {
    // Didn't find a match when more than one character
    // Emulate CListCtrl and try again (once) with this matching the first character
    m_csFind = (wchar_t)wParam;
    FindNext(m_csFind, 0);
  }

  // Set timer going again
  m_FindTimerID = SetTimer(TIMER_FIND, 1000, NULL);
  return 0L;
}

void CCWListCtrl::OnTimer(UINT_PTR nIDEvent)
{
  switch (nIDEvent) {
    case TIMER_FIND:
      KillTimer(TIMER_FIND);
      m_FindTimerID = 0;
      break;
    default:
      CListCtrl::OnTimer(nIDEvent);
      break;
  }
}

bool CCWListCtrl::FindNext(const CString &cs_find, const int iSubItem)
{
  int iItem;
  bool bFound(false);
  CString cs_text;
  const int iNum = GetItemCount();
  const int iFindLen = cs_find.GetLength();

  // Get selected item, if any
  POSITION pos = GetFirstSelectedItemPosition();

  // First search down.
  if (pos == NULL)
    iItem = 0;
  else
    iItem = (int)pos;

  do {
    cs_text = GetItemText(iItem, iSubItem);
    cs_text = cs_text.Mid(0, iFindLen);
    if (cs_text.GetLength() > 0 && cs_find.CompareNoCase(cs_text) == 0) {
      bFound = true;
      break;
    }
    iItem++;
  } while (iItem <= iNum);

  // Not found searching down and we didn't start from the top, now start from the top until
  // we get to where we started!
  if (!bFound && pos != NULL) {
    iItem = 0;
    do {
      cs_text = GetItemText(iItem, iSubItem);
      cs_text = cs_text.Mid(0, iFindLen);
      if (cs_text.GetLength() > 0 && cs_find.CompareNoCase(cs_text) == 0) {
        bFound = true;
        break;
      }
      iItem++;
    } while (iItem != (int)pos);
  }

  if (bFound) {
    SetItemState(iItem, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
    EnsureVisible(iItem, FALSE);
    Invalidate();
  }

  return bFound;
}

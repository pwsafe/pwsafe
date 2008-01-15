/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "stdafx.h"
#include "PWListCtrl.h"
#include "DboxMain.h"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CPWListCtrl::CPWListCtrl()
: m_FindTimerID(0), m_csFind(_T(""))
{
}

CPWListCtrl::~CPWListCtrl()
{
}

BEGIN_MESSAGE_MAP(CPWListCtrl, CListCtrl)
  //{{AFX_MSG_MAP(CPWListCtrl)
  ON_MESSAGE(WM_CHAR, OnCharItemlist)
  ON_WM_DESTROY()
  ON_WM_TIMER()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

LRESULT CPWListCtrl::OnCharItemlist(WPARAM wParam, LPARAM /* lParam */)
{
  const int iSubItem = m_pDbx->IsImageVisible() ? 1 : 0;
  bool bFirst;
  if (m_FindTimerID != 0) {
    KillTimer(TIMER_FIND);
    m_csFind += (TCHAR)wParam;
    bFirst = false;
  } else {
    m_csFind = (TCHAR)wParam;
    bFirst = true;
  }
  if (!FindNext(m_csFind, iSubItem) && !bFirst) {
    // Didn't find a match when more than one character
    // Emulate CListCtrl and try again (once) with this matching the first character
    m_csFind = (TCHAR)wParam;
    FindNext(m_csFind, iSubItem);
  }
  // Set timer going again
  m_FindTimerID = SetTimer(TIMER_FIND, 1000, NULL);
  return 0L;
}

void CPWListCtrl::OnDestroy()
{
  // Remove dummy ImageList. PWTreeCtrl removes the real one!
  m_pDbx->m_pImageList0->DeleteImageList();
  delete m_pDbx->m_pImageList0;
}

void CPWListCtrl::OnTimer(UINT_PTR nIDEvent)
{
  if (nIDEvent == TIMER_FIND) {
    KillTimer(TIMER_FIND);
    m_FindTimerID = 0;
  }
}

bool
CPWListCtrl::FindNext(const CString &cs_find, const int iSubItem)
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

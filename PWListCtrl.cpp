/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
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

LRESULT CPWListCtrl::OnCharItemlist(WPARAM wParam, LPARAM lParam)
{
  bool bFirst;
  if (m_pDbx->IsImageVisible()) {
    if (m_FindTimerID != 0) {
      KillTimer(TIMER_FIND);
      m_csFind += (TCHAR)wParam;
      bFirst = false;
    } else {
      m_csFind = (TCHAR)wParam;
      bFirst = true;
    }
    m_FindTimerID = SetTimer(TIMER_FIND, 500, NULL);
    if (!m_pDbx->FindNext(m_csFind) && !bFirst) {
      // Didn't find a match when more than one character
      // Emulate CListCtrl and try again (once) with this matching the first character
      m_csFind = (TCHAR)wParam;
      m_pDbx->FindNext(m_csFind);
    }
  } else {
    UINT nRepCnt = LOWORD(lParam);
    CListCtrl::OnChar((UINT)wParam, nRepCnt, (UINT)lParam);
  }
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
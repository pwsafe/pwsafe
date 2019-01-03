/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// PWHdrCtrlNoChng.cpp : implementation file
//

#include "stdafx.h"
#include "PWHdrCtrlNoChng.h"

// CPWHdrCtrlNoChng

IMPLEMENT_DYNAMIC(CPWHdrCtrlNoChng, CHeaderCtrl)

CPWHdrCtrlNoChng::CPWHdrCtrlNoChng()
  : m_bStopChange(false)
{
}

CPWHdrCtrlNoChng::~CPWHdrCtrlNoChng()
{
}

BEGIN_MESSAGE_MAP(CPWHdrCtrlNoChng, CHeaderCtrl)
  //{{AFX_MSG_MAP(CPWHdrCtrlNoChng)
  ON_WM_SETCURSOR()
  ON_WM_LBUTTONDBLCLK()
  ON_NOTIFY_REFLECT(HDN_BEGINTRACK, OnBeginTrack) 
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

// CPWHdrCtrlNoChng message handlers

BOOL CPWHdrCtrlNoChng::OnSetCursor(CWnd* /* pWnd */, UINT /* nHitTest */, UINT /* message */)
{
  // Disable by returning TRUE and not calling base class
  //  return CHeaderCtrl::OnSetCursor(pWnd, nHitTest, message);
  return TRUE;
}

void CPWHdrCtrlNoChng::OnLButtonDblClk(UINT /* nFlags */, CPoint /* point */)
{
  // Disable by not calling base case
  //  CHeaderCtrl::OnLButtonDblClk(nFlags, point);
}

void CPWHdrCtrlNoChng::OnBeginTrack(NMHDR *, LRESULT *pLResult)
{
  // Don't allow user to change the size of any columns!
  *pLResult = TRUE;
}

void CPWHdrCtrlNoChng::OnItemChanging(NMHDR *, LRESULT *pLResult)
{
  *pLResult = m_bStopChange ? TRUE : FALSE;
}

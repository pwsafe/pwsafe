/*
* Copyright (c) 2003-2017 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// GEHdrCtrl.cpp : implementation file
//

#include "stdafx.h"

#include "FRHdrCtrl.h"
#include "FindReplaceDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFRHdrCtrl

CFRHdrCtrl::CFRHdrCtrl()
  : m_pGEDlg(NULL), m_state(FR_INVALID)
{
}

CFRHdrCtrl::~CFRHdrCtrl()
{
}

BEGIN_MESSAGE_MAP(CFRHdrCtrl, CHeaderCtrl)
	//{{AFX_MSG_MAP(CFRHdrCtrl)
  ON_WM_LBUTTONDOWN()
  ON_WM_LBUTTONUP()
  ON_WM_LBUTTONDBLCLK()

	ON_NOTIFY_REFLECT(HDN_ITEMCLICK, OnHeaderColumnClicked)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFRHdrCtrl message handlers

void CFRHdrCtrl::OnHeaderColumnClicked(NMHDR *pNotifyStruct, LRESULT *pLResult)
{
  HD_NOTIFY *phdn = reinterpret_cast<HD_NOTIFY *>(pNotifyStruct);
	*pLResult = 0;

  if (m_pGEDlg->GetNumberResults() == 0)
    return;

  if (phdn->iItem != 0) {
    // User wants to sort CListCtrl
    m_pGEDlg->SortRows(phdn->iItem);
    return;
  }

  // If all the entries have been changed, then ignore clicks
  if (m_state == FR_CHANGED)
    return;

  // User wants to select/unselect all
  FRState NewState = (m_state == FR_CHECKED) ? FR_UNCHECKED : FR_CHECKED;

  m_pGEDlg->SetAllSelected(NewState);

  // Update header
  HDITEM hdi = { 0 };
  hdi.mask = HDI_FORMAT | HDI_IMAGE;
  VERIFY(GetItem(0, &hdi));

  hdi.fmt = HDF_CENTER | HDF_IMAGE;
  hdi.iImage = (int)NewState;

  VERIFY(SetItem(0, &hdi));

  m_state = NewState;
}

void CFRHdrCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
  HDHITTESTINFO hdhti = { 0 };
  hdhti.pt = point;

  int iItem = HitTest(&hdhti);

  // Ignore mouse clicks on select column if already all changed
  if (iItem != -1 && (hdhti.flags & HHT_ONHEADER) && hdhti.iItem == 0 && m_state == FR_CHANGED)
    return;

  CHeaderCtrl::OnLButtonDown(nFlags, point);
}

void CFRHdrCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
  HDHITTESTINFO hdhti = { 0 };
  hdhti.pt = point;

  int iItem = HitTest(&hdhti);

  // Ignore mouse clicks on select column if already all changed
  if (iItem != -1 && (hdhti.flags & HHT_ONHEADER) && hdhti.iItem == 0 && m_state == FR_CHANGED)
    return;

  CHeaderCtrl::OnLButtonUp(nFlags, point);
}

void CFRHdrCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
  HDHITTESTINFO hdhti = { 0 };
  hdhti.pt = point;

  int iItem = HitTest(&hdhti);

  // Ignore mouse clicks on select column if already all changed
  if (iItem != -1 && (hdhti.flags & HHT_ONHEADER) && hdhti.iItem == 0 && m_state == FR_CHANGED)
    return;

  CHeaderCtrl::OnLButtonDblClk(nFlags, point);
}

void CFRHdrCtrl::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
  lpMeasureItemStruct->itemHeight = Fonts::GetInstance()->CalcHeight(false) + 4;

  if (lpMeasureItemStruct->itemHeight < 20)
    lpMeasureItemStruct->itemHeight = 20;

  // Remove LVS_OWNERDRAWFIXED style to apply default DrawItem
  ModifyStyle(LVS_OWNERDRAWFIXED, 0);
}

void CFRHdrCtrl::UpdateRowHeight()
{
  // We need to change WINDOWPOS to trigger MeasureItem 
  // http://www.codeproject.com/Articles/1401/Changing-Row-Height-in-an-owner-drawn-Control
  CRect rc = { 0 };
  GetWindowRect(&rc);

  WINDOWPOS wp = { 0 };
  wp.hwnd = m_hWnd;
  wp.cx = rc.Width();
  wp.cy = rc.Height();
  wp.flags = SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER;

  // Add LVS_OWNERDRAWFIXED style for generating MeasureItem event
  ModifyStyle(0, LVS_OWNERDRAWFIXED);

  SendMessage(WM_WINDOWPOSCHANGED, 0, (LPARAM)&wp);
}

LRESULT CFRHdrCtrl::OnSetFont(WPARAM, LPARAM)
{
  LRESULT res = Default();
  UpdateRowHeight();
  return res;
}

void CFRHdrCtrl::DrawItem(LPDRAWITEMSTRUCT)
{
  // DrawItem must be overridden for LVS_OWNERDRAWFIXED style lists
}

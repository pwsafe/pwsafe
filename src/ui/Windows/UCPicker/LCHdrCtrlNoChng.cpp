
// LCHdrCtrlNoChng.cpp : implementation file
//

#include "StdAfx.h"

#include "LCHdrCtrlNoChng.h"

// CLCHdrCtrlNoChng

IMPLEMENT_DYNAMIC(CLCHdrCtrlNoChng, CHeaderCtrl)

CLCHdrCtrlNoChng::CLCHdrCtrlNoChng()
  : m_bStopChange(false)
{
}

CLCHdrCtrlNoChng::~CLCHdrCtrlNoChng()
{
}

BEGIN_MESSAGE_MAP(CLCHdrCtrlNoChng, CHeaderCtrl)
  //{{AFX_MSG_MAP(CLCHdrCtrlNoChng)
  ON_WM_SETCURSOR()
  ON_WM_LBUTTONDBLCLK()
  ON_NOTIFY_REFLECT(HDN_BEGINTRACK, OnBeginTrack)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

// CLCHdrCtrlNoChng message handlers

BOOL CLCHdrCtrlNoChng::OnSetCursor(CWnd * /*pWnd*/, UINT /*nHitTest*/, UINT /*message*/)
{
  // Disable by returning TRUE and not calling base class
  //  return CHeaderCtrl::OnSetCursor(pWnd, nHitTest, message);
  return TRUE;
}

void CLCHdrCtrlNoChng::OnLButtonDblClk(UINT nFlags, CPoint point)
{
  // Disable by not calling base case
  //  CHeaderCtrl::OnLButtonDblClk(nFlags, point);

  if (m_bStopChange)
    return;

  HDHITTESTINFO hdhti;
  std::memset(&hdhti, 0, sizeof(HDHITTESTINFO));

  hdhti.pt = point;

  int index = HitTest(&hdhti);
  if (index == -1)
    return;

  if (hdhti.flags & HHT_ONDIVIDER) {
    if (std::find(m_viFixedColumns.begin(), m_viFixedColumns.end(), index) == m_viFixedColumns.end())
      CHeaderCtrl::OnLButtonDblClk(nFlags, point);
  }
}

void CLCHdrCtrlNoChng::OnBeginTrack(NMHDR *pNHHDR, LRESULT *pLResult)
{
  // Disable by setting *pLResult to TRUE

  *pLResult = FALSE;

  NMHEADER *pHDR = reinterpret_cast<NMHEADER *>(pNHHDR);
  if (m_bStopChange || std::find(m_viFixedColumns.begin(), m_viFixedColumns.end(), pHDR->iItem) != m_viFixedColumns.end()) {
    *pLResult = TRUE;
  }
}

void CLCHdrCtrlNoChng::OnItemChanging(NMHDR *pNHHDR, LRESULT *pLResult)
{
  // Disable by setting *pLResult to TRUE
  *pLResult = FALSE;

  NMHEADER *pHDR = reinterpret_cast<NMHEADER *>(pNHHDR);
  if (m_bStopChange || std::find(m_viFixedColumns.begin(), m_viFixedColumns.end(), pHDR->iItem) != m_viFixedColumns.end()) {
    *pLResult = TRUE;
  }
}

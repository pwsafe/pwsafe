/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

// CPWHdrCtrlNoChng

class CPWHdrCtrlNoChng : public CHeaderCtrl
{
  DECLARE_DYNAMIC(CPWHdrCtrlNoChng)

public:
  CPWHdrCtrlNoChng();
  virtual ~CPWHdrCtrlNoChng();

  void SetStopChangeFlag(bool bStopChange) {m_bStopChange = bStopChange;}
  bool GetStopChangeFlag() {return m_bStopChange;}

protected:
  //{{AFX_MSG(CPWHdrCtrlNoChng)
  afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
  afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
  afx_msg void OnBeginTrack(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg void OnItemChanging(NMHDR *pNotifyStruct, LRESULT *pLResult);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  bool m_bStopChange;
};

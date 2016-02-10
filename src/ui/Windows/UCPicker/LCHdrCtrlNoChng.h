
#pragma once

// CLCHdrCtrlNoChng

#include <vector>

class CLCHdrCtrlNoChng : public CHeaderCtrl
{
  DECLARE_DYNAMIC(CLCHdrCtrlNoChng)

public:
  CLCHdrCtrlNoChng();
  virtual ~CLCHdrCtrlNoChng();

  void SetStopChangeFlag(bool bStopChange) {m_bStopChange = bStopChange;}
  void SetNoChangeColumn(int i) { m_viFixedColumns.push_back(i); };

protected:
  //{{AFX_MSG(CLCHdrCtrlNoChng)
  afx_msg BOOL OnSetCursor(CWnd *pWnd, UINT nHitTest, UINT message);
  afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
  afx_msg void OnBeginTrack(NMHDR *pNotifyStruct, LRESULT *pLResult);
  afx_msg void OnItemChanging(NMHDR *pNotifyStruct, LRESULT *pLResult);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  bool m_bStopChange;
  std::vector<int> m_viFixedColumns;
};

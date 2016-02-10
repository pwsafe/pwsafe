
#pragma once

class CNListCtrl : public CListCtrl
{
public:
  CNListCtrl()
    : m_isortColumn(-1), m_LastSearchColumn(-1), m_LastSearchRow(-1) {}

  void SetSortColumn(int isortColumn) { m_isortColumn = isortColumn; }

protected:
  // Keyboard search
  CString m_LastSearchString;
  CTime  m_LastSearchTime;
  int  m_isortColumn, m_LastSearchColumn, m_LastSearchRow;

  afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);

  DECLARE_MESSAGE_MAP()
};
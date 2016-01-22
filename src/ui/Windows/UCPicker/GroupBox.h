
#pragma once

class CGroupBox : public CButton
{
public:
  CGroupBox();
  virtual ~CGroupBox();

protected:

  virtual void PreSubclassWindow();

  //{{AFX_MSG(CGroupBox)
  afx_msg void OnPaint();
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  CBrush m_brLine;
  CPen m_pnLine;
  CRect m_Box_Rect;
};
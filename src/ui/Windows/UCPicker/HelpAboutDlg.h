#pragma once

#include "afxwin.h"

#include "..\VirtualKeyboard\VKBButton.h"
#include "RichEditCtrlExtn.h"

#include "resource.h"


class CHelpAboutDlg : public CDialog
{
public:
  CHelpAboutDlg(CWnd *pParent /*=NULL*/, CFont *pFont);
  ~CHelpAboutDlg();

  enum { IDD = IDD_HELPABOUT };

  CVKBButton m_btnUnsupported, m_btnReserved, m_btnOK;
  CFont m_fntDialogButtons;

protected:
  virtual BOOL OnInitDialog();
  virtual void DoDataExchange(CDataExchange* pDX);

  // Generated message map functions
  //{{AFX_MSG(CHelpAboutDlg)
  afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
  afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

private:
  CFont *m_pFont;
  CBrush m_pBkBrush;

  CRichEditCtrlExtn m_RECEx;

  wchar_t m_wcReserved[2], m_wcUnsupported[2];
};


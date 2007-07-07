#pragma once

#include "resource.h"
#include "ColumnChooserLC.h"
#include "LVHdrCtrl.h"

// CColumnChooserDlg dialog

class CColumnChooserDlg : public CDialog
{
  DECLARE_DYNAMIC(CColumnChooserDlg)

private:
    //using CDialog::Create

public:
  CColumnChooserDlg(CWnd* pParent = NULL);   // standard constructor
  virtual ~CColumnChooserDlg();
  BOOL Create(UINT nID, CWnd *parent);
  void SetLVHdrCtrlPtr(CLVHdrCtrl *pLVHdrCtrl) {m_pLVHdrCtrl = pLVHdrCtrl;}

  // Dialog Data
  //{{AFX_DATA(CColumnChooserDlg)
  enum { IDD = IDD_COLUMNCHOOSER };
  CColumnChooserLC m_ccListCtrl;
  //}}AFX_DATA

protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual void PostNcDestroy();
  BOOL OnInitDialog();

  //{{AFX_DATA(CColumnChooserDlg)
  afx_msg void OnDestroy();
  afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
  afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
  afx_msg void OnStyleChanged(int nStyleType, LPSTYLESTRUCT lpStyleStruct);
  //}}AFX_DATA

  DECLARE_MESSAGE_MAP()

public:
  CLVHdrCtrl *m_pLVHdrCtrl;
  void *m_pDbx;
};

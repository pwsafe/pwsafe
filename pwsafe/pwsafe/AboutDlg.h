#pragma once

/// \file AboutDlg.h
//-----------------------------------------------------------------------------

#include "afxwin.h"
#include "resource.h"

class CAboutDlg
#if defined(POCKET_PC)
   : public CPwsPopupDialog
#else
   : public CDialog
#endif
{
public:
#if defined(POCKET_PC)
  typedef CPwsPopupDialog	super;
#else
  typedef CDialog			super;
#endif

  CAboutDlg(CWnd* pParent = NULL);

    // Dialog Data
  //{{AFX_DATA(CAddDlg)
  enum { IDD = IDD_ABOUTBOX };
  //}}AFX_DATA
  CString m_appversion;

protected:
  virtual void DoDataExchange(CDataExchange* pDX)    // DDX/DDV support
  {
    super::DoDataExchange(pDX);
  }

protected:
  virtual BOOL OnInitDialog();
  // Generated message map functions
  //{{AFX_MSG(CAboutDlg)
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()
};

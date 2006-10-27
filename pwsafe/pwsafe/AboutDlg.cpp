/// \file AboutDlg.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"

#include "AboutDlg.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CAboutDlg::CAboutDlg(CWnd* pParent)
   : super(CAboutDlg::IDD, pParent)
{
}

BEGIN_MESSAGE_MAP(CAboutDlg, super)
END_MESSAGE_MAP()

BOOL
CAboutDlg::OnInitDialog()
{
  super::OnInitDialog();
  GetDlgItem(IDC_APPVERSION)->SetWindowText(m_appversion);

  return TRUE;
}

/// \file RemindSaveDlg.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "PasswordSafe.h"
#include "corelib/PwsPlatform.h"
#include "ThisMfcApp.h"

#if defined(POCKET_PC)
  #include "pocketpc/resource.h"
#else
  #include "resource.h"
#endif

#include "RemindSaveDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//-----------------------------------------------------------------------------
CRemindSaveDlg::CRemindSaveDlg(CWnd* pParent)
   : CDialog(CRemindSaveDlg::IDD, pParent)
{
   m_dontask = app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("dontasksaveminimize"), FALSE);
}


void CRemindSaveDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Check(pDX, IDC_CLEARCHECK, m_dontask);
}


BEGIN_MESSAGE_MAP(CRemindSaveDlg, CDialog)
END_MESSAGE_MAP()


void CRemindSaveDlg::OnCancel() 
{
   UpdateData(TRUE);
   app.WriteProfileInt(_T(PWS_REG_OPTIONS), _T("dontasksaveminimize"), m_dontask);
   CDialog::OnCancel();
}


void CRemindSaveDlg::OnOK() 
{
   UpdateData(TRUE);
   app.WriteProfileInt(_T(PWS_REG_OPTIONS), _T("dontasksaveminimize"), m_dontask);
   CDialog::OnOK();
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

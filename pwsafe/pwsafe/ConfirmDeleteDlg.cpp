// ConfirmDeleteDlg.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "PasswordSafe.h"

#include "ThisMfcApp.h"
#include "ConfirmDeleteDlg.h"
#include "PwsPlatform.h"

#if defined(POCKET_PC)
  #include "pocketpc/PocketPC.h"
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//-----------------------------------------------------------------------------
CConfirmDeleteDlg::CConfirmDeleteDlg(CWnd* pParent)
   : super(CConfirmDeleteDlg::IDD, pParent)
{
   m_dontaskquestion =
      app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("deletequestion"), FALSE);
}


void CConfirmDeleteDlg::DoDataExchange(CDataExchange* pDX)
{
   super::DoDataExchange(pDX);
   DDX_Check(pDX, IDC_CLEARCHECK, m_dontaskquestion);
}


BEGIN_MESSAGE_MAP(CConfirmDeleteDlg, super)
END_MESSAGE_MAP()


void
CConfirmDeleteDlg::OnCancel() 
{
   app.m_pMainWnd = NULL;
   super::OnCancel();
}


void
CConfirmDeleteDlg::OnOK() 
{
   UpdateData(TRUE);
   app.WriteProfileInt(_T(PWS_REG_OPTIONS), _T("deletequestion"), m_dontaskquestion);
   app.m_pMainWnd = NULL;
   super::OnOK();
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

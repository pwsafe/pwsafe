// ConfirmDeleteDlg.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "PasswordSafe.h"

#include "ThisMfcApp.h"
#include "ConfirmDeleteDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//-----------------------------------------------------------------------------
CConfirmDeleteDlg::CConfirmDeleteDlg(CWnd* pParent)
   : CDialog(CConfirmDeleteDlg::IDD, pParent)
{
   m_dontaskquestion =
      app.GetProfileInt("", "deletequestion", FALSE);
}


void CConfirmDeleteDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Check(pDX, IDC_CLEARCHECK, m_dontaskquestion);
}


BEGIN_MESSAGE_MAP(CConfirmDeleteDlg, CDialog)
END_MESSAGE_MAP()


void
CConfirmDeleteDlg::OnCancel() 
{
   app.m_pMainWnd = NULL;
   CDialog::OnCancel();
}


void
CConfirmDeleteDlg::OnOK() 
{
   UpdateData(TRUE);
   app.WriteProfileInt("", "deletequestion", m_dontaskquestion);
   app.m_pMainWnd = NULL;
   CDialog::OnOK();
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

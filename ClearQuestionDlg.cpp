/// \file ClearQuestionDlg.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "PasswordSafe.h"
#include "ClearQuestionDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//-----------------------------------------------------------------------------
CClearQuestionDlg::CClearQuestionDlg(CWnd* pParent)
   : CDialog(CClearQuestionDlg::IDD, pParent)
{
   m_dontaskquestion =
      app.GetProfileInt("", "dontaskquestion", 0);
}


void CClearQuestionDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Check(pDX, IDC_CLEARCHECK, m_dontaskquestion);
}


BEGIN_MESSAGE_MAP(CClearQuestionDlg, CDialog)
END_MESSAGE_MAP()


void
CClearQuestionDlg::OnCancel() 
{
   app.m_pMainWnd = NULL;
   CDialog::OnCancel();
}

void
CClearQuestionDlg::OnOK() 
{
   UpdateData(TRUE);

   app.WriteProfileInt("", "dontaskquestion", m_dontaskquestion);
   app.m_pMainWnd = NULL;
   CDialog::OnOK();
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

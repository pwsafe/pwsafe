/// \file ClearQuestionDlg.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "PasswordSafe.h"

#include "ThisMfcApp.h"
#include "ClearQuestionDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//-----------------------------------------------------------------------------
CClearQuestionDlg::CClearQuestionDlg(CWnd* pParent)
   : super(CClearQuestionDlg::IDD, pParent)
{
   m_dontaskquestion =
      app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("dontaskquestion"), 0);
}


void CClearQuestionDlg::DoDataExchange(CDataExchange* pDX)
{
   super::DoDataExchange(pDX);
   DDX_Check(pDX, IDC_CLEARCHECK, m_dontaskquestion);
}


BEGIN_MESSAGE_MAP(CClearQuestionDlg, super)
END_MESSAGE_MAP()


void
CClearQuestionDlg::OnCancel() 
{
   app.m_pMainWnd = NULL;
   super::OnCancel();
}

void
CClearQuestionDlg::OnOK() 
{
   UpdateData(TRUE);

   app.WriteProfileInt(_T(PWS_REG_OPTIONS), _T("dontaskquestion"), m_dontaskquestion);
   app.m_pMainWnd = NULL;
   super::OnOK();
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

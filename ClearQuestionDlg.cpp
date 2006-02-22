/// \file ClearQuestionDlg.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "corelib/PWSprefs.h"

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
  m_dontaskquestion =PWSprefs::GetInstance()->
    GetPref(PWSprefs::BoolPrefs::DontAskQuestion);
}


void CClearQuestionDlg::DoDataExchange(CDataExchange* pDX)
{
  BOOL B_dontaskquestion = m_dontaskquestion ? TRUE : FALSE;

  super::DoDataExchange(pDX);
  DDX_Check(pDX, IDC_CLEARCHECK, B_dontaskquestion);
  m_dontaskquestion = B_dontaskquestion == TRUE;
}


BEGIN_MESSAGE_MAP(CClearQuestionDlg, super)
END_MESSAGE_MAP()


void
CClearQuestionDlg::OnCancel() 
{
   super::OnCancel();
}

void
CClearQuestionDlg::OnOK() 
{
   UpdateData(TRUE);

   PWSprefs::GetInstance()->
     SetPref(PWSprefs::BoolPrefs::DontAskQuestion, m_dontaskquestion);
   super::OnOK();
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

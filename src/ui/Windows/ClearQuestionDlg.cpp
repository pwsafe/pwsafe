/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file ClearQuestionDlg.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"

#include "ThisMfcApp.h"
#include "ClearQuestionDlg.h"

#include "core/PWSprefs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//-----------------------------------------------------------------------------
CClearQuestionDlg::CClearQuestionDlg(CWnd* pParent)
  : CPWDialog(CClearQuestionDlg::IDD, pParent)
{
  m_dontaskquestion = PWSprefs::GetInstance()->
    GetPref(PWSprefs::DontAskQuestion);
}

void CClearQuestionDlg::DoDataExchange(CDataExchange* pDX)
{
  BOOL B_dontaskquestion = m_dontaskquestion ? TRUE : FALSE;

  CPWDialog::DoDataExchange(pDX);
  DDX_Check(pDX, IDC_CLEARCHECK, B_dontaskquestion);
  m_dontaskquestion = B_dontaskquestion == TRUE;
}

BEGIN_MESSAGE_MAP(CClearQuestionDlg, CPWDialog)
END_MESSAGE_MAP()

void CClearQuestionDlg::OnCancel() 
{
  CPWDialog::OnCancel();
}

void CClearQuestionDlg::OnOK() 
{
  UpdateData(TRUE);

  PWSprefs::GetInstance()->
    SetPref(PWSprefs::DontAskQuestion, m_dontaskquestion);
  CPWDialog::OnOK();
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

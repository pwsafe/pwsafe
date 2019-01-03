/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// ConfirmDeleteDlg.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"

#include "ThisMfcApp.h"
#include "fonts.h"

#include "ConfirmDeleteDlg.h"
#include "core/PwsPlatform.h"
#include "core/PWSprefs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//-----------------------------------------------------------------------------
CConfirmDeleteDlg::CConfirmDeleteDlg(CWnd* pParent, const int numchildren,
     const StringX sxGroup, const StringX sxTitle, const StringX sxUser)
  : CPWDialog(CConfirmDeleteDlg::IDD, pParent),
  m_numchildren(numchildren), m_sxGroup(sxGroup), m_sxTitle(sxTitle), m_sxUser(sxUser)
{
  m_dontaskquestion = PWSprefs::GetInstance()->
    GetPref(PWSprefs::DeleteQuestion);
}

void CConfirmDeleteDlg::DoDataExchange(CDataExchange* pDX)
{
  BOOL B_dontaskquestion = m_dontaskquestion ? TRUE : FALSE;

  CPWDialog::DoDataExchange(pDX);

  DDX_Check(pDX, IDC_CLEARCHECK, B_dontaskquestion);

  m_dontaskquestion = B_dontaskquestion == TRUE;
}

BEGIN_MESSAGE_MAP(CConfirmDeleteDlg, CPWDialog)
END_MESSAGE_MAP()

BOOL CConfirmDeleteDlg::OnInitDialog()
{

  CPWDialog::OnInitDialog();

  CString cs_text;
  if (m_numchildren > 0) {
    // Group delete
    if (m_numchildren == 1)
      cs_text.LoadString(IDS_NUMCHILD);
    else
      cs_text.Format(IDS_NUMCHILDREN, m_numchildren);

    // Disable/hide entry's info as a group
    GetDlgItem(IDC_ENTRY)->EnableWindow(FALSE);
    GetDlgItem(IDC_ENTRY)->ShowWindow(SW_HIDE);

    // Tell them number of entries in this group & its sub-groups
    GetDlgItem(IDC_DELETECHILDREN)->EnableWindow(TRUE);
    GetDlgItem(IDC_DELETECHILDREN)->SetWindowText(cs_text);

    // Disable/hide checkbox because we *always* ask for groups
    GetDlgItem(IDC_CLEARCHECK)->EnableWindow(FALSE);
    GetDlgItem(IDC_CLEARCHECK)->ShowWindow(SW_HIDE);
  } else {
    StringX sxEntry;
    sxEntry = L"\xab" + m_sxGroup + L"\xbb " +
              L"\xab" + m_sxTitle + L"\xbb " +
              L"\xab" + m_sxUser  + L"\xbb";
    GetDlgItem(IDC_ENTRY)->SetWindowText(sxEntry.c_str());

    // Get Add/Edit font
    CFont *pFont = Fonts::GetInstance()->GetAddEditFont();
    GetDlgItem(IDC_ENTRY)->SetFont(pFont);

    // Disable/hide children info - n/a for a single entry
    GetDlgItem(IDC_DELETECHILDREN)->EnableWindow(FALSE);
    GetDlgItem(IDC_DELETECHILDREN)->ShowWindow(SW_HIDE);

    // Allow user to select not to be asked again
    GetDlgItem(IDC_CLEARCHECK)->EnableWindow(TRUE);
  }

  cs_text.LoadString((m_numchildren > 0) ? IDS_DELGRP : IDS_DELENT);
  GetDlgItem(IDC_DELITEM)->SetWindowText(cs_text);

  return TRUE;  // return TRUE unless you set the focus to a control
}

void CConfirmDeleteDlg::OnCancel() 
{
  CPWDialog::OnCancel();
}

void CConfirmDeleteDlg::OnOK() 
{
  if (m_numchildren == 0) {
    UpdateData(TRUE);
    PWSprefs::GetInstance()->
      SetPref(PWSprefs::DeleteQuestion, m_dontaskquestion);
  }
  CPWDialog::OnOK();
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

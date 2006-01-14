/// \file AddDlg.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "PasswordSafe.h"

#include "ThisMfcApp.h"
#include "DboxMain.h"
#include "AddDlg.h"
#include "PwFont.h"
#include "OptionsPasswordPolicy.h"
#include "corelib/PWCharPool.h"
#include "corelib/PWSprefs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//-----------------------------------------------------------------------------
CAddDlg::CAddDlg(CWnd* pParent)
  : CDialog(CAddDlg::IDD, pParent), m_password(_T("")), m_notes(_T("")),
    m_username(_T("")), m_title(_T("")), m_group(_T("")),
    m_URL(_T("")), m_autotype(_T(""))
{
  m_isExpanded = true; // XXX TBD - get from preference
}


BOOL CAddDlg::OnInitDialog() 
{
  CDialog::OnInitDialog();
 
  SetPasswordFont(GetDlgItem(IDC_PASSWORD));
  if (m_isExpanded) {
    // set text to "<Less" instead of default "More>"
    m_moreLessBtn.SetWindowText(_T("<Less"));
  }
    
  return TRUE;
}


void CAddDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_PASSWORD, (CString&)m_password);
    DDX_Text(pDX, IDC_NOTES, (CString&)m_notes);
    DDX_Text(pDX, IDC_USERNAME, (CString&)m_username);
    DDX_Text(pDX, IDC_TITLE, (CString&)m_title);

    if(!pDX->m_bSaveAndValidate) {
        // We are initializing the dialog.  Populate the groups combo box.
        CComboBox comboGroup;
        comboGroup.Attach(GetDlgItem(IDC_GROUP)->GetSafeHwnd());
        // For some reason, MFC calls us twice when initializing.
        // Populate the combo box only once.
        if(0 == comboGroup.GetCount()) {
            CStringArray aryGroups;
            app.m_core.GetUniqueGroups(aryGroups);
            for(int igrp=0; igrp<aryGroups.GetSize(); igrp++) {
                comboGroup.AddString((LPCTSTR)aryGroups[igrp]);
            }
        }
        comboGroup.Detach();
    }
    DDX_CBString(pDX, IDC_GROUP, (CString&)m_group);
    DDX_Text(pDX, IDC_URL, (CString&)m_URL);
    DDX_Text(pDX, IDC_AUTOTYPE, (CString&)m_autotype);
    DDX_Control(pDX, IDC_MORE, m_moreLessBtn);
}


BEGIN_MESSAGE_MAP(CAddDlg, CDialog)
   ON_BN_CLICKED(ID_HELP, OnHelp)
   ON_BN_CLICKED(IDC_RANDOM, OnRandom)
   ON_BN_CLICKED(IDC_MORE, OnBnClickedMore)
END_MESSAGE_MAP()


void
CAddDlg::OnCancel() 
{
  CDialog::OnCancel();
}


void
CAddDlg::OnOK() 
{
  UpdateData(TRUE);

  //Check that data is valid
  if (m_title.IsEmpty()) {
    AfxMessageBox(_T("This entry must have a title."));
    ((CEdit*)GetDlgItem(IDC_TITLE))->SetFocus();
    return;
  }
  if (m_password.IsEmpty()) {
    AfxMessageBox(_T("This entry must have a password."));
    ((CEdit*)GetDlgItem(IDC_PASSWORD))->SetFocus();
    return;
  }
  if (!m_group.IsEmpty() && m_group[0] == '.') {
    AfxMessageBox(_T("A dot is invalid as the first character of the Group field."));
    ((CEdit*)GetDlgItem(IDC_GROUP))->SetFocus();
    return;
  }
  //End check

  DboxMain* pParent = (DboxMain*) GetParent();
  ASSERT(pParent != NULL);

  if (pParent->Find(m_group, m_title, m_username) != NULL) {
    CMyString temp =
      _T("An item with Group \"") + m_group
      + _T("\", Title \"") + m_title 
      + _T("\" and User Name \"") + m_username
      + _T("\" already exists.");
    AfxMessageBox(temp);
    ((CEdit*)GetDlgItem(IDC_TITLE))->SetSel(MAKEWORD(-1, 0));
    ((CEdit*)GetDlgItem(IDC_TITLE))->SetFocus();
  } else {
    CDialog::OnOK();
  }
}


void CAddDlg::OnHelp() 
{
#if defined(POCKET_PC)
  CreateProcess( _T("PegHelp.exe"), _T("pws_ce_help.html#adddata"), NULL, NULL, FALSE, 0, NULL, NULL, NULL, NULL );
#else
  //WinHelp(0x2008E, HELP_CONTEXT);
  ::HtmlHelp(NULL,
             "pwsafe.chm::/html/entering_pwd.html",
             HH_DISPLAY_TOPIC, 0);
#endif
}


void CAddDlg::OnRandom() 
{
  DboxMain* pParent = (DboxMain*)GetParent();
  ASSERT(pParent != NULL);

  UpdateData(TRUE);
  if (pParent->MakeRandomPassword(this, m_password))
    UpdateData(FALSE);
}
//-----------------------------------------------------------------------------

void CAddDlg::OnBnClickedMore()
{
  // XXX Add resize logic
  if (m_isExpanded) {
    // from more to less
    m_moreLessBtn.SetWindowText(_T("<Less"));
  } else {
    // from less to more
    m_moreLessBtn.SetWindowText(_T("More>"));
  }
  m_isExpanded = !m_isExpanded;
}


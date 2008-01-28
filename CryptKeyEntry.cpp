/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file CryptKeyEntry.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"

#include "ThisMfcApp.h"
#if defined(POCKET_PC)
#include "pocketpc/resource.h"
#include "pocketpc/PocketPC.h"
#else
#include "resource.h"
#include "resource3.h"  // String resources
#endif
#include "corelib/util.h"

#include "CryptKeyEntry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//-----------------------------------------------------------------------------
CCryptKeyEntry::CCryptKeyEntry(CWnd* pParent)
  : CPWDialog(CCryptKeyEntry::IDD, pParent),
  m_cryptkey1(_T("")), m_cryptkey2(_T(""))
{
}

void CCryptKeyEntry::DoDataExchange(CDataExchange* pDX)
{
  CPWDialog::DoDataExchange(pDX);
  DDX_Text(pDX, IDC_CRYPTKEY1, (CString &)m_cryptkey1);
  DDX_Text(pDX, IDC_CRYPTKEY2, (CString &)m_cryptkey2);
}

BEGIN_MESSAGE_MAP(CCryptKeyEntry, CPWDialog)
  ON_BN_CLICKED(ID_HELP, OnHelp)
#if defined(POCKET_PC)
  ON_EN_SETFOCUS(IDC_CRYPTKEY1, OnPasskeySetfocus)
  ON_EN_SETFOCUS(IDC_CRYPTKEY2, OnPasskeySetfocus)
  ON_EN_KILLFOCUS(IDC_CRYPTKEY1, OnPasskeyKillfocus)
  ON_EN_KILLFOCUS(IDC_CRYPTKEY2, OnPasskeyKillfocus)
#endif
END_MESSAGE_MAP()

void
CCryptKeyEntry::OnCancel() 
{
  CPWDialog::OnCancel();
}

void
CCryptKeyEntry::OnOK()
{
  UpdateData(TRUE);

  if (m_cryptkey1 != m_cryptkey2) {
    AfxMessageBox(IDS_ENTRIESDONOTMATCH);
    ((CEdit*)GetDlgItem(IDC_CRYPTKEY2))->SetFocus();
    return;
  }
  if (m_cryptkey1.IsEmpty()) {
    AfxMessageBox(IDS_ENTERKEYANDVERIFY);
    ((CEdit*)GetDlgItem(IDC_CRYPTKEY1))->SetFocus();
    return;
  }

  CPWDialog::OnOK();
}

void
CCryptKeyEntry::OnHelp() 
{
#if defined(POCKET_PC)
  CreateProcess( _T("PegHelp.exe"), _T("pws_ce_help.html#comboentry"), NULL, NULL, FALSE, 0, NULL, NULL, NULL, NULL );
#else
  CString cs_HelpTopic;
  cs_HelpTopic = app.GetHelpFileName() + _T("::/html/create_new_db.html");
  HtmlHelp(DWORD_PTR((LPCTSTR)cs_HelpTopic), HH_DISPLAY_TOPIC);
#endif
}

#if defined(POCKET_PC)
/************************************************************************/
/* Restore the state of word completion when the password field loses   */
/* focus.                                                               */
/************************************************************************/
void CCryptKeyEntry::OnPasskeyKillfocus()
{
  EnableWordCompletion(m_hWnd);
}

/************************************************************************/
/* When the password field is activated, pull up the SIP and disable    */
/* word completion.                                                     */
/************************************************************************/
void CCryptKeyEntry::OnPasskeySetfocus()
{
  DisableWordCompletion(m_hWnd);
}
#endif

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

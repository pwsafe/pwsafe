/*
* Copyright (c) 2003-2013 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file CryptKeyEntry.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"

#include "ThisMfcApp.h"
#include "GeneralMsgBox.h"

#include "CryptKeyEntry.h"

#include "core/util.h"

#include "resource.h"
#include "resource3.h"  // String resources

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//-----------------------------------------------------------------------------
CCryptKeyEntry::CCryptKeyEntry(CWnd* pParent)
  : CPWDialog(CCryptKeyEntry::IDD, pParent),
  m_cryptkey1(L""), m_cryptkey2(L"")
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
END_MESSAGE_MAP()

void CCryptKeyEntry::OnCancel() 
{
  CPWDialog::OnCancel();
}

void CCryptKeyEntry::OnOK()
{
  UpdateData(TRUE);
  CGeneralMsgBox gmb;

  if (m_cryptkey1 != m_cryptkey2) {
    gmb.AfxMessageBox(IDS_ENTRIESDONOTMATCH);
    ((CEdit*)GetDlgItem(IDC_CRYPTKEY2))->SetFocus();
    return;
  }
  if (m_cryptkey1.IsEmpty()) {
    gmb.AfxMessageBox(IDS_ENTERKEYANDVERIFY);
    ((CEdit*)GetDlgItem(IDC_CRYPTKEY1))->SetFocus();
    return;
  }

  CPWDialog::OnOK();
}

void CCryptKeyEntry::OnHelp() 
{
  CString cs_HelpTopic;
  cs_HelpTopic = app.GetHelpFileName() + L"::/html/create_new_db.html";
  HtmlHelp(DWORD_PTR((LPCWSTR)cs_HelpTopic), HH_DISPLAY_TOPIC);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

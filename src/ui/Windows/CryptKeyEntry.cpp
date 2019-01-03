/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
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
CCryptKeyEntry::CCryptKeyEntry(bool isEncrypt, CWnd* pParent)
  : CDialog(CCryptKeyEntry::IDD, pParent),
    m_cryptkey1(L""), m_cryptkey2(L""), m_encrypt(isEncrypt)
{
}

BOOL CCryptKeyEntry::OnInitDialog()
{
  CDialog::OnInitDialog();
  
  GetDlgItem(IDC_VERIFY)->ShowWindow(m_encrypt);
  GetDlgItem(IDC_CRYPTKEY2)->ShowWindow(m_encrypt);
  
  return TRUE;  // return TRUE unless you set the focus to a control
}

void CCryptKeyEntry::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_Text(pDX, IDC_CRYPTKEY1, (CString &)m_cryptkey1);
  DDX_Text(pDX, IDC_CRYPTKEY2, (CString &)m_cryptkey2);
}

BEGIN_MESSAGE_MAP(CCryptKeyEntry, CDialog)
  ON_BN_CLICKED(ID_HELP, OnHelp)
END_MESSAGE_MAP()

void CCryptKeyEntry::OnCancel() 
{
  CDialog::OnCancel();
}

void CCryptKeyEntry::OnOK()
{
  UpdateData(TRUE);
  CGeneralMsgBox gmb;

  if (m_encrypt && (m_cryptkey1 != m_cryptkey2)) {
    gmb.AfxMessageBox(IDS_ENTRIESDONOTMATCH);
    ((CEdit*)GetDlgItem(IDC_CRYPTKEY2))->SetFocus();
    return;
  }
  if (m_cryptkey1.IsEmpty()) {
    gmb.AfxMessageBox(IDS_ENTERKEYANDVERIFY);
    ((CEdit*)GetDlgItem(IDC_CRYPTKEY1))->SetFocus();
    return;
  }

  CDialog::OnOK();
}

void CCryptKeyEntry::OnHelp() 
{
  //  ShowHelp(L"::/html/create_new_db.html");
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

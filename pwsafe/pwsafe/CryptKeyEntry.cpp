/*
 * Copyright (c) 2003-2006 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
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
   : super(CCryptKeyEntry::IDD, pParent)
{
   m_cryptkey1	= _T("");
   m_cryptkey2	= _T("");
}


void CCryptKeyEntry::DoDataExchange(CDataExchange* pDX)
{
   super::DoDataExchange(pDX);
   DDX_Text(pDX, IDC_CRYPTKEY1, (CString &)m_cryptkey1);
   DDX_Text(pDX, IDC_CRYPTKEY2, (CString &)m_cryptkey2);
}


BEGIN_MESSAGE_MAP(CCryptKeyEntry, super)
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
   super::OnCancel();
}


void
CCryptKeyEntry::OnOK()
{
   UpdateData(TRUE);

   if (m_cryptkey1 != m_cryptkey2)
   {
      AfxMessageBox(IDS_ENTRIESDONOTMATCH);
      ((CEdit*)GetDlgItem(IDC_CRYPTKEY2))->SetFocus();
      return;
   }
   if (m_cryptkey1.IsEmpty())
   {
      AfxMessageBox(IDS_ENTERKEYANDVERIFY);
      ((CEdit*)GetDlgItem(IDC_CRYPTKEY1))->SetFocus();
      return;
   }

   super::OnOK();
}


void
CCryptKeyEntry::OnHelp() 
{
#if defined(POCKET_PC)
	CreateProcess( _T("PegHelp.exe"), _T("pws_ce_help.html#comboentry"), NULL, NULL, FALSE, 0, NULL, NULL, NULL, NULL );
#else
    HtmlHelp(DWORD_PTR(_T("pwsafe.chm::/create_new_db.html")), HH_DISPLAY_TOPIC);
#endif
}


#if defined(POCKET_PC)
/************************************************************************/
/* Restore the state of word completion when the password field loses   */
/* focus.                                                               */
/************************************************************************/
void CCryptKeyEntry::OnPasskeyKillfocus()
{
	EnableWordCompletion( m_hWnd );
}


/************************************************************************/
/* When the password field is activated, pull up the SIP and disable    */
/* word completion.                                                     */
/************************************************************************/
void CCryptKeyEntry::OnPasskeySetfocus()
{
	DisableWordCompletion( m_hWnd );
}
#endif

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

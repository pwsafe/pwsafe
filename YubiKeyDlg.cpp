/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/// \file YubiKeyDlg.cpp
//

#include "stdafx.h"
#include "YubiKeyDlg.h"


// YubiKeyDlg dialog

IMPLEMENT_DYNAMIC(CYubiKeyDlg, CPWDialog)

CYubiKeyDlg::CYubiKeyDlg(CWnd* pParent /*=NULL*/)
	: CPWDialog(CYubiKeyDlg::IDD, pParent)
   , m_YKpubID(_T(""))
   , m_YKstatus(_T(""))
{

}

CYubiKeyDlg::~CYubiKeyDlg()
{
}

void CYubiKeyDlg::DoDataExchange(CDataExchange* pDX)
{
   CPWDialog::DoDataExchange(pDX);
   DDX_Text(pDX, IDC_YK_PUBID, m_YKpubID);
   DDV_MaxChars(pDX, m_YKpubID, 44);
   DDX_Text(pDX, IDC_YK_STATUS, m_YKstatus);
}


BEGIN_MESSAGE_MAP(CYubiKeyDlg, CPWDialog)
   ON_BN_CLICKED(IDOK, &CYubiKeyDlg::OnOk)
END_MESSAGE_MAP()


// CYubiKeyDlg message handlers

void CYubiKeyDlg::OnOk()
{
  // validate OTP, blablabla
  UpdateData(TRUE);
  CPWDialog::OnOK();
}

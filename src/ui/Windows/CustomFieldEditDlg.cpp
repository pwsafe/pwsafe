/*
* Copyright (c) 2003-2026 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "StdAfx.h"
#include "CustomFieldEditDlg.h"

//IMPLEMENT_DYNAMIC(CCustomFieldEditDlg, CDialog)

CCustomFieldEditDlg::CCustomFieldEditDlg(CWnd* pParent)
  : CPWDialog(IDD_CUSTOMFIELD_EDIT, pParent),
  m_sensitive(FALSE)
{
}

void CCustomFieldEditDlg::DoDataExchange(CDataExchange* pDX)
{
  CPWDialog::DoDataExchange(pDX);
  DDX_Text(pDX, IDC_CF_NAME, m_name);
  DDX_Text(pDX, IDC_CF_VALUE, m_value);
  DDX_Check(pDX, IDC_CF_SENSITIVE, m_sensitive);
}

BOOL CCustomFieldEditDlg::OnInitDialog()
{
  CPWDialog::OnInitDialog();
  return TRUE;
}

BEGIN_MESSAGE_MAP(CCustomFieldEditDlg, CPWDialog)
END_MESSAGE_MAP()

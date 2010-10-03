/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// AddDescription.cpp : implementation file
//

#include "stdafx.h"
#include "AddDescription.h"

// CAddDescription dialog

IMPLEMENT_DYNAMIC(CAddDescription, CPWDialog)

CAddDescription::CAddDescription(CWnd* pParent, const CString filename,
                                 const CString description)
  : CPWDialog(CAddDescription::IDD, pParent), m_filename(filename),
  m_description(description)
{
}

void CAddDescription::DoDataExchange(CDataExchange* pDX)
{
  CPWDialog::DoDataExchange(pDX);

  DDX_Text(pDX, IDC_STATIC_ATTACHMENTNAME, m_filename);
  DDX_Text(pDX, IDC_OPTIONALDESCRIPTION, m_description);
}

BEGIN_MESSAGE_MAP(CAddDescription, CPWDialog)
END_MESSAGE_MAP()

// CAddDescription message handlers

BOOL CAddDescription::OnInitDialog()
{
  CPWDialog::OnInitDialog();

  CEdit *pWnd = (CEdit *)GetDlgItem(IDC_OPTIONALDESCRIPTION);
  pWnd->SetFocus();
  pWnd->SetSel(0, -1);

  return FALSE;
}

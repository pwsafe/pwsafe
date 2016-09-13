/*
* Copyright (c) 2003-2016 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// Reuse_Attachment.cpp : implementation file
//

#include "stdafx.h"

#include "Reuse_Attachment.h"

#include "afxdialogex.h"


// CReuse_Attachment dialog

IMPLEMENT_DYNAMIC(CReuse_Attachment, CPWDialog)

CReuse_Attachment::CReuse_Attachment(CWnd* pParent /*=NULL*/)
	: CPWDialog(IDD_REUSEATTACHMENT, pParent)
{
}

CReuse_Attachment::~CReuse_Attachment()
{
}

void CReuse_Attachment::DoDataExchange(CDataExchange* pDX)
{
  CPWDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CReuse_Attachment, CPWDialog)
END_MESSAGE_MAP()

// CReuse_Attachment message handlers

BOOL CReuse_Attachment::OnInitDialog()
{
  CPWDialog::OnInitDialog();

  // Set window texts
  GetDlgItem(IDC_IMPORTFILENAME)->SetWindowText(m_sxImportFN.c_str());
  GetDlgItem(IDC_IMPORTFILEPATH)->SetWindowText(m_sxImportFP.c_str());
  GetDlgItem(IDC_EXISTINGFILETITLE)->SetWindowText(m_sxExistingFT.c_str());
  GetDlgItem(IDC_EXISTINGFILENAME)->SetWindowText(m_sxExistingFN.c_str());
  GetDlgItem(IDC_EXISTINGFILEPATH)->SetWindowText(m_sxExistingFP.c_str());

  return TRUE;
}

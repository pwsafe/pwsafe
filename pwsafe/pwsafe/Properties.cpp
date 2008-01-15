/*
 * Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */
// Properties.cpp : implementation file
//

#include "stdafx.h"
#include "Properties.h"

// CProperties dialog

IMPLEMENT_DYNAMIC(CProperties, CPWDialog)

CProperties::CProperties(CWnd* pParent /*=NULL*/)
	: CPWDialog(CProperties::IDD, pParent)
{
}

CProperties::~CProperties()
{
}

void CProperties::DoDataExchange(CDataExchange* pDX)
{
	CPWDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CProperties, CPWDialog)
	ON_BN_CLICKED(IDOK, &CProperties::OnOK)
END_MESSAGE_MAP()


// CProperties message handlers

void CProperties::OnOK()
{
	CPWDialog::OnOK();
}

BOOL CProperties::OnInitDialog()
{
	GetDlgItem(IDC_DATABASENAME)->SetWindowText(m_database);
	GetDlgItem(IDC_DATABASEFORMAT)->SetWindowText(m_databaseformat);
	GetDlgItem(IDC_NUMGROUPS)->SetWindowText(m_numgroups);
	GetDlgItem(IDC_NUMENTRIES)->SetWindowText(m_numentries);
	GetDlgItem(IDC_SAVEDON)->SetWindowText(m_whenlastsaved);
	GetDlgItem(IDC_SAVEDBY)->SetWindowText(m_wholastsaved);
	GetDlgItem(IDC_SAVEDAPP)->SetWindowText(m_whatlastsaved);
	GetDlgItem(IDC_FILEUUID)->SetWindowText(m_file_uuid);
	GetDlgItem(IDC_UNKNOWNFIELDS)->SetWindowText(m_unknownfields);

	return TRUE;
}

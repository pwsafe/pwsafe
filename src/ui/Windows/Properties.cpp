/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// Properties.cpp : implementation file
//

#include "stdafx.h"
#include "Properties.h"
#include "corelib/StringXStream.h" // for ostringstreamT

// CProperties dialog

IMPLEMENT_DYNAMIC(CProperties, CPWDialog)

CProperties::CProperties(const st_DBProperties &st_dbp, CWnd* pParent /*=NULL*/)
: CPWDialog(CProperties::IDD, pParent)
{
  m_database = st_dbp.database.c_str();
  m_databaseformat = st_dbp.databaseformat.c_str();
  m_numgroups = st_dbp.numgroups.c_str();
  m_numentries = st_dbp.numentries.c_str();
  m_whenlastsaved = st_dbp.whenlastsaved.c_str();
  m_wholastsaved = st_dbp.wholastsaved.c_str();
  m_whatlastsaved = st_dbp.whatlastsaved.c_str();
  m_file_uuid = st_dbp.file_uuid.c_str();
  m_unknownfields = st_dbp.unknownfields.c_str();
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

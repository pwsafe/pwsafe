/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
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

BEGIN_MESSAGE_MAP(CProperties, CPWDialog)
  ON_BN_CLICKED(IDOK, &CPWDialog::OnOK)
END_MESSAGE_MAP()

BOOL CProperties::OnInitDialog()
{
  GetDlgItem(IDC_DATABASENAME)->SetWindowText(m_dbp.database.c_str());
  GetDlgItem(IDC_DATABASEFORMAT)->SetWindowText(m_dbp.databaseformat.c_str());
  GetDlgItem(IDC_NUMGROUPS)->SetWindowText(m_dbp.numgroups.c_str());
  GetDlgItem(IDC_NUMENTRIES)->SetWindowText(m_dbp.numentries.c_str());
  GetDlgItem(IDC_SAVEDON)->SetWindowText(m_dbp.whenlastsaved.c_str());
  GetDlgItem(IDC_SAVEDBY)->SetWindowText(m_dbp.wholastsaved.c_str());
  GetDlgItem(IDC_SAVEDAPP)->SetWindowText(m_dbp.whatlastsaved.c_str());
  GetDlgItem(IDC_FILEUUID)->SetWindowText(m_dbp.file_uuid.c_str());
  GetDlgItem(IDC_UNKNOWNFIELDS)->SetWindowText(m_dbp.unknownfields.c_str());

  return TRUE;
}

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
#include "InputBox.h"

// CProperties dialog

IMPLEMENT_DYNAMIC(CProperties, CPWDialog)

CProperties::CProperties(st_DBProperties *pdbp, const bool bReadonly, CWnd *pParent)
  : CPWDialog(CProperties::IDD, pParent), m_bReadOnly(bReadonly), m_pdbp(pdbp),
  m_bChanged(false)
{
  m_old_name = m_pdbp->db_name;
  m_old_description = m_pdbp->db_description;
}

CProperties::~CProperties()
{
}

void CProperties::DoDataExchange(CDataExchange* pDX)
{
  CPWDialog::DoDataExchange(pDX);

  DDX_Control(pDX, IDC_DATABASE_NAME, m_stc_name);
  DDX_Control(pDX, IDC_DATABASE_DESCRIPTION, m_stc_description);
}

BEGIN_MESSAGE_MAP(CProperties, CPWDialog)
  ON_BN_CLICKED(IDOK, OnOK)
  ON_BN_CLICKED(IDC_CHANGE_NAME, OnEditName)
  ON_BN_CLICKED(IDC_CHANGE_DESCRIPTION, OnEditDescription)
END_MESSAGE_MAP()

BOOL CProperties::OnInitDialog()
{
  CPWDialog::OnInitDialog();

  GetDlgItem(IDC_DATABASENAME)->SetWindowText(m_pdbp->database.c_str());
  GetDlgItem(IDC_DATABASEFORMAT)->SetWindowText(m_pdbp->databaseformat.c_str());
  GetDlgItem(IDC_NUMGROUPS)->SetWindowText(m_pdbp->numgroups.c_str());
  GetDlgItem(IDC_NUMENTRIES)->SetWindowText(m_pdbp->numentries.c_str());
  GetDlgItem(IDC_SAVEDON)->SetWindowText(m_pdbp->whenlastsaved.c_str());
  GetDlgItem(IDC_SAVEDBY)->SetWindowText(m_pdbp->wholastsaved.c_str());
  GetDlgItem(IDC_SAVEDAPP)->SetWindowText(m_pdbp->whatlastsaved.c_str());
  GetDlgItem(IDC_FILEUUID)->SetWindowText(m_pdbp->file_uuid.c_str());
  GetDlgItem(IDC_UNKNOWNFIELDS)->SetWindowText(m_pdbp->unknownfields.c_str());

  CString csDBName = m_pdbp->db_name.empty() ? L"N/A" : m_pdbp->db_name.c_str();
  if (csDBName.GetLength() > 32)
    csDBName = csDBName.Left(30) + L"...";

  CString csDBDescription = m_pdbp->db_description.empty() ? L"N/A" : m_pdbp->db_description.c_str();
  if (csDBDescription.GetLength() > 64)
    csDBDescription = csDBDescription.Left(60) + L"...";

  m_stc_name.SetWindowText(csDBName);
  m_stc_description.SetWindowText(csDBDescription);

  // Disable Cancel button - either because R-O or nothing yet altered
  GetDlgItem(IDCANCEL)->EnableWindow(FALSE);

  if (m_bReadOnly) {
    // Hide the Cancel button and centre the OK button
    GetDlgItem(IDCANCEL)->ShowWindow(SW_HIDE);

    CRect dlgRect, btnRect;
    GetClientRect(&dlgRect);

    GetDlgItem(IDOK)->GetWindowRect(&btnRect);
    ScreenToClient(&btnRect);

    int ytop = btnRect.top;
    int xleft = (dlgRect.Width() / 2) - (btnRect.Width() / 2);
    GetDlgItem(IDOK)->SetWindowPos(NULL, xleft, ytop, NULL, NULL, SWP_NOSIZE | SWP_NOZORDER);
  }

  return TRUE;
}

void CProperties::SetChangedStatus() 
{
  if (!m_bReadOnly) {
    if (m_pdbp->db_name != m_old_name ||
        m_pdbp->db_description != m_old_description)
      m_bChanged = true;
  }
}

void CProperties::OnEditName()
{
  CInputBox input(IDS_ENTER_NEW_DB_NAME, m_pdbp->db_name.c_str(), 0, m_bReadOnly, this);
  INT_PTR rc = input.DoModal();
  if (!m_bReadOnly && rc == IDOK) {
    m_pdbp->db_name = input.GetText();
    CString csDBName = m_pdbp->db_name.c_str();
    if (csDBName.GetLength() > 32)
      csDBName = csDBName.Left(30) + L" ...";
    m_stc_name.SetWindowText(csDBName);
    GetDlgItem(IDCANCEL)->EnableWindow(TRUE);
    SetChangedStatus();
  }
}

void CProperties::OnEditDescription()
{
  CInputBox input(IDS_ENTER_NEW_DB_DESCRIPTION, m_pdbp->db_description.c_str(), 0, m_bReadOnly, this);
  INT_PTR rc = input.DoModal();
  if (!m_bReadOnly && rc == IDOK) {
    m_pdbp->db_description = input.GetText();
    CString csDBDescription = m_pdbp->db_description.c_str();
    if (csDBDescription.GetLength() > 64)
      csDBDescription = csDBDescription.Left(60) + L" ...";
    m_stc_description.SetWindowText(csDBDescription);
    GetDlgItem(IDCANCEL)->EnableWindow(TRUE);
    SetChangedStatus();
  }
}

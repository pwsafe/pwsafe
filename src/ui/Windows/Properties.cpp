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
#include "NumUtilities.h"
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
  m_num_att = st_dbp.num_att;
  m_totalunc = st_dbp.totalunc;
  m_totalcmp = st_dbp.totalcmp;
  m_largestunc = st_dbp.largestunc;
  m_largestcmp = st_dbp.largestcmp;
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

  CString tmp1, tmp2, tmp3, tmp4, tmp5;;
  tmp1.Format(_T("%d"), m_num_att);
  if (m_num_att == 0) {
    tmp2 = tmp3 = tmp4 = L"n/a";
  } else {
    double dblVar;
    wchar_t wcbuffer[40];
    dblVar = (m_totalunc + 1023.0) / 1024.0;
    PWSNumUtil::DoubleToLocalizedString(::GetThreadLocale(), dblVar, 1,
                                        wcbuffer, sizeof(wcbuffer) / sizeof(wchar_t));
    wcscat_s(wcbuffer, sizeof(wcbuffer) / sizeof(wchar_t), L" KB");
    tmp2 = wcbuffer;

    dblVar = (m_totalcmp + 1023.0) / 1024.0;;
    PWSNumUtil::DoubleToLocalizedString(::GetThreadLocale(), dblVar, 1,
                                        wcbuffer, sizeof(wcbuffer) / sizeof(wchar_t));
    wcscat_s(wcbuffer, sizeof(wcbuffer) / sizeof(wchar_t), L" KB");
    tmp3 = wcbuffer;

    dblVar = (m_largestunc + 1023.0) / 1024.0;;
    PWSNumUtil::DoubleToLocalizedString(::GetThreadLocale(), dblVar, 1,
                                        wcbuffer, sizeof(wcbuffer) / sizeof(wchar_t));
    wcscat_s(wcbuffer, sizeof(wcbuffer) / sizeof(wchar_t), L" KB");
    tmp4 = wcbuffer;

    dblVar = (m_largestcmp + 1023.0) / 1024.0;;
    PWSNumUtil::DoubleToLocalizedString(::GetThreadLocale(), dblVar, 1,
                                        wcbuffer, sizeof(wcbuffer) / sizeof(wchar_t));
    wcscat_s(wcbuffer, sizeof(wcbuffer) / sizeof(wchar_t), L" KB");
    tmp4 += L" / ";
    tmp4 += wcbuffer;
  }

  GetDlgItem(IDC_NUMATTACHMENTS)->SetWindowText(tmp1);
  GetDlgItem(IDC_TOTALUNCMP)->SetWindowText(tmp2);
  GetDlgItem(IDC_TOTALCMP)->SetWindowText(tmp3);
  GetDlgItem(IDC_ATTLARGEST)->SetWindowText(tmp4);

  return TRUE;
}

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
#include "corelib/StringXStream.h" // for ostringstreamT

// CProperties dialog

IMPLEMENT_DYNAMIC(CProperties, CPWDialog)

CProperties::CProperties(const PWScore &core, CWnd* pParent /*=NULL*/)
: CPWDialog(CProperties::IDD, pParent)
{
  m_database = CString(core.GetCurFile().c_str());

  m_databaseformat.Format(_T("%d.%02d"),
                          core.GetHeader().m_nCurrentMajorVersion,
                          core.GetHeader().m_nCurrentMinorVersion);

  std::vector<stringT> aryGroups;
  core.GetUniqueGroups(aryGroups);
  m_numgroups.Format(_T("%d"), aryGroups.size());

  m_numentries.Format(_T("%d"), core.GetNumEntries());

  time_t twls = core.GetHeader().m_whenlastsaved;
  if (twls == 0) {
    m_whenlastsaved.LoadString(IDS_UNKNOWN);
    m_whenlastsaved.Trim();
  } else {
    m_whenlastsaved =
      CString(PWSUtil::ConvertToDateTimeString(twls,
                                               TMC_EXPORT_IMPORT).c_str());
  }

  if (core.GetHeader().m_lastsavedby.empty() &&
      core.GetHeader().m_lastsavedon.empty()) {
    m_wholastsaved.LoadString(IDS_UNKNOWN);
    m_whenlastsaved.Trim();
  } else {
    CString user = core.GetHeader().m_lastsavedby.empty() ?
      _T("?") : core.GetHeader().m_lastsavedby.c_str();
    CString host = core.GetHeader().m_lastsavedon.empty() ?
      _T("?") : core.GetHeader().m_lastsavedon.c_str();
    m_wholastsaved.Format(_T("%s on %s"), user, host);
  }

  CString wls = core.GetHeader().m_whatlastsaved.c_str();
  if (wls.IsEmpty()) {
    m_whatlastsaved.LoadString(IDS_UNKNOWN);
    m_whenlastsaved.Trim();
  } else
    m_whatlastsaved = wls;

  uuid_array_t file_uuid_array, ref_uuid_array;
  memset(ref_uuid_array, 0x00, sizeof(ref_uuid_array));
  core.GetFileUUID(file_uuid_array);

  if (memcmp(file_uuid_array, ref_uuid_array, sizeof(file_uuid_array)) == 0)
    m_file_uuid = _T("N/A");
  else {
    ostringstreamT os;
    CUUIDGen huuid(file_uuid_array, true); // true for canonical format
    os << huuid;
    m_file_uuid = os.str().c_str();
  }

  int num = core.GetNumRecordsWithUnknownFields();
  if (num != 0 || core.HasHeaderUnknownFields()) {
    const CString cs_Yes(MAKEINTRESOURCE(IDS_YES));
    const CString cs_No(MAKEINTRESOURCE(IDS_NO));
    const CString cs_HdrYesNo = core.HasHeaderUnknownFields() ? cs_Yes : cs_No;

    m_unknownfields.Format(IDS_UNKNOWNFIELDS, cs_HdrYesNo);
    if (num == 0)
      m_unknownfields += cs_No + _T(")");
    else {
      wls.Format(_T("%d"), num);
      m_unknownfields += wls + _T(")");
    }
  } else {
    m_unknownfields.LoadString(IDS_NONE);
  }
  time_t mpwset = core.GetHeader().m_whenmpwset;
  short mpwexpint = core.GetHeader().m_mpwinterval;
  if (mpwset == (time_t)0) {
    m_whenmpwset = _T("Unknown");
    m_mpwexp.Format(_T("%d days"), mpwexpint);
  } else {
    time_t mpwexp = mpwset + (mpwexpint * 24 * 60 * 60);
    m_whenmpwset = PWSUtil::ConvertToDateTimeString(mpwset, TMC_EXPORT_IMPORT).c_str();
    m_mpwexp = PWSUtil::ConvertToDateTimeString(mpwexp, TMC_EXPORT_IMPORT).c_str();
  }
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
  GetDlgItem(IDC_MPWSET)->SetWindowText(m_whenmpwset);
  GetDlgItem(IDC_MPWEXP)->SetWindowText(m_mpwexp);
  GetDlgItem(IDC_UNKNOWNFIELDS)->SetWindowText(m_unknownfields);

  return TRUE;
}

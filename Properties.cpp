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

CProperties::CProperties(const PWScore &core, CWnd* pParent /*=NULL*/)
: CPWDialog(CProperties::IDD, pParent)
{
  m_database = CString(core.GetCurFile());

  m_databaseformat.Format(_T("%d.%02d"),
                          core.GetHeader().m_nCurrentMajorVersion,
                          core.GetHeader().m_nCurrentMinorVersion);

  CStringArray aryGroups;
  core.GetUniqueGroups(aryGroups);
  m_numgroups.Format(_T("%d"), aryGroups.GetSize());

  m_numentries.Format(_T("%d"), core.GetNumEntries());

  time_t twls = core.GetHeader().m_whenlastsaved;
  if (twls == 0) {
    m_whenlastsaved.LoadString(IDS_UNKNOWN);
    m_whenlastsaved.Trim();
  } else {
    m_whenlastsaved =
      CString(PWSUtil::ConvertToDateTimeString(twls, TMC_EXPORT_IMPORT));
  }

  if (core.GetHeader().m_lastsavedby.IsEmpty() &&
      core.GetHeader().m_lastsavedon.IsEmpty()) {
    m_wholastsaved.LoadString(IDS_UNKNOWN);
    m_whenlastsaved.Trim();
  } else {
    CString user = core.GetHeader().m_lastsavedby.IsEmpty() ?
      _T("?") : core.GetHeader().m_lastsavedby;
    CString host = core.GetHeader().m_lastsavedon.IsEmpty() ?
      _T("?") : core.GetHeader().m_lastsavedon;
    m_wholastsaved.Format(_T("%s on %s"), user, host);
  }

  CString wls = core.GetHeader().m_whatlastsaved;
  if (wls.IsEmpty()) {
    m_whatlastsaved.LoadString(IDS_UNKNOWN);
    m_whenlastsaved.Trim();
  } else
    m_whatlastsaved = wls;

  uuid_array_t file_uuid_array, ref_uuid_array;
  memset(ref_uuid_array, 0x00, sizeof(ref_uuid_array));
  core.GetFileUUID(file_uuid_array);

  if (memcmp(file_uuid_array, ref_uuid_array, sizeof(file_uuid_array)) == 0)
    wls = _T("N/A");
  else
    wls.Format(_T("%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x"),
               file_uuid_array[0],  file_uuid_array[1],  file_uuid_array[2],  file_uuid_array[3],
               file_uuid_array[4],  file_uuid_array[5],  file_uuid_array[6],  file_uuid_array[7],
               file_uuid_array[8],  file_uuid_array[9],  file_uuid_array[10], file_uuid_array[11],
               file_uuid_array[12], file_uuid_array[13], file_uuid_array[14], file_uuid_array[15]);
  m_file_uuid = wls;

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
    m_whenmpwset = CString(PWSUtil::ConvertToDateTimeString(mpwset, TMC_EXPORT_IMPORT));
    m_mpwexp = CString(PWSUtil::ConvertToDateTimeString(mpwexp, TMC_EXPORT_IMPORT));
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

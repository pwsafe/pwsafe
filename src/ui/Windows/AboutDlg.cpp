/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file AboutDlg.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"

#include "AboutDlg.h"
// following 2 includes to allow querying
// the current version details
#include "PasswordSafe.h"
#include "ThisMfcApp.h"
#include "GeneralMsgBox.h"
#include "RichEditCtrlExtn.h"
#include "PWSversion.h"
#include "DumpSelect.h"
#include "DboxMain.h"

#include "core/UTF8Conv.h"
#include "core/SysInfo.h"

#include "resource.h"
#include "resource3.h"

// for fetching xml file:
#include <afxinet.h>

using pws_os::CUUID;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CAboutDlg::CAboutDlg(CWnd* pParent)
  : CPWDialog(CAboutDlg::IDD, pParent), 
  m_nMajor(0), m_nMinor(0), m_nBuild(0)
{
  CString verstat;
  // Following since text in quotes is not to be translated
  verstat.Format(IDS_LATEST_VERSION, L"[check_version]");
  m_newVerStatus = verstat;
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
  CPWDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_NEWVER_STATUS, m_RECExNewVerStatus);
  DDX_Control(pDX, IDC_VISIT_WEBSITE, m_RECExWebSite);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CPWDialog)
  ON_BN_CLICKED(IDC_TAKETESTDUMP, OnTakeTestdump)
END_MESSAGE_MAP()

BOOL CAboutDlg::OnInitDialog()
{
  CPWDialog::OnInitDialog();

  const PWSversion *pPWSver = PWSversion::GetInstance();
  m_nMajor = pPWSver->GetMajor();
  m_nMinor = pPWSver->GetMinor();
  m_nBuild = pPWSver->GetBuild();
  CString Revision = pPWSver->GetRevision();
  CString SpecialBuild = pPWSver->GetSpecialBuild();
  
  m_appversion = pPWSver->GetAppVersion();

  const CString cs2go = SysInfo::IsUnderPw2go() ? L"2go " : L" ";
  if (m_nBuild == 0) { // hide build # if zero (formal release)
    m_appversion.Format(L"%s%sV%d.%02d%s (%s)", AfxGetAppName(),
                        static_cast<LPCWSTR>(cs2go),
                        m_nMajor, m_nMinor,
                        static_cast<LPCWSTR>(SpecialBuild),
                        static_cast<LPCWSTR>(Revision));
  } else {
    m_appversion.Format(L"%s%sV%d.%02d.%02d%s (%s)", AfxGetAppName(),
                        static_cast<LPCWSTR>(cs2go),
                        m_nMajor, m_nMinor, m_nBuild,
                        static_cast<LPCWSTR>(SpecialBuild),
                        static_cast<LPCWSTR>(Revision));
  }

#if _WIN64
  // Only add platform information for 64-bit build
  m_appversion += L" 64-bit";
#endif

#ifdef _DEBUG
  m_appversion += L" [D]";
#endif

  CString builtOnPrefix;
  GetDlgItem(IDC_APPBUILTON)->GetWindowText(builtOnPrefix);
  const CString builtOn =  builtOnPrefix + pPWSver->GetBuiltOn();

  GetDlgItem(IDC_APPVERSION)->SetWindowText(m_appversion);
  GetDlgItem(IDC_APPBUILTON)->SetWindowText(builtOn);
  GetDlgItem(IDC_APPCOPYRIGHT)->SetWindowText(app.GetCopyrightString());

  m_RECExNewVerStatus.SetWindowText(m_newVerStatus);
  m_RECExNewVerStatus.RegisterOnLink(OnCheckVersion, (LPARAM)this);

  m_RECExWebSite.SetWindowText(CString(MAKEINTRESOURCE(IDS_VISIT_WEBSITE)));

  if (app.PermitTestdump()) {
    GetDlgItem(IDC_TAKETESTDUMP)->ShowWindow(SW_SHOW);
    GetDlgItem(IDC_TAKETESTDUMP)->EnableWindow();
  }

  return TRUE;  // return TRUE unless you set the focus to a control
}

bool CAboutDlg::OnCheckVersion(const CString &URL, const CString & /* lpszFName */, LPARAM instance)
{
  if (URL == L"[check_version]") {
    CAboutDlg *self = (CAboutDlg *)instance;
    self->CheckNewVer();
    return true;
  } else
    return false;
}

void CAboutDlg::CheckNewVer()
{
  // Get the latest.xml file from our site, compare to version,
  // and notify the user
  // First, make sure database is closed: Sensitive data with an
  // open socket makes me uneasy...

  if (GetMainDlg()->GetNumEntries() != 0) {
    CGeneralMsgBox gmb;
    const CString cs_txt(MAKEINTRESOURCE(IDS_CLOSE_B4_CHECK));
    const CString cs_title(MAKEINTRESOURCE(IDS_CONFIRM_CLOSE));
    INT_PTR rc = gmb.MessageBox(cs_txt, cs_title,
                        (MB_OKCANCEL | MB_ICONQUESTION));
    if (rc == IDCANCEL)
      return; // no hard feelings
    // Close database, prompt for save if changed
    GetMainDlg()->SendMessage(WM_COMMAND, ID_MENUITEM_CLOSE);
    // User could have cancelled save, need to check if really closed:
    if (GetMainDlg()->GetNumEntries() != 0)
      return;
  }
  GetMainDlg()->UpdateWindow(); // show user that we closed database
  ASSERT(GetMainDlg()->GetNumEntries() == 0);
  // safe to open external connection
  m_newVerStatus.LoadString(IDS_TRYING2CONTACT_SERVER);
  UpdateData(FALSE);
  std::wstring latest;
  wchar_t *html_redfont = L"<b><font color=\"red\">";
  wchar_t *html_greenfont = L"<b><font color=\"green\">";
  wchar_t *html_endfont = L"</font></b>";
  switch (CheckLatestVersion(latest)) {
    case CheckVersion::CheckStatus::CANT_CONNECT:
      m_newVerStatus.Format(IDS_CANT_CONTACT_SERVER, html_redfont, html_endfont);
      break;
    case CheckVersion::CheckStatus::UP2DATE:
      m_newVerStatus.Format(IDS_UP2DATE, html_greenfont, html_endfont);
      break;
    case CheckVersion::CheckStatus::NEWER_AVAILABLE:
      {
      CGeneralMsgBox gmb;
      CString newer;
      newer.Format(SysInfo::IsUnderU3() ? IDS_NEWER_AVAILABLE_U3 : IDS_NEWER_AVAILABLE,
                   static_cast<LPCWSTR>(m_appversion),
                   static_cast<LPCWSTR>(latest.c_str()));
      m_newVerStatus.Format(IDS_NEWER_AVAILABLE_SHORT, html_redfont, html_endfont);
      gmb.MessageBox(newer, CString(MAKEINTRESOURCE(IDS_NEWER_CAPTION)), MB_ICONEXCLAMATION);
      break;
      }
    case CheckVersion::CheckStatus::CANT_READ:
      m_newVerStatus.Format(IDS_CANT_READ_VERINFO, html_redfont, html_endfont);
      break;
    default:
      break;
  }

  m_RECExNewVerStatus.SetFont(GetFont());
  m_RECExNewVerStatus.SetWindowText(m_newVerStatus);
  m_RECExNewVerStatus.Invalidate();
  UpdateData(FALSE);

  GetDlgItem(IDOK)->SetFocus();
}

CheckVersion::CheckStatus CAboutDlg::CheckLatestVersion(std::wstring &latest)
{
  CInternetSession session(L"PasswordSafe Version Check");
  CStdioFile *fh;

  // Put up hourglass...this might take a while
  CWaitCursor waitCursor;

  try {
    // Loading the file as binary since we're treating it as UTF-8
    fh = session.OpenURL(L"https://pwsafe.org/latest.xml",
                         1, (INTERNET_FLAG_TRANSFER_BINARY | INTERNET_FLAG_RELOAD));
  } catch (CInternetException *) {
    // throw;
    return CheckVersion::CheckStatus::CANT_CONNECT;
  }

  ASSERT(fh != NULL);
  CString latest_xml;
  unsigned char buff[BUFSIZ + 1];
  StringX chunk;
  UINT nRead;
  CUTF8Conv conv;

  while ((nRead = fh->Read(buff, BUFSIZ)) != 0) {
    buff[nRead] = '\0';
    // change to widechar representation
    if (!conv.FromUTF8(buff, nRead, chunk)) {
      fh->Close();
      delete fh;
      session.Close();
      return CheckVersion::CheckStatus::CANT_READ;
    } else
      latest_xml += chunk.c_str();
  }

  fh->Close();
  delete fh;

  session.Close();
  waitCursor.Restore(); // restore normal cursor

  CheckVersion vh(m_nMajor, m_nMinor, m_nBuild);
  return vh.CheckLatestVersion(LPCWSTR(latest_xml), latest);
}

// Take a test dump
void CAboutDlg::OnTakeTestdump()
{
  CDumpSelect dmpslct;
  dmpslct.DoModal();
}

/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
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
#include "RichEditCtrlExtn.h"

// for fetching & reading version xml file:
#include <afxinet.h>
#include "corelib/UTF8Conv.h"
#include "corelib/tinyxml/tinyxml.h"
#include "corelib/SysInfo.h"

#include "resource.h"
#include "resource3.h"

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
  verstat.Format(IDS_LATEST_VERSION, _T("[check_version]"));
  m_newVerStatus = verstat;
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
  CPWDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_NEWVER_STATUS, m_RECExNewVerStatus);
  DDX_Control(pDX, IDC_VISIT_WEBSITE, m_RECExWebSite);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CPWDialog)
END_MESSAGE_MAP()

BOOL CAboutDlg::OnInitDialog()
{
  CPWDialog::OnInitDialog();

  DWORD dwMajorMinor = app.GetFileVersionMajorMinor();
  DWORD dwBuildRevision = app.GetFileVersionBuildRevision();

  if (dwMajorMinor > 0) {
    m_nMajor = HIWORD(dwMajorMinor);
    m_nMinor = LOWORD(dwMajorMinor);
    m_nBuild = HIWORD(dwBuildRevision);
  }

  // revision is either a number or a number with '+',
  // so we need to get it from the file version string
  // which is of the form "MM, NN, BB, rev"
  CString csFileVersionString, csRevision;
  csFileVersionString = app.GetFileVersionString();
  int revIndex = csFileVersionString.ReverseFind(TCHAR(','));
  if (revIndex >= 0) {
    int len = csFileVersionString.GetLength();
    csRevision = csFileVersionString.Right(len - revIndex - 1);
    csRevision.Trim();
  }
  if (m_nBuild == 0) { // hide build # if zero (formal release)
    m_appversion.Format(_T("%s V%d.%02dF (%s)"), AfxGetAppName(), 
                        m_nMajor, m_nMinor, csRevision);
  } else {
    m_appversion.Format(_T("%s V%d.%02d.%02dF (%s)"), AfxGetAppName(), 
                        m_nMajor, m_nMinor, m_nBuild, csRevision);
  }
#ifdef _DEBUG
  m_appversion += _T(" [Debug]");
#endif
  m_appcopyright = app.GetCopyrightString();

#ifdef DEMO
  m_appversion += _T(" ") + CString(MAKEINTRESOURCE(IDS_DEMO));
#endif

  GetDlgItem(IDC_APPVERSION)->SetWindowText(m_appversion);
  GetDlgItem(IDC_APPCOPYRIGHT)->SetWindowText(m_appcopyright);

  m_RECExNewVerStatus.SetWindowText(m_newVerStatus);
  m_RECExNewVerStatus.RegisterOnLink(OnCheckVersion, (LPARAM)this);

  m_RECExWebSite.SetWindowText(CString(MAKEINTRESOURCE(IDS_VISIT_WEBSITE)));

  return TRUE;
}

bool CAboutDlg::OnCheckVersion(const CString &URL, const CString & /* lpszFName */, LPARAM instance)
{
  if (URL == _T("[check_version]")) {
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
  DboxMain *pDbx = static_cast<DboxMain *>(GetParent());

  if (pDbx->GetNumEntries() != 0) {
    const CString cs_txt(MAKEINTRESOURCE(IDS_CLOSE_B4_CHECK));
    const CString cs_title(MAKEINTRESOURCE(IDS_CONFIRM_CLOSE));
    int rc = MessageBox(cs_txt, cs_title,
      (MB_ICONQUESTION | MB_OKCANCEL));
    if (rc == IDCANCEL)
      return; // no hard feelings
    // Close database, prompt for save if changed
    pDbx->SendMessage(WM_COMMAND, ID_MENUITEM_CLOSE);
    // User could have cancelled save, need to check if really closed:
    if (pDbx->GetNumEntries() != 0)
      return;
  }
  pDbx->UpdateWindow(); // show user that we closed database
  ASSERT(pDbx->GetNumEntries() == 0);
  // safe to open external connection
  m_newVerStatus.LoadString(IDS_TRYING2CONTACT_SERVER);
  UpdateData(FALSE);
  CString latest;
  switch (CheckLatestVersion(latest)) {
    case CANT_CONNECT:
      m_newVerStatus.LoadString(IDS_CANT_CONTACT_SERVER);
      break;
    case UP2DATE:
      m_newVerStatus.LoadString(IDS_UP2DATE);
      break;
    case NEWER_AVAILABLE:
    {
      CString newer;
      newer.Format(SysInfo::IsUnderU3() ? IDS_NEWER_AVAILABLE_U3 : IDS_NEWER_AVAILABLE,
                   m_appversion, latest);
      m_newVerStatus.LoadString(IDS_NEWER_AVAILABLE_SHORT);
      MessageBox(newer, CString(MAKEINTRESOURCE(IDS_NEWER_CAPTION)), MB_ICONEXCLAMATION);
      break;
    }
    case CANT_READ:
      m_newVerStatus.LoadString(IDS_CANT_READ_VERINFO);
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

static bool SafeCompare(const TCHAR *v1, const TCHAR *v2)
{
  ASSERT(v2 != NULL);
  return (v1 != NULL && CString(v1) == v2);
}

/*
* The latest version information is in
* http://passwordsafe.sourceforge.net/latest.xml
*
* And is of the form:
* <VersionInfo>
*  <Product name=PasswordSafe variant=PC major=3 minor=10 build=2 rev=1710 />
*  <Product name=PasswordSafe variant=PPc major=1 minor=9 build=2
*    rev=100 />
*  <Product name=PasswordSafe variant=U3 major=3 minor=10 build=2
*    rev=1710 />
*  <Product name=SWTPasswordSafe variant=Java major=0 minor=6
*    build=0 rev=1230 />
* </VersionInfo>
*
* Note: The "rev" is the svn commit number. Not using it (for now),
*       as I think it's too volatile.
*/

CAboutDlg::CheckStatus CAboutDlg::CheckLatestVersion(CString &latest)
{
  CInternetSession session(_T("PasswordSafe Version Check"));
  CStdioFile *fh;
  // Put up hourglass...this might take a while
  CWaitCursor waitCursor;
  try {
    // Loading the file as binary since we're treating it as UTF-8
    fh = session.OpenURL(_T("http://passwordsafe.sourceforge.net/latest.xml"),
                         1, (INTERNET_FLAG_TRANSFER_BINARY | INTERNET_FLAG_RELOAD));
  } catch (CInternetException *) {
    // throw;
    return CANT_CONNECT;
  }
  ASSERT(fh != NULL);
  CString latest_xml;
#ifndef UNICODE
  CString line;
  while (fh->ReadString(line) == TRUE)
    latest_xml += line;
#else
  unsigned char buff[BUFSIZ+1];
  CMyString chunk;
  UINT nRead;
  CUTF8Conv conv;
  while ((nRead = fh->Read(buff, BUFSIZ)) != 0) {
    buff[nRead] = '\0';
    // change to widechar representation
    if (!conv.FromUTF8(buff, nRead, chunk)) {
      fh->Close();
      delete fh;
      session.Close();
      return CANT_READ;
    } else
      latest_xml += chunk;
  }
#endif /* UNICODE */
  fh->Close();
  delete fh;
  session.Close();
  waitCursor.Restore(); // restore normal cursor
  // Parse the file we just retrieved
  TiXmlDocument doc; 
  if (doc.Parse(latest_xml) == NULL)
    return CANT_READ;
  TiXmlNode *pRoot = doc.FirstChildElement();

  if (!pRoot || !SafeCompare(pRoot->Value(), _T("VersionInfo")))
    return CANT_READ;

  TiXmlNode *pProduct = 0;
  while((pProduct = pRoot->IterateChildren(pProduct)) != NULL) {
    if (SafeCompare(pProduct->Value(), _T("Product"))) {
      TiXmlElement *pElem = pProduct->ToElement();
      if (pElem == NULL)
        return CANT_READ;
      const TCHAR *prodName = pElem->Attribute(_T("name"));
      if (SafeCompare(prodName, _T("PasswordSafe"))) {
        const TCHAR *pVariant = pElem->Attribute(_T("variant"));
        if (pVariant == NULL) continue;
        const CString variant(pVariant);
        // Determine which variant is relevant for us
        if ((SysInfo::IsUnderU3() && variant == _T("U3")) ||
          variant == _T("PC")) {
            int major(0), minor(0), build(0), revision(0);
            pElem->QueryIntAttribute(_T("major"), &major);
            pElem->QueryIntAttribute(_T("minor"), &minor);
            pElem->QueryIntAttribute(_T("build"), &build);
            pElem->QueryIntAttribute(_T("rev"), &revision);
            // Not using svn rev info - too volatile
            if ((major > m_nMajor) ||
              (major == m_nMajor && minor > m_nMinor) ||
              (major == m_nMajor && minor == m_nMinor &&
              build > m_nBuild)
              ) {
                if (build == 0) { // hide build # if zero (formal release)
                  latest.Format(_T("%s V%d.%02d (%d)"), AfxGetAppName(), 
                    major, minor, revision);
                } else {
                  latest.Format(_T("%s V%d.%02d.%02d (%d)"), AfxGetAppName(), 
                    major, minor, build, revision);
                }
                return NEWER_AVAILABLE;
            }
            return UP2DATE;
        } // handled our variant
      } // Product name == PasswordSafe
    } // Product element
  } // IterateChildren
  return CANT_READ;
}

#include "stdafx.h"
#include "ResPWSL.h"
#include "VersionInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// The one and only application object

CWinApp theApp;

using namespace std;

void ShowUsage()
{
  _tprintf(_T("Update Password Safe Resource-Only DLL Version language information\n"));
  _tprintf(_T("USAGE:\n"));
  _tprintf(_T("ResPWSL.exe list <FileName>\n\tLists the version information of a specified module\n\n"));
  _tprintf(_T("ResPWSL.exe apply <FileName> LCID\n\tCreates pwsafeLL.dll or pwsafeLL_CC.dll from the supplied file.\n"));
  _tprintf(_T("\nIt also changes the OriginalFilename and ProductName version strings to reflect the language/location."));
  _tprintf(_T("\nThe locale LCID value should be specified as \"0xnnnn\",\nas specified in \"http://www.microsoft.com/globaldev/nlsweb/default.mspx\""));
  _tprintf(_T("\nUsing a LCID value \"0xmmnn\" where mm=00 will generate the pwsafeLL.dll version,\nwhereas any other value will produce the corresponding pwsafeLL_CC.dll version."));
  _tprintf(_T("\nFor example 0x0007 produces pwsafeEN.dll and 0x0407 produces pwsafeEN_US.dll"));
}

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
  // initialize MFC and print and error on failure
  if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0)) {
    // TODO: change error code to suit your needs
    cerr << _T("Fatal Error: MFC initialization failed") << endl;
    return 2;
  }

  if (argc == 3 && _tcscmp(argv[1], _T("list")) == 0) {
    // 2nd argument is the file path
    CString strFilePath(argv[2]);

    const CVersionInfo vi(strFilePath);
    if (vi.IsValid()) {
      POSITION posTable = vi.GetStringFileInfo().GetFirstStringTablePosition();
      while (posTable) {
        const CStringTable &st = *vi.GetStringFileInfo().GetNextStringTable(posTable);

        _tprintf(_T("String table %s\n------------------------------\n"), st.GetKey());

        POSITION posString = st.GetFirstStringPosition();

        while (posString) {
          const CVersionInfoString &vistr = *st.GetNextString(posString);
          _tprintf(_T("%s=%s\n"), vistr.GetKey(), vistr.GetValue());
        }
        _tprintf(_T("------------------------------\n"));
      }
      return 0;
    } else {
      _tprintf(_T("Failed to get module version information for %s\n"), strFilePath);
      return 1;
    }
  }

  if (argc == 4 && _tcscmp(argv[1], _T("apply")) == 0) {
    // 2nd argument is the file path
    CString csInFilePath(argv[2]);
    LCID locale;

    // 3rd argument is the locale
    // Verify in correct format first - 0xnnnn

    // Length = 6
    if (_tcslen(argv[3]) != 6)
      goto exit;

    // starts with "0x"
    if (_tcsnicmp(argv[3], _T("0x"), 2) != 0)
      goto exit;

    TCHAR *szlcid;
    szlcid = _tcsninc(argv[3], 2);

    // All other characters are valid hex
    if (_tcsspnp(szlcid, _T("0123456789ABCDEFabcdef")) != NULL)
      goto exit;

    _stscanf_s(argv[3], _T("%X"), &locale);

    if (!UpdateRODLL(csInFilePath, locale))
      goto exit;

    return 0;
  }

exit:
  ShowUsage();

  return 1;
}

bool UpdateRODLL(const CString csInFilePath, const LCID locale)
{
  CString csFilename, csProductName, csOutFilePath;
  TCHAR szLang[4], szCtry[4];
  int inum;

  inum = ::GetLocaleInfo(locale, LOCALE_SISO639LANGNAME, szLang, 4);
  if (inum != 3) {
    _tprintf(_T("Invalid locale value supplied 0x%04x\n\n"), locale);
    return false;
  }

  _tcsupr_s(szLang, 3);

  TRACE("LOCALE_SISO639LANGNAME=%s\n", szLang);

  if ((int)locale < 0x0100) {
    csFilename.Format(_T("pwsafe%s"), szLang);
  } else {
    inum = ::GetLocaleInfo(locale, LOCALE_SISO3166CTRYNAME, szCtry, 4);
    if (inum != 3) {
      _tprintf(_T("Invalid locale value supplied 0x%04x\n\n"), locale);
      return false;
    }

    TRACE("LOCALE_SISO3166CTRYNAME=%s\n", szCtry);
    csFilename.Format(_T("pwsafe%s_%s"), szLang, szCtry);
  }

  inum = ::GetLocaleInfo(locale, LOCALE_SLANGUAGE, NULL, 0);
  if (inum > 0) {
    TCHAR * pszLanguage = new TCHAR[inum + 1];
    GetLocaleInfo(locale, LOCALE_SLANGUAGE, pszLanguage, inum);
    csProductName.Format(_T("Password Safe Language DLL for %s"), pszLanguage);
    delete[] pszLanguage;
  } else {
    _tprintf(_T("Unable to determine language from locale 0x%04x\n\n"), locale);
    return false;
  }

  TCHAR path_buffer[_MAX_PATH];
  TCHAR drive[_MAX_DRIVE];
  TCHAR dir[_MAX_DIR];

#if _MSC_VER >= 1400
  _tsplitpath_s((LPCTSTR)csInFilePath, drive, _MAX_DRIVE, dir, _MAX_DIR,
    NULL, 0, NULL, 0);
  _tmakepath_s(path_buffer, _MAX_PATH, drive, dir, (LPCTSTR)csFilename, _T("dll"));
#else
  _tsplitpath((LPCTSTR)csInFilePath, drive, dir, NULL, NULL);
  _tmakepath(path_buffer, drive, dir, (LPCTSTR)csFilename, _T("dll"));
#endif

  csOutFilePath = path_buffer;
  if (!CopyFile(csInFilePath, csOutFilePath, FALSE)) {
    _tprintf(_T("Unable to create file %s\n"), csOutFilePath);
    return false;
  }

  CVersionInfo vi(csOutFilePath);

  vi[_T("OriginalFilename")] = csFilename + _T(".dll");
  vi[_T("ProductName")] = csProductName;

  vi.Save();

  return true;
}

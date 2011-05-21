/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include <iostream>
#include <sstream>
#include <unistd.h>
#include <getopt.h>

#include "PWScore.h"
#include "os/file.h"
#include "core/PWSdirs.h"
#include "core/UTF8Conv.h"
#include "core/Report.h"
#include "core/XML/XMLDefs.h"

#include <termios.h>

using namespace std;

// Fwd declarations:
static void echoOff();
static void echoOn();

static int ImportText(PWScore &core, const StringX &fname);
static int ImportXML(PWScore &core, const StringX &fname);
static const char *status_text(int status);

//-----------------------------------------------------------------

static struct termios oldTermioFlags; // to restore tty echo

static void usage(char *pname)
{
  cerr << "Usage: " << pname << " safe --imp[=file] --text|--xml" << endl
       << "\t safe --exp[=file] --text|--xml" << endl;
}


struct UserArgs {
  UserArgs() : ImpExp(Unset), Format(Unknown) {}
  StringX safe, fname;
  enum {Unset, Import, Export} ImpExp;
  enum {Unknown, XML, Text} Format;
};

bool parseArgs(int argc, char *argv[], UserArgs &ua)
{
  if (argc != 4 && argc != 5)
    return false;
  CUTF8Conv conv;
  if (!conv.FromUTF8((const unsigned char *)argv[1], strlen(argv[1]),
                     ua.safe)) {
    cerr << "Could not convert filename " << argv[1] << " to StringX" << endl;
    exit(2);
  }
  while (1) {
    int option_index = 0;
    static struct option long_options[] = {
      // name, has_arg, flag, val
      {"import", optional_argument, 0, 'i'},
      {"export", optional_argument, 0, 'e'},
      {"text", no_argument, 0, 't'},
      {"xml", no_argument, 0, 'x'},
      {0, 0, 0, 0}
    };

    int c = getopt_long(argc-1, argv+1, "i::e::tx",
                        long_options, &option_index);
    if (c == -1)
      break;

    switch (c) {
    case 'i':
      if (ua.ImpExp == UserArgs::Unset)
        ua.ImpExp = UserArgs::Import;
      else
        return false;
      if (optarg) {
        if (!conv.FromUTF8((const unsigned char *)optarg, strlen(optarg),
                           ua.fname)) {
          cerr << "Could not convert filename "
               << optarg << " to StringX" << endl;
          exit(2);
        }
      }
      break;
    case 'e':
      if (ua.ImpExp == UserArgs::Unset)
        ua.ImpExp = UserArgs::Export;
      else
        return false;
      if (optarg) {
        if (!conv.FromUTF8((const unsigned char *)optarg, strlen(optarg),
                           ua.fname)) {
          cerr << "Could not convert filename "
               << optarg << " to StringX" << endl;
          exit(2);
        }
      }
      break;
    case 'x':
      if (ua.Format == UserArgs::Unknown)
        ua.Format = UserArgs::XML;
      else
        return false;
      break;
    case 't':
      if (ua.Format == UserArgs::Unknown)
        ua.Format = UserArgs::Text;
      else
        return false;
      break;

    default:
      cerr << "Unknown option: " << char(c) << endl;
      return false;
    }
    if (ua.fname.empty())
      ua.fname = (ua.Format == UserArgs::XML) ? L"file.xml" : L"file.txt";
  }
  return true;
}

int main(int argc, char *argv[])
{
  UserArgs ua;
  if (!parseArgs(argc, argv, ua)) {
    usage(argv[0]);
    return 1;
  }

  PWScore core;
  if (!pws_os::FileExists(ua.safe.c_str())) {
    cerr << argv[1] << " - file not found" << endl;
    return 2;
  }
  wstring wpk;
  cout << "Enter Password: ";
  echoOff();
  wcin >> wpk;
  echoOn();
  StringX pk(wpk.c_str());

  int status;
  status = core.CheckPasskey(ua.safe, pk);
  if (status != PWScore::SUCCESS) {
    cout << "CheckPasskey returned: " << status_text(status) << endl;
    goto done;
  }
  {
    CUTF8Conv conv;
    const char *user = getlogin() != NULL ? getlogin() : "unknown";
    StringX locker;
    if (!conv.FromUTF8((const unsigned char *)user, strlen(user), locker)) {
      cerr << "Could not convert user " << user << " to StringX" << endl;
      return 2;
    }
    stringT lk(locker.c_str());
    if (!core.LockFile(ua.safe.c_str(), lk)) {
      wcout << L"Couldn't lock file " << ua.safe
            << L": locked by " << locker << endl;
      status = -1;
      goto done;
    }
  }
  // Since we may be writing the same file we're reading,
  // it behooves us to set the Current File and use its' related
  // functions
  core.SetCurFile(ua.safe);
  status = core.ReadCurFile(pk);
  if (status != PWScore::SUCCESS) {
    cout << "ReadFile returned: " << status_text(status) << endl;
    goto done;
  }

  if (ua.ImpExp == UserArgs::Export) {
    CItemData::FieldBits all(~0L);
    int N;
    if (ua.Format == UserArgs::XML) {
      status = core.WriteXMLFile(ua.fname, all, L"", 0, 0, L' ', N);
    } else { // export text
      status = core.WritePlaintextFile(ua.fname, all, L"", 0, 0, L' ', N);
    }
  } else { // Import
    if (ua.Format == UserArgs::XML) {
      status = ImportXML(core, ua.fname);
    } else { // import text
      status = ImportText(core, ua.fname);
    }
    if (status == PWScore::SUCCESS)
      status = core.WriteCurFile();
  }
  if (status != PWScore::SUCCESS) {
    cout << "Operation returned status: " << status_text(status) << endl;
    goto done;
  }
 done:
  core.UnlockFile(ua.safe.c_str());
  return status;
}

//-----------------------------------------------------------------
static void echoOff()
{
  struct termios nflags;
  tcgetattr(fileno(stdin), &oldTermioFlags);
  nflags = oldTermioFlags;
  nflags.c_lflag &= ~ECHO;
  nflags.c_lflag |= ECHONL;

  if (tcsetattr(fileno(stdin), TCSANOW, &nflags) != 0) {
    cerr << "Couldn't turn off echo\n";
  }
}

static void echoOn()
{
  if (tcsetattr(fileno(stdin), TCSANOW, &oldTermioFlags) != 0) {
    cerr << "Couldn't restore echo\n";
  }
}

static const char *status_text(int status)
{
  switch (status) {
  case PWScore::SUCCESS: return "SUCCESS";
  case PWScore::FAILURE: return "FAILURE";
  case PWScore::CANT_OPEN_FILE: return "CANT_OPEN_FILE";
  case PWScore::USER_CANCEL: return "USER_CANCEL";
  case PWScore::WRONG_PASSWORD: return "WRONG_PASSWORD";
  case PWScore::BAD_DIGEST: return "BAD_DIGEST";
  case PWScore::UNKNOWN_VERSION: return "UNKNOWN_VERSION";
  case PWScore::NOT_SUCCESS: return "NOT_SUCCESS";
  case PWScore::ALREADY_OPEN: return "ALREADY_OPEN";
  case PWScore::INVALID_FORMAT: return "INVALID_FORMAT";
  case PWScore::USER_EXIT: return "USER_EXIT";
  case PWScore::XML_FAILED_VALIDATION: return "XML_FAILED_VALIDATION";
  case PWScore::XML_FAILED_IMPORT: return "XML_FAILED_IMPORT";
  case PWScore::LIMIT_REACHED: return "LIMIT_REACHED";
  case PWScore::UNIMPLEMENTED: return "UNIMPLEMENTED";
  case PWScore::NO_ENTRIES_EXPORTED: return "NO_ENTRIES_EXPORTED";
  default: return "UNKNOWN ?!";
  }
}

static int
ImportText(PWScore &core, const StringX &fname)
{
  int numImported(0), numSkipped(0), numPWHErrors(0), numRenamed(0);
  std::wstring strError;
  wchar_t delimiter = L' ';
  wchar_t fieldSeparator = L'\t';
  StringX ImportedPrefix;
  bool bImportPSWDsOnly = false;

  // Create report as we go
  CReport rpt;
  rpt.StartReport(L"Import_Text", core.GetCurFile().c_str());
  wstring str(L"Text file being imported: ");
  str += core.GetCurFile().c_str();
  rpt.WriteLine(str.c_str());
  rpt.WriteLine();

  Command *pcmd = NULL;
  int rc = core.ImportPlaintextFile(ImportedPrefix, fname, fieldSeparator,
                                    delimiter, bImportPSWDsOnly,
                                    strError,
                                    numImported, numSkipped,
                                    numPWHErrors, numRenamed,
                                    rpt, pcmd);
  switch (rc) {
  case PWScore::CANT_OPEN_FILE:
  case PWScore::INVALID_FORMAT:
  case PWScore::FAILURE:
    delete pcmd;
    break;
  case PWScore::SUCCESS:
  case PWScore::OK_WITH_ERRORS:
    // deliberate fallthru
  default:
    {
      rc = core.Execute(pcmd);

      rpt.WriteLine();
      wstring op(bImportPSWDsOnly ? L"Updated " : L"Imported ");
      wstring entries(numImported == 1 ? L" entry" : L" entries");
      wostringstream os;
      os << op << numImported << entries;
      rpt.WriteLine(os.str().c_str());

      if (numSkipped != 0) {
        wostringstream oss;
        entries = (numSkipped == 1 ?  L" entry" : L" entries");
        oss << L"\nSkipped " << numSkipped << entries;
        rpt.WriteLine(oss.str().c_str());
      }

      if (numPWHErrors != 0) {
        wostringstream oss;
        entries = (numPWHErrors == 1 ?  L" entry" : L" entries");
        oss << L"\nwith Password History errors" << numPWHErrors << entries;
        rpt.WriteLine(oss.str().c_str());
      }

      if (numRenamed != 0) {
        wostringstream oss;
        entries = (numRenamed == 1 ?  L" entry" : L" entries");
        oss << L"\nRenamed " << numRenamed << entries;
        rpt.WriteLine(oss.str().c_str());
      }
      break;
    }
  } // switch
  rpt.EndReport();
  return rc;
}

static int
ImportXML(PWScore &core, const StringX &fname)
{
  const std::wstring XSDfn(L"pwsafe.xsd");
  std::wstring XSDFilename = PWSdirs::GetXMLDir() + XSDfn;

#if USE_XML_LIBRARY == MSXML || USE_XML_LIBRARY == XERCES
  if (!pws_os::FileExists(XSDFilename)) {
    wcerr << L"Can't find XML Schema Definition file"
          << XSDFilename << endl
          << L"Can't import without it." << endl;
    return PWScore::XML_FAILED_IMPORT;
  }
#endif

  std::wstring ImportedPrefix;
  std::wstring dir;
  std::wstring strXMLErrors, strSkippedList, strPWHErrorList, strRenameList;
  int numValidated, numImported, numSkipped, numRenamed, numPWHErrors;
  bool bImportPSWDsOnly = false;

  
  // Create report as we go
  CReport rpt;
  std::wstring str_text;
  rpt.StartReport(L"Import_XML", core.GetCurFile().c_str());
  str_text = L"XML file being imported: ";
  str_text += fname.c_str();
  rpt.WriteLine(str_text);
  rpt.WriteLine();
  std::vector<StringX> vgroups;
  Command *pcmd = NULL;

  int rc = core.ImportXMLFile(ImportedPrefix.c_str(), fname.c_str(),
                              XSDFilename.c_str(), bImportPSWDsOnly,
                              strXMLErrors, strSkippedList, strPWHErrorList,
                              strRenameList, numValidated, numImported,
                              numSkipped, numPWHErrors, numRenamed,
                              rpt, pcmd);

  switch (rc) {
  case PWScore::XML_FAILED_VALIDATION:
    rpt.WriteLine(strXMLErrors.c_str());
    str_text = L"File ";
    str_text += fname.c_str();
    str_text += L" failed to validate.";
    delete pcmd;
    break;
  case PWScore::XML_FAILED_IMPORT:
    rpt.WriteLine(strXMLErrors.c_str());
    str_text = L"File ";
    str_text += fname.c_str();
    str_text += L" validated but hadd errors during import.";
    delete pcmd;
    break;
  case PWScore::SUCCESS:
  case PWScore::OK_WITH_ERRORS:
    if (pcmd != NULL)
      rc = core.Execute(pcmd);

    if (!strXMLErrors.empty() ||
        numRenamed > 0 || numPWHErrors > 0) {
      wstring csErrors;
      if (!strXMLErrors.empty())
        csErrors = strXMLErrors + L"\n";

      if (!csErrors.empty()) {
        rpt.WriteLine(csErrors.c_str());
      }

      wstring cs_renamed, cs_PWHErrors, cs_skipped;
      if (numSkipped > 0) {
        rpt.WriteLine(wstring(L"The following records were skipped:"));
        wostringstream oss;
        oss << L" / skipped " << numSkipped;
        cs_skipped = oss.str();
        rpt.WriteLine(strSkippedList.c_str());
        rpt.WriteLine();
      }
      if (numPWHErrors > 0) {
        rpt.WriteLine(wstring(L"The following records had errors in their Password History:"));
        wostringstream oss;
        oss << L" / with Password History errors " << numPWHErrors;
        cs_PWHErrors = oss.str();
        rpt.WriteLine(strPWHErrorList.c_str());
        rpt.WriteLine();
      }
      if (numRenamed > 0) {
        rpt.WriteLine(wstring(L"The following records were renamed as an entry already exists in your database or in the Import file:"));
        wostringstream oss;
        oss << L" / renamed " << numRenamed;
        cs_renamed = oss.str();
        rpt.WriteLine(strRenameList.c_str());
        rpt.WriteLine();
      }

      wostringstream os2;
      os2 << L"File " << fname << L" was imported (entries validated"
          << numValidated << L" / imported " << numImported
          << cs_skipped <<  cs_renamed << cs_PWHErrors;
      str_text = os2.str();
    } else {
      const wstring validate(numValidated == 1 ? L" entry" : L" entries");
      const wstring import(numImported == 1 ? L" entry" : L" entries");
      wostringstream oss;
      oss << L"Validated " << numValidated << validate << L'\t'
          << L"Imported " << numImported << import << endl;
      str_text = oss.str();
    }

    break;
  default:
    ASSERT(0);
  } // switch

    // Finish Report
  rpt.WriteLine(str_text);
  rpt.EndReport();
  return rc;
}

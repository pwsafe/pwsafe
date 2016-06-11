/*
* Copyright (c) 2003-2017 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include <iostream>
#include <sstream>
#include <unistd.h>
#include <getopt.h>
#include <regex>

#include "PWScore.h"
#include "os/file.h"
#include "core/PWSdirs.h"
#include "core/UTF8Conv.h"
#include "core/Report.h"
#include "core/XML/XMLDefs.h"
#include "../../core/core.h"

#include <termios.h>

#include "../wxWidgets/SearchUtils.h"

using namespace std;

// Fwd declarations:
static void echoOff();
static void echoOn();

int OpenCore(PWScore& core, const StringX& safe);

static int ImportText(PWScore &core, const StringX &fname);
static int ImportXML(PWScore &core, const StringX &fname);
static const char *status_text(int status);

// These are the new operations. Each returns the code to exit with
static int CreateNewSafe(const StringX& filename);
static int SearchForEntries(PWScore &core, const wstring &searchText, bool ignoreCase,
                            const wstring &restrictToEntries, const wstring &fieldsToSearch);

StringX GetPassphrase(const wstring& prompt);
StringX GetNewPassphrase();

int OpenCoreAndSearch(const StringX &safe, const wstring &searchText, bool ignoreCase,
                      const wstring &restrictToEntries, const wstring &fieldsToSearch);
CItemData::FieldType String2FieldType(const stringT& str);

//-----------------------------------------------------------------

static struct termios oldTermioFlags; // to restore tty echo

static void usage(char *pname)
{
  cerr << "Usage: " << pname << " safe --imp[=file] --text|--xml" << endl
       << "\t safe --exp[=file] --text|--xml" << endl
  << "\t safe --new" << endl
  << "\t safe --search=<search text> [--ignore-case] [--subset<OP><string>[/iI]] [--fields=<comma-separated fieldnames>]" << endl
  << "\t\t where OP is one of ==, !==, ^= !^=, $=, !$=, ~=, !~=" << endl
  << "\t\t\t = => exactly similar" << endl
  << "\t\t\t ^ => begins-with" << endl
  << "\t\t\t $ => ends with"
  << "\t\t\t ^ => contains"
  << "\t\t\t ! => negation" << endl
  << "\t\t a trailing /i or /I at the end of subset string makes the operation case insensitive or sensitive respectively" << endl;
}

std::ostream& operator<<(std::ostream& os, const StringX& str)
{
    return os << str.c_str();
}

std::ostream& operator<<(std::ostream& os, const wstring& str)
{
    return os << str.c_str();
}

struct UserArgs {
  UserArgs() : Operation(Unset), Format(Unknown), ignoreCase{false} {}
  StringX safe, fname;
  enum {Unset, Import, Export, CreateNew, Search} Operation;
  enum {Unknown, XML, Text} Format;

  // used for search
  wstring searchedText;
  wstring searchedFields;
  wstring searchedSubset;
  bool ignoreCase;
};

void Utf82StringX(const char* filename, StringX& sname)
{
    CUTF8Conv conv;
    if (!conv.FromUTF8((const unsigned char *)filename, strlen(filename),
                       sname)) {
        cerr << "Could not convert " << filename << " to StringX" << endl;
        exit(2);
    }
}

wstring Utf82wstring(const char* utf8str)
{
  StringX sx;
  Utf82StringX(utf8str, sx);
  return stringx2std(sx);
}

bool parseArgs(int argc, char *argv[], UserArgs &ua)
{
  if (argc < 3) // must give us a safe and an operation
    return false;

  Utf82StringX(argv[1], ua.safe);

  while (1) {
    int option_index = 0;
    static struct option long_options[] = {
      // name, has_arg, flag, val
      {"import", optional_argument, 0, 'i'},
      {"export", optional_argument, 0, 'e'},
      {"text", no_argument, 0, 't'},
      {"xml", no_argument, 0, 'x'},
      {"new", no_argument, 0, 'n'},
      {"search", required_argument, 0, 's'},
      {"subset", required_argument, 0, 'u'},
      {"fields", required_argument, 0, 'f'},
      {"ignore-case", optional_argument, 0, 'c'},
      {0, 0, 0, 0}
    };

    int c = getopt_long(argc-1, argv+1, "i::e::tx",
                        long_options, &option_index);
    if (c == -1)
      break;

    switch (c) {
    case 'i':
      if (ua.Operation == UserArgs::Unset)
        ua.Operation = UserArgs::Import;
      else
        return false;
      if (optarg) Utf82StringX(optarg, ua.fname);
      break;
    case 'e':
      if (ua.Operation == UserArgs::Unset)
        ua.Operation = UserArgs::Export;
      else
        return false;
      if (optarg) Utf82StringX(optarg, ua.fname);
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
    case 'n':
      if (ua.Operation == UserArgs::Unset)
        ua.Operation = UserArgs::Import;
      else
        return false;
      break;

    case 's':
      if (ua.Operation == UserArgs::Unset) {
        ua.Operation = UserArgs::Search;
        assert(optarg);
        ua.searchedText = Utf82wstring(optarg);
        break;
      }
      else
        return false;

    case 'u':
        assert(optarg);
        ua.searchedSubset = Utf82wstring(optarg);
        break;

    case 'f':
        assert(optarg);
        ua.searchedFields = Utf82wstring(optarg);
        break;

    case 'c':
        if (optarg && std::regex_match(optarg, std::regex("yes|true", std::regex::icase)))
          ua.ignoreCase = true;
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

    if (ua.Operation == UserArgs::CreateNew) {
        return CreateNewSafe(ua.safe);
    }
    else if (ua.Operation == UserArgs::Search) {
      return OpenCoreAndSearch(ua.safe, ua.searchedText, ua.ignoreCase, ua.searchedSubset, ua.searchedFields);
    }

  PWScore core;
  int ret = OpenCore(core, ua.safe);
  if (ret != PWScore::SUCCESS)
    return ret;

  int status = PWScore::SUCCESS;
  if (ua.Operation == UserArgs::Export) {
    CItemData::FieldBits all(~0L);
    int N;
    if (ua.Format == UserArgs::XML) {
      status = core.WriteXMLFile(ua.fname, all, L"", 0, 0, L' ', L"", N);
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
  int numImported(0), numSkipped(0), numPWHErrors(0), numRenamed(0), numNoPolicy(0);
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
                                    numPWHErrors, numRenamed, numNoPolicy,
                                    rpt, pcmd);
  switch (rc) {
  case PWScore::CANT_OPEN_FILE:
  case PWScore::INVALID_FORMAT:
  case PWScore::FAILURE:
    delete pcmd;
    break;
  case PWScore::SUCCESS:
  case PWScore::OK_WITH_ERRORS:
    // deliberate fallthrough
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
  std::wstring strXMLErrors, strSkippedList, strPWHErrorList, strRenameList;
  int numValidated(0), numImported(0), numSkipped(0), numRenamed(0), numPWHErrors(0);
  int numNoPolicy(0), numRenamedPolicies(0), numShortcutsRemoved(0), numEmptyGroupsRemoved(0);
  bool bImportPSWDsOnly = false;

  
  // Create report as we go
  CReport rpt;
  std::wstring str_text;
  rpt.StartReport(L"Import_XML", core.GetCurFile().c_str());
  str_text = L"XML file being imported: ";
  str_text += fname.c_str();
  rpt.WriteLine(str_text);
  rpt.WriteLine();
  Command *pcmd = NULL;

  int rc = core.ImportXMLFile(ImportedPrefix.c_str(), fname.c_str(),
                              XSDFilename.c_str(), bImportPSWDsOnly,
                              strXMLErrors, strSkippedList, strPWHErrorList,
                              strRenameList, numValidated, numImported,
                              numSkipped, numPWHErrors, numRenamed,
                              numNoPolicy, numRenamedPolicies, numShortcutsRemoved, numEmptyGroupsRemoved,
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

static int CreateNewSafe(const StringX& filename)
{
    if ( pws_os::FileExists(filename.c_str()) ) {
        cerr << filename << " - already exists" << endl;
        exit(1);
    }

    const StringX passkey = GetNewPassphrase();

    PWScore core;
    core.SetCurFile(filename);
    core.NewFile(passkey);
    const int status = core.WriteCurFile();

    if (status != PWScore::SUCCESS)
        cerr << "Could not create " << filename << ": " << status_text(status);

    return status;
}

StringX GetNewPassphrase()
{
    StringX passphrase[2];
    wstring prompt[2] = {L"Enter passphrase: ", L"Enter the same passphrase again"};

    do {
        passphrase[0] = GetPassphrase(prompt[0]);
        passphrase[1] = GetPassphrase(prompt[1]);

        if (passphrase[0] != passphrase[1]) {
            cerr << "The two passphrases do not match. Please try again" << endl;
            continue;
        }
        if (passphrase[0].length() == 0) {
            cerr << "Invalid passphrase. Please try again" << endl;
            continue;
        }

        break;
    }
    while(1);

    return passphrase[0];
}

StringX GetPassphrase(const wstring& prompt)
{
    wstring wpk;
    wcout << prompt;
    echoOff();
    wcin >> wpk;
    echoOn();
    return StringX(wpk.c_str());
}

struct Restriction {
  CItemData::FieldType field;
  PWSMatch::MatchRule rule;
  wstring value;
  bool caseSensitive;
};

PWSMatch::MatchRule Str2MatchRule( const wstring &s)
{
  static const std::map<wstring, PWSMatch::MatchRule> rulemap{
    {L"==", PWSMatch::MR_EQUALS},
    {L"!==", PWSMatch::MR_NOTEQUAL},
    {L"^=", PWSMatch::MR_BEGINS},
    {L"!^=", PWSMatch::MR_NOTBEGIN},
    {L"$=", PWSMatch::MR_ENDS},
    {L"!$=", PWSMatch::MR_NOTEND},
    {L"~=", PWSMatch::MR_CONTAINS},
    {L"!~=", PWSMatch::MR_NOTCONTAIN}
  };
  const auto itr = rulemap.find(s);
  if ( itr != rulemap.end() )
    return itr->second;
  return PWSMatch::MR_INVALID;
}

bool CaseSensitive(const wstring &str)
{
  assert(str.length() == 0 || (str.length() == 2 && str[0] == '/' && (str[1] == L'i' || str[1] == 'I')));
  return str.length() == 0 || str[0] == L'i';
}
std::vector<Restriction> ParseSearchedEntryRestrictions(const wstring &restrictToEntries)
{
  std::vector<Restriction> restrictions;
  if ( !restrictToEntries.empty() ) {
    std::wregex restrictPattern(L"([[:alpha:]]+)([!]?[=^$~]=)([^;]+?)(/[iI])?(;|$)");
    std::wsregex_iterator pos(restrictToEntries.cbegin(), restrictToEntries.cend(), restrictPattern);
    std::wsregex_iterator end;
    for_each( pos, end, [&restrictions](const wsmatch &m) {
      restrictions.push_back( {String2FieldType(m.str(1)), Str2MatchRule(m.str(2)), m.str(3), CaseSensitive(m.str(4))} );
    });
  }
  return restrictions;
}

int OpenCore(PWScore& core, const StringX& safe)
{
  if (!pws_os::FileExists(safe.c_str())) {
    wcerr << safe << " - file not found" << endl;
    return 2;
  }

  StringX pk = GetPassphrase(L"Enter Password: ");

  int status;
  status = core.CheckPasskey(safe, pk);
  if (status != PWScore::SUCCESS) {
    cout << "CheckPasskey returned: " << status_text(status) << endl;
    return status;
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
    if (!core.LockFile(safe.c_str(), lk)) {
      wcout << L"Couldn't lock file " << safe
      << L": locked by " << locker << endl;
      status = -1;
      return status;
    }
  }
  // Since we may be writing the same file we're reading,
  // it behooves us to set the Current File and use its' related
  // functions
  core.SetCurFile(safe);
  status = core.ReadCurFile(pk);
  if (status != PWScore::SUCCESS) {
    cout << "ReadFile returned: " << status_text(status) << endl;
  }
  return status;
}
using String2FieldTypeMap = std::map<stringT, CItemData::FieldType>;

// Reverse of CItemData::FieldName
String2FieldTypeMap  InitFieldTypeMap()
{
    String2FieldTypeMap ftmap;
    stringT retval;
    LoadAString(retval, IDSC_FLDNMGROUPTITLE);      ftmap[retval] = CItem::GROUPTITLE;
    LoadAString(retval, IDSC_FLDNMUUID);            ftmap[retval] = CItem::UUID;
    LoadAString(retval, IDSC_FLDNMGROUP);           ftmap[retval] = CItem::GROUP;
    LoadAString(retval, IDSC_FLDNMTITLE);           ftmap[retval] = CItem::TITLE;
    LoadAString(retval, IDSC_FLDNMUSERNAME);        ftmap[retval] = CItem::USER;
    LoadAString(retval, IDSC_FLDNMNOTES);           ftmap[retval] = CItem::NOTES;
    LoadAString(retval, IDSC_FLDNMPASSWORD);        ftmap[retval] = CItem::PASSWORD;
#undef CTIME
    LoadAString(retval, IDSC_FLDNMCTIME);           ftmap[retval] = CItem::CTIME;
    LoadAString(retval, IDSC_FLDNMPMTIME);          ftmap[retval] = CItem::PMTIME;
    LoadAString(retval, IDSC_FLDNMATIME);           ftmap[retval] = CItem::ATIME;
    LoadAString(retval, IDSC_FLDNMXTIME);           ftmap[retval] = CItem::XTIME;
    LoadAString(retval, IDSC_FLDNMRMTIME);          ftmap[retval] = CItem::RMTIME;
    LoadAString(retval, IDSC_FLDNMURL);             ftmap[retval] = CItem::URL;
    LoadAString(retval, IDSC_FLDNMAUTOTYPE);        ftmap[retval] = CItem::AUTOTYPE;
    LoadAString(retval, IDSC_FLDNMPWHISTORY);       ftmap[retval] = CItem::PWHIST;
    LoadAString(retval, IDSC_FLDNMPWPOLICY);        ftmap[retval] = CItem::POLICY;
    LoadAString(retval, IDSC_FLDNMXTIMEINT);        ftmap[retval] = CItem::XTIME_INT;
    LoadAString(retval, IDSC_FLDNMRUNCOMMAND);      ftmap[retval] = CItem::RUNCMD;
    LoadAString(retval, IDSC_FLDNMDCA);             ftmap[retval] = CItem::DCA;
    LoadAString(retval, IDSC_FLDNMSHIFTDCA);        ftmap[retval] = CItem::SHIFTDCA;
    LoadAString(retval, IDSC_FLDNMEMAIL);           ftmap[retval] = CItem::EMAIL;
    LoadAString(retval, IDSC_FLDNMPROTECTED);       ftmap[retval] = CItem::PROTECTED;
    LoadAString(retval, IDSC_FLDNMSYMBOLS);         ftmap[retval] = CItem::SYMBOLS;
    LoadAString(retval, IDSC_FLDNMPWPOLICYNAME);    ftmap[retval] = CItem::POLICYNAME;
    LoadAString(retval, IDSC_FLDNMKBSHORTCUT);      ftmap[retval] = CItem::KBSHORTCUT;
    return ftmap;;
}

CItemData::FieldType String2FieldType(const stringT& str)
{
    static const String2FieldTypeMap ftmap = InitFieldTypeMap();
    auto itr = ftmap.find(str);
    if (itr != ftmap.end())
       return itr->second;
    return CItem::LAST_DATA;;
}

CItemData::FieldBits ParseFieldsToSearh(const wstring &fieldsToSearch)
{
  CItemData::FieldBits fields;
  if ( !fieldsToSearch.empty() ) {
    std::wsregex_token_iterator pos(fieldsToSearch.cbegin(), fieldsToSearch.cend(), std::wregex(L";"), -1);
    std::wsregex_token_iterator end;
    for_each( pos, end, [&fields](const wstring &field) { fields.set(String2FieldType(field));} );
  }
  return fields;
}

bool ItemMatches(const CItemData &item, const StringX &searchText)
{
  using FT = CItemData::FieldType;
  FT fields[] = {   CItemData::GROUP,
                    CItemData::TITLE,
                    CItemData::USER,
                    CItemData::PASSWORD,
                    CItemData::NOTES,
                    CItemData::URL,
                    CItemData::EMAIL,
                    CItemData::RUNCMD,
                    CItemData::AUTOTYPE,
                    CItemData::XTIME_INT
  };

  return std::find_if( std::begin(fields), std::end(fields), [&item, &searchText](FT f) {
    return item.GetFieldValue(f).find(searchText) != StringX::npos;
  }) != std::end(fields);

}

int OpenCoreAndSearch(const StringX &safe, const wstring &searchText, bool ignoreCase,
                      const wstring &restrictToEntries, const wstring &fieldsToSearch)
{
  PWScore core;
  int status = OpenCore(core, safe);
  if ( status == PWScore::SUCCESS )
    status = SearchForEntries(core, searchText, ignoreCase, restrictToEntries, fieldsToSearch);
  return status;
}

int SearchForEntries(PWScore &core, const wstring &searchText, bool ignoreCase,
                     const wstring &restrictToEntries, const wstring &fieldsToSearch)
{
  assert( !searchText.empty() );

  std::vector<Restriction> restrictions = ParseSearchedEntryRestrictions(restrictToEntries);

  if ( !restrictToEntries.empty() && restrictions.empty() ) {
    cerr << "Could not parse [" << restrictToEntries << " ]for restricting searched entries" << endl;
    return PWScore::FAILURE;
  }

  CItemData::FieldBits fields = ParseFieldsToSearh(fieldsToSearch);
  if (fieldsToSearch.empty())
    fields.set();
  if ( !fieldsToSearch.empty() && fields.none() ) {
    cerr << "Could not parse [" << fieldsToSearch << " ]for restricting searched fields" << endl;
    return PWScore::FAILURE;
  }

  const Restriction dummy{ CItem::LAST_DATA, PWSMatch::MR_INVALID, wstring{}, true};
  const Restriction r = restrictions.size() > 0? restrictions[0]: dummy;

  ::FindMatches(std2stringx(searchText), ignoreCase, fields, restrictions.size() > 0, r.value, r.field, r.rule, r.caseSensitive,
              core.GetEntryIter(), core.GetEntryEndIter(), get_second<ItemList>{}, [](ItemListIter itr){
                wcout << itr->second.GetGroup() << " - " << itr->second.GetTitle() << " - " << itr->second.GetUser() << endl;
  });
  return PWScore::SUCCESS;
}

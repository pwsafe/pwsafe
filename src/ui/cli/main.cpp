/*
* Copyright (c) 2003-2025 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "stdafx.h"
#ifdef _WIN32
#include "resource.h"
#endif /* _WIN32 */

#include <iostream>
#include <sstream>
#include <cassert>
#ifndef _WIN32
#include <getopt.h>
#include <libgen.h> // for basename()
#else
#include "os/windows/getopt.h"
#endif
#include <string>
#include <map>
#include <functional>

#include "./search.h"
#include "./argutils.h"
#include "./searchaction.h"
#include "./strutils.h"
#include "./diff.h"
#include "./safeutils.h"
#include "./impexp.h"
#include "./cli-version.h"

#include "core/PWScore.h"
#include "os/file.h"
#include "core/UTF8Conv.h"
#include "core/Report.h"
#include "core/XML/XMLDefs.h"

using namespace std;


#ifdef _WIN32
// Windows doesn't have POSIX basename, so we roll our own:

static char *basename(const char *path)
{
  static char retval[_MAX_FNAME];
  if (_splitpath_s(path,
                   nullptr, 0, // drive
                   nullptr, 0, // dir
                   retval, sizeof(retval),
                   nullptr, 0 // ext
                   ) == 0)
    return retval;
  else
    return "";
}
#endif


int SaveCore(PWScore &core, const UserArgs &);

// These are the new operations. Each returns the code to exit with
static int CreateNewSafe(PWScore &core, const StringX &filename, const StringX &passphrase, bool);
static int Sync(PWScore &core, const UserArgs &ua);
static int Merge(PWScore &core, const UserArgs &ua);

//-----------------------------------------------------------------

using pre_op_fn = function<int(PWScore &, const StringX &, const StringX &, bool)>;
using main_op_fn = function<int(PWScore &, const UserArgs &)>;
using post_op_fn = function<int(PWScore &, const UserArgs &)>;

auto null_op = [](PWScore &, const UserArgs &)-> int{ return PWScore::SUCCESS;};

struct pws_op {
  pre_op_fn pre_op;
  main_op_fn main_op;
  post_op_fn post_op;
};


const map<UserArgs::OpType, pws_op> pws_ops = {
  { UserArgs::Import,     {OpenCore,        Import,     SaveCore}},
  { UserArgs::Export,     {OpenCore,        Export,     null_op}},
  { UserArgs::CreateNew,  {CreateNewSafe,   null_op,    SaveCore}},
  { UserArgs::Add,        {OpenCore,        AddEntry,   SaveCore}},
  { UserArgs::Search,     {OpenCore,        Search,     SaveAfterSearch}},
  { UserArgs::Diff,       {OpenCore,        Diff,       null_op}},
  { UserArgs::Sync,       {OpenCore,        Sync,       SaveCore}},
  { UserArgs::Merge,      {OpenCore,        Merge,      SaveCore}},
};

static wstring usage_string = LR"usagestring(
Usage: %PROGNAME% safe --imp[=file] --text|--xml

       %PROGNAME% safe --exp[=file] --text|--xml

       %PROGNAME% safe --create

       %PROGNAME% safe --add=field1=value1,field2=value2,...

       %PROGNAME% safe --search=<text> [--ignore-case]
                      [--subset=<Field><OP><string>[/iI] [--fields=f1,f2,..]
                      [--delete | --update=Field1=Value1,Field2=Value2,.. | --print[=field1,field2...] ] [--yes]
                      [--generate-totp]

       %PROGNAME% safe --diff=<other-safe> [ --subset=<Field><OP><Value>[/iI] ]
                      [--fields=f1,f2,..] [--unified | --context | --sidebyside]
                      [--colwidth=column-size]

       %PROGNAME% safe --synchronize=<other-safe> [ --subset=<Field><OP><string>[/iI] ] [ --fields=f1,f2,.. ] [--yes]

       %PROGNAME% safe --merge=<other-safe> [ --subset=<Field><OP><Value>[/iI] ] [--yes]

                        where OP is one of ==, !==, ^= !^=, $=, !$=, ~=, !~=
                         = => exactly similar
                         ^ => begins with
                         $ => ends with
                         ~ => contains
                         ! => negation
                        a trailing /i => case insensitive, /I => case sensitive

       %PROGNAME% --help[=operation]

       Note that --passphrase <passphrase> and --passphrase2 <2nd passphrase> may be used to skip the prompt
       for the master passphrase(s). However, this should be avoided if possible for security reasons.

       Valid field names are:
       %FIELDNAMES%

       Note:
        ° Field names and values that contain whitespace characters must be quoted (e.g. "Created Time").
        ° Times are expected in Universal Time Coordinated (UTC) format ("YYYY/MM/DD hh:mm:ss").
        ° The current time can also be referenced using the keyword "now".
)usagestring";

static std::wstring help_create_string = LR"helpstring(
 Example: Creating a database

            %PROGNAME% pwsafe.psafe3 --create

          This creates the empty database "pwsafe.psafe3".
)helpstring";

static std::wstring help_add_string = LR"helpstring(
 Example: Adding an entry

          To add an entry to a database, the title is mandatory. All other fields are optional.
          If no password is specified, a password will be generated for the entry.

            %PROGNAME% pwsafe.psafe3 --add=Title="Login"

          The simplest way to add a new entry to a database, specifying the minimum required fields.

            %PROGNAME% pwsafe.psafe3 --add=Title="Bank Account",Username="John Doe",Password=SecretPassword,"Created Time"="2020/01/01 12:00:00"

          This adds an entry with the title "Bank Account" to the database "pwsafe.psafe3" which does not belong to any group.

            %PROGNAME% pwsafe.psafe3 --add=Group=Email,Title=Yahoo,Username="Richard Miles",Password=SecretPassword,"Created Time"=now

          Similar to the previous example, except that the entry is added to the "Email" group. If the group does not exist, it is created.
)helpstring";

static std::wstring help_update_string = LR"helpstring(
 Example: Updating an entry

          In this example, an entry containing "Richard Miles" is searched for.

            %PROGNAME% pwsafe.psafe3 --search="Richard Miles" --update=Title=Google

          If such an entry is found, its title is updated after the user has confirmed this.
)helpstring";

static std::wstring help_search_string = LR"helpstring(
 Example: Searching for an entry

          The search option is very helpful for referring to specific entries. It can search the entire database for a specific textual occurrence,
          as well as content in specific fields of entries to further narrow the search. This is particularly helpful when multiple entries have the
          same text, such as the same title.

            %PROGNAME% pwsafe.psafe3 --search="Login" --print=Title,Username

          This would output the title and username of the entries found that contain "Login".

            %PROGNAME% pwsafe.psafe3 --search="Login" --subset=Username=="John Doe" --print=Title,Username

          This search command would limit the results to all entries containing the occurrence "Login" and the username "John Doe".
)helpstring";

static std::wstring help_delete_string = LR"helpstring(
 Example: Deleting an entry

            %PROGNAME% pwsafe.psafe3 --search="Richard Miles" --delete

          This will delete the entry that matches the search criteria after the deletion is confirmed.

            %PROGNAME% pwsafe.psafe3 --search="John Doe" --delete --yes

          This deletes the found entry without asking for confirmation.
)helpstring";

static std::wstring help_synchronize_string = LR"helpstring(
 Example: Synchronizing databases

            %PROGNAME% pwsafeA.psafe3 --synchronize=pwsafeB.psafe3

          This synchronizes database pwsafeA.psafe3 with database pwsafeB.psafe3.
)helpstring";

struct pws_help_example {
  wstring operation;
  wstring help_string;
};

const map<wstring, wstring> pws_help_examples = {
  { L"create",      help_create_string      },
  { L"add",         help_add_string         },
  { L"update",      help_update_string      },
  { L"search",      help_search_string      },
  { L"delete",      help_delete_string      },
  { L"sync",        help_synchronize_string },
  { L"synchronize", help_synchronize_string },
};

static void usage(char *pname)
{
  std::wstring s_pname = Utf82wstring(pname);
  std::wstringstream ss_fieldnames;
  constexpr auto names_per_line = 5;
  auto nnames = 0;
  const auto fieldnames = GetValidFieldNames();
  for (const stringT &name: fieldnames) {
    if (nnames)
    ss_fieldnames << L", ";
    if ( nnames++ % names_per_line == 0 )
      ss_fieldnames << L"\n\t\t";
    ss_fieldnames << name;
	}

  std::wstring s_fieldnames = ss_fieldnames.str();
  const std::wstring s_fn_placeholder{L"%FIELDNAMES%"};
  auto pos = usage_string.find(s_fn_placeholder);
  if (pos != std::string::npos) {
    usage_string.replace(pos, s_fn_placeholder.length(), s_fieldnames);
  }

  const std::wstring s_pn_placeholder{L"%PROGNAME%"};
  const auto pname_len = s_pname.length();

  for(auto itr = usage_string.find(s_pn_placeholder); itr != std::string::npos; itr = usage_string.find(s_pn_placeholder, itr + pname_len)) {
    usage_string.replace(itr, s_pn_placeholder.length(), s_pname);
  }

  wcerr << s_pname << L" version " << CLI_MAJOR_VERSION << L"." << CLI_MINOR_VERSION << L"." << CLI_REVISION << endl;
  wcerr << usage_string;
  wcerr << '\n';
}

static wstring replace_progname_placeholder(const wstring &help_string, const wstring &progname)
{
  const wstring placeholder{L"%PROGNAME%"};
  const auto progname_len = progname.length();
  wstring new_help_string{help_string};

  for(auto itr = new_help_string.find(placeholder); itr != string::npos; itr = new_help_string.find(placeholder, itr + progname_len)) {
    new_help_string.replace(itr, placeholder.length(), progname);
  }
  return new_help_string;
}

static bool help(char *pname, const wstring &help_arg)
{
  auto itr = pws_help_examples.find(help_arg);
  if (itr != pws_help_examples.end()) {
    wcout << replace_progname_placeholder(itr->second, Utf82wstring(pname)) << endl;
  }
  else {
    wcerr << L"Unsupported help argument: " << help_arg << endl << endl;
    wcerr << L"Supported arguments are:" << endl;
    for (auto const& help : pws_help_examples) {
      wcerr << L" - " << help.first << endl;
    }
    wcerr << endl;
    return false;
  }
  return true;
}

#if 0
// Can't get this to work with shift of > 32 bits - compiler bug?
constexpr bool no_dup_short_option2(uint64_t bits, const option *p)
{
  return p->name == 0 ||
          (!(bits & (1 << (p->val - 'A'))) && no_dup_short_option2(bits | (1 << (p->val - 'A')), p+1));
}

constexpr bool no_dup_short_option(const struct option *p)
{
  return no_dup_short_option2(uint64_t{}, p);
}
#endif

bool parseArgs(int argc, char *argv[], UserArgs &ua)
{
  if (argc < 2) {       // 0: app name, 1: help option ?
    return false;
  }
  else if (argc > 2) {  // 0: app name, 1: db name, 2: operation name
    Utf82StringX(argv[1], ua.safe);
    argc--, argv++;     // skip db name for getopt_long
  }

  try {
    static const char* short_options = "i::e::txcs:b:f:oa:u:p::rl:vyd:gjknz:m:w:P:Q:GVh::";
    static constexpr struct option long_options[] = {
      // name,          has_arg,            flag,    val
      {"import",        optional_argument,  nullptr, 'i'},
      {"export",        optional_argument,  nullptr, 'e'},
      {"text",          no_argument,        nullptr, 't'},
      {"xml",           no_argument,        nullptr, 'x'},
      {"create",        no_argument,        nullptr, 'c'},
      {"search",        required_argument,  nullptr, 's'},
      {"subset",        required_argument,  nullptr, 'b'},
      {"fields",        required_argument,  nullptr, 'f'},
      {"ignore-case",   no_argument,        nullptr, 'o'},
      {"add",           required_argument,  nullptr, 'a'},
      {"update",        required_argument,  nullptr, 'u'},
      {"print",         optional_argument,  nullptr, 'p'},
      {"delete",        no_argument,        nullptr, 'r'},
      {"clear",         required_argument,  nullptr, 'l'},
      {"newpass",       no_argument,        nullptr, 'v'},
      {"yes",           no_argument,        nullptr, 'y'},
      {"diff",          required_argument,  nullptr, 'd'},
      {"unified",       no_argument,        nullptr, 'g'},
      {"context",       no_argument,        nullptr, 'j'},
      {"sidebyside",    no_argument,        nullptr, 'k'},
      {"dry-run",       no_argument,        nullptr, 'n'},
      {"synchronize",   required_argument,  nullptr, 'z'},
      {"merge",         required_argument,  nullptr, 'm'},
      {"colwidth",      required_argument,  nullptr, 'w'},
      {"passphrase",    required_argument,  nullptr, 'P'},
      {"passphrase2",   required_argument,  nullptr, 'Q'},
      {"generate-totp", no_argument,        nullptr, 'G'},
      {"verbose",       no_argument,        nullptr, 'V'},
      {"help",          optional_argument,  nullptr, 'h'},
      {nullptr,         0,                  nullptr,  0 }
    };

    while (1) {
      int option_index = 0;

#if 0 // see comment above no_dup_short_option
      static_assert(no_dup_short_option(long_options), "Short option used twice");
#endif

      int c = getopt_long(argc, argv,
          short_options, long_options,
          &option_index);

      if (c == -1)
        break;

      switch (c) {
      case 'i':
        ua.SetMainOp(UserArgs::Import, optarg);
        break;

      case 'e':
        ua.SetMainOp(UserArgs::Export, optarg);
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

      case 'c':
        ua.SetMainOp(UserArgs::CreateNew);
        break;

      case 's':
        assert(optarg);
        ua.SetMainOp(UserArgs::Search, optarg);
        ua.ignoreCase = true; // Default behavior is case sensitiv search.
        break;                // See 'fCaseSensitive' in FindMatches (SearchUtils.h), which reverses the logic.

      case 'd':
        assert(optarg);
        ua.SetMainOp(UserArgs::Diff, optarg);
        break;

      case 'z':
        assert(optarg);
        ua.SetMainOp(UserArgs::Sync, optarg);
        break;

      case 'm':
        assert(optarg);
        ua.SetMainOp(UserArgs::Merge, optarg);
        break;

      case 'b':
        assert(optarg);
        ua.SetSubset(Utf82wstring(optarg));
        break;

      case 'f':
        assert(optarg);
        ua.SetFields(Utf82wstring(optarg));
        break;

      case 'o':
        ua.ignoreCase = false; // Results in 'not case sensitiv'. See also option 's' and
        break;                 // 'fCaseSensitive' in FindMatches (SearchUtils.h), which reverses the logic.

      case 'a':
        assert(optarg);
        ua.SetMainOp(UserArgs::Add, optarg);
        break;

      case 'y':
        ua.confirmed = true;
        break;

      case 'r':
        ua.SearchAction = UserArgs::Delete;
        break;

      case 'p':
        ua.SearchAction = UserArgs::Print;
        if (optarg) ua.opArg2 = Utf82wstring(optarg);
        break;

      case 'u':
        ua.SearchAction = UserArgs::Update;
        assert(optarg);
        ua.SetFieldValues(Utf82wstring(optarg));
        break;

      case 'l':
        ua.SearchAction = UserArgs::ClearFields;
        assert(optarg);
        ua.opArg2 = Utf82wstring(optarg);
        break;

      case 'v':
        ua.SearchAction = UserArgs::ChangePassword;
        break;

      case 'g':
        ua.dfmt = UserArgs::DiffFmt::Unified;
        break;

      case 'j':
        ua.dfmt = UserArgs::DiffFmt::Context;
        break;

      case 'k':
        ua.dfmt = UserArgs::DiffFmt::SideBySide;
        break;

      case 'n':
        ua.dry_run = true;
        break;

      case 'w':
        assert(optarg);
        ua.colwidth = atoi(optarg);
        break;

      case 'P':
        assert(optarg);
        Utf82StringX(optarg, ua.passphrase[0]);
        break;

      case 'Q':
        assert(optarg);
        Utf82StringX(optarg, ua.passphrase[1]);
        break;

      case 'G':
        ua.SearchAction = UserArgs::GenerateTotpCode;
        break;

      case 'V':
        ua.verbosity_level++;
        break;

      case 'h':
        ua.SetMainOp(UserArgs::Help, optarg);
        break;

      case '?': // unknown option is alread reported by getopt_long and a
      case ':': // missing option argument is reported by getopt_long as well
        return false;

      default:
        wcerr << L"Unexpected getopt_long return value: " << static_cast<wchar_t>(c) << endl;
        return false;
      } // switch
    } // while
  }
  catch (const std::invalid_argument &ex) {
    wcerr << L"Error: " << ex.what() << endl << endl;
    return false;
  }
  return true;
}

#ifdef _WIN32

CWinApp theApp;

static bool winInit()
{
  HMODULE hModule = ::GetModuleHandle(nullptr);
  if (hModule != nullptr) {
    // initialize MFC and print and error on failure
    if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0)) {
      wprintf(L"Fatal Error: MFC initialization failed\n");
      return false;
    } else {
      return true;
    }
  } else {
    wprintf(L"Fatal Error: GetModuleHandle failed\n");
    return false;
  }
}
#endif // _WIN32

int main(int argc, char *argv[])
{

#ifdef _WIN32
  if (!winInit())
    return 1;
#endif // _WIN32

  // prevent ptrace and creation of core dumps in release build
  if (!pws_os::DisableDumpAttach()) {
    wcerr << L"Failed to block ptrace and core dumps" << endl;
    exit(1);
  }

  UserArgs ua;
  if (!parseArgs(argc, argv, ua)) {
    usage(basename(argv[0]));
    return 1;
  }
  if (ua.Operation == UserArgs::Help) {
    if (help(basename(argv[0]), ua.opArg)) {
      return 0;
    }
    usage(basename(argv[0]));
    return 1;
  }

  int status = 1;
  auto itr = pws_ops.find(ua.Operation);

  if (itr != pws_ops.end()) {
    const bool openReadOnly = ua.Operation == UserArgs::Export || ua.Operation == UserArgs::Diff ||
                              (ua.Operation == UserArgs::Search && (ua.SearchAction == UserArgs::Print || ua.SearchAction == UserArgs::GenerateTotpCode));
    PWScore core;
    try {
      status = itr->second.pre_op(core, ua.safe, ua.passphrase[0], openReadOnly);
      if ( status == PWScore::SUCCESS) {
        status = itr->second.main_op(core, ua);
        if (status == PWScore::SUCCESS)
          status = itr->second.post_op(core, ua);
      }
    }
    catch(const exception &e) {
      wcerr << e.what() << endl;
      status = PWScore::FAILURE;
    }

    if (!openReadOnly) // unlock if locked by pre_op
      core.UnlockFile(ua.safe.c_str());
    return status;
  }
  wcerr << L"No main operation specified" << endl;
  return status;
}

static int CreateNewSafe(PWScore &core, const StringX &filename, const StringX &passphrase, bool)
{
    if ( pws_os::FileExists(filename.c_str()) ) {
        wcerr << filename << L" - already exists" << endl;
        exit(1);
    }

    const StringX passkey = passphrase.empty() ? GetNewPassphrase() : passphrase;
    core.SetCurFile(filename);
    core.NewFile(passkey);

    return PWScore::SUCCESS;
}

int SaveCore(PWScore &core, const UserArgs &ua)
{
  if (!ua.dry_run)
    return core.WriteCurFile();

  return PWScore::SUCCESS;
}

int Sync(PWScore &core, const UserArgs &ua)
{
  const StringX otherSafe{std2stringx(ua.opArg)};
  PWScore otherCore;
  int status = OpenCore(otherCore, otherSafe, ua.passphrase[1]);
  if ( status == PWScore::SUCCESS ) {
    CReport rpt;
    int numUpdated = 0;
    core.Synchronize(&otherCore,
                      ua.fields,          // fields to sync
                      ua.subset.valid(),  // filter?
                      ua.subset.value,    // filter value
                      ua.subset.field,    // field to filter by
                      ua.subset.rule,     // type of match rule for filtering
                      numUpdated,
                      &rpt,               // Must be non-null
                      NULL                // Cancel mechanism. We don't need one
    );
    otherCore.UnlockFile(otherSafe.c_str());
  }
  return status;
}

int Merge(PWScore &core, const UserArgs &ua)
{
  const StringX otherSafe{std2stringx(ua.opArg)};
  PWScore otherCore;
  int status = OpenCore(otherCore, otherSafe, ua.passphrase[1]);
  if ( status == PWScore::SUCCESS ) {
    CReport rpt;
    core.Merge(&otherCore,
               ua.subset.valid(),  // filter?
               ua.subset.value,    // filter value
               ua.subset.field,    // field to filter by
               ua.subset.rule,     // type of match rule for filtering
               &rpt,               // Must be non-null
               nullptr             // Cancel mechanism. We don't need one
    );
    otherCore.UnlockFile(otherSafe.c_str());
  }
  return status;
}

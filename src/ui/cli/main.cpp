/*
* Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
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


static void usage(char *pname)
{
  std::wstring s_pname = Utf82wstring(pname);
  std::wstring usage_str = LR"usagestring(
Usage: %PROGNAME% safe --imp[=file] --text|--xml

       %PROGNAME% safe --exp[=file] --text|--xml

       %PROGNAME% safe --create

       %PROGNAME% safe --add=field1=value1,field2=value2,...

       %PROGNAME% safe --search=<text> [--ignore-case]
                      [--subset=<Field><OP><string>[/iI] [--fields=f1,f2,..]
                      [--delete | --update=Field1=Value1,Field2=Value2,.. | --print[=field1,field2...] ] [--yes]
                      [--generate-totp]

       %PROGNAME% safe --diff=<other-safe>  [ --subset=<Field><OP><Value>[/iI] ]
                      [--fields=f1,f2,..] [--unified | --context | --sidebyside]
                      [--colwidth=column-size]

       %PROGNAME% safe --sync=<other-safe>  [ --subset=<Field><OP><string>[/iI] ] [ --fields=f1,f2,.. ] [--yes]

       %PROGNAME% safe --merge=<other-safe> [ --subset=<Field><OP><Value>[/iI] ] [--yes]

                        where OP is one of ==, !==, ^= !^=, $=, !$=, ~=, !~=
                         = => exactly similar
                         ^ => begins with
                         $ => ends with
                         ~ => contains
                         ! => negation
                        a trailing /i => case insensitive, /I => case sensitive

       Note that --passphrase <passphrase> and --passphrase2 <2nd passphrase> may be used to skip the prompt
       for the master passphrase(s). However, this should be avoided if possible for security reasons.

       Valid field names are:
       %FIELDNAMES%

       Note:
        ° Field names and values that contain whitespace characters must be quoted (e.g. "Created Time").
        ° Times are expected in Universal Time Coordinated (UTC) format ("YYYY/MM/DD hh:mm:ss").
        ° The current time can also be referenced using the keyword "now".

       Examples:

       1) Creating a database

            %PROGNAME% pwsafe.psafe3 --create

          This creates the empty database "pwsafe.psafe3".

       2) Adding an entry

          To add an entry to a database, the title is mandatory. All other fields are optional.
          If no password is specified, a password will be generated for the entry.

            %PROGNAME% pwsafe.psafe3 --add=Title="Login"

          The simplest way to add a new entry to a database, specifying the minimum required fields.

            %PROGNAME% pwsafe.psafe3 --add=Title="Bank Account",Username="John Doe",Password=SecretPassword,"Created Time"="2020/01/01 12:00:00"

          This adds an entry with the title "Bank Account" to the database "pwsafe.psafe3" which does not belong to any group.

            %PROGNAME% pwsafe.psafe3 --add=Group=Email,Title=Yahoo,Username="Richard Miles",Password=SecretPassword,"Created Time"=now

          Similar to the previous example, except that the entry is added to the "Email" group. If the group does not exist, it is created.

       3) Updating an entry

          In this example, an entry containing "Richard Miles" is searched for.

            %PROGNAME% pwsafe.psafe3 --search="Richard Miles" --update=Title=Google

          If such an entry is found, its title is updated after the user has confirmed this.

       4) Searching for an entry

          The search option is very helpful for referring to specific entries. It can search the entire database for a specific textual occurrence,
          as well as content in specific fields of entries to further narrow the search. This is particularly helpful when multiple entries have the
          same text, such as the same title.

            %PROGNAME% pwsafe.psafe3 --search="Login" --print=Title,Username

          This would output the title and username of the entries found that contain "Login".

            %PROGNAME% pwsafe.psafe3 --search="Login" --subset=Username=="John Doe" --print=Title,Username

          This search command would limit the results to all entries containing the occurrence "Login" and the username "John Doe".

       5) Deleting an entry

            %PROGNAME% pwsafe.psafe3 --search="Richard Miles" --delete

          This will delete the entry that matches the search criteria after the deletion is confirmed.

            %PROGNAME% pwsafe.psafe3 --search="John Doe" --delete --yes

          This deletes the found entry without asking for confirmation.
)usagestring";

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
  auto pos = usage_str.find(s_fn_placeholder);
  if (pos != std::string::npos) {
    usage_str.replace(pos, s_fn_placeholder.length(), s_fieldnames);
  }

  const std::wstring s_pn_placeholder{L"%PROGNAME%"};
  const auto pname_len = s_pname.length();

  for(auto itr = usage_str.find(s_pn_placeholder); itr != std::string::npos; itr = usage_str.find(s_pn_placeholder, itr + pname_len)) {
    usage_str.replace(itr, s_pn_placeholder.length(), s_pname);
  }

  wcerr << s_pname << L" version " << CLI_MAJOR_VERSION << L"." << CLI_MINOR_VERSION << L"." << CLI_REVISION << endl;
  wcerr << usage_str;
  wcerr << '\n';
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
  if (argc < 3) // must give us a safe and an operation
    return false;

  Utf82StringX(argv[1], ua.safe);

  try {

      while (1) {
          int option_index = 0;
          static constexpr struct option long_options[] = {
              // name, has_arg, flag, val
              {"import",      optional_argument,  0, 'i'},
              {"export",      optional_argument,  0, 'e'},
              {"text",        no_argument,        0, 't'},
              {"xml",         no_argument,        0, 'x'},
              {"create",      no_argument,        0, 'c'},
              //  {"new",         no_argument,        0, 'c'},
                {"search",      required_argument,  0, 's'},
                {"subset",      required_argument,  0, 'b'},
                {"fields",      required_argument,  0, 'f'},
                {"ignore-case", optional_argument,  0, 'o'},
                {"add",         required_argument,  0, 'a'},
                {"update",      required_argument,  0, 'u'},
                {"print",       optional_argument,  0, 'p'},
                //  {"remove",      no_argument,        0, 'r'},
                  {"delete",      no_argument,        0, 'r'},
                  {"clear",       required_argument,  0, 'l'},
                  {"newpass",     no_argument,        0, 'v'},
                  {"yes",         no_argument,        0, 'y'},
                  {"diff",        required_argument,  0, 'd'},
                  {"unified",     no_argument,        0, 'g'},
                  {"context",     no_argument,        0, 'j'},
                  {"sidebyside",  no_argument,        0, 'k'},
                  {"dry-run",     no_argument,        0, 'n'},
                  {"synchronize", no_argument,        0, 'z'},
                  //  {"synch",       no_argument,        0, 'z'},
                    {"merge",       required_argument,  0, 'm'},
                    {"colwidth",    required_argument,  0, 'w'},
                    {"passphrase",  required_argument,  0, 'P'},
                    {"passphrase2", required_argument,  0, 'Q'},
                    {"generate-totp", no_argument,      0, 'G'},
                    {"verbose",     no_argument,        0, 'V'},
                    {0, 0, 0, 0}
          };

#if 0 // see comment above no_dup_short_option
          static_assert(no_dup_short_option(long_options), "Short option used twice");
#endif

          int c = getopt_long(argc - 1, argv + 1, "i::e::txcs:b:f:oa:u:pryd:gjknz:m:P:Q:GV",
              long_options, &option_index);
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
              break;

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
              if (optarg && std::regex_match(optarg, std::regex("yes|true", std::regex::icase)))
                  ua.ignoreCase = true;
              break;

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

          default:
              wcerr << L"Unknown option: " << static_cast<wchar_t>(c) << endl;
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

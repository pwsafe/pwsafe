/*
 * Created by Saurav Ghosh
 * Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

#include "./safeutils.h"
#include "./safeutils-internal.h"
#include "./strutils.h"

#include "../../core/PWScore.h"
#include "../../os/file.h"
#include "../../core/core.h"

#include <iostream>
#include <unistd.h>
#include <termios.h>


using namespace std;

StringX GetPassphrase(const wstring& prompt);
// Fwd declarations:
static void echoOff();
static void echoOn();
static struct termios oldTermioFlags; // to restore tty echo

static void InitPWPolicy(PWPolicy &pwp, PWScore &core, const UserArgs::FieldUpdates &updates);

int OpenCore(PWScore& core, const StringX& safe)
{
  if (!pws_os::FileExists(safe.c_str())) {
    wcerr << safe << " - file not found" << endl;
    return 2;
  }

  StringX pk = GetPassphrase(L"Enter Password [" + stringx2std(safe) + L"]: ");

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
      wcerr << "Could not convert user " << user << " to StringX" << endl;
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

StringX GetPassphrase(const wstring& prompt)
{
    wstring wpk;
    wcerr << prompt;
    echoOff();
    wcin >> wpk;
    echoOn();
    return StringX(wpk.c_str());
}


StringX GetNewPassphrase()
{
    StringX passphrase[2];
    wstring prompt[2] = {L"Enter passphrase: ", L"Enter the same passphrase again"};

    do {
        passphrase[0] = GetPassphrase(prompt[0]);
        passphrase[1] = GetPassphrase(prompt[1]);

        if (passphrase[0] != passphrase[1]) {
            wcerr << "The two passphrases do not match. Please try again" << endl;
            continue;
        }
        if (passphrase[0].length() == 0) {
            wcerr << "Invalid passphrase. Please try again" << endl;
            continue;
        }

        break;
    }
    while(1);

    return passphrase[0];
}

static void echoOff()
{
  struct termios nflags;
  tcgetattr(fileno(stdin), &oldTermioFlags);
  nflags = oldTermioFlags;
  nflags.c_lflag &= ~ECHO;
  nflags.c_lflag |= ECHONL;

  if (tcsetattr(fileno(stdin), TCSANOW, &nflags) != 0) {
    wcerr << "Couldn't turn off echo\n";
  }
}

static void echoOn()
{
  if (tcsetattr(fileno(stdin), TCSANOW, &oldTermioFlags) != 0) {
    wcerr << "Couldn't restore echo\n";
  }
}

int AddEntryWithFields(PWScore &core, const UserArgs::FieldUpdates &fieldValues,
                      wostream &errstream)
{

  CItemData item;
  item.CreateUUID();
  int status = PWScore::SUCCESS;
  using FieldValue = UserArgs::FieldValue;

  bool got_passwd{false}, got_title{false};
  // Check if the user specified a password also
  find_if(fieldValues.begin(), fieldValues.end(),
              [&got_title, &got_passwd](const FieldValue &fv) {
    const CItemData::FieldType field{get<0>(fv)};
    got_passwd = got_passwd || (field == CItemData::PASSWORD);
    got_title  = got_title  || (field == CItemData::TITLE);
    return got_title && got_passwd;
  });

  if (!got_title) {
    errstream << L"Title must be specified for new entries" << endl;
    return PWScore::FAILURE;
  }

  if ( !got_passwd ) {
    // User didn't specify a password on command-line. Generate one
    PWPolicy pwp;
    InitPWPolicy(pwp, core, fieldValues);
    item.SetFieldValue(CItemData::PASSWORD, pwp.MakeRandomPassword());
  }

  for_each(fieldValues.begin(), fieldValues.end(), [&item](const FieldValue &fv) {
    item.SetFieldValue(get<0>(fv), get<1>(fv));
  });

  if (status == PWScore::SUCCESS)
    status = core.Execute(AddEntryCommand::Create(&core, item));

  return status;
}

int AddEntry(PWScore &core, const UserArgs &ua)
{
  return AddEntryWithFields(core, ua.fieldValues, wcerr);
}

void InitPWPolicy(PWPolicy &pwp, PWScore &core, const UserArgs::FieldUpdates &updates)
{
  auto pnitr = find_if(updates.begin(),
                       updates.end(),
                       [](const UserArgs::FieldValue &fv) {
    return get<0>(fv) == CItemData::POLICYNAME;
  });

  if (pnitr != updates.end()) {
    const StringX polname{get<1>(*pnitr)};
    if ( !core.GetPolicyFromName(polname, pwp) )
      throw std::invalid_argument("No such password policy: " + toutf8(stringx2std(polname)));
  }
  else {
    if ( InitPWPolicy(pwp, core) != PWScore::SUCCESS )
      throw std::logic_error("Error initializing default password policy");
  }
}

int InitPWPolicy(PWPolicy &pwp, PWScore &core)
{
  StringX polname;
  LoadAString(polname, IDSC_DEFAULT_POLICY);
  if ( !core.GetPolicyFromName(polname, pwp) ) {
    return PWScore::FAILURE;
  }
  return PWScore::SUCCESS;
}

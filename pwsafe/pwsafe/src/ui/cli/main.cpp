/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include <iostream>
#include <unistd.h>

#include "PWScore.h"
#include "os/file.h"
#include "core/UTF8Conv.h"

using namespace std;

static void usage(char *pname)
{
  cerr << "Usage: " << pname << " safe passkey" << endl;
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
  default: return "UNKNOWN !!!";
  }
}

int main(int argc, char *argv[])
{
  if (argc != 3) {
    usage(argv[0]);
    return 1;
  }

  PWScore core;
  StringX fname;
  CUTF8Conv conv;
  if (!conv.FromUTF8((const unsigned char *)argv[1], strlen(argv[1]), fname)) {
    cerr << "Could not convert filename " << argv[1] << " to StringX" << endl;
    return 2;
  }
  if (!pws_os::FileExists(fname.c_str())) {
    cerr << argv[1] << " - file not found" << endl;
    return 2;
  }
  StringX pk;
  if (!conv.FromUTF8((const unsigned char *)argv[2], strlen(argv[2]), pk)) {
    cerr << "Could not convert passkey " << argv[2] << " to StringX" << endl;
    return 2;
  }
  int status;
  status = core.CheckPasskey(fname, pk);
  cout << "CheckPasskey returned: " << status_text(status) << endl;
  if (status != PWScore::SUCCESS)
    goto done;
  {
    const char *user = getlogin() != NULL ? getlogin() : "unknown";
    StringX locker;
    if (!conv.FromUTF8((const unsigned char *)user, strlen(user), locker)) {
      cerr << "Could not convert user " << user << " to StringX" << endl;
      return 2;
    }
    stringT lk(locker.c_str());
    if (!core.LockFile(fname.c_str(), lk)) {
      wcout << L"Couldn't lock file " << fname
            << L": locked by " << locker << endl;
      status = -1;
      goto done;
    }
  }
  status = core.ReadFile(fname, pk);
  cout << "ReadFile returned: " << status_text(status) << endl;
  if (status != PWScore::SUCCESS)
    goto done;
  cout << argv[1] << " has " << core.GetNumEntries() << " entries" << endl;
  {
    CItemData::FieldBits bits(~0L);
    for (ItemListConstIter iter = core.GetEntryIter();
         iter != core.GetEntryEndIter(); iter++) {
      const CItemData &ci = iter->second;
      CItemData *base = ci.IsDependent() ? core.GetBaseEntry(&ci) : NULL;
      StringX text = ci.GetPlaintext('|', bits, '-', base);
      wcout << text << endl;
    }
  }
 done:
  core.UnlockFile(fname.c_str());
  return status;
}

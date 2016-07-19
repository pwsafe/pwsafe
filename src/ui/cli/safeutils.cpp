#include "./safeutils.h"
#include "./strutils.h"

#include "../../core/PWScore.h"
#include "../../os/file.h"

#include <iostream>
#include <unistd.h>
#include <termios.h>


using namespace std;

StringX GetPassphrase(const wstring& prompt);
// Fwd declarations:
static void echoOff();
static void echoOn();
static struct termios oldTermioFlags; // to restore tty echo


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


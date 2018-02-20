/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
* \file Linux-specific implementation of media.h
*/

#include "../media.h"
#include "../file.h"
#include "../utf8conv.h"

#include <cstdlib>
#include <cstdio>

using namespace std;

stringT pws_os::GetMediaType(const stringT &sfilename)
{
  /**
   * Simplest way to get the mime type of file is via file(1) command
   * popen()/pclose() is one approach, but we need to protect against
   * a filename of the form "foo;/bin/rm -rf *" (Little Bobby tables).
   * One solution is to fork/exec with dup2.
   * I think it's easier (a) to stat the file first (b) quote the
   * filename.
   */

  if (!pws_os::FileExists(sfilename))
    return _T("unknown");
  
  stringT command = _T("/usr/bin/file -b --mime-type '") + sfilename + _T("'");

  // need to convert command to char *
  size_t clen = pws_os::wcstombs(nullptr, 0, command.c_str(), command.length());
  if (clen <= 0)
    return _T("unknown");
  char *cmd = new char[clen+1];
  pws_os::wcstombs(cmd, clen, command.c_str(), command.length());

  
  FILE *pf = popen(cmd, "r");
  delete[] cmd;
  char pret[64];
  if (fgets(pret, sizeof(pret), pf) == nullptr) {
    pclose(pf);
    return _T("unknown");
  }
  pclose(pf);
  // need to convert pret to wchar_t
  wchar_t pwret[4*sizeof(pret)];
  mbstowcs(pwret, sizeof(pwret), pret, sizeof(pret));
  stringT retval(pwret);
  if (!retval.empty())
    retval.pop_back(); // get rid of pesky newline
  return retval;
}

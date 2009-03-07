/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#ifndef __RUN_H
#define __RUN_H

#include "typedefs.h"
#include "../corelib/StringX.h"

#ifdef _WIN32
typedef BOOL (*AT_PROC)(HWND);

struct st_autotype_ddl {
  AT_PROC pInit;   // Pointer to   Initialise function in pws_at(_D).dll
  AT_PROC pUnInit; // Pointer to UnInitialise function in pws_at(_D).dll
  HWND hCBWnd;     // Handle to Window to receive SendMessage for processing
                   //   It is the main DboxMain window.

  st_autotype_ddl()
    : pInit(NULL), pUnInit(NULL), hCBWnd(NULL) {}

  st_autotype_ddl(const st_autotype_ddl &that)
    : pInit(that.pInit), pUnInit(that.pUnInit),
      hCBWnd(that.hCBWnd) {}

  st_autotype_ddl &operator=(const st_autotype_ddl &that)
  {
    if (this != &that) {
      pInit = that.pInit;
      pUnInit = that.pUnInit;
      hCBWnd = that.hCBWnd;
    }
    return *this;
  }
};
#endif

namespace pws_os {
  /**
   * getruncmd return path to the command to be run based on Windows
   * runcmd    splits string into command and its parameters
   * issuecmd  executes the command (also used from LaunchBrowser)
   *
   * getruncmd uses Windows Run command search rules or Liunx equivalent
   */
  extern StringX getruncmd(const StringX &sxFile, bool &bfound);

#ifdef _WIN32
  extern bool runcmd(const StringX &execute_string, const StringX &sxAutotype,
                     const st_autotype_ddl &autotype_ddl);
  extern bool issuecmd(const StringX &sxFile, const StringX &sxParameters, 
                       const StringX &sxAutotype,
                       const st_autotype_ddl &autotype_ddl);
#else
  extern bool runcmd(const StringX &execute_string, const StringX &sxAutotype);
  extern bool issuecmd(const StringX &sxFile, const StringX &sxParameters, 
                       const StringX &sxAutotype);
#endif
};

#endif /* __RUN_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:

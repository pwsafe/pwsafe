/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * Interface for a "smart autotype" object. "Smart" in this context
 * means that we don't start simulating user keystrokes until a new
 * window has appeared and is ready to take input. This allows us
 * to "seamlessly" combine browse-to (or run) and autotype in
 * one operation.
 */

#ifndef __RUN_H
#define __RUN_H

#include "typedefs.h"
#include "../corelib/StringX.h"

struct st_run_impl; // helper structure, platform-dependant

class PWSRun {
public:
  PWSRun();
  ~PWSRun();

  bool isValid() const; // false if failed to init st_run_impl
  void Set(void *data); // set platform-dependant data
  bool UnInit(); // platform-dependant

  /**
   * getruncmd return path to the command to be run based on Windows
   * runcmd    splits string into command and its parameters
   * issuecmd  executes the command (also used from LaunchBrowser)
   *
   * getruncmd uses Windows Run command search rules or Linux equivalent
   */
  StringX getruncmd(const StringX &sxFile, bool &bfound);

  bool runcmd(const StringX &run_command, const StringX &sxAutotype);
  bool issuecmd(const StringX &sxFile, const StringX &sxParameters, 
                const StringX &sxAutotype);
private:
  st_run_impl *impl;
};

#endif /* __RUN_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:

/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * \file Linux-specific implementation of run.h - INCOMPLETE & UNTESTED
 */

#include "../run.h"
#include <wx/process.h>

#if 0
// Platform-specific implementation details
// Currently unused
struct st_run_impl {
};

static st_run_impl *run_impl = nullptr;
#endif /* 0 */

PWSRun::PWSRun()
{
}

PWSRun::~PWSRun()
{
}

bool PWSRun::isValid() const
{
  return (pImpl != nullptr);
}

void PWSRun::Set(void *) const
{
}

bool PWSRun::UnInit() // currently only needed in Windows pImpl.
{
  return true;
}

StringX PWSRun::getruncmd(const StringX &sxFile, bool &bfound) const
{
  // Stub!
  bfound = true;
  return sxFile;
}

bool PWSRun::runcmd(const StringX &runcommand, const bool &bAutotype) const
{
  UNREFERENCED_PARAMETER(bAutotype);
  if (runcommand.empty())
    return false;
  ::wxExecute(runcommand.c_str());
  return true;
}

bool PWSRun::issuecmd(const StringX &sxFile, const StringX &sxParameters, 
                      const bool &bAutotype) const
{
  UNREFERENCED_PARAMETER(sxFile);
  UNREFERENCED_PARAMETER(sxParameters);
  UNREFERENCED_PARAMETER(bAutotype);
  // Stub!
  return false;
}

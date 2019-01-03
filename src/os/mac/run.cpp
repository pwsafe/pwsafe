/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * \file MacOS-specific implementation of run.h - INCOMPLETE & UNTESTED
 */

#include "../run.h"

// Platform-specific implementation details
struct st_run_impl {
};

PWSRun::PWSRun()
{
  pImpl = new st_run_impl;
}

PWSRun::~PWSRun()
{
  delete pImpl;
}

bool PWSRun::isValid() const
{
  return (pImpl != NULL);
}

void PWSRun::Set(void *) const
{
}

bool UnInit() // currently only needed in Windows pImpl.
{
  return true;
}

StringX PWSRun::getruncmd(const StringX &sxFile, bool &bfound) const
{
  // Stub!
  bfound = true;
  return sxFile;
}

bool PWSRun::runcmd(const StringX &, const bool &) const
{
  // Stub!
  return false;
}

bool PWSRun::issuecmd(const StringX &, const StringX &,
                      const bool &) const
{
  // Stub!
  return false;
}

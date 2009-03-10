/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * \file Linux-specific implementation of run.h - INCOMPLETE & UNTESTED
 */

#include "../run.h"

// Platform-specific implementation details
struct st_run_impl {
};

PWSRun::PWSRun()
{
  impl = new st_run_impl;
}

PWSRun::~PWSRun()
{
  delete impl;
}

bool PWSRun::isValid() const
{
  return (impl != NULL);
}

void PWSRun::Set(void *)
{
}

bool UnInit() // currently only needed in Windows impl.
{
  return true;
}

StringX PWSRun::getruncmd(const StringX &sxFile, bool &bfound)
{
	// Stub!
  bfound = true;
	return sxFile;
}

bool PWSRun::runcmd(const StringX &execute_string, const StringX &sxAutotype)
{
	// Stub!
	return false;
}

bool PWSRun::issuecmd(const StringX &sxFile, const StringX &sxParameters, 
                      const StringX &sxAutotype)
{
	// Stub!
	return false;
}

/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * \file Linux-specific implementation of run.h - DUMMY!
 */

#include "../run.h"

StringX pws_os::getruncmd(const StringX &sxFile, bool &bfound)
{
	// Stub!
  bfound = true;
	return sxFile;
}

bool pws_os::runcmd(const StringX &execute_string, const StringX &sxAutotype)
{
	// Stub!
	return false;
}

bool pws_os::issuecmd(const StringX &sxFile, const StringX &sxParameters, 
                      const StringX &sxAutotype)
{
	// Stub!
	return false;
}

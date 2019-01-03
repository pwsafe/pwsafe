/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * \file Windows-specific implementation of sleep.h
 */

#include "../typedefs.h"
#include "../sleep.h"

void pws_os::sleep_ms(unsigned int milliseconds)
{
  ::Sleep(milliseconds);
}

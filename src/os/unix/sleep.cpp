/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * \file Linux-specific implementation of sleep.h
 */

#include <errno.h>
#include <time.h>
#include "../sleep.h"

void pws_os::sleep_ms(unsigned int milliseconds)
{
  struct timespec tv;
  /* Construct the timespec from the number of whole seconds...  */
  tv.tv_sec = static_cast<time_t>(milliseconds / 1000);
  /* ... and the remainder in nanoseconds.  */
  tv.tv_nsec = static_cast<long>((milliseconds % 1000) * 1000000);

  while (1) {
    /* Sleep for the time specified in tv.  If interrupted by a
       signal, place the remaining time left to sleep back into tv.  */
    int rval = nanosleep (&tv, &tv);
    if (rval == 0) /* Completed the entire sleep time; all done.  */
      break;
    else if (errno == EINTR) /* Interrupted by a signal.  Try again.  */
      continue;
    else /* Some other error; bail out.  */
      break;
  }
}

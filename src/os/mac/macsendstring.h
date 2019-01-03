/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/*
 * macsendstring.h - interface for sending a given string as keyboard input
 * to the mac application in focus.
 *
 * Has been tested only os OS X 10.6.3 in the initial incarnation
 *
 */

#ifndef __XSENDSTRING_H__
#define __XSENDSTRING_H__

namespace pws_os {
  void SendString(const char* str, unsigned delayMS);
  bool MacSimulateApplicationSwitch(unsigned delayMS);
  bool SelectAll();
};
#endif

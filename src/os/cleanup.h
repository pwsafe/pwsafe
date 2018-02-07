/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#ifndef _OSCLEANUP_H
#define _OSCLEANUP_H


namespace pws_os {
  typedef void (*handler_t)(int, void *);
  void install_cleanup_handler(handler_t h, void *p);
  void uninstall_cleanup_handler();
}

#endif /* _OSCLEANUP_H */

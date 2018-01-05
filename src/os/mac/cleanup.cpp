/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "../cleanup.h"
#include <signal.h>
#include <iostream>

namespace {
  const int siglist[] = {SIGINT, SIGQUIT, SIGILL, SIGSEGV, SIGTERM, SIGBUS};
  
  void *closure = nullptr;
  pws_os::handler_t handler = nullptr;

  extern "C" void internal_handler( int s)
  {
    handler(s, closure);
  }
}

void pws_os::install_cleanup_handler(pws_os::handler_t h, void *p)
{
  int status;
  struct sigaction act;
  
  closure = p;
  handler = h;

  act.sa_handler = internal_handler;
  act.sa_flags = 0;
  sigemptyset(&act.sa_mask);
  for (int i = 0; i < sizeof(siglist)/sizeof(siglist[0]); i++)
    sigaction(siglist[i], &act, NULL);
}

void pws_os::uninstall_cleanup_handler()
{
  int status;
  struct sigaction act;
  
  closure = nullptr;
  handler = nullptr;

  act.sa_handler = SIG_DFL;
  act.sa_flags = 0;
  sigemptyset(&act.sa_mask);
  for (int i = 0; i < sizeof(siglist)/sizeof(siglist[0]); i++)
    sigaction(siglist[i], &act, NULL);
}

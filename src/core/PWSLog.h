/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#ifndef _PWSLOG_H
#define _PWSLOG_H

#include "../os/typedefs.h"

#include <deque>

class PWSLog
{
public:
  PWSLog() {}
  virtual ~PWSLog() {}

  static PWSLog *GetLog(); // singleton
  static void DeleteLog();
  
  void Add(const stringT &sLogRecord);
  stringT DumpLog();

private:
  static PWSLog *self;
  std::deque<stringT> m_log;
};

#endif /* _PWSLOG_H */

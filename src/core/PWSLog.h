/*
* Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#ifndef _PWSLOG_H
#define _PWSLOG_H

#include "os/typedefs.h"
#include "os/logit.h"

#include <deque>

#define PWS_LOGIT_CONCAT(str) PWS_LOGIT_HEADER L ## str

// Now the actual logging macros
#define PWS_LOGIT PWSLog::GetLog()->Add(pws_os::Logit(PWS_LOGIT_HEADER, __FILE__, __FUNCTION__))
#define PWS_LOGIT_ARGS0(str) PWSLog::GetLog()->Add(pws_os::Logit(PWS_LOGIT_CONCAT(str), \
                                                   __FILE__, __FUNCTION__))
#define PWS_LOGIT_ARGS(format_str, ...) PWSLog::GetLog()->Add(pws_os::Logit(PWS_LOGIT_CONCAT(format_str), \
                                                              __FILE__, __FUNCTION__, __VA_ARGS__))


class PWSLog
{
public:
  virtual ~PWSLog() {}

  static PWSLog *GetLog(); // singleton
  static void DeleteLog();
  
  void Add(const stringT &sLogRecord);
  stringT DumpLog() const;

private:
  PWSLog() {}
  static PWSLog *self;
  std::deque<stringT> m_log;
};

#endif /* _PWSLOG_H */

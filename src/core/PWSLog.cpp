/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "PWSLog.h"
#include "Util.h"
#include "StringXStream.h"

using namespace std;

#define NUM_LOG_ENTRIES 256

PWSLog *PWSLog::self = nullptr;

PWSLog *PWSLog::GetLog()
{
  if (self == nullptr) {
    self = new PWSLog();
    // The following sets the queue size once, avoiding
    // memory management @ each call to Add(). (reference TBD)
    self->m_log.resize(NUM_LOG_ENTRIES, _T(" "));
  }
  return self;
}

void PWSLog::DeleteLog()
{
  delete self;
  self = nullptr;
}

void PWSLog::Add(const stringT &sLogRecord)
{
  static const stringT sb(_T(" "));
  stringT sTimeStamp;
  PWSUtil::GetTimeStamp(sTimeStamp);

  // m_log preloaded, so pop_fornt is always valid (see GetLog).
  m_log.pop_front();
  m_log.push_back(sTimeStamp + sb + sLogRecord);
}

stringT PWSLog::DumpLog()
{
  const TCHAR *sHeader = _T("US04 ");
  ostringstreamT stLog;

  // Start with header for Userstream
  stLog << sHeader;

  // Then total number of records
  stLog << m_log.size() << _T(" ");

  deque<stringT>::const_reverse_iterator criter;
  deque<stringT>::const_reverse_iterator crend = m_log.rend();

  // Now add records - last first
  for (criter = m_log.rbegin(); criter != crend; criter++) {
    if (criter->length() > 1) {
      // First add record length, then record
      stLog << criter->length() << _T(" ");
      stLog << criter->c_str() << _T(" ");
    }
  }

  return stLog.str();
}

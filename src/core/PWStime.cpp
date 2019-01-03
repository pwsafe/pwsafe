/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/** \file
* Implementation of PWStime
*/

#include <ctime>
#include <cstring>
#include <algorithm>

#include "PWStime.h"
#include "Util.h"

using namespace std;

// Pesky WinDef.h...
#ifdef min
#undef min
#endif

PWStime::PWStime()
{
  setTime(std::time(nullptr));
}

PWStime::PWStime(const PWStime &pwt)
{
  memcpy(m_time, pwt.m_time, sizeof(m_time));
}

PWStime::PWStime(std::time_t t)
{
  setTime(t);
}

PWStime::PWStime(const unsigned char *pbuf)
{
  memcpy(m_time, pbuf, TIME_LEN);
}

PWStime::~PWStime()
{
}

PWStime &PWStime::operator=(const PWStime &that)
{
  if (this != &that) 
    memcpy(m_time, that.m_time, sizeof(m_time));
  return *this;

}

PWStime &PWStime::operator=(std::time_t t)
{
  setTime(t);
  return *this;
}

PWStime::operator time_t() const
{
  unsigned char ta[sizeof(time_t)] = {0};
  memcpy(ta, m_time, std::min(sizeof(m_time), sizeof(ta)));
  return getInt<time_t>(ta);
}

PWStime::operator const unsigned char *() const
{
  return m_time;
}

PWStime::operator const char *() const
{
  return reinterpret_cast<const char *>(m_time);
}

void PWStime::setTime(time_t t)
{
  unsigned char ta[sizeof(t)] = {0};
  memset(m_time, 0, sizeof(m_time)); // needed if sizeof(t) < TIME_LEN
  putInt(ta, t);
  memcpy(m_time, ta, std::min(sizeof(m_time), sizeof(ta)));
}

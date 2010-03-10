/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#include "../KeySend.h"
#include "../sleep.h"
#include "./xsendstring.h"
#include "../../corelib/Util.h"

CKeySend::CKeySend() : m_delayMS(10) //default delay after each keystroke is 10 ms
{
}

CKeySend::~CKeySend()
{
}

void CKeySend::SendString(const StringX &data)
{
  unsigned char* str = 0;
  int len = 0;
  ConvertString(data, str, len);
  if (len && str && str[0])
    ::SendString(reinterpret_cast<const char*>(str), ATMETHOD_AUTO, m_delayMS);
  delete [] str;
}

void CKeySend::SetDelay(unsigned d)
{
  m_delayMS = d;
}

// SetAndDelay allows users to put \d500\d10 in autotype and
// then it will cause a delay of half a second then subsequent
// key stokes will be delayed by 10 ms 
// thedavecollins 2004-08-05

void CKeySend::SetAndDelay(unsigned d) {
  SetDelay(d);
  pws_os::sleep_ms(m_delayMS);
}

//it doesn't matter in X what the CAPSLOCK state is
void CKeySend::SetCapsLock(const bool /*bState*/)
{
}

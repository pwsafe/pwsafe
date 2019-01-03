/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// UTF8Conv.h
//-----------------------------------------------------------------------------
#ifndef __UTF8CONV_H
#define __UTF8CONV_H

#include "StringX.h"

/** \file
* A utility class to convert between UTF-8 and StringX
*/

class CUTF8Conv
{
public:
  CUTF8Conv(bool cp_acp=false) : m_utf8(nullptr), m_utf8Len(0), m_utf8MaxLen(0),
                                 m_wc(nullptr), m_wcMaxLen(0), m_tmp(nullptr),
                                 m_tmpMaxLen(0), m_cp_acp(cp_acp) {}
  ~CUTF8Conv();
  CUTF8Conv(const CUTF8Conv &) = delete;
  CUTF8Conv &operator=(const CUTF8Conv &) = delete;
  // In following, char * is managed internally. Caller must NOT
  // allocate or deallocate it!
  bool ToUTF8(const StringX &data, const unsigned char *&utf8, size_t &utf8Len);
  // In following, char * is managed by caller.
  bool FromUTF8(const unsigned char *utf8, size_t utf8Len, StringX &data);

private:
  // following pointers allocated dynamically and monotonically increase in size
  // for efficiency w/o arbitrary restrictions
  // deallocated by d'tor
  unsigned char *m_utf8;
  size_t m_utf8Len;
  size_t m_utf8MaxLen;
  wchar_t *m_wc;
  size_t m_wcMaxLen;
  unsigned char *m_tmp;
  size_t m_tmpMaxLen;
  bool m_cp_acp; // if set, FromUTF8 uses CP_ACP, instead UTF8 encoding
};

#endif /* __UTF8CONV_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:

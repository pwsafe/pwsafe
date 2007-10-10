/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */
// UTF8Conv.h
//-----------------------------------------------------------------------------
#ifndef __UTF8CONV_H
#define __UTF8CONV_H
#include "MyString.h"

/** \file
 * A utility class to convert between UTF-8 and CMyString
 */

class CUTF8Conv {
 public:
 CUTF8Conv() : m_utf8(NULL), m_utf8Len(0), m_utf8MaxLen(0),
    m_wc(NULL), m_wcMaxLen(0), m_tmp(NULL), m_tmpMaxLen(0) {}
  ~CUTF8Conv();
  // In following, char * is managed internally. Caller must NOT
  // allocate or deallocate it!
  bool ToUTF8(const CMyString &data, const unsigned char *&utf8, int &utf8Len);
  // In following, char * is managed by caller.
  bool FromUTF8(const unsigned char *utf8, int utf8Len, CMyString &data);
 private:
  CUTF8Conv(const CUTF8Conv &); // not supported
  CUTF8Conv &operator=(const CUTF8Conv &); // ditto
  // following pointers allocated dynamically and monotically increase in size
  // for efficiency w/o arbitrary restrictions
  // deallocated by d'tor
  unsigned char *m_utf8;
  int m_utf8Len;
  int m_utf8MaxLen;
  wchar_t *m_wc;
  int m_wcMaxLen;
  unsigned char *m_tmp;
  int m_tmpMaxLen;

};

#endif /* __UTF8CONV_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:

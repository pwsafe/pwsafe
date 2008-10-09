/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/** \file
* Implementation of CUTF8Conv
*/
#include "UTF8Conv.h"
#include "Util.h"
#include "os/utf8conv.h"

CUTF8Conv::~CUTF8Conv()
{
  if (m_utf8 != NULL) {
    trashMemory(m_utf8, m_utf8MaxLen * sizeof(m_utf8[0]));
    delete[] m_utf8;
  }
  if (m_wc != NULL) {
    trashMemory(m_wc, m_wcMaxLen);
    delete[] m_wc;
  }
  if (m_tmp != NULL) {
    trashMemory(m_tmp, m_tmpMaxLen * sizeof(m_tmp[0]));
    delete[] m_tmp;
  }
}

bool CUTF8Conv::ToUTF8(const StringX &data,
                       const unsigned char *&utf8, int &utf8Len)
{
  // If we're not in Unicode, call MultiByteToWideChar to get from
  // current codepage to Unicode, and then WideCharToMultiByte to
  // get to UTF-8 encoding.

  if (data.empty()) {
    utf8Len = 0;
    return true;
  }
  wchar_t *wcPtr; // to hide UNICODE differences
  size_t wcLen; // number of wide chars needed
#ifndef UNICODE
  // first get needed wide char buffer size
  wcLen = pws_os::mbstowcs(NULL, 0, data.c_str(), size_t(-1), false);
  if (wcLen == 0) { // uh-oh
    ASSERT(0);
    m_utf8Len = 0;
    return false;
  }
  // Allocate buffer (if previous allocation was smaller)
  if (wcLen > m_wcMaxLen) {
    if (m_wc != NULL)
      trashMemory(m_wc, m_wcMaxLen * sizeof(m_wc[0]));
    delete[] m_wc;
    m_wc = new wchar_t[wcLen];
    m_wcMaxLen = wcLen;
  }
  // next translate to buffer
  wcLen = pws_os::mbstowcs(m_wc, wcLen, data.c_str(), size_t(-1), false);
  ASSERT(wcLen != 0);
  wcPtr = m_wc;
#else
  wcPtr = const_cast<wchar_t *>(data.c_str());
  wcLen = data.length()+1;
#endif
  // first get needed utf8 buffer size
  size_t mbLen = pws_os::wcstombs(NULL, 0, wcPtr, size_t(-1));

  if (mbLen == 0) { // uh-oh
    ASSERT(0);
    m_utf8Len = 0;
    return false;
  }
  // Allocate buffer (if previous allocation was smaller)
  if (mbLen > m_utf8MaxLen) {
    if (m_utf8 != NULL)
      trashMemory(m_utf8, m_utf8MaxLen);
    delete[] m_utf8;
    m_utf8 = new unsigned char[mbLen];
    m_utf8MaxLen = mbLen;
  }
  // Finally get result
  m_utf8Len = pws_os::wcstombs((char *)m_utf8, mbLen, wcPtr, wcLen);
  ASSERT(m_utf8Len != 0);
  m_utf8Len--; // remove unneeded null termination
  utf8 = m_utf8;
  utf8Len = m_utf8Len;
  return true;
}

// In following, char * is managed by caller.
bool CUTF8Conv::FromUTF8(const unsigned char *utf8, int utf8Len,
                         StringX &data)
{
  // Call MultiByteToWideChar to get from UTF-8 to Unicode.
  // If we're not in Unicode, call WideCharToMultiByte to
  // get to current codepage.
  //
  // Due to a bug in pre-3.08 versions, data may be in ACP
  // instead of utf-8. We try to detect and workaround this.

  if (utf8Len == 0 || (utf8Len == 1 && utf8[0] == '\0')) {
    data = _T("");
    return true;
  }

  ASSERT(utf8 != NULL);

  // first get needed wide char buffer size
  size_t wcLen = pws_os::mbstowcs(NULL, 0, (char *)utf8, size_t(-1));
  if (wcLen == 0) { // uh-oh
    // it seems that this always returns non-zero, even if encoding
    // broken. Therefore, we'll give a consrevative value here,
    // and try to recover later
    TRACE(_T("FromUTF8: Couldn't get buffer size - guessing!"));
    wcLen = 3 * utf8Len;
  }
  // Allocate buffer (if previous allocation was smaller)
  if (wcLen > m_wcMaxLen) {
    if (m_wc != NULL)
      trashMemory(m_wc, m_wcMaxLen);
    delete[] m_wc;
    m_wc = new wchar_t[wcLen];
    m_wcMaxLen = wcLen;
  }
  // next translate to buffer
  wcLen = pws_os::mbstowcs(m_wc, wcLen, (char *)utf8, size_t(-1));
#ifdef _WIN32
  if (wcLen == 0) {
    DWORD errCode = GetLastError();
    switch (errCode) {
    case ERROR_INSUFFICIENT_BUFFER:
      TRACE("INSUFFICIENT BUFFER"); break;
    case ERROR_INVALID_FLAGS:
      TRACE("INVALID FLAGS"); break;
    case ERROR_INVALID_PARAMETER:
      TRACE("INVALID PARAMETER"); break;
    case ERROR_NO_UNICODE_TRANSLATION:
      // try to recover
      TRACE("NO UNICODE TRANSLATION");
      wcLen = MultiByteToWideChar(CP_ACP,        // code page
                                  0,             // character-type options
                                  LPSTR(utf8),   // string to map
                                  -1,            // -1 means null-terminated
                                  m_wc, wcLen);  // output buffer
      if (wcLen > 0) {
        TRACE(_T("FromUTF8: recovery succeeded!"));
      }
      break;
    default:
      ASSERT(0);
    }
  }
#endif /* _WIN32 */
  ASSERT(wcLen != 0);
#ifdef UNICODE
  if (wcLen != 0) {
    m_wc[wcLen-1] = TCHAR('\0');
    data = m_wc;
    return true;
  } else
    return false;
#else /* Go from Unicode to Locale encoding */
  // first get needed utf8 buffer size
  size_t mbLen = pws_os::wcstombs(NULL, 0, m_wc, size_t(-1), false);
  if (mbLen == 0) { // uh-oh
    ASSERT(0);
    data = _T("");
    return false;
  }
  // Allocate buffer (if previous allocation was smaller)
  if (mbLen > m_tmpMaxLen) {
    if (m_tmp != NULL)
      trashMemory(m_tmp, m_tmpMaxLen);
    delete[] m_tmp;
    m_tmp = new unsigned char[mbLen];
    m_tmpMaxLen = mbLen;
  }
  // Finally get result
  size_t tmplen = pws_os::wcstombs((char *)m_tmp, mbLen, m_wc, size_t(-1), false);
  ASSERT(tmpLen == mbLen);
  m_tmp[mbLen-1] = '\0'; // char, no need to _T()...
  data = (char *)m_tmp;
  ASSERT(!data.empty());
  return true;
#endif /* !UNICODE */
}

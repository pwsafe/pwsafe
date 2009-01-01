/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/** \file
* Implementation file for the PWSclipboard class
*/

#include <afxole.h>
#include "PWSclipboard.h"
#include "util.h"

static CLIPFORMAT CF_CLIPBOARD_VIEWER_IGNORE;

PWSclipboard::PWSclipboard()
  : m_set(false)
{
  memset(m_digest, 0, sizeof(m_digest));

  // Spelling counts - must be exact!
  CF_CLIPBOARD_VIEWER_IGNORE = (CLIPFORMAT)RegisterClipboardFormat(_T("Clipboard Viewer Ignore"));
}

PWSclipboard::~PWSclipboard()
{
  // ClearData(); - current application behaviour allows user to keep
  // data after application exit. 
}

bool PWSclipboard::SetData(const StringX &data, bool isSensitive, CLIPFORMAT cfFormat)
{
  // Dummy data
  HGLOBAL hDummyGlobalMemory = ::GlobalAlloc(GMEM_MOVEABLE, 2 * sizeof(TCHAR));
  LPTSTR pDummyGlobalLock = (LPTSTR)::GlobalLock(hDummyGlobalMemory);

  PWSUtil::strCopy(pDummyGlobalLock, 2, _T("\0") , 1);
  ::GlobalUnlock(hDummyGlobalMemory);

  // Real data
  unsigned int uGlobalMemSize = (data.length() + 1) * sizeof(TCHAR);
  HGLOBAL hGlobalMemory = ::GlobalAlloc(GMEM_MOVEABLE, uGlobalMemSize);
  LPTSTR pGlobalLock = (LPTSTR)::GlobalLock(hGlobalMemory);

  PWSUtil::strCopy(pGlobalLock, data.length() + 1, data.c_str(), data.length());
  ::GlobalUnlock(hGlobalMemory);

  COleDataSource *pods = new COleDataSource; // deleted automagically
  pods->CacheGlobalData(CF_CLIPBOARD_VIEWER_IGNORE, hDummyGlobalMemory);
  pods->CacheGlobalData(cfFormat, hGlobalMemory);
  pods->SetClipboard();
  m_set = isSensitive; // don't set if !isSensitive, so won't be cleared
  if (m_set) {
    // identify data in clipboard as ours, so as not to clear the wrong data later
    // of course, we don't want an extra copy of a password floating around
    // in memory, so we'll use the hash
    SHA256 ctx;
    const TCHAR *str = data.c_str();
    ctx.Update((const unsigned char *)str, data.length()*sizeof(TCHAR));
    ctx.Final(m_digest);
  }
  return m_set;
}

bool PWSclipboard::ClearData()
{
  if (m_set) {
    COleDataObject odo;
    StringX data;
    odo.AttachClipboard();
    HANDLE hData = odo.GetGlobalData(CLIPBOARD_TEXT_FORMAT);
    if (hData != NULL) {
      LPCTSTR pData = (LPCTSTR)::GlobalLock(hData);
      SIZE_T dwlength = ::GlobalSize(hData) - sizeof(TCHAR); // less trailing null
      if (dwlength < 1)
        return !m_set;

      // check if the data on the clipboard is the same we put there
      unsigned char digest[SHA256::HASHLEN];
      SHA256 ctx;
      ctx.Update((unsigned char *)pData, dwlength);
      ctx.Final(digest);
      if (memcmp(digest, m_digest, SHA256::HASHLEN) == 0) {
        trashMemory((void *)pData, dwlength);
        StringX blank(_T(""));
        SetData(blank, false);
        memset(m_digest, '\0', SHA256::HASHLEN);
      }
      ::GlobalUnlock(hData);
      ::GlobalFree(hData);
    }
  }
  return !m_set;
}

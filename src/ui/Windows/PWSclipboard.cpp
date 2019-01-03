/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
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
#include "core/util.h"

static CLIPFORMAT CF_CLIPBOARD_VIEWER_IGNORE;

PWSclipboard::PWSclipboard()
  : m_set(false)
{
  memset(m_digest, 0, sizeof(m_digest));

  // Spelling counts - must be exact!
  CF_CLIPBOARD_VIEWER_IGNORE = (CLIPFORMAT)RegisterClipboardFormat(L"Clipboard Viewer Ignore");
}

PWSclipboard::~PWSclipboard()
{
  // ClearCPData(); - current application behaviour allows user to keep
  // data after application exit. 
}

bool PWSclipboard::SetData(const StringX &data, bool isSensitive, CLIPFORMAT cfFormat)
{
  // Dummy data
  HGLOBAL hDummyGlobalMemory = ::GlobalAlloc(GMEM_MOVEABLE, 2 * sizeof(wchar_t));
  LPTSTR pDummyGlobalLock = (LPTSTR)::GlobalLock(hDummyGlobalMemory);

  PWSUtil::strCopy(pDummyGlobalLock, 2, L"\0" , 1);
  ::GlobalUnlock(hDummyGlobalMemory);

  // Real data
  size_t uGlobalMemSize = (data.length() + 1) * sizeof(wchar_t);
  HGLOBAL hGlobalMemory = ::GlobalAlloc(GMEM_MOVEABLE, uGlobalMemSize);
  LPTSTR pGlobalLock = (LPTSTR)::GlobalLock(hGlobalMemory);

  PWSUtil::strCopy(pGlobalLock, data.length() + 1, data.c_str(), data.length());
  ::GlobalUnlock(hGlobalMemory);

  COleDataSource *pods = new COleDataSource; // deleted automagically by SetClipboard below
  pods->CacheGlobalData(CF_CLIPBOARD_VIEWER_IGNORE, hDummyGlobalMemory);
  pods->CacheGlobalData(cfFormat, hGlobalMemory);
  pods->SetClipboard();
  pods = NULL; // As deleted by SetClipboard above

  m_set = isSensitive; // don't set if !isSensitive, so won't be cleared
  if (m_set) {
    // identify data in clipboard as ours, so as not to clear the wrong data later
    // of course, we don't want an extra copy of a password floating around
    // in memory, so we'll use the hash
    SHA256 ctx;
    const wchar_t *str = data.c_str();
    ctx.Update((const unsigned char *)str, data.length() * sizeof(wchar_t));
    ctx.Final(m_digest);
  }
  return m_set;
}

bool PWSclipboard::ClearCBData()
{
  if (m_set) {
    COleDataObject odo;
    StringX data;
    odo.AttachClipboard();
    HANDLE hData = odo.GetGlobalData(CLIPBOARD_TEXT_FORMAT);
    if (hData != NULL) {
      LPCTSTR pData = (LPCTSTR)::GlobalLock(hData);
      SIZE_T dwlength = ::GlobalSize(hData) - sizeof(wchar_t); // less trailing null
      if (dwlength < 1)
        return !m_set;

      // check if the data on the clipboard is the same we put there
      unsigned char digest[SHA256::HASHLEN];
      SHA256 ctx;
      ctx.Update((unsigned char *)pData, dwlength);
      ctx.Final(digest);
      if (memcmp(digest, m_digest, SHA256::HASHLEN) == 0) {
        trashMemory((void *)pData, dwlength);
        StringX blank(L"");
        SetData(blank, false);
        memset(m_digest, 0, SHA256::HASHLEN);
      }
      ::GlobalUnlock(hData);
      ::GlobalFree(hData);
    }
  }
  return !m_set;
}

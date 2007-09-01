/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */

/** \file
 * Implementation file for the PWSclipboard class
 */

#include <afxole.h>
#include "PWSclipboard.h"
#include "util.h"

PWSclipboard::PWSclipboard()
  : m_set(false)
{
  memset(m_digest, 0, sizeof(m_digest));
}

PWSclipboard::~PWSclipboard()
{
  // ClearData(); - current application behaviour allows user to keep
  // data after application exit. 
}

bool
PWSclipboard::SetData(const CMyString &data, bool isSensitive, CLIPFORMAT cfFormat)
{
  unsigned int uGlobalMemSize = (data.GetLength() + 1) * sizeof(TCHAR);
  HGLOBAL hGlobalMemory = ::GlobalAlloc(GMEM_MOVEABLE, uGlobalMemSize);
  LPTSTR pGlobalLock = (LPTSTR)::GlobalLock(hGlobalMemory);

  PWSUtil::strCopy(pGlobalLock, data.GetLength() + 1, data ,data.GetLength());
  ::GlobalUnlock(hGlobalMemory);

  COleDataSource *pods = new COleDataSource; // deleted automagically
  pods->CacheGlobalData(cfFormat, hGlobalMemory);
  pods->SetClipboard();
  m_set = isSensitive; // don't set if !isSensitive, so won't be cleared
  if (m_set) {
    // identify data in clipboard as ours, so as not to clear the wrong data later
    // of course, we don't want an extra copy of a password floating around
    // in memory, so we'll use the hash
    SHA256 ctx;
    const TCHAR *str = (const TCHAR *)data;
    ctx.Update((const unsigned char *)str, data.GetLength()*sizeof(TCHAR));
    ctx.Final(m_digest);
  }
  return m_set;
}

bool
PWSclipboard::ClearData()
{
  if (m_set) {
    COleDataObject odo;
    CMyString data;
    odo.AttachClipboard();
    HANDLE hData = odo.GetGlobalData(CLIPBOARD_TEXT_FORMAT);
    if (hData != NULL) {
      LPCTSTR pData = (LPCTSTR)::GlobalLock(hData);
      DWORD dwlength =  ::GlobalSize(hData) - sizeof(TCHAR); // less trailing null
      if (dwlength < 1)
        return !m_set;

      // check if the data on the clipboard is the same we put there
      unsigned char digest[SHA256::HASHLEN];
      SHA256 ctx;
      ctx.Update((unsigned char *)pData, dwlength);
      ctx.Final(digest);
      if (memcmp(digest, m_digest, SHA256::HASHLEN) == 0) {
        trashMemory((void *)pData, dwlength);
        CMyString blank(_T(""));
        SetData(blank, false);
        memset(m_digest, '\0', SHA256::HASHLEN);
      }
      ::GlobalUnlock(hData);
      ::GlobalFree(hData);
    }
  }
  return !m_set;
}

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
PWSclipboard::SetData(const CMyString &data, bool isSensitive)
{
  unsigned int uGlobalMemSize = (data.GetLength() + 1) * sizeof(TCHAR);
  HGLOBAL hGlobalMemory = GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE,
                                      uGlobalMemSize);
  LPTSTR pGlobalLock = (LPTSTR)GlobalLock(hGlobalMemory);

  PWSUtil::strCopy(pGlobalLock, data.GetLength() + 1, data ,data.GetLength());

  GlobalUnlock(hGlobalMemory);

  m_set = false;
  if (OpenClipboard(AfxGetMainWnd()->GetSafeHwnd()) == TRUE) {
    if (EmptyClipboard() != TRUE) {
      TRACE("The clipboard was not emptied correctly");
    }
    if (SetClipboardData(
                         CLIPBOARD_TEXT_FORMAT,
                         hGlobalMemory) == NULL) {
      TRACE("The data was not pasted into the clipboard correctly");
      GlobalFree(hGlobalMemory); // wasn't passed to Clipboard
    } else {
      // identify data in clipboard as ours, so as not to clear the wrong data later
      // of course, we don't want an extra copy of a password floating around
      // in memory, so we'll use the hash
      const TCHAR *str = (const TCHAR *)data;
      SHA256 ctx;
      ctx.Update((const unsigned char *)str, data.GetLength());
      ctx.Final(m_digest);
      m_set = isSensitive; // don't set if !isSensitive, so won't be cleared
    }
    if (CloseClipboard() != TRUE) {
      TRACE("The clipboard could not be closed");
    }
  } else {
    TRACE("The clipboard could not be opened correctly");
    GlobalFree(hGlobalMemory); // wasn't passed to Clipboard
  }
  return m_set;
}

bool
PWSclipboard::ClearData()
{
  if (m_set) {
    if (OpenClipboard(AfxGetMainWnd()->GetSafeHwnd()) != TRUE) {
      TRACE("The clipboard could not be opened correctly");
      return false;
    }
    
    if (IsClipboardFormatAvailable(CLIPBOARD_TEXT_FORMAT) != 0) {
      HGLOBAL hglb = GetClipboardData(CLIPBOARD_TEXT_FORMAT);
      if (hglb != NULL) {
        LPTSTR lptstr = (LPTSTR)GlobalLock(hglb);
        if (lptstr != NULL) {
          // check identity of data in clipboard
          unsigned char digest[SHA256::HASHLEN];
          SHA256 ctx;
          ctx.Update((const unsigned char *)lptstr, PWSUtil::strLength(lptstr));
          ctx.Final(digest);
          if (memcmp(digest, m_digest, SHA256::HASHLEN) == 0) {
            trashMemory( lptstr, PWSUtil::strLength(lptstr));
            GlobalUnlock(hglb);
            if (EmptyClipboard() == TRUE) {
              memset(m_digest, '\0', SHA256::HASHLEN);
              m_set = false;
            } else {
              TRACE("The clipboard was not emptied correctly");
            }
          } else { // hashes match
            GlobalUnlock(hglb);
          }
        } // lptstr != NULL
      } // hglb != NULL
    } // IsClipboardFormatAvailable
    if (CloseClipboard() != TRUE) {
      TRACE("The clipboard could not be closed");
    }
  }
  return !m_set;
}

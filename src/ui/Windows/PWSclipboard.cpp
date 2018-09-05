/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
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

class PWSOleDataSource : public COleDataSource
{
public:
  PWSOleDataSource(HGLOBAL hg, size_t hgLen, bool isSensitive) : COleDataSource()
  {
    m_isSensitive = isSensitive;
    s_hg = hg;
    s_hgLen = hgLen;
  }
  virtual BOOL OnRenderGlobalData(LPFORMATETC,  HGLOBAL* phGlobal)
  {
    if (s_hg != NULL) {
      *phGlobal = s_hg;
#ifdef CLEAR_CLIPBOARD_ON_TIMER /* disable for now, as problematic */
      if (m_isSensitive) {
        // zap clipboard 30 seconds after paste:
        AfxGetMainWnd()->SetTimer(TIMER_ID, 30000, timerCallback);
      }
#endif
      return TRUE;
    } else
      return FALSE;
  }
  ~PWSOleDataSource()
  {
    AfxGetMainWnd()->KillTimer(TIMER_ID);
  }
private:
  bool m_isSensitive;
  static HGLOBAL s_hg;
  static size_t s_hgLen;
  static const UINT TIMER_ID = 576;
  static void CALLBACK timerCallback(HWND, UINT, UINT_PTR, DWORD);
};

HGLOBAL PWSOleDataSource::s_hg;
size_t PWSOleDataSource::s_hgLen;
void PWSOleDataSource::timerCallback(HWND, UINT, UINT_PTR, DWORD)
{
  if (s_hg != NULL) {
    LPCTSTR pData = (LPCTSTR)::GlobalLock(s_hg);
    if (pData) {
      trashMemory((void *)pData, s_hgLen);
      ::GlobalUnlock(s_hg);
      ::GlobalFree(s_hg);

      s_hg = NULL; s_hgLen = 0;
      StringX blank(L"");
      PWSclipboard cb;
      cb.SetData(blank, false);
    }
    else { // already freed by OLE subsys
      s_hg = NULL; s_hgLen = 0;
      pws_os::IssueError(L"Clipboard callback: lock failed", false);
    }
  }
  AfxGetMainWnd()->KillTimer(TIMER_ID);
}


bool PWSclipboard::SetData(const StringX &data, bool isSensitive)
{
  // Dummy data
  const size_t uDummyLen = 2;
  HGLOBAL hDummyGlobalMemory = ::GlobalAlloc(GMEM_MOVEABLE, uDummyLen * sizeof(wchar_t));
  LPTSTR pDummyGlobalLock = (LPTSTR)::GlobalLock(hDummyGlobalMemory);

  PWSUtil::strCopy(pDummyGlobalLock, uDummyLen, L"\0" , 1);
  ::GlobalUnlock(hDummyGlobalMemory);

  // Real data
  size_t uGlobalMemSize = (data.length() + 1) * sizeof(wchar_t);
  HGLOBAL hGlobalMemory = ::GlobalAlloc(GMEM_MOVEABLE, uGlobalMemSize);
  LPTSTR pGlobalLock = (LPTSTR)::GlobalLock(hGlobalMemory);

  PWSUtil::strCopy(pGlobalLock, data.length() + 1, data.c_str(), data.length());
  ::GlobalUnlock(hGlobalMemory);

  // Following is deleted automagically by SetClipboard() below
  COleDataSource *pods = new PWSOleDataSource(hGlobalMemory, uGlobalMemSize, isSensitive);
  pods->CacheGlobalData(CF_CLIPBOARD_VIEWER_IGNORE, hDummyGlobalMemory);
  pods->DelayRenderData(CLIPBOARD_TEXT_FORMAT); // so we can trigger timer upon paste
  pods->SetClipboard();

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

      if (pData) {
        SIZE_T dwlength = ::GlobalSize(hData);

        if (dwlength > sizeof(wchar_t) + 1) {
          dwlength -= sizeof(wchar_t); // skip trailing null

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
        }
        ::GlobalUnlock(hData);
        ::GlobalFree(hData);
      }
      else { // already freed by OLE subsys
        pws_os::IssueError(L"ClearCBData: lock failed", false);
      }
    }
  }
  return !m_set;
}

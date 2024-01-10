/*
* Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
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

#include "core/PWSprefs.h"
#include "core/Util.h"

static CLIPFORMAT CF_CLIPBOARD_VIEWER_IGNORE; // see below
// Following hide PasswordSafe clipboard pastes from Cloud Clipboard and Clipboard History
// Reference https://docs.microsoft.com/en-us/windows/win32/dataxchg/clipboard-formats#cloud-clipboard-and-clipboard-history-formats
static CLIPFORMAT CF_EXCLUDE_CLIPBOARD_CONTENT_FROM_MONITOR_PROCESSING;
static CLIPFORMAT CF_CAN_INCLUDE_IN_CLIPBOARD_HISTORY;
static CLIPFORMAT CF_CAN_UPLOAD_TO_CLOUD_CLIPBOARD;

class ClipboardChecker
{
public:
  ClipboardChecker()
    :
    m_podo(NULL),
    m_hData(NULL),
    m_pData(NULL),
    m_dwlength(0)
  { }

  virtual ~ClipboardChecker()
  {
    Reset();
  }

  void Reset()
  {
    ReleaseClipboardResources();
    memset(m_digest, 0, sizeof(m_digest));
  }

  void ReleaseClipboardResources() {
    delete m_podo;
    m_podo = NULL;
    if (m_hData) {
      if (m_dwlength > 0)
        trashMemory((void*)m_pData, m_dwlength);
      ::GlobalUnlock(m_hData);
      ::GlobalFree(m_hData);
    }
    m_dwlength = 0;
    m_pData = NULL;
    m_hData = NULL;
  }

  ClipboardStatus Initialize()
  {
    Reset();
    StringX data;

    HWND hwndClipboard = GetOpenClipboardWindow();
    BOOL bIsAvailable = ::IsClipboardFormatAvailable(CF_UNICODETEXT);

    if (hwndClipboard) {
      PWSTRACE(L"ClipboardChecker::Initialize: hwndClipboard=%x bIsAvailable=%d: FAIL: hwndClipboard non-NULL\n", hwndClipboard, bIsAvailable);
      return ClipboardStatus::ClipboardNotAvailable;
    }

    m_podo = new COleDataObject;
    m_podo->AttachClipboard();
    m_podo->EnsureClipboardObject();
    if (!m_podo->m_lpDataObject) {
      delete m_podo;
      m_podo = NULL;
      PWSTRACE(L"ClipboardChecker::Initialize: hwndClipboard=%x bIsAvailable=%d: FAIL: m_lpDataObject==NULL\n", hwndClipboard, bIsAvailable);
      return ClipboardStatus::ClipboardNotAvailable;
    }

    m_hData = m_podo->GetGlobalData(CLIPBOARD_TEXT_FORMAT);
    delete m_podo;
    m_podo = NULL;
    if (!m_hData) {
      ReleaseClipboardResources();
      if (!hwndClipboard)
        hwndClipboard = GetOpenClipboardWindow();
      PWSTRACE(L"ClipboardChecker::Initialize: hwndClipboard=%x bIsAvailable=%d: FAIL: m_hData==NULL\n", hwndClipboard, bIsAvailable);
      return (hwndClipboard != NULL || bIsAvailable) ? ClipboardStatus::ClipboardNotAvailable : ClipboardStatus::SuccessSensitiveNotPresent;
    }

    m_pData = (LPCTSTR)::GlobalLock(m_hData);
    if (!m_pData) {
      ReleaseClipboardResources();
      PWSTRACE(L"ClipboardChecker::Initialize: hwndClipboard=%x bIsAvailable=%d: FAIL: m_pData==NULL\n", hwndClipboard, bIsAvailable);
      return ClipboardStatus::Error;
    }

    m_dwlength = ::GlobalSize(m_hData) - sizeof(wchar_t); // less trailing null
    if (m_dwlength > 0) {
      SHA256 hasher;
      hasher.Update((unsigned char*)m_pData, m_dwlength);
      hasher.Final(m_digest);
    } else {
      PWSTRACE(L"ClipboardChecker::Initialize: Success: m_dwlength==0\n");
    }

    ReleaseClipboardResources();
    // Success but caller determines presence.
    return SuccessSensitiveNotPresent;
  }

  ClipboardStatus isDigestMatch(unsigned char digest[SHA256::HASHLEN])
  {
    ClipboardStatus result = Initialize();
    if (result != SuccessSensitiveNotPresent) {
      return result;
    }
    // Refine SuccessNotPresent result into SuccessPresent if digests match.
    result = memcmp(digest, m_digest, SHA256::HASHLEN) == 0 ? SuccessSensitivePresent : SuccessSensitiveNotPresent;
    if (result == SuccessSensitiveNotPresent)
      PWSTRACE(L"ClipboardChecker::isDigestMatch: digests do not match.\n");
    return result;
  }

public:
  COleDataObject *m_podo;
  HANDLE m_hData;
  LPCTSTR m_pData;
  SIZE_T m_dwlength;
  unsigned char m_digest[SHA256::HASHLEN] = { 0 };
};

PWSclipboard::PWSclipboard()
  : m_bSensitiveDataOnClipboard(false)
{
  memset(m_digest, 0, sizeof(m_digest));

  // Following is more of a gentlemen's agreement than anything else.
  // See http://www.clipboardextender.com/developing-clipboard-aware-programs-for-windows/ignoring-clipboard-updates-with-the-cf_clipboard_viewer_ignore-clipboard-format
  CF_CLIPBOARD_VIEWER_IGNORE = (CLIPFORMAT)RegisterClipboardFormat(L"Clipboard Viewer Ignore");
  // Following for clipboard functionality added in Windows 10
  CF_EXCLUDE_CLIPBOARD_CONTENT_FROM_MONITOR_PROCESSING = (CLIPFORMAT)RegisterClipboardFormat(L"ExcludeClipboardContentFromMonitorProcessing");
  CF_CAN_INCLUDE_IN_CLIPBOARD_HISTORY = (CLIPFORMAT)RegisterClipboardFormat(L"CanIncludeInClipboardHistory");
  CF_CAN_UPLOAD_TO_CLOUD_CLIPBOARD = (CLIPFORMAT)RegisterClipboardFormat(L"CanUploadToCloudClipboard");
}

PWSclipboard::~PWSclipboard()
{
  // ClearCPData(); - current application behaviour allows user to keep
  // data after application exit. 
}

bool PWSclipboard::SetData(const StringX &data, bool isSensitive, CLIPFORMAT cfFormat)
{
  unsigned char sensitiveDataDigest[SHA256::HASHLEN] = { 0 };
  if (isSensitive) {
    // Caller is copying sensitive data to the clipboard (i.e., password or similar).
    // Retain the hash of the sensitive data so its existence on the clipboard can
    // be checked later without requiring a cleartext copy of the sensitive data.
    SHA256 hasher;
    const wchar_t* str = data.c_str();
    hasher.Update((const unsigned char*)str, data.length() * sizeof(wchar_t));
    hasher.Final(sensitiveDataDigest);

    // Do nothing if the sensitive data is already on the clipboard.
    if (m_bSensitiveDataOnClipboard &&
        memcmp(sensitiveDataDigest, m_digest, SHA256::HASHLEN) == 0 &&
        GetLastSensitiveItemPresent() == SuccessSensitivePresent)
      return true;
  }

  memset(m_digest, 0, sizeof(m_digest));
  m_bSensitiveDataOnClipboard = false;

  // Dummy data
  HGLOBAL hDummyGlobalMemory = ::GlobalAlloc(GMEM_MOVEABLE, 2 * sizeof(wchar_t));
  if (!hDummyGlobalMemory)
    return false;

  LPTSTR pDummyGlobalLock = (LPTSTR)::GlobalLock(hDummyGlobalMemory);
  if (!pDummyGlobalLock)
    return false;

  PWSUtil::strCopy(pDummyGlobalLock, 2, L"\0" , 1);
  pDummyGlobalLock = NULL;
  ::GlobalUnlock(hDummyGlobalMemory);

  // Real data
  size_t uGlobalMemSize = (data.length() + 1) * sizeof(wchar_t);
  HGLOBAL hGlobalMemory = ::GlobalAlloc(GMEM_MOVEABLE, uGlobalMemSize);
  if (!hGlobalMemory) {
    ::GlobalFree(hDummyGlobalMemory);
    return false;
  }

  LPTSTR pGlobalLock = (LPTSTR)::GlobalLock(hGlobalMemory);
  if (!pGlobalLock) {
    ::GlobalFree(hDummyGlobalMemory);
    return false;
  }

  PWSUtil::strCopy(pGlobalLock, data.length() + 1, data.c_str(), data.length());
  pGlobalLock = NULL;
  ::GlobalUnlock(hGlobalMemory);

  COleDataSource *pods = new COleDataSource; // deleted automagically by SetClipboard below
  pods->CacheGlobalData(CF_CLIPBOARD_VIEWER_IGNORE, hDummyGlobalMemory);
  if (PWSprefs::GetInstance()->GetPref(PWSprefs::ExcludeFromClipboardHistory)) { // default is true
    pods->CacheGlobalData(CF_EXCLUDE_CLIPBOARD_CONTENT_FROM_MONITOR_PROCESSING, hDummyGlobalMemory);
    pods->CacheGlobalData(CF_CAN_INCLUDE_IN_CLIPBOARD_HISTORY, hDummyGlobalMemory);
  }
  pods->CacheGlobalData(CF_CAN_UPLOAD_TO_CLOUD_CLIPBOARD, hDummyGlobalMemory);

  pods->CacheGlobalData(cfFormat, hGlobalMemory);
  pods->SetClipboard();
  pods = NULL; // As deleted by SetClipboard above

  if (isSensitive) {
    m_bSensitiveDataOnClipboard = true;
    memcpy(m_digest, sensitiveDataDigest, sizeof(m_digest));
  }

  return m_bSensitiveDataOnClipboard;
}

ClipboardStatus PWSclipboard::ClearCBData()
{
  ClipboardStatus result = GetLastSensitiveItemPresent();
  if (result != SuccessSensitivePresent)
    return result;
  StringX blank(L"");
  SetData(blank, false);
  memset(m_digest, 0, SHA256::HASHLEN);
  m_bSensitiveDataOnClipboard = false;
  // Sensitive data was present, now removed.
  return SuccessSensitivePresent;
}

ClipboardStatus PWSclipboard::GetLastSensitiveItemPresent()
{
  if (!m_bSensitiveDataOnClipboard)
    return SuccessSensitiveNotPresent;
  ClipboardChecker clipboardChecker;
  ClipboardStatus result = clipboardChecker.isDigestMatch(m_digest);
  if (result == Error || result == SuccessSensitiveNotPresent)
    memset(m_digest, 0, SHA256::HASHLEN);
  return result;
}


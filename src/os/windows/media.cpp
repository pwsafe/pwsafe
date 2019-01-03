/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
* \file Windows-specific implementation of media.h
*/

#include "../media.h"
#include "../debug.h"
#include <windows.h>

// Windows uses the file extension to get the Media/MIME type

stringT pws_os::GetMediaType(const stringT &sfilename)
{
  HRESULT hResult;
  TCHAR *pwzMimeOut = NULL;
  stringT sMediaType(_T("unknown"));

  wchar_t extn[_MAX_EXT];
  _tsplitpath_s(sfilename.c_str(), NULL, 0, NULL, 0, NULL, 0, extn, _MAX_EXT);

  // Note 1: FMFD_IGNOREMIMETEXTPLAIN not defined (UrlMon.h) if still supporting Windows XP
  // Note 2: FMFD_RETURNUPDATEDIMGMIMES not defined (UrlMon.h) in SDK 7.1A - need SDK 8.1 or later
  // Hardcode values for now
  DWORD dwMimeFlags = FMFD_URLASFILENAME | 0x4 /*FMFD_IGNOREMIMETEXTPLAIN*/ | 0x20 /*FMFD_RETURNUPDATEDIMGMIMES*/;
  hResult = FindMimeFromData(NULL, extn, NULL, 0, NULL, dwMimeFlags, &pwzMimeOut, 0);

  if (SUCCEEDED(hResult)) {
    sMediaType = pwzMimeOut;
    CoTaskMemFree(pwzMimeOut);
  }

  return sMediaType;
}

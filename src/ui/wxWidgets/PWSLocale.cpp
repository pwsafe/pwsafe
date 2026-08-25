/*
 * Copyright (c) 2003-2026 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file PWSLocale.cpp
*
*/

// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "os/debug.h"
#include "PWSLocale.h"

wxString PWSLocale::appendUTF8(const wxString& ev)
{
  if (ev.empty() || ev.EndsWith(".UTF-8")) return ev;
  return ev + ".UTF-8";
}

#ifdef LOCALE_WX322

bool PWSLocale::UseDefault()
{
  auto ret = wxUILocale::UseDefault();
  setMacLocale(PWSGetCurrentName());
  return ret;
}

/*
 * Some languages have multiple locale variations.  (e.g. en_US, en_GB, etc.)
 * Just using the two letter language identifier (e.g. en.UTF-8) does not work
 * in some cases; there needs to be a region as well (e.g. en_US.UTF-8), not doing
 * so causes some inconsistent results. Specifically, the date format seems to
 * default to en_GB in WX but not in native macOS controls, such as the date picker.
 * This checks the user environment setting, if the language matches, use the env locale.
 * If not, use whatever WX guessed.
 */
void PWSLocale::ChooseLocale(wxLanguage language)
{
  const wxLanguageInfo *langInfo = GetLanguageInfo(language);
  if (langInfo != nullptr) {
    wxString envString;
    wxLocaleIdent sysLocaleId = PWSLocale::GetSystemLocaleId();
    if (langInfo->CanonicalName == sysLocaleId.GetLanguage() && !sysLocaleId.GetRegion().empty()) {
      envString = sysLocaleId.GetName();

    } else if (!langInfo->CanonicalRef.empty()) {
      envString = langInfo->CanonicalRef;

    } else {
      envString = langInfo->CanonicalName;
    }
    wxString ev8 = appendUTF8(envString);
    if ( !ev8.empty() ) {
      wxUILocale::UseLocaleName(ev8);
      setMacLocale(ev8);
    }
  }
  pws_os::Trace(L"Current wx   locale is: %ls", static_cast<const wchar_t *>(PWSLocale::PWSGetCurrentName()));
  pws_os::Trace(L"Current libc locale is: %s", setlocale(LC_ALL, NULL));
}

  #ifdef __WXMAC__
  void PWSLocale::setMacLocale(const char *loc)
  {
    if (loc && !setlocale(LC_ALL, loc)) {
      pws_os::Trace(L"Failed to set locale to: %s", loc);
    }

    // This value must be set for mac OS starting with version 11, but is no problem for earlier versions, see:
    // https://trac.wxwidgets.org/ticket/19023
    // https://docs.wxwidgets.org/3.2/classwx_locale.html
    int major, minor;
    wxGetOsVersion(&major, &minor);
    if (major == 11 || (major == 12 && minor < 3)) {
      setlocale(LC_NUMERIC, "C");
    }
  }
  #else // __WXMAC__
  void PWSLocale::setMacLocale(const char *) {}; // no-op if not macOS
  #endif // __WXMAC__

#else // LOCALE_WX322

bool PWSLocale::UseDefault()
{
  // Because the old version did this, possibily as a problem work around.
  setlocale(LC_CTYPE, "");
  setlocale(LC_TIME, "");
  return false;
}

void PWSLocale::ChooseLocale(wxLanguage language)
{
  const wxLanguageInfo *langInfo = GetLanguageInfo(language);
  if (langInfo != nullptr) {
    wxString ev8 = appendUTF8(langInfo->CanonicalName);
    if ( !ev8.empty() ) {
      setlocale(LC_CTYPE, ev8.c_str());
      setlocale(LC_TIME, ev8.c_str());
    }
  }
  if (m_pwslocale)
    pws_os::Trace(L"Current wx   locale is: %ls", static_cast<const wchar_t *>(m_pwslocale->PWSGetCurrentName()));

  pws_os::Trace(L"Current libc locale is: %s", setlocale(LC_ALL, NULL));
}

PWSLocale *PWSLocale::m_pwslocale = nullptr;

#endif // LOCALE_WX322

/*
 * Initial version created by Richard Powell
 *
 * Copyright (c) 2003-2026 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

#ifndef _PWSLOCALE_H_
#define _PWSLOCALE_H_

// wx3.2.2 and later can use wxUILocale and some improved logic
#if wxCHECK_VERSION(3, 2, 2)
#define LOCALE_WX322
#endif

class PWSMacLocale
{
public:
#ifdef __WXMAC__
  static void setMacLocale(const char *loc);
#else
  static void setMacLocale(const char *) {}; // no-op if not macOS
#endif
};

#ifdef LOCALE_WX322
#include <wx/uilocale.h>

  class PWSLocale : public wxUILocale, public PWSMacLocale
  {
  public:
    static bool UseDefault() {
      auto ret = wxUILocale::UseDefault();
      setMacLocale(PWSGetCurrentName());
      return ret;
    }
    static wxString PWSGetCurrentName() { return wxUILocale::GetCurrent().GetName(); };
  };

#else // LOCALE_WX322

  // Pre-wx3.2.2 compatible version
  class PWSLocale : public wxLocale, public PWSMacLocale
  {
  public:
    PWSLocale() {};
    static bool UseDefault() {
      // Because the old version did this, possibily as problem work arounds.
      setlocale(LC_CTYPE, "");
      setlocale(LC_TIME, "");
      return false;
    };
    wxString PWSGetCurrentName() { return wxLocale::GetCanonicalName(); };
  };
#endif // LOCALE_WX322

#endif // _PWSLOCALE_H_

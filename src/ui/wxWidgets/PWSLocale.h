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
// All supported macOS versions work best with wxWidgets 3.2.4 and above.
// Unix/Linux is compatible as far back as wxWidgets 3.0.5
#if wxCHECK_VERSION(3, 2, 2)
#define LOCALE_WX322
#endif

#ifdef LOCALE_WX322
#include <wx/uilocale.h>

  class PWSLocale : public wxUILocale
  {
  public:
    static bool UseDefault() {
      auto ret = wxUILocale::UseDefault();
      setMacLocale(PWSGetCurrentName());
      return ret;
    }
    static void UseLocaleName(const wxString& envString) {
      wxString ev8 = setUTF8(envString);
      if ( !ev8.empty() ) {
        wxUILocale::UseLocaleName(ev8);
        setMacLocale(ev8);
      }
    }
    static wxString PWSGetCurrentName() { return wxUILocale::GetCurrent().GetName(); };

  private:
    static wxString setUTF8(const wxString& ev) {
      if (ev.empty() || ev.EndsWith(".UTF-8")) return ev;
      return ev + ".UTF-8";
    }

  #ifdef __WXMAC__
    static void setMacLocale(const char *loc) {
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
    static void setMacLocale(const char *) {}; // no-op if not macOS
  #endif // __WXMAC__
  };

#else // LOCALE_WX322

  // Pre-wx3.2.2 compatible version
  class PWSLocale : public wxLocale
  {
  public:
    PWSLocale() {};
    static bool UseDefault() {
      // Because the old version did this, possibily as a problem work around.
      setlocale(LC_CTYPE, "");
      setlocale(LC_TIME, "");
      return false;
    }
    static void UseLocaleName(const wxString& envString) {
      wxString ev8 = setUTF8(envString);
      if ( !ev8.empty() ) {
        setlocale(LC_CTYPE, ev8.c_str());
        setlocale(LC_TIME, ev8.c_str());
      }
    }
    wxString PWSGetCurrentName() { return wxLocale::GetCanonicalName(); };

  private:
    static wxString setUTF8(const wxString& ev) {
      if (ev.empty() || ev.EndsWith(".UTF-8")) return ev;
      return ev + ".UTF-8";
    }
  };
#endif // LOCALE_WX322

#endif // _PWSLOCALE_H_

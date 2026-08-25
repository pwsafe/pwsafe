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
    static bool UseDefault();
    static void ChooseLocale(wxLanguage language);
    static wxString PWSGetCurrentName() { return wxUILocale::GetCurrent().GetName(); };

  private:
    static wxString appendUTF8(const wxString& ev);
    static void setMacLocale(const char *loc);
  };

#else // LOCALE_WX322

  // Pre-wx3.2.2 compatible version
  class PWSLocale : public wxLocale
  {
  public:
    PWSLocale() {};
    ~PWSLocale() { m_pwslocale = nullptr; }
    static PWSLocale *m_pwslocale;

    static bool UseDefault();
    static void ChooseLocale(wxLanguage language);
    wxString PWSGetCurrentName() { return wxLocale::GetCanonicalName(); };

  private:
    static wxString appendUTF8(const wxString& ev);
  };
#endif // LOCALE_WX322

#endif // _PWSLOCALE_H_

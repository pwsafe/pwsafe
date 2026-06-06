/*
* Copyright (c) 2026 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

#include <windows.h>

#if defined(PWSAFE_USE_DARKMODE32)
#include "darkmode32/DMSubclass.h"
#endif

#if !defined(PWSAFE_USE_DARKMODE32) || defined(_DARKMODE_NOT_USED)
namespace DarkMode
{
  enum class DarkModeType : unsigned char
  {
    light = 0,
    dark = 1,
    classic = 3
  };

  [[nodiscard]] inline bool isEnabled() noexcept { return false; }
  [[nodiscard]] inline bool isExperimentalActive() noexcept { return false; }
  [[nodiscard]] inline bool isWindowsModeEnabled() noexcept { return false; }
  [[nodiscard]] inline bool isDarkModeReg() { return false; }

  inline void initDarkMode() {}
  inline void initDarkModeConfig(UINT) {}
  inline void setDarkModeConfig(UINT) {}
  inline void setDarkModeConfig() {}
  inline void setDefaultColors(bool) {}
  inline void setSysColor(int, COLORREF) noexcept {}
  inline COLORREF setDisabledTextColor(COLORREF clr) noexcept { return clr; }

  [[nodiscard]] inline COLORREF getBackgroundColor() noexcept { return ::GetSysColor(COLOR_WINDOW); }
  [[nodiscard]] inline COLORREF getCtrlBackgroundColor() noexcept { return ::GetSysColor(COLOR_WINDOW); }
  [[nodiscard]] inline COLORREF getDlgBackgroundColor() noexcept { return ::GetSysColor(COLOR_3DFACE); }
  [[nodiscard]] inline COLORREF getErrorBackgroundColor() noexcept { return RGB(255, 222, 222); }
  [[nodiscard]] inline COLORREF getHotBackgroundColor() noexcept { return ::GetSysColor(COLOR_HIGHLIGHT); }
  [[nodiscard]] inline COLORREF getTextColor() noexcept { return ::GetSysColor(COLOR_WINDOWTEXT); }
  [[nodiscard]] inline COLORREF getDisabledTextColor() noexcept { return ::GetSysColor(COLOR_GRAYTEXT); }
  [[nodiscard]] inline COLORREF getEdgeColor() noexcept { return ::GetSysColor(COLOR_WINDOWFRAME); }
  [[nodiscard]] inline COLORREF getHotEdgeColor() noexcept { return ::GetSysColor(COLOR_HIGHLIGHT); }
  [[nodiscard]] inline COLORREF getViewGridlinesColor() noexcept { return ::GetSysColor(COLOR_BTNFACE); }
  [[nodiscard]] inline COLORREF getViewBackgroundColor() noexcept { return ::GetSysColor(COLOR_WINDOW); }
  [[nodiscard]] inline COLORREF getViewTextColor() noexcept { return ::GetSysColor(COLOR_WINDOWTEXT); }

  [[nodiscard]] inline HBRUSH getBackgroundBrush() noexcept { return ::GetSysColorBrush(COLOR_WINDOW); }
  [[nodiscard]] inline HBRUSH getCtrlBackgroundBrush() noexcept { return ::GetSysColorBrush(COLOR_WINDOW); }
  [[nodiscard]] inline HBRUSH getDlgBackgroundBrush() noexcept { return ::GetSysColorBrush(COLOR_3DFACE); }

  inline void setWindowEraseBgSubclass(HWND) {}
  inline void setWindowMenuBarSubclass(HWND) {}
  inline void setDarkWndNotifySafe(HWND, bool = true) {}
  inline void enableThemeDialogTexture(HWND, bool) {}
  inline void setDarkRichEdit(HWND) {}
  inline void setStatusBarCtrlSubclass(HWND) {}
  inline void setChildCtrlsSubclassAndTheme(HWND, bool = true, bool = true) {}
  inline void setListViewCtrlSubclass(HWND) {}
  inline void setDarkListView(HWND) {}
} // namespace DarkMode
#endif

namespace PwsDarkMode
{
  constexpr UINT kForceClassicConfig = static_cast<UINT>(DarkMode::DarkModeType::classic);
  // Mode 4 follows Windows, using classic/native styling for light mode.
  constexpr UINT kFollowSystemClassicOrDarkConfig = 4;

  namespace detail
  {
    inline void SendThemeAndColorChanged(HWND hWnd)
    {
      ::SendMessage(hWnd, WM_THEMECHANGED, 0, 0);
      ::SendMessage(hWnd, WM_SYSCOLORCHANGE, 0, 0);
    }

    inline BOOL CALLBACK SendThemeAndColorChangedToChild(HWND hWnd, LPARAM)
    {
      SendThemeAndColorChanged(hWnd);
      return TRUE;
    }
  } // namespace detail

  inline UINT ToDarkModeConfig(int displayModePreference)
  {
    switch (displayModePreference) {
      case 1:
        return kForceClassicConfig;
      case 2:
        return static_cast<UINT>(DarkMode::DarkModeType::dark);
      default:
        return kFollowSystemClassicOrDarkConfig;
    }
  }

  // Windows High Contrast is an accessibility mode with its own enforced palette;
  // applications must defer to it rather than impose their own colours.
  inline bool IsHighContrastActive()
  {
    HIGHCONTRAST hc{};
    hc.cbSize = sizeof(hc);
    if (::SystemParametersInfo(SPI_GETHIGHCONTRAST, sizeof(hc), &hc, 0) == FALSE)
      return false;
    return (hc.dwFlags & HCF_HIGHCONTRASTON) != 0;
  }

  inline void ApplyDisplayModePreference(int displayModePreference)
  {
    if (IsHighContrastActive()) {
      // Defer entirely to the OS: force classic config so DarkMode::isEnabled() is
      // false everywhere (which disables all of the custom dark painting, since it
      // all keys off that flag), and skip the GetSysColor overrides below so comctl32
      // sees the real High Contrast colours.
      DarkMode::setDarkModeConfig(kForceClassicConfig);
      return;
    }

    DarkMode::setDarkModeConfig(ToDarkModeConfig(displayModePreference));
    DarkMode::setDefaultColors(true);
    // setDefaultColors seeds disabledText from the OS COLOR_GRAYTEXT -- a light-mode
    // grey that's too dim to read on the dark background. Lift it to a legible mid-grey
    // that still reads as disabled (clearly dimmer than getTextColor()).
    DarkMode::setDisabledTextColor(RGB(150, 150, 150));
    DarkMode::setSysColor(COLOR_WINDOW, DarkMode::getBackgroundColor());
    DarkMode::setSysColor(COLOR_WINDOWTEXT, DarkMode::getTextColor());
    DarkMode::setSysColor(COLOR_BTNFACE, DarkMode::getViewGridlinesColor());
  }

  inline void RepaintWindow(HWND hWnd)
  {
    if (hWnd == nullptr || !::IsWindow(hWnd))
      return;

    DarkMode::setDarkWndNotifySafe(hWnd, true);
    detail::SendThemeAndColorChanged(hWnd);
    ::EnumChildWindows(hWnd, detail::SendThemeAndColorChangedToChild, 0);
    ::RedrawWindow(hWnd, nullptr, nullptr,
                   RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_ALLCHILDREN |
                   RDW_ERASENOW | RDW_UPDATENOW);
  }

  inline void Initialize(int displayModePreference)
  {
    DarkMode::initDarkMode();
    ApplyDisplayModePreference(displayModePreference);
  }
} // namespace PwsDarkMode

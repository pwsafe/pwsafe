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
} // namespace DarkMode
#endif

namespace PwsDarkMode
{
  constexpr UINT kFollowSystemConfig = 2;

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
        return static_cast<UINT>(DarkMode::DarkModeType::light);
      case 2:
        return static_cast<UINT>(DarkMode::DarkModeType::dark);
      default:
        return kFollowSystemConfig;
    }
  }

  inline void ApplyDisplayModePreference(int displayModePreference)
  {
    DarkMode::setDarkModeConfig(ToDarkModeConfig(displayModePreference));
    DarkMode::setDefaultColors(true);
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

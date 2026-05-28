// SPDX-License-Identifier: BSD-3-Clause license

/*
 * Copyright (c) 2025 Anthony Lee Stark. All rights reserved.
 *
 * This project is based on and includes modified code from:
 * project 'win32-darkmode' by ysc3839 (MIT License),
 * available at: https://github.com/ysc3839/win32-darkmode
 * and project 'darkmodelib' by ozone10 (MPL-2.0 License),
 * available at: https://github.com/ozone10/darkmodelib
 *
 * The respective original licenses apply to portions of this code.
 * See the `licenses/` folder for more information.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *	  list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *	  this list of conditions and the following disclaimer in the documentation
 *	  and/or other materials provided with the distribution.
 *
 * 3. Neither the name of Anthony Lee Stark (@anthonyleestark) nor the names of
 *	  its contributors may be used to endorse or promote products derived from
 *	  this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#pragma once
#include <windows.h>
#include <uxtheme.h>
#include <vsstyle.h>

#define _DARKMODE_SUPPORT_OLDER_OS

/// DarkMode helper namespace
namespace DarkModeHelper {

	/**
	 * @enum IMMERSIVE_HC_CACHE_MODE
	 * @brief Cache modes for immersive high contrast settings.
	 * 
	 * Used with the `GetIsImmersiveColorUsingHighContrast` function to specify whether to use cached values or refresh them.
	 * - `IHCM_USE_CACHED_VALUE`: Use the cached value for high contrast color usage.
	 * - `IHCM_REFRESH`: Refresh the cache and retrieve the current value.
	 * 
	 * @note This enum is specific to the Windows 10 dark mode implementation and may not be applicable to all versions of Windows.
	 * @see GetIsImmersiveColorUsingHighContrast()
	 * @see IsHighContrast()
	 * 
	 * @remarks The behavior of high contrast settings can affect the appearance of dark mode, and this enum allows for more efficient 
	 *          retrieval of these settings when needed.
	 * @since Windows 10 version 1809 (build 17763)
	 */
	enum IMMERSIVE_HC_CACHE_MODE {
		IHCM_USE_CACHED_VALUE,
		IHCM_REFRESH
	};

	/**
	 * @brief Preferred app mode for dark mode settings.
	 * 
	 * Used with the `SetPreferredAppMode` function to specify the preferred dark mode behavior for an application.
	 * - `Default`: Use the system default behavior.
	 * - `AllowDark`: Allow dark mode for the application.
	 * - `ForceDark`: Force dark mode for the application.
	 * - `ForceLight`: Force light mode for the application.
	 * 
	 * @note This enum is specific to the Windows 10 version 1903 dark mode implementation and may not be 
	 * 		 applicable to all versions of Windows.
	 * @see SetPreferredAppMode()
	 * @since Windows 10 version 1903 (build 18362)
	 */
	enum class PreferredAppMode {
		Default,
		AllowDark,
		ForceDark,
		ForceLight,
		Max
	};

#if defined(_DARKMODE_SUPPORT_OLDER_OS)

	/**
	 * @enum WINDOWCOMPOSITIONATTRIB
	 * @brief Window composition attributes for older Windows versions.
	 * 
	 * Used with the `SetWindowCompositionAttribute` function to specify various attributes related to window composition and theming.
	 * - `WCA_UNDEFINED`: Undefined attribute.
	 * - `WCA_NCRENDERING_ENABLED`: Enable non-client area rendering.
	 * - `WCA_NCRENDERING_POLICY`: Set non-client area rendering policy.
	 * - `WCA_TRANSITIONS_FORCEDISABLED`: Force disable transitions.
	 * - `WCA_ALLOW_NCPAINT`: Allow non-client area painting.
	 * - `WCA_CAPTION_BUTTON_BOUNDS`: Get caption button bounds.
	 * - `WCA_NONCLIENT_RTL_LAYOUT`: Set non-client area right-to-left layout.
	 * - `WCA_FORCE_ICONIC_REPRESENTATION`: Force iconic representation.
	 * - `WCA_EXTENDED_FRAME_BOUNDS`: Get extended frame bounds.
	 * - `WCA_HAS_ICONIC_BITMAP`: Check if iconic bitmap is present.
	 * - `WCA_THEME_ATTRIBUTES`: Get theme attributes.
	 * - `WCA_NCRENDERING_EXILED`: Exile non-client area rendering.
	 * - `WCA_NCADORNMENTINFO`: Get non-client area adornment information.
	 * - `WCA_EXCLUDED_FROM_LIVEPREVIEW`: Exclude from live preview.
	 * - `WCA_VIDEO_OVERLAY_ACTIVE`: Check if video overlay is active.
	 * - `WCA_FORCE_ACTIVEWINDOW_APPEARANCE`: Force active window appearance.
	 * - `WCA_DISALLOW_PEEK`: Disallow peek.
	 * - `WCA_CLOAK`: Cloak the window.
	 * - `WCA_CLOAKED`: Check if the window is cloaked.
	 * - `WCA_ACCENT_POLICY`: Set accent policy.
	 * - `WCA_FREEZE_REPRESENTATION`: Freeze window representation.
	 * - `WCA_EVER_UNCLOAKED`: Check if the window was ever uncloaked.
	 * - `WCA_VISUAL_OWNER`: Get visual owner.
	 * - `WCA_HOLOGRAPHIC`: Check if the window is holographic.
	 * - `WCA_EXCLUDED_FROM_DDA`: Exclude from dynamic data exchange.
	 * - `WCA_PASSIVEUPDATEMODE`: Set passive update mode.
	 * - `WCA_USEDARKMODECOLORS`: Use dark mode colors.
	 * - `WCA_LAST`: Sentinel value for the last attribute.
	 * 
	 * @note This enum is specific to older Windows versions and may not be applicable to 
	 * 		 newer versions of Windows that have built-in dark mode support.
	 * @see SetWindowCompositionAttribute()
	 * @since Windows 7 and later (with dark mode support added in Windows 10)
	 */
	enum WINDOWCOMPOSITIONATTRIB {
		WCA_UNDEFINED = 0,
		WCA_NCRENDERING_ENABLED = 1,
		WCA_NCRENDERING_POLICY = 2,
		WCA_TRANSITIONS_FORCEDISABLED = 3,
		WCA_ALLOW_NCPAINT = 4,
		WCA_CAPTION_BUTTON_BOUNDS = 5,
		WCA_NONCLIENT_RTL_LAYOUT = 6,
		WCA_FORCE_ICONIC_REPRESENTATION = 7,
		WCA_EXTENDED_FRAME_BOUNDS = 8,
		WCA_HAS_ICONIC_BITMAP = 9,
		WCA_THEME_ATTRIBUTES = 10,
		WCA_NCRENDERING_EXILED = 11,
		WCA_NCADORNMENTINFO = 12,
		WCA_EXCLUDED_FROM_LIVEPREVIEW = 13,
		WCA_VIDEO_OVERLAY_ACTIVE = 14,
		WCA_FORCE_ACTIVEWINDOW_APPEARANCE = 15,
		WCA_DISALLOW_PEEK = 16,
		WCA_CLOAK = 17,
		WCA_CLOAKED = 18,
		WCA_ACCENT_POLICY = 19,
		WCA_FREEZE_REPRESENTATION = 20,
		WCA_EVER_UNCLOAKED = 21,
		WCA_VISUAL_OWNER = 22,
		WCA_HOLOGRAPHIC = 23,
		WCA_EXCLUDED_FROM_DDA = 24,
		WCA_PASSIVEUPDATEMODE = 25,
		WCA_USEDARKMODECOLORS = 26,
		WCA_LAST = 27
	};

	/**
	 * @struct WINDOWCOMPOSITIONATTRIBDATA
	 * @brief Data structure for window composition attributes.
	 * 
	 * Used with the `SetWindowCompositionAttribute` function to specify the attribute, data, 
	 * and size for window composition settings.
	 * - `Attrib`: The window composition attribute to set or query.
	 * - `pvData`: A pointer to the data associated with the attribute.
	 * - `cbData`: The size of the data in bytes.
	 * 
	 * @note This structure is specific to older Windows versions and may not be applicable to 
	 * newer versions of Windows that have built-in dark mode support.
	 * @see SetWindowCompositionAttribute()
	 * @since Windows 7 and later (with dark mode support added in Windows 10)
	 */
	struct WINDOWCOMPOSITIONATTRIBDATA {
		WINDOWCOMPOSITIONATTRIB Attrib;
		PVOID pvData;
		SIZE_T cbData;
	};
#endif

/// Function typename declarations
#if defined(_DARKMODE_SUPPORT_OLDER_OS)
	using fnSetWindowCompositionAttribute = BOOL(WINAPI*)(HWND hWnd, WINDOWCOMPOSITIONATTRIBDATA*);
#endif
	using fnShouldAppsUseDarkMode = auto (WINAPI*)() -> bool; // ordinal 132
	using fnAllowDarkModeForWindow = auto (WINAPI*)(HWND hWnd, bool allow) -> bool; // ordinal 133
#if defined(_DARKMODE_SUPPORT_OLDER_OS)
	using fnAllowDarkModeForApp = auto (WINAPI*)(bool allow) -> bool; // ordinal 135, in 1809
#endif
	using fnFlushMenuThemes = void (WINAPI*)(); // ordinal 136
	using fnRefreshImmersiveColorPolicyState = void (WINAPI*)(); // ordinal 104
	using fnIsDarkModeAllowedForWindow = auto (WINAPI*)(HWND hWnd) -> bool; // ordinal 137
	using fnGetIsImmersiveColorUsingHighContrast = auto (WINAPI*)(IMMERSIVE_HC_CACHE_MODE mode) -> bool; // ordinal 106
	using fnOpenNcThemeData = auto (WINAPI*)(HWND hWnd, LPCWSTR pszClassList) -> HTHEME; // ordinal 49
	// Windows 10 version 1903 build number 18362
	using fnShouldSystemUseDarkMode = auto (WINAPI*)() -> bool; // ordinal 138
	using fnSetPreferredAppMode = auto (WINAPI*)(PreferredAppMode appMode) -> PreferredAppMode; // ordinal 135, in 1903
	using fnIsDarkModeAllowedForApp = auto (WINAPI*)() -> bool; // ordinal 139

/// Global variables
	extern bool g_darkModeSupported;
	extern bool g_darkModeEnabled;

/// DarkMode helpers

	/**
	 * @brief Check if dark mode is supported and enabled for the application.
	 * 
	 * This function checks if the operating system supports dark mode and if it is currently enabled for the application. 
	 * It may also check user preferences and system settings to determine the effective dark mode state.
	 * 
	 * @return `true` if dark mode is supported and enabled for the application, `false` otherwise.
	 */
	[[nodiscard]] bool ShouldAppsUseDarkMode(void) noexcept;

	/**
	 * @brief Allows or disallows dark mode for a specific window.
	 * 
	 * This function enables or disables dark mode for a given window handle. When dark mode is allowed for a window, 
	 * it will use dark-themed colors and styles if the system is in dark mode. If dark mode is disallowed, 
	 * the window will use light-themed colors and styles regardless of the system theme.
	 * 
	 * @param hWnd The handle to the window for which to allow or disallow dark mode.
	 * @param isAllowed `true` to allow dark mode for the window, `false` to disallow it.
	 * @return `true` if the operation was successful, `false` otherwise.
	 */
	bool AllowDarkModeForWindow(HWND hWnd, bool isAllowed) noexcept;

	/**
	 * @brief Checks if the system is currently using high contrast mode.
	 * 
	 * This function checks the current system settings to determine if high contrast mode is enabled. 
	 * High contrast mode is an accessibility feature that changes the color scheme of the user interface 
	 * to improve readability for users with visual impairments. When high contrast mode is enabled, 
	 * it may affect the appearance of dark mode and other visual styles.
	 * 
	 * @return `true` if high contrast mode is enabled, `false` otherwise.
	 * @remarks High contrast mode can override dark mode settings, so it's important to check this state when applying dark mode themes to ensure that the application remains accessible to all users.
	 */
	[[nodiscard]] bool IsHighContrast(void);

#if defined(_DARKMODE_SUPPORT_OLDER_OS)

	/**
	 * @brief Refreshes the theme color of a window's title bar.
	 * 
	 * This function forces a refresh of the title bar theme color for a specified window. 
	 * It may be necessary to call this function after changing dark mode settings or system colors 
	 * to ensure that the title bar updates to reflect the new theme.
	 * 
	 * @param hWnd The handle to the window for which to refresh the title bar theme color.
	 * 
	 * @remarks This function is particularly useful for older Windows versions that do not automatically 
	 * 			update the title bar colors when dark mode settings change.
	 */
	void RefreshTitleBarThemeColor(HWND hWnd);
	
	/**
	 * @brief Sets the theme color for a window's title bar.
	 * 
	 * This function applies a dark or light theme color to the title bar of a specified window. 
	 * It may also refresh the title bar theme color to ensure that the changes take effect immediately.
	 * 
	 * @param hWnd The handle to the window for which to set the title bar theme color.
	 * @param isDark `true` to set a dark theme color, `false` to set a light theme color.
	 */
	void SetTitleBarThemeColor(HWND hWnd, BOOL isDark);

#endif

	/**
	 * @brief Checks if a given message is a color scheme change message.
	 * 
	 * This function checks if the provided message and parameters correspond to a color scheme change event. 
	 * Color scheme change messages are sent by the system when the user changes the color scheme or theme settings, 
	 * and applications can respond to these messages to update their appearance accordingly.
	 * 
	 * @param lParam The message parameter to check.
	 * 
	 * @return `true` if the message is a color scheme change message, `false` otherwise.
	 */
	[[nodiscard]] bool IsColorSchemeChangeMessage(LPARAM lParam);

	/**
	 * @brief Checks if a given message is a color scheme change message.
	 * 
	 * This function checks if the provided message and parameters correspond to a color scheme change event. 
	 * Color scheme change messages are sent by the system when the user changes the color scheme or theme settings, 
	 * and applications can respond to these messages to update their appearance accordingly.
	 * 
	 * @param message The message identifier to check.
	 * @param lParam The message parameter to check.
	 * 
	 * @return `true` if the message is a color scheme change message, `false` otherwise.
	 */
	[[nodiscard]] bool IsColorSchemeChangeMessage(UINT message, LPARAM lParam);

	/**
	 * @brief Allows or disallows dark mode for the application.
	 * 
	 * This function enables or disables dark mode for the entire application. 
	 * When dark mode is allowed, the application can use dark-themed UI elements and colors.
	 * 
	 * @param isAllowed `true` to allow dark mode, `false` to disallow it.
	 */
	void AllowDarkModeForApp(bool isAllowed) noexcept;

	/**
	 * @brief Enables dark mode for scroll bars on a window and all its child controls.
	 * 
	 * This function applies dark mode styling to scroll bars for a specified window and all of its child controls. 
	 * It ensures that scroll bars are consistent with the dark mode theme, improving the overall visual coherence 
	 * of the application when dark mode is enabled.
	 * 
	 * @param hWnd The handle to the window for which to enable dark mode for scroll bars and its child controls.
	 */
	void EnableDarkScrollBarForWindowAndChildren(HWND hWnd);

	/**
	 * @brief Initializes dark mode support for the application.
	 * 
	 * This function performs necessary initialization steps to enable dark mode support for the application. 
	 * It may involve checking for dark mode support, setting up necessary hooks or callbacks, and applying 
	 * initial dark mode settings based on user preferences and system settings.
	 * 
	 * @remarks This function should be called during the application's initialization phase to ensure 
	 * 			that dark mode is properly set up before any windows are created or displayed.
	 */
	void InitDarkMode(void);

	/**
	 * @brief Sets the dark mode state for the application.
	 * 
	 * This function enables or disables dark mode for the application based on the provided parameters. 
	 * It may also apply necessary fixes for dark scroll bars if dark mode is enabled.
	 * 
	 * @param useDark `true` to enable dark mode, `false` to disable it.
	 * @param doFixDarkScrollbar `true` to apply fixes for dark scroll bars when dark mode is enabled, `false` to skip these fixes.
	 * 
	 * @remarks This function can be called to toggle dark mode on or off for the application at runtime, 
	 * 			allowing for dynamic theme changes based on user preferences or system settings.
	 */
	void SetDarkMode(bool useDark, bool doFixDarkScrollbar);
};

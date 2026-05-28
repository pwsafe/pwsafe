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


#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif


#include "DarkMode.h"
#include "IatHook.h"
using namespace DarkModeHelper;


#include "WinVerHelper.h"
using namespace WinVerHelper;

// Supported Windows version
static unsigned long g_buildNumber = 0;
#if defined(_DARKMODE_SUPPORT_OLDER_OS)
static constexpr unsigned short MinSupportVersion = WinVer::WIN10_VER_1809;
#else
static constexpr unsigned short MinSupportVersion = WinVer::WIN10_VER_22H2;
#endif


// For module management
#include "ModuleHelper.h"
using namespace ModuleHelper;


#if defined(_MSC_VER) && _MSC_VER >= 1800
	#pragma warning(disable : 4191)
#elif defined(__GNUC__)
	#include <cwchar>
#endif


// Dark Mode function pointers
#if defined(_DARKMODE_SUPPORT_OLDER_OS)
static fnSetWindowCompositionAttribute pfSetWindowCompositionAttribute = nullptr;
#endif
// Windows 10 version 1809 build number 17763
static fnShouldAppsUseDarkMode pfShouldAppsUseDarkMode = nullptr;
static fnAllowDarkModeForWindow pfAllowDarkModeForWindow = nullptr;
#if defined(_DARKMODE_SUPPORT_OLDER_OS)
static fnAllowDarkModeForApp pfAllowDarkModeForApp = nullptr;
#endif
static fnFlushMenuThemes pfFlushMenuThemes = nullptr;
static fnRefreshImmersiveColorPolicyState pfRefreshImmersiveColorPolicyState = nullptr;
static fnIsDarkModeAllowedForWindow pfIsDarkModeAllowedForWindow = nullptr;
static fnGetIsImmersiveColorUsingHighContrast pfGetIsImmersiveColorUsingHighContrast = nullptr;
static fnOpenNcThemeData pfOpenNcThemeData = nullptr;
// Windows 10 version 1903 build number 18362
static fnShouldSystemUseDarkMode pfShouldSystemUseDarkMode = nullptr;
static fnSetPreferredAppMode pfSetPreferredAppMode = nullptr;


// Global variables initialization
bool DarkModeHelper::g_darkModeSupported	= false;
bool DarkModeHelper::g_darkModeEnabled		= false;


// Should application use Dark Mode or not
bool DarkModeHelper::ShouldAppsUseDarkMode(void) noexcept
{
	if (pfShouldAppsUseDarkMode)
		return pfShouldAppsUseDarkMode();

	return false;
}

// Allow dark mode for the application or not
void DarkModeHelper::AllowDarkModeForApp(bool isAllowed) noexcept
{
	if (pfSetPreferredAppMode != nullptr)
		pfSetPreferredAppMode(isAllowed ? PreferredAppMode::ForceDark : PreferredAppMode::Default);

#if defined(_DARKMODE_SUPPORT_OLDER_OS)
	else if (pfAllowDarkModeForApp != nullptr)
		pfAllowDarkModeForApp(isAllowed);
#endif
}

// Allow Dark mode for a specific window or not
bool DarkModeHelper::AllowDarkModeForWindow(HWND hWnd, bool allow) noexcept
{
	if (g_darkModeSupported && (pfAllowDarkModeForWindow != nullptr))
		return pfAllowDarkModeForWindow(hWnd, allow);

	return false;
}

// Is system currently in high contrast mode
bool DarkModeHelper::IsHighContrast(void)
{
	HIGHCONTRASTW highContrast{};
	highContrast.cbSize = sizeof(HIGHCONTRASTW);
	if (SystemParametersInfoW(SPI_GETHIGHCONTRAST, sizeof(HIGHCONTRASTW), &highContrast, FALSE) == TRUE)
		return (highContrast.dwFlags & HCF_HIGHCONTRASTON) == HCF_HIGHCONTRASTON;

	return false;
}

#if defined(_DARKMODE_SUPPORT_OLDER_OS)
// Set title bar theme color
void DarkModeHelper::SetTitleBarThemeColor(HWND hWnd, BOOL isDark)
{
	using namespace WinVerHelper;
	if (g_buildNumber < WinVer::WIN10_VER_1903)
		SetPropW(hWnd, L"UseImmersiveDarkModeColors", reinterpret_cast<HANDLE>(static_cast<intptr_t>(isDark)));
	else if (pfSetWindowCompositionAttribute != nullptr) {
		WINDOWCOMPOSITIONATTRIBDATA data{ WCA_USEDARKMODECOLORS, &isDark, sizeof(isDark) };
		pfSetWindowCompositionAttribute(hWnd, &data);
	}
}

// Refresh title bar theme color
void DarkModeHelper::RefreshTitleBarThemeColor(HWND hWnd)
{
	BOOL isDark = FALSE;
	if (pfIsDarkModeAllowedForWindow != nullptr && pfShouldAppsUseDarkMode != nullptr) {
		if (pfIsDarkModeAllowedForWindow(hWnd) && pfShouldAppsUseDarkMode() && !IsHighContrast())
			isDark = TRUE;
	}

	SetTitleBarThemeColor(hWnd, isDark);
}
#endif

// Check for color scheme change message
bool DarkModeHelper::IsColorSchemeChangeMessage(LPARAM lParam)
{
	bool result = false;
	if ((lParam != 0) // NULL
		&& (_wcsicmp(reinterpret_cast<LPCWSTR>(lParam), L"ImmersiveColorSet") == 0)
		&& pfRefreshImmersiveColorPolicyState != nullptr) {
		pfRefreshImmersiveColorPolicyState();
		result = true;
	}

	if (pfGetIsImmersiveColorUsingHighContrast)
		pfGetIsImmersiveColorUsingHighContrast(IHCM_REFRESH);

	return result;
}

// Check for color scheme change message
bool DarkModeHelper::IsColorSchemeChangeMessage(UINT message, LPARAM lParam)
{
	if (message == WM_SETTINGCHANGE)
		return IsColorSchemeChangeMessage(lParam);

	return false;
}

// Flush menu themes
static void FlushMenuThemes() noexcept
{
	if (pfFlushMenuThemes)
		pfFlushMenuThemes();
}


/// For limitting dark scroll bar to specific windows and their children

#include <mutex>
#include <unordered_set>

static std::mutex				g_darkScrollBarMutex;
static std::unordered_set<HWND> g_darkScrollBarWindows;

// Enable dark scroll bars for a specific window and its children
void DarkModeHelper::EnableDarkScrollBarForWindowAndChildren(HWND hWnd)
{
	const std::lock_guard<std::mutex> lock(g_darkScrollBarMutex);
	g_darkScrollBarWindows.insert(hWnd);
}

// Is a specific window or its parent using dark scroll bars or not
static bool IsWindowOrParentUsingDarkScrollBar(HWND hWnd)
{
	HWND hRoot = GetAncestor(hWnd, GA_ROOT);

	const std::lock_guard<std::mutex> lock(g_darkScrollBarMutex);
	auto hasElement = [](const auto& container, HWND hWndToCheck) -> bool {
#if (defined(_MSC_VER) && (_MSVC_LANG >= 202002L)) || (__cplusplus >= 202002L)
		return container.contains(hWndToCheck);
#else
		return container.count(hWndToCheck) != 0;
#endif
	};

	if (hasElement(g_darkScrollBarWindows, hWnd))
		return true;

	return (hWnd != hRoot && hasElement(g_darkScrollBarWindows, hRoot));
}

// Custom OpenNcThemeData() replacement
static HTHEME WINAPI MyOpenNcThemeData(HWND hWnd, LPCWSTR pszClassList)
{
	if (std::wcscmp(pszClassList, WC_SCROLLBAR) == 0) {
		if (IsWindowOrParentUsingDarkScrollBar(hWnd)) {
			hWnd = nullptr;
			pszClassList = L"Explorer::ScrollBar";
		}
	}

	return pfOpenNcThemeData(hWnd, pszClassList);
}

// Fix dark scroll bar
static void FixDarkScrollBar()
{
	const ModuleHandle moduleComctl(L"comctl32.dll");
	if (moduleComctl.isLoaded())
	{
		auto* addr = FindDelayLoadThunkInModule(moduleComctl.get(), "uxtheme.dll", 49); // OpenNcThemeData
		if (addr != nullptr) // && pfOpenNcThemeData != nullptr) // checked in InitDarkMode
			replaceFunction<fnOpenNcThemeData>(addr, MyOpenNcThemeData);
	}
}

// Initialize Dark Mode
void DarkModeHelper::InitDarkMode()
{
	static bool isInit = false;
	if (isInit)
		return;

	using namespace WinVerHelper;
	DWORD major = 0, minor = 0, buildNumber = 0;
	if (!getOSVersionNumber(major, minor, buildNumber))
		return;

	g_buildNumber = buildNumber;

	if (major == 10 && minor == 0 && g_buildNumber >= MinSupportVersion)
	{
		const ModuleHandle moduleUxtheme(L"uxtheme.dll");
		if (!moduleUxtheme.isLoaded())
			return;

		bool isAllFuncLoadedOK = false;

#if defined(_DARKMODE_SUPPORT_OLDER_OS)
		if (g_buildNumber < WinVer::WIN10_VER_1903)
			isAllFuncLoadedOK |= moduleUxtheme.loadFunction(pfAllowDarkModeForApp, 135);
		else
#endif
			isAllFuncLoadedOK |= moduleUxtheme.loadFunction(pfSetPreferredAppMode, 135);

		// Load all necessary functions
		isAllFuncLoadedOK |= moduleUxtheme.loadFunction(pfOpenNcThemeData, 49);
		isAllFuncLoadedOK |= moduleUxtheme.loadFunction(pfRefreshImmersiveColorPolicyState, 104);
		isAllFuncLoadedOK |= moduleUxtheme.loadFunction(pfShouldAppsUseDarkMode, 132);
		isAllFuncLoadedOK |= moduleUxtheme.loadFunction(pfAllowDarkModeForWindow, 133);
		isAllFuncLoadedOK |= moduleUxtheme.loadFunction(pfFlushMenuThemes, 136);
		isAllFuncLoadedOK |= moduleUxtheme.loadFunction(pfIsDarkModeAllowedForWindow, 137);

		if (isAllFuncLoadedOK)
			g_darkModeSupported = true;

		moduleUxtheme.loadFunction(pfGetIsImmersiveColorUsingHighContrast, 106);

#if defined(_DARKMODE_SUPPORT_OLDER_OS)
		if (g_buildNumber < WinVer::WIN10_VER_2004)	{
			ModuleHandle moduleUser32(L"user32.dll", ModuleHandle::InitMode::GetModuleHandle);
			moduleUser32.loadFunction(pfSetWindowCompositionAttribute, "SetWindowCompositionAttribute");
		}
#endif
		isInit = true;
	}
}

// Set Dark Mode
void DarkModeHelper::SetDarkMode(bool useDarkMode, bool doFixDarkScrollbar)
{
	if (g_darkModeSupported)
	{
		AllowDarkModeForApp(useDarkMode);
		FlushMenuThemes();
		if (doFixDarkScrollbar)
			FixDarkScrollBar();

		// Set the flag
		g_darkModeEnabled = useDarkMode && ShouldAppsUseDarkMode() && !IsHighContrast();
	}
}


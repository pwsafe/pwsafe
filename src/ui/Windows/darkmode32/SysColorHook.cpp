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


#include "SysColorHook.h"
#include "DarkMode.h"
#include "ModuleHelper.h"

using namespace SysColorHook;
using namespace DarkModeHelper;
using namespace ModuleHelper;


// Dark mode external IatHook
#if !defined(_DARKMODE_EXTERNAL_IATHOOK)
	#include "IatHook.h"
#else
	extern PIMAGE_THUNK_DATA FindAddressByName(void* moduleBase, PIMAGE_THUNK_DATA impName, PIMAGE_THUNK_DATA impAddr, const char* funcName);
	extern PIMAGE_THUNK_DATA FindAddressByOrdinal(void* moduleBase, PIMAGE_THUNK_DATA impName, PIMAGE_THUNK_DATA impAddr, uint16_t ordinal);
	extern PIMAGE_THUNK_DATA FindIatThunkInModule(void* moduleBase, const char* dllName, const char* funcName);
	extern PIMAGE_THUNK_DATA FindDelayLoadThunkInModule(void* moduleBase, const char* dllName, const char* funcName);
	extern PIMAGE_THUNK_DATA FindDelayLoadThunkInModule(void* moduleBase, const char* dllName, uint16_t ordinal);
#endif


/// For hooking GetSysColor for comboboxex' list box and list view's gridlines
static fnGetSysColor pfGetSysColor	= nullptr;
static bool g_isGetSysColorHooked	= false;
static int g_hookRefCount			= 0;

/// Custom colors for overridden system color indices
static COLORREF g_clrWindow		= RGB(32, 32, 32);
static COLORREF g_clrText		= RGB(224, 224, 224);
static COLORREF g_clrGridLines	= RGB(100, 100, 100);


// Override system color
void SysColorHook::SetMySysColor(int nIndex, COLORREF clr) noexcept
{
	switch (nIndex)
	{
	case COLOR_WINDOW:
		g_clrWindow = clr;
		break;

	case COLOR_WINDOWTEXT:
		g_clrText = clr;
		break;

	case COLOR_BTNFACE:
		g_clrGridLines = clr;
		break;

	default:
		break;
	}
}

// Custom GetSysColor() replacement
static DWORD WINAPI GetSysColorProc(int nIndex)
{
	if (!g_darkModeEnabled)
		return GetSysColor(nIndex);

	switch (nIndex)
	{
	case COLOR_WINDOW:
		return g_clrWindow;

	case COLOR_WINDOWTEXT:
		return g_clrText;

	case COLOR_BTNFACE:
		return g_clrGridLines;

	default:
		return GetSysColor(nIndex);
	}
}

// Install system color hook
bool SysColorHook::HookSysColor()
{
	const ModuleHandle moduleComctl(L"comctl32.dll");
	if (!moduleComctl.isLoaded())
		return false;

	if (pfGetSysColor == nullptr || !g_isGetSysColorHooked)
	{
		auto* addr = FindIatThunkInModule(moduleComctl.get(), "user32.dll", "GetSysColor");
		if (addr != nullptr)
		{
			pfGetSysColor = replaceFunction<fnGetSysColor>(addr, GetSysColorProc);
			g_isGetSysColorHooked = true;
		}
		else
			return false;
	}

	if (g_isGetSysColorHooked)
		++g_hookRefCount;

	return true;
}

// Uninstall system color hook
void SysColorHook::UnhookSysColor()
{
	const ModuleHandle moduleComctl(L"comctl32.dll");
	if (!moduleComctl.isLoaded())
		return;

	if (!g_isGetSysColorHooked)
		return;

	if (g_hookRefCount > 0)
		--g_hookRefCount;

	if (g_hookRefCount == 0)
	{
		auto* addr = FindIatThunkInModule(moduleComctl.get(), "user32.dll", "GetSysColor");
		if (addr != nullptr)
		{
			replaceFunction<fnGetSysColor>(addr, pfGetSysColor);
			g_isGetSysColorHooked = false;
		}
	}
}

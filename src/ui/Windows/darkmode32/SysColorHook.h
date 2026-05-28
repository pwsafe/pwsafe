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

namespace SysColorHook
{
	/**
	 * @brief Function pointer type for the original `GetSysColor` function.
	 * 
	 * This type is used to store the address of the original `GetSysColor` function before it is hooked,
	 * allowing the hook to call the original function for indices that are not overridden.
	 * 
	 * @param nIndex The index of the system color to retrieve.
	 * @return The color value corresponding to the specified system color index.
	 * 
	 * @remarks The `GetSysColor` function is a Windows API function that retrieves the current color of a specified display element,
	 * 			such as the background color of a window or the text color of a control. By hooking this function, 
	 * 			we can return custom color values for specific indices to achieve a consistent dark mode appearance.
	 */
	using fnGetSysColor = auto (WINAPI*)(int nIndex)->DWORD;

	/**
	 * @brief Override specific system colors with custom values.
	 * 
	 * @param nIndex The index of the system color to override.
	 * @param clr The custom color value to use for the specified system color.
	 * 
	 * @remarks This function allows for dynamic customization of system colors, 
	 * 			which can be useful for implementing dark mode or other visual themes.
	 * @remarks Supported system color indices include:
	 * - `COLOR_WINDOW`: Background of ComboBoxEx list.
	 * - `COLOR_WINDOWTEXT`: Text color of ComboBoxEx list.
	 * - `COLOR_BTNFACE`: Gridline color in ListView (when applicable).
	 * @remarks The hook will intercept calls to `GetSysColor` and return the specified custom color for the targeted indices, 
	 * 			while allowing other system colors to be retrieved normally.
	 */
	void SetMySysColor(int nIndex, COLORREF clr) noexcept;

	/**
	 * @brief Installs a hook to override the `GetSysColor` function for specific color indices.
	 * 
	 * This function replaces the `GetSysColor` function in the import address table (IAT) of the target module 
	 * with a custom implementation that returns overridden color values for certain indices.
	 * 
	 * @return `true` if the hook was successfully installed; otherwise, `false`.
	 */
	bool HookSysColor();

	/**
	 * @brief Uninstalls the previously installed `GetSysColor` hook.
	 * 
	 * This function restores the original `GetSysColor` function in the IAT, removing any overrides for system colors.
	 * It also manages reference counting to ensure that the hook is only removed when no longer needed.
	 */
	void UnhookSysColor();
};

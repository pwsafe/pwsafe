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
#include "ModuleHelper.h"

namespace WinVerHelper
{
	enum WinVer {
		// Windows 10 versions
		WIN10_VER_1507 = (unsigned short)10240,	// Original release
		WIN10_VER_1511 = (unsigned short)10586,	// November Update
		WIN10_VER_1607 = (unsigned short)14393,	// Anniversary Update
		WIN10_VER_1703 = (unsigned short)15063,	// Creators Update
		WIN10_VER_1709 = (unsigned short)16299,	// Fall Creators Update
		WIN10_VER_1803 = (unsigned short)17134,	// April 2018 Update
		WIN10_VER_1809 = (unsigned short)17763,	// October 2018 Update
		WIN10_VER_1903 = (unsigned short)18362,	// May 2019 Update
		WIN10_VER_1909 = (unsigned short)18363,	// November 2019 Update
		WIN10_VER_2004 = (unsigned short)19041,	// May 2020 Update
		WIN10_VER_20H2 = (unsigned short)19042,	// October 2020 Update
		WIN10_VER_21H1 = (unsigned short)19043,	// May 2021 Update
		WIN10_VER_21H2 = (unsigned short)19044,	// November 2021 Update
		WIN10_VER_22H2 = (unsigned short)19045,	// October 2022 Update

		// Windows 11 versions
		WIN11_VER_21H2 = (unsigned short)22000,	// Original release
		WIN11_VER_22H2 = (unsigned short)22621,	// 2022 Update
		WIN11_VER_23H2 = (unsigned short)22631,	// 2023 Update
		WIN11_VER_24H2 = (unsigned short)26100,	// 2024 Update
		WIN11_VER_25H2 = (unsigned short)26200,	// 2025 Update
	};

	// Get Windows OS version build number
	inline bool getOSVersionNumber(DWORD &major, DWORD &minor, DWORD &buildNumber) noexcept {
		using namespace ModuleHelper;
		ModuleHandle moduleNtDll(L"ntdll.dll", ModuleHandle::InitMode::GetModuleHandle);
		if (moduleNtDll.isLoaded()) {
			using fnRtlGetNtVersionNumbers = void (WINAPI*)(LPDWORD major, LPDWORD minor, LPDWORD build);
			fnRtlGetNtVersionNumbers RtlGetNtVersionNumbers = nullptr;
			if (moduleNtDll.loadFunction(RtlGetNtVersionNumbers, "RtlGetNtVersionNumbers") && RtlGetNtVersionNumbers)
			{
				RtlGetNtVersionNumbers(&major, &minor, &buildNumber);
				buildNumber &= ~0xF0000000;
				return true;
			}
		}

		return false;
	}

	// Check if is a Windows version or later
	[[nodiscard]] inline bool isWinVer_OrLater(WORD wVersion, WORD wBuildNumber = 0) noexcept {
		DWORDLONG dwlConditionMask = 0;
		VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL);
		VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, VER_GREATER_EQUAL);
		VER_SET_CONDITION(dwlConditionMask, VER_BUILDNUMBER, VER_GREATER_EQUAL);

		OSVERSIONINFOEXW osvi{ sizeof(OSVERSIONINFOEX) };
		osvi.dwMajorVersion = HIBYTE(wVersion);
		osvi.dwMinorVersion = LOBYTE(wVersion);
		osvi.dwBuildNumber = wBuildNumber;

		return !!VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_BUILDNUMBER, dwlConditionMask);
	}

	// Is Windows 10 version or later???
	[[nodiscard]] inline bool isWindows10_OrLater() noexcept {
		return isWinVer_OrLater(0x0a00, WIN10_VER_1507);
	}

	// Is Windows 11 version or later???
	[[nodiscard]] inline bool isWindows11_OrLater() noexcept {
		return isWinVer_OrLater(0x0a00, WIN11_VER_21H2);
	}
};

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

/*
 * This file includes modified code from 'PolyHook 2.0' by stevemk14ebr (MIT License),
 * available at: https://github.com/stevemk14ebr/PolyHook_2_0/blob/master/sources/IatHook.cpp
 * See the `licenses/` folder for more information.
 */

#pragma once

#include <windows.h>

#include <cstdint>

/**
 * @brief Converts a Relative Virtual Address (RVA) to an absolute Virtual Address (VA) based on the module's base address.
 * 
 * @param base The base address of the module.
 * @param rva The Relative Virtual Address to convert.
 * @return The absolute Virtual Address corresponding to the given RVA.
 * 
 * @remarks This function takes a base address and an RVA, and calculates the absolute VA by adding the RVA to the base address.
 * 			It is commonly used when parsing PE headers to locate specific data or code sections within a module.
 */
template <typename T, typename T1, typename T2>
inline constexpr T RVA2VA(T1 base, T2 rva) {
	return reinterpret_cast<T>(reinterpret_cast<ULONG_PTR>(base) + rva);
}

/**
 * @brief Retrieves a pointer to a specific data directory entry from the PE headers of a module.
 * 
 * @param moduleBase The base address of the module to retrieve the data directory from.
 * @param entryID The index of the data directory entry to retrieve (e.g., IMAGE_DIRECTORY_ENTRY_IMPORT).
 * @return A pointer to the specified data directory entry, or nullptr if the entry is not present.
 * 
 * @remarks This function parses the PE headers of the specified module to locate the data directory entry 
 * 			corresponding to the given index.
 * 			It returns a pointer to the entry, which can be used to access important information such as the import table, 
 * 			export table, or other relevant data structures defined in the PE format.
 * 			If the specified entry is not present in the PE headers, it returns nullptr.
 */
template <typename T>
inline constexpr T DataDirectoryFromModuleBase(void* moduleBase, size_t entryID) {
	const auto* dosHdr = static_cast<PIMAGE_DOS_HEADER>(moduleBase);
	const auto* ntHdr = RVA2VA<PIMAGE_NT_HEADERS>(moduleBase, static_cast<DWORD>(dosHdr->e_lfanew));
	const auto* dataDir = ntHdr->OptionalHeader.DataDirectory;
	return RVA2VA<T>(moduleBase, dataDir[entryID].VirtualAddress);
}

/**
 * @brief Find the IAT entry for a given function in a specified module and return a pointer to it.
 * 
 * @param moduleBase The base address of the module to search within.
 * @param dllName The name of the DLL that contains the function (e.g., "user32.dll").
 * @param funcName The name of the function to find (e.g., "GetSysColor").
 * @return A pointer to the IAT entry for the specified function, or nullptr if not found.
 *
 * @remarks This function searches the Import Address Table (IAT) of the specified module for an entry corresponding to the given function name.
 * 			If found, it returns a pointer to the IAT entry, which can be used for hooking or unhooking the function.
 * 			If the function is not found in the IAT, it returns nullptr.
 */
inline PIMAGE_THUNK_DATA FindAddressByName(void* moduleBase, PIMAGE_THUNK_DATA impName, PIMAGE_THUNK_DATA impAddr, const char* funcName) {
	for (; impName->u1.Ordinal != 0; ++impName, ++impAddr) {
		if (IMAGE_SNAP_BY_ORDINAL(impName->u1.Ordinal)) {
			continue;
		}

		const auto* import = RVA2VA<PIMAGE_IMPORT_BY_NAME>(moduleBase, impName->u1.AddressOfData);
		if (strcmp(reinterpret_cast<const char*>(import->Name), funcName) != 0) {
			continue;
		}
		return impAddr;
	}
	return nullptr;
}

/**
 * @brief Find the IAT entry for a given function in a specified module by ordinal and return a pointer to it.
 * 
 * @param moduleBase The base address of the module to search within.
 * @param dllName The name of the DLL that contains the function (e.g., "user32.dll").
 * @param ordinal The ordinal number of the function to find (e.g., 123).
 * @return A pointer to the IAT entry for the specified function, or nullptr if not found.
 * 
 * @remarks This function searches the Import Address Table (IAT) of the specified module for an entry corresponding to the given ordinal number.
 * 			If found, it returns a pointer to the IAT entry, which can be used for hooking or unhooking the function.
 * 			If the function is not found in the IAT, it returns nullptr.
 */
inline PIMAGE_THUNK_DATA FindAddressByOrdinal(void* /*moduleBase*/, PIMAGE_THUNK_DATA impName, PIMAGE_THUNK_DATA impAddr, uint16_t ordinal) {
	for (; impName->u1.Ordinal != 0; ++impName, ++impAddr) {
		if (IMAGE_SNAP_BY_ORDINAL(impName->u1.Ordinal) && IMAGE_ORDINAL(impName->u1.Ordinal) == ordinal) {
			return impAddr;
		}
	}
	return nullptr;
}

/**
 * @brief Find the IAT entry for a given function in a specified module's import table and return a pointer to it.
 * 
 * @param moduleBase The base address of the module to search within.
 * @param dllName The name of the DLL that contains the function (e.g., "user32.dll").
 * @param funcName The name of the function to find (e.g., "GetSysColor").
 * @return A pointer to the IAT entry for the specified function, or nullptr if not found.
 * 
 * @remarks This function searches the Import Address Table (IAT) of the specified module for an entry corresponding to the given function name.
 * 			If found, it returns a pointer to the IAT entry, which can be used for hooking or unhooking the function.
 * 			If the function is not found in the IAT, it returns nullptr.
 */
inline PIMAGE_THUNK_DATA FindIatThunkInModule(void* moduleBase, const char* dllName, const char* funcName) {
	auto* imports = DataDirectoryFromModuleBase<PIMAGE_IMPORT_DESCRIPTOR>(moduleBase, IMAGE_DIRECTORY_ENTRY_IMPORT);
	for (; imports->Name != 0; ++imports) {
		if (_stricmp(RVA2VA<LPCSTR>(moduleBase, imports->Name), dllName) != 0) {
			continue;
		}

		auto* origThunk = RVA2VA<PIMAGE_THUNK_DATA>(moduleBase, imports->OriginalFirstThunk);
		auto* thunk = RVA2VA<PIMAGE_THUNK_DATA>(moduleBase, imports->FirstThunk);
		return FindAddressByName(moduleBase, origThunk, thunk, funcName);
	}
	return nullptr;
}

/**
 * @brief Find the delay-load IAT entry for a given function in a specified module's delay-load import table 
 * 		  and return a pointer to it.
 * 
 * @param moduleBase The base address of the module to search within.
 * @param dllName The name of the DLL that contains the function (e.g., "user32.dll").
 * @param funcName The name of the function to find (e.g., "GetSysColor").
 *
 * @remarks This function searches the delay-load Import Address Table (IAT) of the specified module 
 * 			for an entry corresponding to the given function name.
 * 			If found, it returns a pointer to the IAT entry, which can be used for hooking or unhooking the function.
 * 			If the function is not found in the delay-load IAT, it returns nullptr.
 */
inline PIMAGE_THUNK_DATA FindDelayLoadThunkInModule(void* moduleBase, const char* dllName, const char* funcName) {
	auto* imports = DataDirectoryFromModuleBase<PIMAGE_DELAYLOAD_DESCRIPTOR>(moduleBase, IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT);
	for (; imports->DllNameRVA != 0; ++imports) {
		if (_stricmp(RVA2VA<LPCSTR>(moduleBase, imports->DllNameRVA), dllName) != 0){
			continue;
		}

		auto* impName = RVA2VA<PIMAGE_THUNK_DATA>(moduleBase, imports->ImportNameTableRVA);
		auto* impAddr = RVA2VA<PIMAGE_THUNK_DATA>(moduleBase, imports->ImportAddressTableRVA);
		return FindAddressByName(moduleBase, impName, impAddr, funcName);
	}
	return nullptr;
}

/**
 * @brief Find the delay-load IAT entry for a given function in a specified module's delay-load import table 
 * 		  by ordinal and return a pointer to it.
 * 
 * @param moduleBase The base address of the module to search within.
 * @param dllName The name of the DLL that contains the function (e.g., "user32.dll").
 * @param ordinal The ordinal number of the function to find (e.g., 123).
 *
 * @remarks This function searches the delay-load Import Address Table (IAT) of the specified module 
 * 			for an entry corresponding to the given ordinal number.
 * 			If found, it returns a pointer to the IAT entry, which can be used for hooking or unhooking the function.
 * 			If the function is not found in the delay-load IAT, it returns nullptr.
 */
inline PIMAGE_THUNK_DATA FindDelayLoadThunkInModule(void* moduleBase, const char* dllName, uint16_t ordinal) {
	auto* imports = DataDirectoryFromModuleBase<PIMAGE_DELAYLOAD_DESCRIPTOR>(moduleBase, IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT);
	for (; imports->DllNameRVA != 0; ++imports) {
		if (_stricmp(RVA2VA<LPCSTR>(moduleBase, imports->DllNameRVA), dllName) != 0) {
			continue;
		}

		auto* impName = RVA2VA<PIMAGE_THUNK_DATA>(moduleBase, imports->ImportNameTableRVA);
		auto* impAddr = RVA2VA<PIMAGE_THUNK_DATA>(moduleBase, imports->ImportAddressTableRVA);
		return FindAddressByOrdinal(moduleBase, impName, impAddr, ordinal);
	}
	return nullptr;
}

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

#define DM_VERSION_MAJOR						1
#define DM_VERSION_MINOR						0
#define DM_VERSION_PATCH						1
#define DM_VERSION_REVISION						0

#define DM_PROJECT_NAME							L"darkmode32plus"
#define DM_PRODUCT_INFO							L"Native Dark Mode Library for Win32 API Applications"
#define DM_COMPANY_NAME							L"Stark Personal"
#define DM_COPYRIGHT_INFO						L"Copyright (c) 2025 Anthony Lee Stark. All rights reserved."

#define DM_VERSION_PRODUCT_NAME					DM_PRODUCT_INFO
#define DM_VERSION_INTERNAL_NAME				L"darkmode32plus.lib"
#define DM_VERSION_FILE_DESCRIPTION 			L"darkmode32plus v1.0.1.0"
#define DM_VERSION_COMPANY_NAME					DM_COMPANY_NAME
#define DM_VERSION_LEGAL_COPYRIGHT				DM_COPYRIGHT_INFO

#define DM_VERSION_PRODUCT_VALUE 				L"1.0"
#define DM_VERSION_FILE_VALUE					L"1.0.1.0"
#define DM_VERSION_PRODUCT_DIGITAL_VALUE 		DM_VERSION_MAJOR,DM_VERSION_MINOR
#define DM_VERSION_FILE_DIGITAL_VALUE 			DM_VERSION_MAJOR,DM_VERSION_MINOR,DM_VERSION_PATCH,DM_VERSION_REVISION

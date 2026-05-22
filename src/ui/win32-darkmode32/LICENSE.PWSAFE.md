# Password Safe provenance for win32-darkmode32

This directory contains a Password Safe copy of `darkmode32plus`, a Win32
dark-mode helper library originally published at:

https://github.com/anthonyleestark/darkmode32plus

The upstream project has its own repository history. The upstream baseline
reviewed for this copy is:

- Repository: `anthonyleestark/darkmode32plus`
- Commit: `03ca2e956df57af4eadeec9076bb44ac596a9572`
- Commit date: 2026-03-12

Marton Anka forked the project at:

https://github.com/martona/darkmode32plus

He then fixed a Windows build-number bug, added dark-mode-aware `MessageBoxW`
support, and integrated the resulting sources into Password Safe.

## Directory license summary

The upstream `darkmode32plus` files are primarily distributed under the
BSD-3-Clause license by Anthony Lee Stark, with source-level notices for
incorporated work from other projects. Password Safe-local additions are
identified below.

This file is an attribution and provenance ledger for the copy in Password
Safe. It does not remove, narrow, or replace the notices in individual source
files.

## File attribution

| File(s) | Origin / copyright holder(s) | Applicable license / notes |
| --- | --- | --- |
| `DarkMode.h`, `DarkMode.cpp`, `SysColorHook.h`, `SysColorHook.cpp`, `ModuleHelper.h`, `WinVerHelper.h`, `Version.h` | Anthony Lee Stark, 2025. Upstream project also credits `win32-darkmode` by ysc3839 / Richard Yu and `darkmodelib` by ozone10 for incorporated portions. | BSD-3-Clause for `darkmode32plus` portions. Incorporated portions retain their respective upstream licenses: MIT for `win32-darkmode`; MPL-2.0 for `darkmodelib` material as identified by source headers. Upstream also carries an MIT notice for ozone10 in `licenses/darkmodelib/LICENSE-MIT.md`. `DarkMode.cpp` also contains a Password Safe-local bug fix by Marton Anka that preserves the containing file's BSD-3-Clause licensing. |
| `DMSubclass.h`, `DMSubclass.cpp` | Anthony Lee Stark, 2025. Source comments identify Notepad++ dark-mode code originally by Adam D. Walling (`adzm`), with modifications by ozone10 and the Notepad++ team. | BSD-3-Clause in this distribution as stated by the source comments, including the upstream note that permission was obtained to relicense the relevant Notepad++-derived code under BSD-3-Clause. `DMSubclass.h` also contains a Password Safe-local declaration for `DarkMessageBox` by Marton Anka, preserving the containing file's BSD-3-Clause licensing. |
| `IatHook.h` | Anthony Lee Stark, 2025. Includes modified code from PolyHook 2.0 by Stephen Eckels / `stevemk14ebr`. | BSD-3-Clause for `darkmode32plus` portions; MIT for incorporated PolyHook 2.0 portions. |
| `UAHMenuBar.h` | Anthony Lee Stark, 2025. Source comments identify original UAH menu-bar code by Adam D. Walling (`adzm`), 2021. | BSD-3-Clause for `darkmode32plus` portions; MIT for incorporated UAHMenuBar portions. |
| `DarkMessageBox.cpp` | Marton Anka, 2026. | MIT License, as stated in the file header. |
| `darkmode32.vcxproj`, `darkmode32.vcxproj.filters` | Derived from upstream `darkmode32plus` Visual Studio project metadata and adapted for Password Safe by Marton Anka. | BSD-3-Clause for upstream-derived project metadata; Password Safe integration changes follow the same license as the upstream project metadata unless otherwise noted. |

## Third-party projects named by this directory

- `darkmode32plus`: https://github.com/anthonyleestark/darkmode32plus
- `win32-darkmode`: https://github.com/ysc3839/win32-darkmode
- `darkmodelib`: https://github.com/ozone10/darkmodelib
- Notepad++: https://github.com/notepad-plus-plus/notepad-plus-plus
- PolyHook 2.0: https://github.com/stevemk14ebr/PolyHook_2_0
- UAHMenuBar source by Adam D. Walling (`adzm`): https://gist.github.com/adzm/2f82b2b5c7a3c6f007397e5ed885d0a6

## Local Password Safe changes relative to the reviewed upstream baseline

- `DarkMessageBox.cpp` was added to provide dark-mode-aware `MessageBoxW`
  support.
- `DMSubclass.h` was updated to declare `DarkMessageBox`.
- `DarkMode.cpp` was updated so the detected Windows build number is stored
  before checking the minimum supported version.
- `darkmode32.vcxproj` and `darkmode32.vcxproj.filters` were adapted to build
  the library inside the Password Safe MSVC/CMake layout.

/*
* Copyright (c) 2003-2026 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file PWSversion.cpp
//-----------------------------------------------------------------------------

#include "PWSversion.h"
#include "StringX.h" // for Format()

#include "version.h"

// version.h is generated (via CMake or the legacy pre_build.vbs scripts) from
// either src/ui/Windows/version.in (MFC build) or src/ui/wxWidgets/version.in
// (wxWidgets build), which unfortunately don't use the same macro names.
// Map both flavours onto a single set of names used below.

// Two-step indirection needed so that e.g. WIDEN(VCS_VERSION) widens the
// *value* of VCS_VERSION rather than pasting L onto the macro name itself.
#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)

#if defined(PWS_VERSION_MAJOR)
// Windows/MFC-style version.h - see src/ui/Windows/version.in
#define PWSVER_MAJOR PWS_VERSION_MAJOR
#define PWSVER_MINOR PWS_VERSION_MINOR
#define PWSVER_BUILD PWS_REVISION
#define PWSVER_VERSTRING PWS_VERSTRING
#ifdef PWS_SPECIALBUILD_STR
#define PWSVER_SPECIALBUILD WIDEN(PWS_SPECIALBUILD_STR)
#endif
#else
// wxWidgets-style version.h - see src/ui/wxWidgets/version.in
#define PWSVER_MAJOR MAJORVERSION
#define PWSVER_MINOR MINORVERSION
#define PWSVER_BUILD REVISION
#define PWSVER_VERSTRING WIDEN(VCS_VERSION)
#define PWSVER_SPECIALBUILD SPECIALBUILD
#endif

PWSversion *PWSversion::self = nullptr;

PWSversion *PWSversion::GetInstance()
{
  if (self == nullptr) {
    self = new PWSversion();
  }
  return self;
}

void PWSversion::DeleteInstance()
{
  delete self;
  self = nullptr;
}

PWSversion::PWSversion()
  : m_nMajor(PWSVER_MAJOR), m_nMinor(PWSVER_MINOR), m_nBuild(PWSVER_BUILD),
    m_Revision(PWSVER_VERSTRING), m_bModified(false)
{
#ifdef PWSVER_SPECIALBUILD
  m_SpecialBuild = PWSVER_SPECIALBUILD;
#endif

  m_builtOn = stringT(WIDEN(__DATE__)) + L" " + stringT(WIDEN(__TIME__));
  m_bModified = !m_Revision.empty() && m_Revision.back() == L'+';
}

stringT PWSversion::FormatVersionString(int major, int minor, int build)
{
  stringT version;
  if (build == 0) // hide build # if zero (formal release)
    Format(version, L"%d.%d", major, minor);
  else
    Format(version, L"%d.%d.%d", major, minor, build);
  return version;
}

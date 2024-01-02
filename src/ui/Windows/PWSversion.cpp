/*
* Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file PWSversion.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"

#include "PWSversion.h"
#include "version.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)

PWSversion *PWSversion::self = NULL;

PWSversion *PWSversion::GetInstance()
{
  if (self == NULL) {
    self = new PWSversion();
  }
  return self;
}

void PWSversion::DeleteInstance()
{
  delete self;
  self = NULL;
}

PWSversion::PWSversion()
  : m_nMajor(PWS_VERSION_MAJOR), m_nMinor(PWS_VERSION_MINOR), m_nBuild(PWS_REVISION),
    m_Revision(PWS_VERSTRING), m_bModified(false)
{
#ifdef PWS_SPECIALBUILD_STR
  m_SpecialBuild = PWS_SPECIALBUILD_STR;
#endif

  m_builtOn = CString(__DATE__) + CString(L" ") + CString(__TIME__);
  m_bModified = m_Revision.Right(1) == L"+";
}

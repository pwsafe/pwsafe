/*
* Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#include "SysInfo.h"
#include "os/env.h"
//-----------------------------------------------------------------
//
// Singleton class to provide system-specific information,
// such as hostname, username, pid
//-----------------------------------------------------------------

SysInfo *SysInfo::self = nullptr;

SysInfo *SysInfo::GetInstance()
{
  if (self == nullptr)
    self = new SysInfo;
  return self;
}

void SysInfo::DeleteInstance()
{
  delete self;
  self = nullptr;
}

SysInfo::SysInfo()
{
  m_euser = m_ruser = pws_os::getusername();
  m_esysname = m_rsysname = pws_os::gethostname();
  m_ProcessID = pws_os::getprocessid();
}


bool SysInfo::IsUnderPw2go()
{
  return !pws_os::getenv("PWS2GO", false).empty();
}

// The wxWidgets UI is used for Linux and macOS
bool SysInfo::IsWXUI()
{
#if defined(__linux__) || defined(__APPLE__)
  return true;
#else
  return false;
#endif
}

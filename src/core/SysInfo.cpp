/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
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

bool SysInfo::IsUnderU3()
{
  return !pws_os::getenv("U3_ENV_VERSION", false).empty();
}

bool SysInfo::IsUnderPw2go()
{
  return !pws_os::getenv("PWS2GO", false).empty();
}

bool SysInfo::IsLinux()
{
#ifdef __linux__
  return true;
#else
  return false;
#endif
}

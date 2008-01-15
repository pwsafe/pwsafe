/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#include "SysInfo.h"
//-----------------------------------------------------------------
//
// Singleton class to provide system-specific information,
// such as hostname, username, pid
//-----------------------------------------------------------------

#include <LMCONS.H> // for UNLEN definition

SysInfo *SysInfo::self = NULL;

SysInfo *SysInfo::GetInstance()
{
  if (self == NULL)
    self = new SysInfo;
  return self;
}

void SysInfo::DeleteInstance()
{
  delete self;
  self = NULL;
}

SysInfo::SysInfo()
{
  TCHAR user[UNLEN + sizeof(TCHAR)];
  TCHAR sysname[MAX_COMPUTERNAME_LENGTH + sizeof(TCHAR)];
  //  ulen INCLUDES the trailing blank
  DWORD ulen = UNLEN + sizeof(TCHAR);
  if (::GetUserName(user, &ulen)== FALSE) {
    user[0] = TCHAR('?');
    user[1] = TCHAR('\0');
    ulen = 2;
  }
  ulen--;

  //  slen EXCLUDES the trailing blank
  DWORD slen = MAX_COMPUTERNAME_LENGTH + sizeof(TCHAR);
  if (::GetComputerName(sysname, &slen) == FALSE) {
    sysname[0] = TCHAR('?');
    sysname[1] = TCHAR('\0');
    slen = 1;
  }
  m_euser = m_ruser = CString(user, ulen);
  m_esysname = m_rsysname = CString(sysname, slen);
  m_ProcessID.Format(_T("%08d"), GetCurrentProcessId());
}

CString SysInfo::GetEnv(const char *env)
{
  ASSERT(env != NULL);
  CString retval;
#if _MSC_VER < 1400
  retval = getenv(env);
#else
  char* value;
  size_t requiredSize;

  getenv_s(&requiredSize, NULL, 0, env);
  if (requiredSize > 0) {
    value = new char[requiredSize];
    ASSERT(value);
    if (value != NULL) {
      getenv_s( &requiredSize, value, requiredSize, env);
      retval = value;
      delete[] value;
      // make sure path has trailing '\'
      // yeah, this breaks non-dir getenvs - sosueme
      if (retval[retval.GetLength()-1] != TCHAR('\\'))
        retval += _T("\\");
    }
  }
#endif
  return retval;
}

bool SysInfo::IsUnderU3()
{
  return !GetEnv("U3_ENV_VERSION").IsEmpty();
}

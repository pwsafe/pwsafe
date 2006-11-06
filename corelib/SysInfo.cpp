/*
 * Copyright (c) 2003-2006 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
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
  m_user = CString(user, ulen);
  m_sysname = CString(sysname, slen);
  m_ProcessID.Format("%08d", GetCurrentProcessId());
}


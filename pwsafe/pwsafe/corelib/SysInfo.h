/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
#ifndef __SYSINFO_H
#define __SYSINFO_H

// SysInfo.h
//-----------------------------------------------------------------
//
// Singleton class to provide system-specific information,
// such as hostname, username, pid
//-----------------------------------------------------------------
#include "MyString.h"

class SysInfo
{
public:
  static SysInfo *GetInstance(); // singleton
  static void DeleteInstance();

  static bool IsUnderU3();

  const CString &GetCurrentUser() const {return m_user;}
  const CString &GetCurrentHost() const {return m_sysname;}
  const CString &GetCurrentPID() const {return m_ProcessID;}

  static CString GetEnv(const char *env); // wrapper for ::getenv()

private:
  SysInfo();
  ~SysInfo() {};

  static SysInfo *self;

  CString m_user, m_sysname, m_ProcessID;
};
#endif /* __SYSINFO_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:

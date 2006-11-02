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

  const CString &GetCurrentUser() const {return m_user;}
  const CString &GetCurrentHost() const {return m_sysname;}
  const CString &GetCurrentPID() const {return m_ProcessID;}

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

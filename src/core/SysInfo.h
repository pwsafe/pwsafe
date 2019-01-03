/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#ifndef __SYSINFO_H
#define __SYSINFO_H

// SysInfo.h
//-----------------------------------------------------------------
//
// Singleton class to provide system-specific information,
// such as hostname, username, pid
//
// As in Unix, we support the concept of an "effective" v.s. "real"
// name. The "real" is read from the system API, and is immutable.
// The "effective" is initially set to be == the real value, but
// may be overridden by the relevant SetEffective*() member function.
//-----------------------------------------------------------------
#include "os/typedefs.h"

class SysInfo
{
public:
  static SysInfo *GetInstance(); // singleton
  static void DeleteInstance();

  static bool IsUnderU3();
  static bool IsUnderPw2go();
  static bool IsLinux();

  void SetEffectiveUser(const stringT &u) {m_euser = u;}
  void SetEffectiveHost(const stringT &h) {m_esysname = h;}

  const stringT &GetRealUser() const {return m_ruser;}
  const stringT &GetRealHost() const {return m_rsysname;}
  const stringT &GetEffectiveUser() const {return m_euser;}
  const stringT &GetEffectiveHost() const {return m_esysname;}
  const stringT &GetCurrentPID() const {return m_ProcessID;}

private:
  SysInfo();
  ~SysInfo() {};

  static SysInfo *self;

  stringT m_ruser, m_rsysname;
  stringT m_euser, m_esysname;
  stringT m_ProcessID;
};
#endif /* __SYSINFO_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:

/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
#ifndef __PWSDIRS_H
#define __PWSDIRS_H
// PWSdirs.h
// Provide directories used by application
//
// Note that GetConfigDir will return value of environment var
// PWS_PREFSDIR if defined.
//
//-----------------------------------------------------------------------------
#include "MyString.h"
#include "SysInfo.h"
#include <stack>

class PWSdirs
{
 public:
  PWSdirs() {} // only need to create an object for push/pop
  PWSdirs(const CString &dir) {Push(dir);} // convenience: create & push
  ~PWSdirs(); // does a repeated Pop, so we're back where we started
  
  static CString GetSafeDir(); // default database location
  static CString GetConfigDir(); // pwsafe.cfg location
  static CString GetXMLDir(); // XML .xsd .xsl files
  static CString GetHelpDir(); // help file(s)
  static CString GetExeDir(); // location of executable

  void Push(const CString &dir); // cd to dir after saving current dir
  void Pop(); // cd to last dir, nop if stack empty
  
 private:
  static CString GetEnv(const char *env) {return SysInfo::GetEnv(env);}
  static CString GetMFNDir(); // wrapper for ::GetModuleFileName()
  std::stack<CString> dirs;
};
#endif /* __PWSDIRS_H */

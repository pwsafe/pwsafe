/*
 * Copyright (c) 2003-2006 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
#ifndef __PWSDIRS_H
#define __PWSDIRS_H
// PWSdirs.h
// Provide directories used by application
//-----------------------------------------------------------------------------
#include "MyString.h"

class PWSdirs
{
  public:
    static CString GetSafeDir(); // default database location
    static CString GetConfigDir(); // pwsafe.cfg location
    static CString GetXMLDir(); // XML .xsd .xsl files
    static CString GetHelpDir(); // help file(s)
    static CString GetExeDir(); // location of executable
  private:
    static CString GetEnv(const char *env); // wrapper for ::getenv()
    static CString GetMFNDir(); // wrapper for ::GetModuleFileName()
};
#endif /* __PWSDIRS_H */

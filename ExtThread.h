/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
#pragma once

#include <afxwin.h>

class CExtThread : public CWinThread
{
  public:
    CExtThread(AFX_THREADPROC proc, LPVOID p);
    ~CExtThread() {}
    static CExtThread * BeginThread(AFX_THREADPROC proc, LPVOID p);
};

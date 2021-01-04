/*
* Copyright (c) 2003-2021 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
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

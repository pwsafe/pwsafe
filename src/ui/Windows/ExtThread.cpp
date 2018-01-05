/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include <afxwin.h>
#include "ExtThread.h"

CExtThread::CExtThread(AFX_THREADPROC proc, LPVOID p)
  : CWinThread(proc, p)
{
  m_bAutoDelete = TRUE;
}

CExtThread * CExtThread::BeginThread(AFX_THREADPROC proc, LPVOID p)
{
  CExtThread *thread = new CExtThread(proc, p);
  if (!thread->CreateThread()) {
    delete thread;
    return NULL;
  }
  return thread;
}

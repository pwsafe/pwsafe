/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */

#include "stdafx.h"
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
  if(!thread->CreateThread( )) {
    delete thread;
    return NULL;
  }
  return thread;
}


/*
* Copyright (c) 2003-2015 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file DragDropAttachment.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"

#include "DragDropAttachment.h"
#include "DboxMain.h" // For SendMessage

BEGIN_MESSAGE_MAP(CDragDropAttachment, CStatic)
  ON_WM_DROPFILES()
END_MESSAGE_MAP()

void CDragDropAttachment::OnDropFiles(HDROP hDropInfo)
{
  CStatic::OnDropFiles(hDropInfo);

  UINT nCntFiles = DragQueryFile(hDropInfo, 0xFFFFFFFF, 0, 0);

  // Shouldn't have zero files if called!
  if (nCntFiles == 0)
    return;

  if (nCntFiles > 1) {
    const CString cs_errmsg = L"Sorry, currently there is a limit of only one attachment per entry";
    ::AfxMessageBox(cs_errmsg);
    return;
  }

  wchar_t szBuf[MAX_PATH];
  ::DragQueryFile(hDropInfo, 0, szBuf, sizeof(szBuf));

  // Get parent to process this file
  CWnd *pWnd = GetParent();
  ASSERT(pWnd);

  // Use SendMessage rather than PastMessage so that szBuf doesn't go out of scope
  // Send nCntFiles even though only one attachment is supported at the moment
  pWnd->SendMessage(PWS_MSG_DROPPED_FILE, (WPARAM)szBuf, nCntFiles);
}

/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#ifndef _OSDEBUG_H
#define _OSDEBUG_H

#include "typedefs.h"
#ifdef _WIN32
#include "wtypes.h"
#endif

namespace pws_os {
  // MFC TRACE equivalent in MFC, non-MFC Windows and non-Windows
  void Trace(LPCTSTR lpszFormat, ...);
  void Trace0(LPCTSTR lpszFormat);

  // Opens a messagebox or write to debugger window 
  // with text of last system error, titlebar
  // is csFunction
  DWORD IssueError(const stringT &csFunction, bool bMsgBox = true);

  /*
    Outputs a printable version of memory dump (hex + ascii)

    parameters:
      pmemory   - pointer to memory to format
      length    - length memory to format in bytes
      cs_prefix - prefix each line with this
      maxnum    - maximum hex characters dumped per line
  */
  void HexDump(unsigned char *pmemory, const int &length,
               const stringT &cs_prefix = _S(""), const int &maxnum = 16);

  /**
     This disables the ability to create a coredump and to attach a debugger to the process
     on Release builds (if OS supports this, of course).
  */
  bool DisableDumpAttach();
}

#endif /* _OSDEBUG_H */

/*
 * Copyright (c) 2003-2014 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

#include <wx/clipbrd.h>
#include <wx/dataobj.h>

/*

  NOTE: In VS2013 wxWidgets 3.0.x builds:
    Both <wx/clipbrd.h> & <wx/dataobj.h> cause 51 warnings about using unsecure
    versions of standard calls, such as 'wcscpy' instead of 'wcscpy_s', if any
    previously inluded header file includes <string> even though pre-processor
    variables _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES and
    _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT are defined.

    The solution is to ensure that any header files containing <string>, e.g.
    "core/StringX.h", are placed after these two wxWidgets include statements.

  This issue did not occur with wxWidgets 2.8.12.

  For this reason, "pwsclip.h", which includes "core/StringX.h" that also includes
  <string>, is placed here after <wx/clipbrd.h> & <wx/dataobj.h>.

*/

#include "pwsclip.h"

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

bool PWSclip::SetData(const StringX &text)
{
  if (wxTheClipboard->Open()) {
    wxTheClipboard->SetData(new wxTextDataObject(text.c_str()));
    wxTheClipboard->Close();
    return true;
  } else
    return false;
}

bool PWSclip::ClearData()
{
  // TBD
  return false;
}

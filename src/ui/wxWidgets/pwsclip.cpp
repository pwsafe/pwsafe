/*
 * Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */
#include "pwsclip.h"
#include <wx/clipbrd.h>
#include <wx/dataobj.h>

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

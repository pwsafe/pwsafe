/*
* Copyright (c) 2003-2015 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
* \file Linux-specific implementation of media.h
*/

// *** UNTESTED ***

#include "../media.h"
#include "../debug.h"

#include <stdlib.h>
#include <stdio.h>
#include <gio/gio.h>

// Linux uses the file contents to get the Media/MIME type
// NOT SURE WHAT gio DOES - DOES IT OPEN THE FILE?

stringT pws_os::GetMediaType(const stringT &sfilename)
{
  stringT sMediaType(_T(""));
  gboolean is_certain = FALSE;

  TCHAR *content_type = g_content_type_guess(sfilename.c_str(), NULL, 0, &is_certain);

  if (content_type != NULL) {
    sMediaType = g_content_type_get_mime_type(content_type);
  }

  g_free(content_type);

  return sMediaType;
}

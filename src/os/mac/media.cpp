/**
* \file Mac-specific implementation of media.h
*/

#include <wx/filename.h>
#include <wx/mimetype.h>

#include "../media.h"
#include "../file.h"
#include "../utf8conv.h"

#include <cstdlib>
#include <cstdio>

#include "wxUtilities.h"

using namespace std;

static const char *unknown_type = "unknown";

stringT pws_os::GetMediaType(const stringT & sfilename)
{
  wxString result(unknown_type);
  wxFileName wxfn(sfilename.c_str());
  
  if (wxfn.GetExt().empty())
    return tostdstring(unknown_type);
  
  wxMimeTypesManager mimeTypesManager;
  wxFileType* wxft = mimeTypesManager.GetFileTypeFromExtension(wxfn.GetExt());
  
  if(! wxft || ! wxft->GetMimeType(&result) ) {
      return tostdstring(unknown_type);
  }
  return tostdstring(_(result));
}

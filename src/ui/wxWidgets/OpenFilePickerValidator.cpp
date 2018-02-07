/*
 * Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "./OpenFilePickerValidator.h"
#include "./wxutils.h"
#include "../../os/file.h"

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

bool COpenFilePickerValidator::TransferFromWindow() {
  if (GetWindow() && GetWindow()->IsKindOf(&wxFilePickerCtrl::ms_classInfo)) {
    wxFilePickerCtrl* ctrl = dynamic_cast<wxFilePickerCtrl *>(GetWindow());
    wxASSERT(ctrl);
    m_str = ctrl->GetPath();
    return true;
  }
  return false;
}

bool COpenFilePickerValidator::TransferToWindow() {
  if (GetWindow() && GetWindow()->IsKindOf(&wxFilePickerCtrl::ms_classInfo)) {
    wxFilePickerCtrl* ctrl = dynamic_cast<wxFilePickerCtrl *>(GetWindow());
    wxASSERT(ctrl);
    ctrl->SetPath(m_str);
    return true;
  }
  return false;
}

bool COpenFilePickerValidator::Validate(wxWindow * parent) {
  if (GetWindow() && GetWindow()->IsKindOf(&wxFilePickerCtrl::ms_classInfo)) {
    wxFilePickerCtrl* ctrl = dynamic_cast<wxFilePickerCtrl *>(GetWindow());
    wxASSERT(ctrl);
    wxString path = ctrl->GetPath();
    if (path.IsEmpty()) {
      wxMessageBox(wxString() << _("You must select a valid file to continue.") << wxT("\n\n") << path,
                              _("You haven't selected any files"), wxOK | wxICON_EXCLAMATION, parent);
    }
    else if (pws_os::FileExists(tostdstring(path))) {
      return true;
    }
    else {
      //path is blank on Linux/gtk. May be its not so on other platforms
      wxMessageBox(wxString() << _("Selected file doesn't exist.") << wxT("\n\n") << path,
                              _("Please select a valid file"), wxOK | wxICON_EXCLAMATION, parent);
      return false;
    }
  }
  return false;
}

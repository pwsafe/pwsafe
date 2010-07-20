#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "./OpenFilePickerValidator.h"
#include "./wxutils.h"
#include "../../os/file.h"

bool COpenFilePickerValidator::TransferFromWindow() {
  if (GetWindow() && GetWindow()->IsKindOf(&wxFilePickerCtrl::ms_classInfo)) {
    wxFilePickerCtrl* ctrl = dynamic_cast<wxFilePickerCtrl*>(GetWindow());
    wxASSERT(ctrl);
    m_str = ctrl->GetPath();
    return true;
  }
  return false;
}

bool COpenFilePickerValidator::TransferToWindow() {
  if (GetWindow() && GetWindow()->IsKindOf(&wxFilePickerCtrl::ms_classInfo)) {
    wxFilePickerCtrl* ctrl = dynamic_cast<wxFilePickerCtrl*>(GetWindow());
    wxASSERT(ctrl);
    ctrl->SetPath(m_str);
    return true;
  }
  return false;
}

bool COpenFilePickerValidator::Validate(wxWindow */*parent*/) {
  if (GetWindow() && GetWindow()->IsKindOf(&wxFilePickerCtrl::ms_classInfo)) {
    wxFilePickerCtrl* ctrl = dynamic_cast<wxFilePickerCtrl*>(GetWindow());
    wxASSERT(ctrl);
    wxString path = ctrl->GetPath();
    if (pws_os::FileExists(tostdstring(path))) {
      return true;
    }
    else {
      //path is blank on Linux/gtk. May be its not so on other platforms
      wxMessageBox(wxString() << wxT("Selected file doesn't exist.\n\n") << path,
                              wxT("Please select a valid file"), wxOK | wxICON_EXCLAMATION);
      return false;
    }
  }
  return false;
}

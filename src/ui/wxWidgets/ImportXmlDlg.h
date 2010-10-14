#ifndef __IMPORTXMLDLG_H__
#define __IMPORTXMLDLG_H__

#include <wx/dialog.h> // Base class: wxDialog

class wxCheckBox;
class wxTextCtrl;

class CImportXMLDlg : public wxDialog {

  DECLARE_CLASS( CImportXMLDlg )
  
public:
  CImportXMLDlg(wxWindow* parent);

public:
  bool importUnderGroup;
  wxString groupName;
  
  bool importPasswordsOnly;
  
  wxString filepath;
  
private:
  wxCheckBox* CheckBox(const wxString& label, bool* validatorTarget);
  wxTextCtrl* TextCtrl(wxString* validatorTarget);

};

#endif // __IMPORTXMLDLG_H__

#ifndef __IMPORTTEXTDLG_H__
#define __IMPORTTEXTDLG_H__

#include <wx/dialog.h> // Base class: wxDialog
#include <wx/grid.h>
#include <wx/collpane.h>
#include "../../os/typedefs.h"

class wxBoxSizer;
class wxSizerFlags;
class wxRadioButton;

//Usage: Instantiate this class and if ShowModal() returns Ok, check
//the member variables which are automatically set by validators
class CImportTextDlg : public wxDialog {

  DECLARE_CLASS( CImportTextDlg )
  DECLARE_EVENT_TABLE()

public:
  CImportTextDlg(wxWindow* parent);
  virtual ~CImportTextDlg();

  void CreateControls();

  wxString filepath;
  
  bool delimiterComma;
  bool delimiterSpace;
  bool delimiterTab;
  bool delimiterSemicolon;
  bool delimiterOther;
  wxString strDelimiterOther;
  wxString strDelimiterLine;
  
  bool importUnderGroup;
  wxString groupName;
  
  bool importPasswordsOnly;
  
  TCHAR FieldSeparator() const;
  
private:
  wxCollapsiblePane* CreateParsingOptionsPane(wxBoxSizer* dlgSizer);
  wxCollapsiblePane* CreateImportOptionsPane(wxBoxSizer* dlgSizer);
  wxBoxSizer* CreateVerticalButtonSizer(long flags);

  //convenience functions
  wxCheckBox* CheckBox(wxWindow* parent, const wxString& label, bool* validatorTarget);
  wxTextCtrl* TextCtrl(wxWindow* parent, wxString* validatorTarget);
  wxRadioButton* RadioButton(wxWindow* parent, const wxString& label, bool* validatorTarget, int flags = 0);
};

#endif // __IMPORTTEXTDLG_H__

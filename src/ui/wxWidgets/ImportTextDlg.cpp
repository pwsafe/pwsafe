#include "ImportTextDlg.h"

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

/*
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/combobox.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/statbox.h>
#include <wx/checkbox.h>
#include <wx/grid.h>
#include <wx/wx.h>
*/
#include <wx/collpane.h>
#include <wx/spinctrl.h>
#include <wx/statline.h>
#include <wx/display.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <wx/gbsizer.h>
#include "../../corelib/StringX.h"
#include "./wxutils.h"
#include "../../corelib/PwsPlatform.h"
#include <wx/valgen.h>
#include <wx/filepicker.h>
#include "../../os/file.h"

IMPLEMENT_CLASS( CImportTextDlg, wxDialog )

BEGIN_EVENT_TABLE( CImportTextDlg, wxDialog )
END_EVENT_TABLE()

CImportTextDlg::CImportTextDlg(wxWindow* parent) :  wxDialog(parent, 
                                                            wxID_ANY, 
                                                            wxT("Import Text Settings"),
                                                            wxDefaultPosition,
                                                            wxDefaultSize,
                                                            wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER),
                                                    delimiterComma(false),
                                                    delimiterSpace(true),
                                                    delimiterTab(false),
                                                    delimiterSemicolon(false),
                                                    delimiterOther(false),
                                                    importUnderGroup(false),
                                                    importPasswordsOnly(false)
{
  //since the controls aren't direct children of the dialog but instead are
  //parented by wxCollipsiblePane, we need to validate recursively
  SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
  CreateControls();
}

CImportTextDlg::~CImportTextDlg()
{
}

enum { TopMargin = 20, BottomMargin = 20, SideMargin = 30, RowSeparation = 10, ColSeparation = 20};


wxCollapsiblePane* CImportTextDlg::CreateImportOptionsPane(wxBoxSizer* dlgSizer)
{
  const wxSizerFlags Left = wxSizerFlags().Proportion(0).Border(wxLEFT, SideMargin);
  const wxSizerFlags Right = wxSizerFlags().Proportion(0).Border(wxRIGHT, SideMargin);
  const wxSizerFlags noResizeFlags = wxSizerFlags().Proportion(0);//.Align(wxALIGN_CENTER_VERTICAL);
  const wxSizerFlags resizeFlags = wxSizerFlags().Proportion(1).Expand();
  
  wxCollapsiblePane* pane = new wxCollapsiblePane(this, wxID_ANY, wxT("Import options"));
  wxWindow* paneWindow = pane->GetPane();
  
  wxBoxSizer* vertSizer = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer* horzSizer = new wxBoxSizer(wxHORIZONTAL);
  
  horzSizer->Add(CheckBox(paneWindow, wxT("Import under group"), &importUnderGroup), Left);
  horzSizer->AddSpacer(ColSeparation);
  horzSizer->Add(TextCtrl(paneWindow, &groupName));
  
  vertSizer->Add(horzSizer);
  vertSizer->AddSpacer(RowSeparation);
  vertSizer->Add(CheckBox(paneWindow, wxT("Import to change passwords of existing entries ONLY"), 
                                  &importPasswordsOnly), Left);
  
  dlgSizer->Add(pane, wxSizerFlags().Proportion(0).Expand().Border(wxLEFT|wxRIGHT, SideMargin/2));
  paneWindow->SetSizer(vertSizer);
  vertSizer->SetSizeHints(paneWindow);
  
  return pane;
}

wxBoxSizer* CImportTextDlg::CreateVerticalButtonSizer(long flags)
{
  wxBoxSizer* box = new wxBoxSizer(wxVERTICAL);
  box->AddSpacer(TopMargin);
  
  long buttons[] = {wxID_OK, wxID_CANCEL, wxID_HELP};
  for (size_t idx = 0; idx < NumberOf(buttons); ++idx) {
    if ((flags & buttons[idx]) == buttons[idx]){
      box->Add(new wxButton(this, buttons[idx]));
      if (idx < (NumberOf(buttons)-1)) {
        box->AddSpacer(RowSeparation);
      }
    }
  }
  box->AddSpacer(BottomMargin);
  return box;
}

wxCheckBox* CImportTextDlg::CheckBox(wxWindow* parent, const wxString& label, bool* validatorTarget)
{
  return new wxCheckBox(parent, wxID_ANY, label, wxDefaultPosition, wxDefaultSize, 0,
                          wxGenericValidator(validatorTarget));
}

wxRadioButton* CImportTextDlg::RadioButton(wxWindow* parent, const wxString& label, bool* validatorTarget,
                                          int flags /*= 0*/)
{
  return new wxRadioButton(parent, wxID_ANY, label, wxDefaultPosition, wxDefaultSize, flags,
                            wxGenericValidator(validatorTarget));
}

wxTextCtrl* CImportTextDlg::TextCtrl(wxWindow* parent, wxString* validatorTarget)
{
  return new wxTextCtrl(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, 
                                  wxTextValidator(wxFILTER_NONE, validatorTarget));
}

wxCollapsiblePane* CImportTextDlg::CreateParsingOptionsPane(wxBoxSizer* dlgSizer)
{
  const wxSizerFlags Left = wxSizerFlags().Proportion(0).Border(wxLEFT, SideMargin);
  const wxSizerFlags Right = wxSizerFlags().Proportion(0).Border(wxRIGHT, SideMargin);

  wxCollapsiblePane* pane = new wxCollapsiblePane(this, wxID_ANY, wxT("Delimiters"));
  wxWindow* paneWindow = pane->GetPane();

  wxGridBagSizer* gridSizer = new wxGridBagSizer(RowSeparation, ColSeparation);
  gridSizer->Add(RadioButton(paneWindow, wxT("Comma"), &delimiterComma, wxRB_GROUP), wxGBPosition(0,0), wxDefaultSpan, wxLEFT, SideMargin);
  gridSizer->Add(RadioButton(paneWindow, wxT("Space"), &delimiterSpace), wxGBPosition(0,1));
  gridSizer->Add(RadioButton(paneWindow, wxT("Semicolon"), &delimiterSemicolon), wxGBPosition(0,2), wxDefaultSpan, wxRIGHT, SideMargin);
  
  gridSizer->Add(RadioButton(paneWindow, wxT("Tab"), &delimiterTab), wxGBPosition(1,0), wxDefaultSpan, wxLEFT, SideMargin);
  wxFlexGridSizer* otherSizer = new wxFlexGridSizer(2, 0, ColSeparation);
  otherSizer->Add(RadioButton(paneWindow, wxT("Other"), &delimiterOther));
  otherSizer->Add(TextCtrl(paneWindow, &strDelimiterOther));
  gridSizer->Add(otherSizer, wxGBPosition(1,1), wxGBSpan(1,2), wxRIGHT, SideMargin);

  wxBoxSizer* delimSizer = new wxBoxSizer(wxHORIZONTAL);
  delimSizer->Add(new wxStaticText(paneWindow, wxID_ANY, wxT("Line delimiter in Notes field:")), Left);
  delimSizer->AddSpacer(ColSeparation);
  delimSizer->Add(TextCtrl(paneWindow, &strDelimiterLine));
  gridSizer->Add(delimSizer, wxGBPosition(2,0), wxGBSpan(1,3));
  
  dlgSizer->Add(pane, wxSizerFlags().Proportion(0).Expand().Border(wxLEFT|wxRIGHT, SideMargin/2));
  paneWindow->SetSizer(gridSizer);
  gridSizer->SetSizeHints(paneWindow);
  
  return pane;
}

struct COpenFilePickerValidator: public wxValidator
{
  COpenFilePickerValidator(wxString& str) : m_str(str) {}
  virtual wxObject* Clone() const { return new COpenFilePickerValidator(m_str); }
  virtual bool TransferFromWindow() {
    if (GetWindow() && GetWindow()->IsKindOf(&wxFilePickerCtrl::ms_classInfo)) {
      wxFilePickerCtrl* ctrl = dynamic_cast<wxFilePickerCtrl*>(GetWindow());
      wxASSERT(ctrl);
      m_str = ctrl->GetPath();
      return true;
    }
    return false;
  }
  
  virtual bool TransferToWindow() {
    if (GetWindow() && GetWindow()->IsKindOf(&wxFilePickerCtrl::ms_classInfo)) {
      wxFilePickerCtrl* ctrl = dynamic_cast<wxFilePickerCtrl*>(GetWindow());
      wxASSERT(ctrl);
      ctrl->SetPath(m_str);
      return true;
    }
    return false;
  }

  virtual bool Validate (wxWindow */*parent*/) {
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
private:
  wxString& m_str;
  DECLARE_NO_COPY_CLASS(COpenFilePickerValidator)
};


void CImportTextDlg::CreateControls()
{
  const wxSizerFlags Left = wxSizerFlags().Proportion(0).Border(wxLEFT, SideMargin).Expand();

  wxBoxSizer* mainSizer = new wxBoxSizer(wxHORIZONTAL);
  
  wxBoxSizer* dlgSizer = new wxBoxSizer(wxVERTICAL);
  dlgSizer->AddSpacer(TopMargin);  //add a margin at the top

  wxString strPrompt(wxT("Select a text file to import:"));
  
  dlgSizer->Add(new wxStaticText(this, wxID_ANY, strPrompt), Left);
  dlgSizer->AddSpacer(RowSeparation);
  dlgSizer->Add(new wxFilePickerCtrl(this, wxID_ANY, wxEmptyString, 
                                          strPrompt, wxT("*.txt"), 
                                          wxDefaultPosition, wxDefaultSize, 
                                          wxFLP_DEFAULT_STYLE | wxFLP_USE_TEXTCTRL, 
                                          COpenFilePickerValidator(filepath)), Left);
  dlgSizer->AddSpacer(RowSeparation);
  
  wxCollapsiblePane* optionsPane = CreateImportOptionsPane(dlgSizer);
  dlgSizer->AddSpacer(RowSeparation);

  wxCollapsiblePane* delimitersPane = CreateParsingOptionsPane(dlgSizer);
  dlgSizer->AddSpacer(RowSeparation);
  
  dlgSizer->AddSpacer(BottomMargin);  //set a bottom margin
  
  optionsPane->Expand();
  delimitersPane->Expand();
  
  mainSizer->Add(dlgSizer, wxSizerFlags().Proportion(1).Expand());
  mainSizer->Add(CreateVerticalButtonSizer(wxID_OK|wxID_CANCEL|wxID_HELP), wxSizerFlags().Border(wxLEFT|wxRIGHT, SideMargin/2));
  
  SetSizerAndFit(mainSizer);
}

TCHAR CImportTextDlg::FieldSeparator() const
{
  if (delimiterComma)
    return wxT(',');
    
  if (delimiterSpace)
    return wxT(' ');
    
  if (delimiterTab)
    return wxT('\t');
    
  if (delimiterSemicolon)
    return wxT(';');
    
  if (delimiterOther)
    return strDelimiterOther[0];
    
  wxFAIL_MSG(wxT("No delimiter selected"));
  return  wxT(',');
}


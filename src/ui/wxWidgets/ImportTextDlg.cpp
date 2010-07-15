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
#include "../../corelib/StringX.h"
#include "./wxutils.h"

IMPLEMENT_CLASS( CImportTextDlg, wxDialog )

BEGIN_EVENT_TABLE( CImportTextDlg, wxDialog )
END_EVENT_TABLE()

CImportTextDlg::CImportTextDlg(wxWindow* parent) : wxDialog(parent, 
                                                            wxID_ANY, 
                                                            wxT("Import Text Settings"),
                                                            wxDefaultPosition,
                                                            wxDefaultSize,
                                                            wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER)
{
  CreateControls();
}

CImportTextDlg::~CImportTextDlg()
{
}

enum {  ID_COMBO_CHARSET = 100,
        ID_COMBO_LANGUAGE,
        ID_SPIN_ROWSTART,
        ID_CHECK_COMMA,
        ID_CHECK_SPACE,
        ID_CHECK_TAB,
        ID_CHECK_SEMICOLON,
        ID_CHECK_OTHER,
        ID_TEXT_OTHER_SEPARATOR,
        ID_CHECK_MERGE_DELIMITERS,
        ID_COMBO_DELIMITERS,
        ID_CHECK_QUOTEDASTEXT,
        ID_GRID_PREVIEW
};

enum { TopMargin = 20, BottomMargin = 20, SideMargin = 30, RowSeparation = 20, ColSeparation = 10};

wxCollapsiblePane* CImportTextDlg::CreateFileFormatsPane(wxBoxSizer* dlgSizer)
{
  const wxSizerFlags Left = wxSizerFlags().Proportion(0).Border(wxLEFT, SideMargin);
  const wxSizerFlags Right = wxSizerFlags().Proportion(0).Border(wxRIGHT, SideMargin);
  const wxSizerFlags noResizeFlags = wxSizerFlags().Proportion(0);//.Align(wxALIGN_CENTER_VERTICAL);
  const wxSizerFlags resizeFlags = wxSizerFlags().Proportion(1).Expand();
  
  wxCollapsiblePane* pane = new wxCollapsiblePane(this, wxID_ANY, wxT("File Format"));
  wxWindow* paneWindow = pane->GetPane();
  
  wxFlexGridSizer* flexSizer = new wxFlexGridSizer(3, ColSeparation, RowSeparation);
  flexSizer->AddGrowableCol(1, 1); //only the middle column expands
  flexSizer->SetFlexibleDirection(wxHORIZONTAL);
  flexSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_NONE);
  
  flexSizer->Add(new wxStaticText(paneWindow, wxID_ANY, wxT("Character set")), Left);
  flexSizer->Add(new wxComboBox(paneWindow, ID_COMBO_CHARSET), resizeFlags);
  flexSizer->Add(new wxButton(paneWindow, wxID_OK, wxT("&Ok")), Right);

  flexSizer->Add(new wxStaticText(paneWindow, wxID_ANY, wxT("Language")), Left);
  flexSizer->Add(new wxComboBox(paneWindow, ID_COMBO_LANGUAGE), resizeFlags);
  flexSizer->Add(new wxButton(paneWindow, wxID_CANCEL, wxT("&Cancel")), Right);
  
  flexSizer->Add(new wxStaticText(paneWindow, wxID_ANY, wxT("From ro&w")), Left);
  flexSizer->Add(new wxSpinCtrl(paneWindow, ID_SPIN_ROWSTART), noResizeFlags);
  flexSizer->Add(new wxButton(paneWindow, wxID_HELP, wxT("&Help")), Right);

  dlgSizer->Add(pane, wxSizerFlags().Proportion(0).Expand().Border(wxLEFT|wxRIGHT, SideMargin/2));
  paneWindow->SetSizer(flexSizer);//, wxSizerFlags().Proportion(0).Expand().Border(wxLEFT|wxRIGHT, SideMargin));
  flexSizer->SetSizeHints(paneWindow);
  
  return pane;
}

wxCollapsiblePane* CImportTextDlg::CreateParsingOptionsPane(wxBoxSizer* dlgSizer)
{
  const wxSizerFlags Left = wxSizerFlags().Proportion(0).Border(wxLEFT, SideMargin);
  const wxSizerFlags Right = wxSizerFlags().Proportion(0).Border(wxRIGHT, SideMargin);

  wxCollapsiblePane* pane = new wxCollapsiblePane(this, wxID_ANY, wxT("Delimiters"));
  wxWindow* paneWindow = pane->GetPane();

  wxGridSizer* gridSizer = new wxGridSizer(3, ColSeparation, RowSeparation);
  gridSizer->Add(new wxCheckBox(paneWindow, ID_CHECK_COMMA, wxT("Comma")), Left);
  gridSizer->Add(new wxCheckBox(paneWindow, ID_CHECK_SPACE, wxT("Space")));
  gridSizer->Add(new wxCheckBox(paneWindow, ID_CHECK_SEMICOLON, wxT("Semicolon")), Right);
  
  gridSizer->Add(new wxCheckBox(paneWindow, ID_CHECK_TAB, wxT("Tab")), Left);
  wxFlexGridSizer* otherSizer = new wxFlexGridSizer(2, 0, ColSeparation);
  otherSizer->Add(new wxCheckBox(paneWindow, ID_CHECK_OTHER, wxT("Other")));
  otherSizer->Add(new wxTextCtrl(paneWindow, ID_TEXT_OTHER_SEPARATOR));
  gridSizer->Add(otherSizer);
  gridSizer->Add(new wxCheckBox(paneWindow, ID_CHECK_MERGE_DELIMITERS, wxT("Merge Delimiters")), Right);

  gridSizer->Add(new wxCheckBox(paneWindow, ID_CHECK_QUOTEDASTEXT, wxT("Quoted field as text")));
  wxGridSizer* textDelimSizer = new wxGridSizer(2, 0, ColSeparation);
  textDelimSizer->Add(new wxStaticText(paneWindow, wxID_ANY, wxT("Text Delimiter")));
  textDelimSizer->Add(new wxComboBox(paneWindow, ID_COMBO_DELIMITERS, wxEmptyString, wxDefaultPosition, wxSize(75,-1)));
  gridSizer->Add(textDelimSizer);
  
  dlgSizer->Add(pane, wxSizerFlags().Proportion(0).Expand().Border(wxLEFT|wxRIGHT, SideMargin/2));
  paneWindow->SetSizer(gridSizer);
  gridSizer->SetSizeHints(paneWindow);
  
  return pane;
}

void CImportTextDlg::CreateControls()
{
  
  wxBoxSizer* dlgSizer = new wxBoxSizer(wxVERTICAL);
  dlgSizer->AddSpacer(TopMargin);  //add a margin at the top

  //some reusable flags
  const wxSizerFlags noResizeFlags = wxSizerFlags().Proportion(0);//.Align(wxALIGN_CENTER_VERTICAL);
  const wxSizerFlags resizeFlags = wxSizerFlags().Proportion(1).Expand();
  
  /*
  wxBoxSizer* topHeaderSizer = new wxBoxSizer(wxHORIZONTAL);
  topHeaderSizer->Add(new wxStaticText(this, wxID_ANY, wxT("File Format")));
  topHeaderSizer->AddSpacer(ColSeparation);
  topHeaderSizer->Add(new wxStaticLine(this), growFlags);
  dlgSizer->Add(topHeaderSizer, wxSizerFlags().Proportion(0).Expand().Border(wxLEFT|wxRIGHT, SideMargin/2));
  dlgSizer->AddSpacer(RowSeparation);
   */
  
  wxCollapsiblePane* formatsPane = CreateFileFormatsPane(dlgSizer);
  dlgSizer->AddSpacer(RowSeparation);

  /*
  wxBoxSizer* headerSizer = new wxBoxSizer(wxHORIZONTAL);
  headerSizer->Add(new wxStaticText(this, wxID_ANY, wxT("Delimiters")));
  headerSizer->AddSpacer(ColSeparation);
  headerSizer->Add(new wxStaticLine(this), resizeFlags);
  dlgSizer->Add(headerSizer, wxSizerFlags().Proportion(0).Expand().Border(wxLEFT|wxRIGHT, SideMargin/2));
  dlgSizer->AddSpacer(RowSeparation);
  */
  
  wxCollapsiblePane* delimitersPane = CreateParsingOptionsPane(dlgSizer);
  dlgSizer->AddSpacer(RowSeparation);
  
  wxBoxSizer* previewSizer = new wxBoxSizer(wxHORIZONTAL);
  previewSizer->Add(new wxStaticText(this, wxID_ANY, wxT("Preview")));
  previewSizer->AddSpacer(ColSeparation);
  previewSizer->Add(new wxStaticLine(this), resizeFlags);
  dlgSizer->Add(previewSizer, wxSizerFlags().Proportion(0).Expand().Border(wxLEFT|wxRIGHT, SideMargin/2));
  dlgSizer->AddSpacer(RowSeparation);
  
  wxGrid* grid = new ImportTextGrid(this, ID_GRID_PREVIEW);
  grid->SetTable(new ImportTextGridTable(wxT("/tmp/abc.csv")));
  dlgSizer->Add(grid, wxSizerFlags().Proportion(1).Expand().Border(wxLEFT|wxRIGHT, SideMargin));
  dlgSizer->AddSpacer(BottomMargin);  //set a bottom margin
  
  formatsPane->Expand();
  delimitersPane->Expand();
  
  SetSizerAndFit(dlgSizer);
}

wxSize CImportTextDlg::DoGetBestSize() const
{
  wxSize size = wxDialog::DoGetBestSize();

  wxDisplay disp;
  wxRect screen = disp.GetClientArea();
  screen.SetHeight(screen.GetHeight()*0.8);
//  wxMessageBox(wxString() << wxT("Best size is ") << size );

  size.SetHeight(std::min(size.GetHeight(), screen.GetHeight()));
  
  return size;
}

  
IMPLEMENT_CLASS(ImportTextGrid, wxGrid)
  
wxSize ImportTextGrid::DoGetBestSize() const
{
  wxSize size = wxGrid::DoGetBestSize();

  wxDisplay disp;
  wxRect screen = disp.GetClientArea();
  screen.SetHeight(screen.GetHeight()*0.8);
//  wxMessageBox(wxString() << wxT("Best size is ") << size );

  size.SetHeight(std::min(size.GetHeight(), screen.GetHeight()));
  
  return size;
}

IMPLEMENT_CLASS(ImportTextGridTable, wxGridTableBase)

#if not wxCHECK_VERSION(2,9,1)
void InitConverter(wxConvAuto& conv, wxFileInputStream& file)
{
  //read 64 bytes and pass it to the converter all
  //at once to help it detect the BOM correcly
  char bytes[64];
  file.Read(bytes, sizeof(bytes));
  const size_t nRead = file.LastRead();
  if (nRead) {
    conv.ToWChar(NULL, 0, bytes, nRead);
    file.Ungetch(bytes, nRead);
  }
}
#endif

ImportTextGridTable::ImportTextGridTable(const wxString& textfile)
{
  //
  //prior to 2.9.1, wxConvAuto + wxTextInputStream didn't play too well together
  //see http://trac.wxwidgets.org/changeset/63064
  //
  //So we create a wxConvAuto object and correctly initialize it, and explicitly
  //pass it to wxTextInputStream as parameter
  //
  wxFileInputStream file(textfile);
  if (file.IsOk()) {
    int nRead = 0;
#if not wxCHECK_VERSION(2,9,1)
    wxConvAuto conv;
    InitConverter(conv, file);
    wxTextInputStream input(file, wxT(" \t"), conv);
#else
    wxTextInputStream input(file);
#endif
    while(!file.Eof()) {
      wxString line = input.ReadLine();
      if (!line.IsEmpty()) {
        m_lines.Add(line);
        nRead++;
      }
    }
    wxMessageBox(wxString() << nRead << wxT(" Items read from ") << textfile);
  }
}

int ImportTextGridTable::GetNumberRows()
{
  //wxMessageBox(wxString() << m_lines.Count() << wxT(" items to import"));
  return m_lines.Count();
  //return 100;
};

int ImportTextGridTable::GetNumberCols()
{
  return 8;
}

bool ImportTextGridTable::IsEmptyCell(int row, int col)
{
  return false;
}

wxString ImportTextGridTable::GetValue(int row, int col)
{
  return wxString() << wxT("Row ") << row << wxT(", Col ") << col ;
}

wxString ImportTextGridTable::GetColLabelValue(int col)
{
  return wxString() << wxT("Column ") << col;
}

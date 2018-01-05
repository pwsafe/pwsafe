/*
 * Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */
//-----------------------------------------------------------------------------
/*
 * This code creates a wxGrid inside a wxCollapsiblePane, along with controls to set the number 
 * of rows and columns in the grid dynamically.  If a large number of rows and columns are added
 * to the grid while the pane is collapsed, the grid and the top-level dialog window become too 
 * big to fit on the screen when the collapsible pane is expanded.  It might not be possible to
 * access the rows at the bottom or columns to the right.
 * 
 * However, if the rows & columns are added to the grid with the collapsible pane already expanded,
 * the grid doesn't resize.  If required, scrollboxes appear automatically and the grid remains the 
 * same size.
 * 
 * To build - g++ -o gridpane `wx-config --cxxflags` gridpane.cpp `wx-config --libs`
 *
 */

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/grid.h>
#include <wx/collpane.h>
#include <wx/spinctrl.h>
#include <wx/display.h>

wxSize GetCurrentUsableDisplaySize(wxWindow* win) 
{
  const int disp_id = wxDisplay::GetFromPoint(win->GetScreenPosition());
  const wxSize disp_size = (disp_id == wxNOT_FOUND? ::wxGetClientDisplayRect().GetSize()
                                                    : wxDisplay(disp_id).GetClientArea().GetSize());
  return disp_size;
}

wxString& operator<<(wxString& str, const wxSize& size) {
  return str << wxT('[') << size.x << wxT(',') << size.y << wxT(']');
}

class TestApp: public wxApp
{
  public:
    bool OnInit();
};

IMPLEMENT_APP(TestApp)

class IGridSizer {
  public:
    virtual wxSize GetGridBestSize(const wxGrid *grid, const wxSize &size) = 0;
};

class TestGrid: public wxGrid {

    IGridSizer *m_gridSizer;

  public:
    TestGrid( IGridSizer *gridSizer): wxGrid(), m_gridSizer(gridSizer)
    {}

    TestGrid ( IGridSizer *gridSizer, wxWindow* parent, wxWindowID id,
          const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
          long style = wxWANTS_CHARS, const wxString& name = wxPanelNameStr ): wxGrid( parent, id, pos, size, style, name ),
                                             m_gridSizer(gridSizer)
    {}

    virtual wxSize DoGetBestSize() const {
      wxSize sz = wxGrid::DoGetBestSize();
      return m_gridSizer->GetGridBestSize( this, sz );
    }
};

class TestDialog: public wxDialog, IGridSizer
{
    wxSize m_collapsedSize; // client size when collapsed

    void LogSizes(const wxString& when );
    void AdjustPanelBestSize( const wxSize & panelBestSize, wxSize &finalPanelSize, wxSize &finalClientSize);

  public:
    TestDialog(bool use_testgrid);

    wxSize GetGridBestSize( const wxGrid *grid, const wxSize &size );

  protected:
    void OnPopulate(wxCommandEvent& evt);
    void OnLogSizes(wxCommandEvent& evt);
    void OnQuit(wxCommandEvent& evt);
    void OnCollpaneChanged(wxCollapsiblePaneEvent& evt);
    void DoResize(wxCommandEvent& evt);

  DECLARE_EVENT_TABLE()

};

enum { MAX_ROWS = 10000, MAX_COLS = 500, NUM_ROWS = 200, NUM_COLS = 50 };
enum { ID_GRID = 100, ID_SPIN_ROW, ID_SPIN_COL, ID_COLLPANE, ID_POPULATE, ID_LOGSIZES };

DEFINE_EVENT_TYPE( EVT_RESIZE_COLLPANE )

BEGIN_EVENT_TABLE(TestDialog, wxDialog)
  EVT_INIT_DIALOG(TestDialog::OnInitDialog)
  EVT_BUTTON(ID_POPULATE, TestDialog::OnPopulate)
  EVT_BUTTON(ID_LOGSIZES, TestDialog::OnLogSizes)
  EVT_COMMAND(wxID_ANY, EVT_RESIZE_COLLPANE, TestDialog::DoResize)
END_EVENT_TABLE()

bool TestApp::OnInit()
{
  TestDialog* dlg = new TestDialog( true );
  dlg->ShowModal();
  return false;
}

TestDialog::TestDialog( bool use_testgrid ) : wxDialog(static_cast<wxWindow*>(0), wxID_ANY, wxString(wxT("Pane-in-the-Grid Test Application"))), m_collapsedSize(wxDefaultSize)
{
  wxBoxSizer* dlgSizer = new wxBoxSizer(wxVERTICAL);

  // Create controls to set the number of rows and columns in the grid
  wxBoxSizer* rowcolSizer = new wxBoxSizer(wxHORIZONTAL);
  rowcolSizer->Add(new wxStaticText(this, wxID_ANY, wxT("Rows: ")), wxSizerFlags().Border());
  wxSpinCtrl* rowSpin = new wxSpinCtrl(this, ID_SPIN_ROW);
  rowSpin->SetRange(1, MAX_ROWS); rowSpin->SetValue(NUM_ROWS);
  rowcolSizer->Add(rowSpin, wxSizerFlags().Border().Proportion(1));
  rowcolSizer->AddSpacer(20);
  rowcolSizer->Add(new wxStaticText(this, wxID_ANY, wxT("Columns: ")), wxSizerFlags().Border());
  wxSpinCtrl* colSpin = new wxSpinCtrl(this, ID_SPIN_COL);
  colSpin->SetRange(1, MAX_COLS); colSpin->SetValue(NUM_COLS);
  rowcolSizer->Add(colSpin, wxSizerFlags().Border().Proportion(1));
  rowcolSizer->AddSpacer(20);
  wxButton* populateButton = new wxButton(this, ID_POPULATE, wxT("&Set Rows and Columns"));
  rowcolSizer->Add(populateButton, wxSizerFlags().Right().Border());
  rowcolSizer->AddSpacer(20);
  rowcolSizer->Add(new wxButton(this, ID_LOGSIZES, wxT("Log sizes")), wxSizerFlags().Right().Border());

  dlgSizer->Add(rowcolSizer, wxSizerFlags().Border());

  int style = wxCP_DEFAULT_STYLE | (use_testgrid? 0: wxCP_NO_TLW_RESIZE);

  //create the collapsible pane
  wxCollapsiblePane* pane = new wxCollapsiblePane(this, ID_COLLPANE, wxT("Expand/Collapse to see/hide the grid"), wxDefaultPosition,
                          wxDefaultSize, style);

  if (!use_testgrid)
    pane->Connect( wxEVT_COMMAND_COLLPANE_CHANGED, wxCollapsiblePaneEventHandler(TestDialog::OnCollpaneChanged), NULL, this );

  //create the grid inside the collapsible pane by passing pane->GetPane() as parent
  wxGrid *grid;
  if (use_testgrid)
    grid = new TestGrid( this, pane->GetPane(), ID_GRID );
  else
    grid = new wxGrid(pane->GetPane(), ID_GRID);

  grid->CreateGrid(NUM_ROWS, NUM_COLS);

  // Create the containing sizer to resize the grid on expand/collapse
  wxBoxSizer *gridSizer = new wxBoxSizer(wxVERTICAL);
  gridSizer->Add(grid, wxSizerFlags().Expand().Proportion(1));

  // That sizer should be used by the collapsible panel's pane
  pane->GetPane()->SetSizer(gridSizer);

  //add the collapsible pane to the dialog's sizer
  dlgSizer->Add(pane, wxSizerFlags().Border().Expand().Proportion(0));

  dlgSizer->Add(CreateSeparatedButtonSizer(wxCANCEL), wxSizerFlags().Border().Expand());

  SetSizerAndFit(dlgSizer);
}

void TestDialog::OnCollpaneChanged(wxCollapsiblePaneEvent& evt)
{
  if ( evt.GetCollapsed() ) {
    SetClientSize( m_collapsedSize );
    Layout();
  }
  else {
    // Let this notification blow over before doing anything complicated
    wxCommandEvent cmd(EVT_RESIZE_COLLPANE, GetId());
    GetEventHandler()->AddPendingEvent(cmd);
  }
}

wxSize TestDialog::GetGridBestSize( const wxGrid *grid, const wxSize &size )
{
  wxSize finalGridSize, finalClientSize;
  AdjustPanelBestSize( size, finalGridSize, finalClientSize );
  wxString log;
  log << wxT("\nAdjusted size of grid from ") << size << wxT(" to ") << finalGridSize ;
  wxLogDebug( log );
  return finalGridSize;
}

void TestDialog::DoResize(wxCommandEvent& evt)
{
  LogSizes( wxT("Before resizing") );

  wxCollapsiblePane *cpane = wxDynamicCast( FindWindow(ID_COLLPANE), wxCollapsiblePane );

  wxString log;

  // This is how big the grid wants to be
  const wxSize panelBestSize = cpane->GetPane()->GetBestSize();

  log << wxT("\nThe grid wants to become ") << panelBestSize;
  wxLogDebug (log );

  wxSize finalPanelSize, finalClientSize;

  AdjustPanelBestSize( panelBestSize, finalPanelSize, finalClientSize );

  LogSizes( wxT("Before setting panel size") );
  cpane->GetPane()->SetSize(finalPanelSize);
  LogSizes( wxT("After setting panel size") );
  //cpane->GetPane()->Layout();
  //LogSizes( wxT("After panel layout") );
  SetClientSize(finalClientSize);
  LogSizes( wxT("After setting dialog client size") );

  //Layout();
  //LogSizes( wxT("After dialog layout") );
}

void TestDialog::AdjustPanelBestSize( const wxSize & panelBestSize, wxSize &finalPanelSize, wxSize &finalClientSize)
{
  wxString log;
  const wxSize clientSize = GetClientSize();

  log << wxT("\nClient size is ") << clientSize;

  wxCollapsiblePane *cpane = wxDynamicCast( FindWindow(ID_COLLPANE), wxCollapsiblePane );

  // size of borders, margins and other controls
  const wxSize restSize = clientSize - cpane->GetSize();

  log << wxT("\nSize of dialog minus collpane is ") << restSize;

  // For that, this is how big the dialog has to be.  If expected client width is less that current width, we keep
  // the same width.  For height, we just add the height required by the panel to our current client height since
  // so far the panel's client height was 0
  const wxSize dlgBestClientSize( clientSize.x > panelBestSize.x? clientSize.x: panelBestSize.x + restSize.x,
                    panelBestSize.y + clientSize.y );

  log << wxT("\nFor that, the dialog has to be ") << dlgBestClientSize;

  // Available screen size (minus any app bars, status bars, etc
  const wxSize dispSize = GetCurrentUsableDisplaySize(this);

  log << wxT("\nBut the display size is only ") << dispSize;

  wxSize decoSize( GetSize() - clientSize );
  log << wxT("\nWindow decorations are ") << GetSize() << wxT(" minus ") << clientSize << wxT(", that is ") << decoSize;

  if ( decoSize.x == 0 )
    decoSize.x = 2*wxSystemSettings::GetMetric( wxSYS_BORDER_X, this);
  if (decoSize.y == 0 )
    decoSize.y = 2*wxSystemSettings::GetMetric( wxSYS_BORDER_Y, this) + wxSystemSettings::GetMetric(wxSYS_CAPTION_Y, this);

  log << wxT("\nFinal Window decorations are ") << decoSize;

  // This is how big the client size could be.  Basically, the entire screen minus all window decorations
  const wxSize maxClientSize = dispSize - decoSize;

  log << wxT("\nSo the max possible client size is ") << maxClientSize;

  // We only get as big as we need to be, so as to not cover up the entire screen unnecessarily
  finalClientSize.Set( wxMin(maxClientSize.x, dlgBestClientSize.x), wxMin(maxClientSize.y, dlgBestClientSize.y) );

  log << wxT("\nSo the final client size is ") << finalClientSize;

  finalPanelSize.Set( wxMin( (finalClientSize.x - restSize.x), panelBestSize.x), finalClientSize.y - restSize.y - cpane->GetSize().y );

  log << wxT("\nSo the final panel size is ") << finalPanelSize;
  wxLogDebug(log);
}

void TestDialog::LogSizes(const wxString& when)
{
  wxGrid* grid = wxDynamicCast(FindWindow(ID_GRID), wxGrid);
  wxCollapsiblePane *cpane = wxDynamicCast( FindWindow(ID_COLLPANE), wxCollapsiblePane );
  wxWindow *panel = cpane->GetPane();
  wxString log;
  log << when << wxT(":\n")     << wxT("grid")     << wxT("\t\t")   << grid->GetSize()   << wxT('\t') << grid->GetBestSize()   << wxT("\n")
                  << wxT("CollPane")   << wxT("\t")   << cpane->GetSize() << wxT('\t') << cpane->GetBestSize()   << wxT("\n")
                  << wxT("Panel")   << wxT("\t\t")   << panel->GetSize() << wxT('\t') << panel->GetBestSize()   << wxT("\n")
                  << wxT("Dialog")   << wxT("\t\t")   << GetSize()     << wxT('\t') << GetBestSize()       << wxT("\n");

  wxLogDebug(log);
}

void TestDialog::OnLogSizes(wxCommandEvent& evt)
{
  LogSizes( wxT("On Demand") );
}

void TestDialog::OnPopulate(wxCommandEvent& evt)
{
  wxGrid* grid = wxDynamicCast(FindWindow(ID_GRID), wxGrid);
  wxSpinCtrl* rowSpin = wxDynamicCast(FindWindow(ID_SPIN_ROW), wxSpinCtrl);
  wxSpinCtrl* colSpin = wxDynamicCast(FindWindow(ID_SPIN_COL), wxSpinCtrl);

  grid->DeleteRows(0, grid->GetNumberRows());
  grid->DeleteCols(0, grid->GetNumberCols());

  const int num_rows = rowSpin->GetValue();
  const int num_cols = colSpin->GetValue();

  grid->AppendRows(num_rows);
  grid->AppendCols(num_cols);

  for(size_t row = 0; row < num_rows; ++row) {
    for(size_t col=0; col < num_cols; ++col) {
      wxString data;
      data << wxT("Row ") << row << wxT(", col ") << col;
      grid->SetCellValue(row, col, data);
    }
  }

  wxCollapsiblePane *cpane = wxDynamicCast( FindWindow(ID_COLLPANE), wxCollapsiblePane );
  wxWindow *panel = cpane->GetPane();

  if ( cpane->IsCollapsed() && !m_collapsedSize.IsFullySpecified() )
      m_collapsedSize = GetClientSize(); // remember the collapsed size, but only once

  LogSizes( wxT("Before Invalidation") );

  cpane->InvalidateBestSize();
  panel->InvalidateBestSize();
  grid->InvalidateBestSize();
  InvalidateBestSize();

  LogSizes( wxT("After-Invalidation") );

  panel->GetSizer()->Layout();
  GetSizer()->Layout();

  LogSizes( wxT("After-layout") );

  wxString log;
  log << wxT("Client size: ") << GetClientSize() << wxT(", window size: ") << GetSize() << wxT("\n");
  wxLogDebug( log );
}

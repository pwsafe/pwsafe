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

class TestApp: public wxApp
{
	public:
		bool OnInit();
};

IMPLEMENT_APP(TestApp)

class TestDialog: public wxDialog
{
	public:
		TestDialog();

	protected:
		void OnPopulate(wxCommandEvent& evt);
		void OnQuit(wxCommandEvent& evt);

};


enum { MAX_ROWS = 10000, MAX_COLS = 500, NUM_ROWS = 200, NUM_COLS = 50 };
enum { ID_GRID = 100, ID_SPIN_ROW, ID_SPIN_COL };

bool TestApp::OnInit()
{
	TestDialog* dlg = new TestDialog;
	dlg->ShowModal();
	return false;
}

TestDialog::TestDialog() : wxDialog(static_cast<wxWindow*>(0), wxID_ANY, wxT("Pane-in-the-Grid Test Application"))
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
	wxButton* populateButton = new wxButton(this, wxID_ANY, wxT("&Set Rows and Columns"));
	rowcolSizer->Add(populateButton, wxSizerFlags().Right().Border());

	dlgSizer->Add(rowcolSizer, wxSizerFlags().Border());

	//create the collapsible pane
	wxCollapsiblePane* pane = new wxCollapsiblePane(this, wxID_ANY, wxT("Expand/Collapse to see/hide the grid"));

	//create the grid inside the collapsible pane
	wxGrid* grid = new wxGrid(pane->GetPane(), ID_GRID);
	grid->CreateGrid(NUM_ROWS, NUM_COLS);

	// Create the containing sizer to resize the grid on expand/collapse
	wxBoxSizer *gridSizer = new wxBoxSizer(wxVERTICAL);
	gridSizer->Add(grid, wxSizerFlags().Expand().Proportion(1));

	// That sizer should be used by the collapsible panel's pane
	pane->GetPane()->SetSizer(gridSizer);

	//add the collapsible pane to the dialog's sizer
	dlgSizer->Add(pane, wxSizerFlags().Border().Expand().Proportion(1));

	dlgSizer->Add(CreateSeparatedButtonSizer(wxCANCEL), wxSizerFlags().Border().Expand());

	SetSizerAndFit(dlgSizer);

	Connect(populateButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(TestDialog::OnPopulate));
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
}


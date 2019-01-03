/*
 * Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "./pwsqrcodedlg.h"
#include "./wxutils.h"

#include <algorithm>

#include <wx/statline.h>

#include "./pwsqrencode.h"

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

////////////////////////////////////////////////////////////////////////////
// PasswordSafeSerach implementation
IMPLEMENT_CLASS( PWSQRCodeDlg, wxDialog )

BEGIN_EVENT_TABLE(PWSQRCodeDlg, wxDialog)
	EVT_BUTTON(wxID_CLOSE, PWSQRCodeDlg::OnClose)
	EVT_INIT_DIALOG(PWSQRCodeDlg::OnInitDialog)
	EVT_TIMER(wxID_ANY, PWSQRCodeDlg::OnTimer)
END_EVENT_TABLE()


PWSQRCodeDlg::PWSQRCodeDlg(wxWindow* parent,
		                   const StringX &data,
						   const wxString& dlgTitle,
					       const int seconds,
					       const wxPoint &pos,
					       const wxSize &size,
					       long style,
					       const wxString &name): wxDialog(parent, wxID_ANY, dlgTitle, pos, size, style, name), timer(this), secondsRemaining(seconds)
{
	CreateControls(data);
}
 
void PWSQRCodeDlg::CreateControls(const StringX &data)
{
	wxSizer *dlgSizer = new wxBoxSizer(wxVERTICAL);

	dlgSizer->AddSpacer(TopMargin);

	wxBoxSizer *promptSizer = new wxBoxSizer(wxHORIZONTAL);
	promptSizer->Add( new wxStaticText(this, wxID_ANY, _T("Closing in ")) );
	promptSizer->Add( secondsText = new wxStaticText(this, wxID_ANY, _T("")) );
	dlgSizer->Add(promptSizer);

	dlgSizer->AddSpacer(RowSeparation);
	dlgSizer->Add( new wxStaticLine(this), wxSizerFlags().Expand() );
	dlgSizer->AddSpacer(RowSeparation);

	wxBitmap bmp = QRCodeBitmap(data);
	if (bmp.IsOk() ) {
		dlgSizer->Add( new wxStaticBitmap(this, wxID_ANY, bmp), wxSizerFlags().Expand().Proportion(1) );
	}
	else {
		dlgSizer->Add( new wxStaticText(this, wxID_ANY, _T("Could not generate QR code")) );
	}

	dlgSizer->AddSpacer(RowSeparation);
	dlgSizer->Add( CreateSeparatedButtonSizer(wxCLOSE), wxSizerFlags().Expand() );
	dlgSizer->AddSpacer(BottomMargin);
	wxSizer *hSizer = new wxBoxSizer(wxHORIZONTAL);
	hSizer->Add(dlgSizer, wxSizerFlags().Border(wxLEFT|wxRIGHT, SideMargin).Expand().Proportion(1));
	SetSizerAndFit(hSizer);
}

void PWSQRCodeDlg::OnClose(wxCommandEvent &/*evt*/)
{
	EndModal(wxID_CLOSE);
}

void PWSQRCodeDlg::OnTimer(wxTimerEvent &/*evt*/)
{
	if ( --secondsRemaining > 0 ) {
		UpdateTimeRemaining();
		timer.Start(1000);
	} else {
		EndModal(0);
	}
}

void PWSQRCodeDlg::OnInitDialog(wxInitDialogEvent &/*evt*/)
{
	// true => oneshot. We don't want to be called every millisecond
	timer.Start( 1000, true );
	UpdateTimeRemaining();
}

void PWSQRCodeDlg::UpdateTimeRemaining()
{
	secondsText->SetLabel( wxString::Format(_T("%d seconds"), secondsRemaining) );
}


#ifdef __TEST_QR_CODE__
///////////////////////////////////////////////////////
// build it like this
// g++ -D__TEST_QR_CODE__ -o qrtest
//                  `/usr/bin/wx-config-3.0 --debug=yes --unicode=yes --inplace --cxxflags --libs`
//                  -DUNICODE -I../.. pwsqrcodedlg.cpp  -lcore -los -lcore -lqrencode -L../../../lib/unicodedebug
//
#include <wx/cmdline.h>
#include <wx/filename.h>
class QRTestApp: public wxApp {
	public:

		void OnInitCmdLine( wxCmdLineParser &parser) {
			wxAppConsole::OnInitCmdLine(parser);
			static const wxCmdLineEntryDesc cmdLineDesc[] =
			{
				{ wxCMD_LINE_PARAM, nullptr, nullptr, "secret", wxCMD_LINE_VAL_STRING, 0},
				{ wxCMD_LINE_NONE }
			};
			parser.SetDesc(cmdLineDesc);
		}

		bool OnInit() override {
			if ( !wxApp::OnInit() )
				return false;

			if ( this->argc != 2 ) {
				std::cerr << "usage: " << wxFileName(argv[0]).GetFullName() << " <text to generate qr code for>\n";
				return false;
			} else {
				PWSQRCodeDlg dlg(nullptr, StringX(argv[1]), "Scan this QR Code and comapre with the program argument" );
				dlg.ShowModal();
				// Normall we return true here
				return false;
			}
		}
};

wxIMPLEMENT_APP(QRTestApp);
#endif


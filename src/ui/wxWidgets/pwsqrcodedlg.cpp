/*
 * Copyright (c) 2003-2017 Rony Shapiro <ronys@pwsafe.org>.
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

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

////////////////////////////////////////////////////////////////////////////
// PasswordSafeSerach implementation
IMPLEMENT_CLASS( PWSQRCodeDlg, wxDialog )

BEGIN_EVENT_TABLE(PWSQRCodeDlg, wxDialog)
	EVT_BUTTON(wxID_CLOSE, PWSQRCodeDlg::OnClose)
END_EVENT_TABLE()

PWSQRCodeDlg::PWSQRCodeDlg(wxWindow* parent,
		                   const StringX &data,
						   const wxString& description,
					       const int seconds,
					       const wxString &dlgTitle,
					       const wxPoint &pos,
					       const wxSize &size,
					       long style,
					       const wxString &name): wxDialog(parent, wxID_ANY, dlgTitle, pos, size, style, name)
{
	CreateControls(data, description);
}
 
void PWSQRCodeDlg::CreateControls(const StringX &data, const wxString &description)
{
	wxSizer *dlgSizer = new wxBoxSizer(wxVERTICAL);

	dlgSizer->AddSpacer(TopMargin);

	dlgSizer->Add( new wxStaticText(this, wxID_ANY, description) );

	dlgSizer->AddSpacer(RowSeparation);

	dlgSizer->Add( new wxButton(this, wxID_CLOSE) );

	SetSizerAndFit(dlgSizer);
}

void PWSQRCodeDlg::OnClose(wxCommandEvent &evt)
{
	EndModal(wxID_CLOSE);
}

#ifdef __TEST_QR_CODE__
///////////////////////////////////////////////////////
// build it like this
// g++ -D__TEST_QR_CODE__ -o qrtest
//                  `/usr/bin/wx-config-3.0 --debug=yes --unicode=yes --inplace --cxxflags --libs`
//                  -DUNICODE -I../.. pwsqrcodedlg.cpp  -lcore -los -lcore -L../../../lib/unicodedebug
//
#include <wx/cmdline.h>
class QRTestApp: public wxApp {
	public:

		void OnInitCmdLine( wxCmdLineParser &parser) {
			wxAppConsole::OnInitCmdLine(parser);
			static const wxCmdLineEntryDesc cmdLineDesc[] =
			{
				{ wxCMD_LINE_PARAM, NULL, NULL, "secret", wxCMD_LINE_VAL_STRING, 0},
				{ wxCMD_LINE_NONE }
			};
			parser.SetDesc(cmdLineDesc);
		}

		bool OnInit() override {
			if ( !wxApp::OnInit() )
				return false;

			if ( this->argc != 2 ) {
				std::cerr << "usage: " << basename(argv[0]) << " <text to generate qr code for>\n";
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


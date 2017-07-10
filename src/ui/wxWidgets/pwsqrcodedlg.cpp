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

#include <wx/statline.h>

#include <qrencode.h>

#include "../../core/UTF8Conv.h"
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

/*
 * caller must check for null bitmap for errors
 */
wxBitmap QRCodeBitmap( const StringX &data )
{

	using QRcodePtr = std::unique_ptr<QRcode, decltype(&QRcode_free)>;


	CUTF8Conv conv;
	const unsigned char *utf8str = nullptr;
	size_t utf8len = 0;
	if ( !conv.ToUTF8(data, utf8str, utf8len) )
		return wxBitmap();

	auto qc_data = reinterpret_cast<const char *>(utf8str);
	QRcodePtr qc(QRcode_encodeString8bit(qc_data, 0, QR_ECLEVEL_H), &QRcode_free);

	const size_t nbytes = (qc->width + 7)/8;
	std::vector<char> xbmp(nbytes);

	const auto max_idx = std::min(nbytes, static_cast<size_t>(qc->width/8));
	for (auto n = 0; n < max_idx; ++n) {
		auto start = n*8;
		xbmp[n] = 	((qc->data[start+0] & 1) << 0) |
					((qc->data[start+1] & 1) << 1) |
					((qc->data[start+2] & 1) << 2) |
					((qc->data[start+3] & 1) << 3) |
					((qc->data[start+4] & 1) << 4) |
					((qc->data[start+5] & 1) << 5) |
					((qc->data[start+6] & 1) << 6) |
					((qc->data[start+7] & 1) << 7);
	}

	if (nbytes > max_idx) {
		for (auto i = 0; i < qc->width%8; ++i) {
			xbmp[nbytes-1] |= (qc->data[max_idx*8 + i] & 1);
		}
	}

	const int height = qc->width;
	return wxBitmap( xbmp.data(), qc->width, height);
}

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

	dlgSizer->Add( new wxStaticLine(this) );

	dlgSizer->AddSpacer(RowSeparation);

	wxBitmap bmp = QRCodeBitmap(data);
	if (bmp.IsOk() ) {
		dlgSizer->Add( new wxStaticBitmap(this, wxID_ANY, bmp), wxSizerFlags().Expand() );
	}
	else
		dlgSizer->Add( new wxStaticText(this, wxID_ANY, _T("Could not generate QR code")) );

	dlgSizer->AddSpacer(RowSeparation);
	dlgSizer->Add( new wxStaticLine(this) );
	dlgSizer->Add( new wxButton(this, wxID_CLOSE), wxSizerFlags().Right().Bottom() );

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


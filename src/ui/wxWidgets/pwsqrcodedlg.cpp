/*
 * Copyright (c) 2003-2017 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <algorithm>

#include <wx/statline.h>

#include <qrencode.h>

#include "core/UTF8Conv.h"
#include "pwsqrcodedlg.h"
#include "wxutils.h"

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


/*
 * converts the data to UTF8, generates the QR code
 * and creates xbmp data out of it, which is used to
 * generate a wxBitmap
 *
 * See https://en.wikipedia.org/wiki/X_BitMap
 *
 * Caller must check for null bitmap for errors.
 * No return codes. Sorry
 */
wxBitmap QRCodeBitmap( const StringX &data )
{
	using QRcodePtr = std::unique_ptr<QRcode, decltype(&QRcode_free)>;
	using std::vector;

	CUTF8Conv conv;
	const unsigned char *utf8str = nullptr;
	size_t utf8len = 0;
	if ( !conv.ToUTF8(data, utf8str, utf8len) )
		return wxBitmap();

	auto qc_data = reinterpret_cast<const char *>(utf8str);
	QRcodePtr qc(QRcode_encodeString8bit(qc_data, 0, QR_ECLEVEL_L), &QRcode_free);
	if ( !qc || !qc->width )
		return wxBitmap();

	constexpr unsigned margin = 24;
	constexpr unsigned scale = 8;

	// This is only for the ease of computation, to avoid filling and
	// tracking bytes partially while generating the xbm data
	static_assert( scale % 8 == 0, "QRcode must scale up by a factor of 8");
	static_assert( margin % scale == 0, "margin must be a multiple of scale");

	// in pixels
	const unsigned imageWidth = 2*margin + scale*qc->width;

	const vector<char> pxTopMargin( (imageWidth*margin)/8, 0 );
	const vector<char> pxSideMargin( margin/8, 0);

	// This converts a line of QR data to a line of scaled-up pixels
	// The last bit of QR byte, if 0 => white, else black
	auto qr2xbmp = [&qc, scale](int line) {
		vector<char> pixels(qc->width*scale/8);
		auto fill_index = pixels.begin();
		auto from = qc->data + line*qc->width;
		auto to = from + qc->width;
		std::for_each(from, to, [&](char byte) {
				const auto bytesToFill = scale/8;
				// All bits in these bytes are for the same pixel, so
				// its either all 1s, or all 0s
				std::fill_n(fill_index, bytesToFill, byte & 1? 0xff: 0);
				std::advance(fill_index, bytesToFill);
		});
		return pixels;
	};

	vector<char> xbmp(pxTopMargin);
	xbmp.reserve( imageWidth*imageWidth/8 );

	// For each line of QR data, generate a line of pixels
	for (auto line = 0; line < qc->width; ++line) {
		vector<char> pxline(pxSideMargin);
		pxline.reserve( imageWidth/8 );
		const auto qrdata = qr2xbmp(line);
		wxASSERT( qrdata.size() == (imageWidth - 2*margin)/8 );
		pxline.insert(pxline.end(), qrdata.begin(), qrdata.end());
		pxline.insert(pxline.end(), pxSideMargin.begin(), pxSideMargin.end());
		wxASSERT( pxline.size() == imageWidth/8 );
		// And repeat that line "scale" times
		for (unsigned n = 0; n < scale; ++n)
			xbmp.insert(xbmp.end(), pxline.begin(), pxline.end());
	}
	xbmp.insert(xbmp.end(), pxTopMargin.begin(), pxTopMargin.end());

	return wxBitmap(xbmp.data(), imageWidth, imageWidth);
}

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


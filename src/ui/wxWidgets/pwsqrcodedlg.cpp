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

#include <algorithm>

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
	QRcodePtr qc(QRcode_encodeString8bit(qc_data, 2, QR_ECLEVEL_H), &QRcode_free);

	//constexpr auto intendedImageWidth = 300;
	constexpr auto margin = 24;
	constexpr int scale = 8;//(intendedImageWidth - 2*margin)/qc->width;
	//scale = scale <= 0? 1: scale;
	const int imageWidth = 2*margin + scale*qc->width;


	const size_t nbytes = (imageWidth*imageWidth + 7)/8;
	std::cout << "QR code width: " << qc->width << ", version " << qc->version << '\n'
			<<	"image bytes: " << nbytes << '\n'
			<<	"image width: " << imageWidth << '\n';

	//wxASSERT_MSG(imageWidth*imageWidth == nbytes, _T("QRcode image width calculation is incorrect"));

	std::vector<char> xbmp(nbytes);

	auto write_bits = [&xbmp](size_t start, size_t count, int bitval) {
		constexpr char rbits[] = {0, 0b1, 0b11, 0b111, 0b1111, 0b11111, 0b111111, 0b1111111};

		if (start % 8) {
			// num bits set in the last byte
			const auto nset = start%8;
			const auto last_byte = start/8;
			// clear out the leftmost bits to be set now
			const char b = (rbits[nset] & xbmp[last_byte]);
			if (bitval) {
				const auto mask = (rbits[8 - nset] << nset);
				xbmp[last_byte] |= mask ;
			}
		}

		const char byte = bitval? 0xff: 0;
		for (size_t n = 0; n < count/8; n++)
			xbmp[start/8 + n] = byte;

		if ((start + count) % 8) {
			xbmp[start/8 + count/8 + 1] = rbits[count%8];
		}
	};

	using std::fill_n;
	using std::copy_n;

	// fill top margin
	write_bits(0, imageWidth*margin, 0);
	std::cout << "Top margin covers " << imageWidth*margin << '\n';

	const int height = qc->width;
	for (auto row = 0; row < height; ++row) {

		const auto bmpRowStart = (8*row + margin)*imageWidth;
		std::cout << "Image row " << row << ": " << bmpRowStart << '\n';

		wxASSERT_MSG(bmpRowStart < nbytes*8 - imageWidth, _T("QRcode image width calculation is incorrect"));
		
		// left margin
		write_bits(bmpRowStart, margin, 0);

		auto bmpQRDataIndex = bmpRowStart + margin;
		for (auto col = 0; col < qc->width; ++col) {
			wxASSERT(bmpQRDataIndex < bmpRowStart + imageWidth);
			const auto qcrow = row;
			const auto qcidx = qcrow*qc->width + col;
			const auto bit = qc->data[qcidx] & 1;

			// This relies on the fact that scale is 8
			//const auto byte = bit? 0xff: 0;
			//xbmp[bmpQRDataIndex + col] = byte;
			//bmpQRDataIndex++;
			write_bits(bmpQRDataIndex + col, scale, bit);
			bmpQRDataIndex += scale;
		}

		// right margin
		//fill_n(xbmp.begin() + bmpQRDataIndex, (imageWidth - (bmpQRDataIndex - bmpRowStart))/8, 0);
		write_bits(bmpQRDataIndex, imageWidth - (bmpQRDataIndex - bmpRowStart), 0);

		// copy the row "scale -1" times over
		for (auto s = 1; s < scale; ++s) {
			auto src = bmpRowStart/8;
			auto dst = bmpRowStart/8 + s*imageWidth/8;
			const auto nbytes = imageWidth/8;
			std::cout << "copying " << nbytes << " bytes in image row " << s << " : " << src << " => " << dst <<  '\n';
			copy_n(xbmp.begin() + src, nbytes, xbmp.begin() + dst);
		}

	}

	// fill bottom margin
	write_bits(nbytes - imageWidth*margin, imageWidth*margin, 0);

	return wxBitmap( xbmp.data(), imageWidth, imageWidth);
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
	dlgSizer->AddSpacer(RowSeparation);
	dlgSizer->Add( new wxStaticLine(this), wxSizerFlags().Expand() );
	dlgSizer->AddSpacer(RowSeparation);

	wxBitmap bmp = QRCodeBitmap(data);
	if (bmp.IsOk() ) {
		dlgSizer->Add( new wxStaticBitmap(this, wxID_ANY, bmp), wxSizerFlags().Expand().Proportion(1) );
	}
	else
		dlgSizer->Add( new wxStaticText(this, wxID_ANY, _T("Could not generate QR code")) );

	dlgSizer->AddSpacer(RowSeparation);
	dlgSizer->Add( new wxStaticLine(this), wxSizerFlags().Expand() );
	dlgSizer->AddSpacer(RowSeparation);
	wxStdDialogButtonSizer *btnSizer = new wxStdDialogButtonSizer;
	btnSizer->AddButton( new wxButton(this, wxID_CLOSE) );
	btnSizer->Realize();
	dlgSizer->Add( btnSizer, wxSizerFlags().Expand() );
	dlgSizer->AddSpacer(BottomMargin);

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


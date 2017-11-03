//
//  pwsqrencode-linux.cpp
//
//
//  Created by Saurav Ghosh on 26/10/17.
//
//

#include "pwsqrencode.h"

#include <qrencode.h>

#include "../../core/UTF8Conv.h"

#include <wx/bitmap.h>
#include <vector>
#include <algorithm>

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

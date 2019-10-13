/*
*  pwsqrencode.mm
*  pwsafe-xcode6
*
* Created by Saurav Ghosh on 26/10/17.
* Copyright (c) 2017-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#import <QuartzCore/CoreImage.h>

#include "../../core/UTF8Conv.h"

#include <wx/bitmap.h>

wxBitmap QRCodeBitmap( const StringX &data )
{
	CIFilter *qrFilter = [CIFilter filterWithName:@"CIQRCodeGenerator"];
	if ( qrFilter ) {
		
    [qrFilter setDefaults];

		CUTF8Conv conv;
		const unsigned char *utf8bytes = nullptr;
		size_t nBytes = 0;
		
		if ( conv.ToUTF8(data, utf8bytes, nBytes) ) {
      NSData *qrData = nil;
			NSString * utf8str = [[NSString alloc] initWithBytes:utf8bytes length:nBytes encoding:NSUTF8StringEncoding ];
      if (utf8str) {
        // This will probably fail for most multibyte charsets
				qrData = [utf8str dataUsingEncoding:NSISOLatin1StringEncoding];
        [utf8str release];
      }

      if (qrData == nil) return wxBitmap();

      [qrFilter setValue:qrData forKey:@"inputMessage"];
      [qrFilter setValue:@"H" forKey:@"inputCorrectionLevel"];
      CIImage *qrImage = [qrFilter outputImage];

      // This code scales the image without interpolation so its not blurry
      CGColorSpaceRef csref = CGColorSpaceCreateDeviceGray();
      if ( csref ) {

        CGContextRef bmpCtxt = CGBitmapContextCreate(nil, CGRectGetWidth(qrImage.extent)*8,
                                                          CGRectGetHeight(qrImage.extent)*8,
                                                          8,  // bpp. This is the min, even though we logically need 1 bpp for b/w image
                                                          0,  // bytes per row. 0 => calculate automatically
                                                          csref,
                                                          (CGBitmapInfo)kCGImageAlphaNone);
        if (bmpCtxt) {

          CGContextSetInterpolationQuality(bmpCtxt, kCGInterpolationNone);
          CGContextScaleCTM(bmpCtxt, 8.0f, 8.0f);

          CIContext *ctx = [CIContext context];
          CGImageRef bitmap = [ctx createCGImage:qrImage fromRect:qrImage.extent];

          CGContextDrawImage(bmpCtxt, qrImage.extent, bitmap);
          CGImageRef scaledBitmap = CGBitmapContextCreateImage(bmpCtxt);

          CGContextRelease(bmpCtxt);

          return wxBitmap(scaledBitmap);
        }

        CGColorSpaceRelease(csref);
      }
		}
	}
	return wxBitmap();
}

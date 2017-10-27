//
//  pwsqrencode-mac.cpp
//  pwsafe-xcode6
//
//  Created by Saurav Ghosh on 26/10/17.
//  Copyright (c) 2017 Open Source Software. All rights reserved.
//

#import <QuartzCore/CoreImage.h>

//#include "./pwsqrencode.h"

#include "../../core/UTF8Conv.h"

#include <wx/bitmap.h>

wxBitmap QRCodeBitmap( const StringX &data )
{
	CIFilter *qrFilter = [CIFilter filterWithName:@"CIQRCodeGenerator"];
	if ( qrFilter ) {
		
		CUTF8Conv conv;
		const unsigned char *utf8bytes = nullptr;
		size_t nBytes = 0;
		
		if ( conv.ToUTF8(data, utf8bytes, nBytes) ) {
			NSString * utf8str = [[NSString alloc] initWithBytes:utf8bytes length:nBytes encoding:NSUTF8StringEncoding ];
			if (utf8str) {
				NSData *data = [utf8str dataUsingEncoding:NSISOLatin1StringEncoding];

				if ( data ) {
          [qrFilter setValue:data forKey:@"inputMessage"];
					CIImage *qrImage = qrFilter.outputImage;
					CGAffineTransform xform = CGAffineTransformMakeScale(8, 8);
					CIImage *scaledImage = [qrImage imageByApplyingTransform: xform];
          CGImage *renderedImage = [[CIContext context] createCGImage:scaledImage fromRect:scaledImage.extent];
					return wxBitmap(renderedImage);
				}
			}
		}
	}
	return wxBitmap();
}

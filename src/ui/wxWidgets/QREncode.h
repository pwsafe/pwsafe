//
//  pwsqrencode-linux.h
//
//
//  Created by Saurav Ghosh on 26/10/17.
//
//

#ifndef ____pwsqrencode_linux__
#define ____pwsqrencode_linux__

class wxBitmap;

#ifndef _STRINGX_H
#include "../../core/StringX.h"
#endif

wxBitmap QRCodeBitmap( const StringX &data );

#endif /* defined(____pwsqrencode_linux__) */

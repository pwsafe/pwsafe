/*
 * Initial version created as 'pwsqrencode-linux.h'
 * by Saurav Ghosh on 26/10/17.
 * 
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file QREncode.h
*
*/

#ifndef ____pwsqrencode_linux__
#define ____pwsqrencode_linux__

class wxBitmap;

#ifndef _STRINGX_H
#include "../../core/StringX.h"
#endif

wxBitmap QRCodeBitmap( const StringX &data );

#endif /* defined(____pwsqrencode_linux__) */

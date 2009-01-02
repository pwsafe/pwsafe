/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/*
* This routine doesn't do anything as Xerces is a validating XML Parser.
* However, it is present to mimic Expat's version and contains similar data.
*
* Note: Xerces uses wchar_t even in non-Unicode mode.
*/

#include "../XMLDefs.h"

#if USE_XML_LIBRARY == XERCES

// XML File Import constants - used by Expat and Xerces and will be by MSXML
#include "../XMLFileValidation.h"

// Xerces validation includes
#include "XFileValidator.h"

// PWS includes
#include "../../StringX.h"

#include <map>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace std;

XFileValidator::XFileValidator()
{
}

#endif /* USE_XML_LIBRARY == XERCES */
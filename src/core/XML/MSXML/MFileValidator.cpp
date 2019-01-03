/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/*
* This routine doesn't do anything as MS XML is a validating XML Parser.
*
* Non-unicode builds will need convert any results from parsing the XML
* document from UTF-16 to ASCII.  This is done in the XFileSAX2Handlers routines:
* 'startelement' for attributes and 'characters' & 'ignorableWhitespace'
* for element data.
*/

#include "../XMLDefs.h"    // Required if testing "USE_XML_LIBRARY"

#if USE_XML_LIBRARY == MSXML

// XML File Import constants - used by Xerces and MSXML
#include "../XMLFileValidation.h"

// MSXML validation includes
#include "MFileValidator.h"

using namespace std;

MFileValidator::MFileValidator()
{
}

#endif /* USE_XML_LIBRARY == MSXML */

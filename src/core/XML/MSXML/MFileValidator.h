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

#ifndef __MFILEVALIDATOR_H
#define __MFILEVALIDATOR_H

// XML File Import constants - used by Xerces and will be by MSXML
#include "../XMLFileValidation.h"

class MFileValidator : public XMLFileValidation
{
public:
  MFileValidator();
};

#endif /* __MFILEVALIDATOR_H */

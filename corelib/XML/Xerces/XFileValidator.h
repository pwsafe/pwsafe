/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/*
* This routine doesn't do anything as Xerces is a validating XML Parser.
* However, it is present to mimic Expat's version and contains similar data
* to streamline import processing.
*
* Note: Xerces uses wchar_t even in non-Unicode mode.
*/

#ifndef __XFILEVALIDATOR_H
#define __XFILEVALIDATOR_H

// XML File Import constants - used by Expat and Xerces and will be by MSXML
#include "../XMLFileValidation.h"

// PWS includes
#include "../../StringX.h"

#include <vector>
#include <map>

// Xerces includes
#include <xercesc/util/XercesDefs.hpp>

XERCES_CPP_NAMESPACE_USE

class XFileValidator : public XMLFileValidation
{
public:
  XFileValidator();
};

#endif /* __XFILEVALIDATOR_H */

/*
www.sourceforge.net/projects/tinyxml
Original code (2.0 and earlier )copyright (c) 2000-2006 Lee Thomason (www.grinninglizard.com)

This software is provided 'as-is', without any express or implied 
warranty. In no event will the authors be held liable for any 
damages arising from the use of this software.

Permission is granted to anyone to use this software for any 
purpose, including commercial applications, and to alter it and 
redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must
not claim that you wrote the original software. If you use this
software in a product, an acknowledgment in the product documentation
would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.
*/

/*
 * THIS FILE WAS ALTERED BY Rony Shapiro, for integration into the PasswordSafe
 * project http://passwordsafe.sourceforge.net/
 */

#include "../../stdafx.h"
#include "tinyxml.h"

// The goal of the seperate error file is to make the first
// step towards localization. tinyxml (currently) only supports
// english error messages, but the could now be translated.
//
// It also cleans up the code a bit.
//

const TCHAR* TiXmlBase::errorString[ TIXML_ERROR_STRING_COUNT ] =
{
	_T("No error"),
	_T("Error"),
	_T("Failed to open file"),
	_T("Memory allocation failed."),
	_T("Error parsing Element."),
	_T("Failed to read Element name"),
	_T("Error reading Element value."),
	_T("Error reading Attributes."),
	_T("Error: empty tag."),
	_T("Error reading end tag."),
	_T("Error parsing Unknown."),
	_T("Error parsing Comment."),
	_T("Error parsing Declaration."),
	_T("Error document empty."),
	_T("Error null (0) or unexpected EOF found in input stream."),
	_T("Error parsing CDATA."),
	_T("Error when TiXmlDocument added to document, because TiXmlDocument can only be at the root."),
};

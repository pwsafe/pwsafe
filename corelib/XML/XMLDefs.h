/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#ifndef __XMLDEFS_H
#define __XMLDEFS_H

// XML Libraries
#define EXPAT  1
#define MSXML  2
#define XERCES 3

#if USE_XML_LIBRARY == EXPAT
#ifdef UNICODE
#define XML_UNICODE_WCHAR_T
#else
#undef XML_UNICODE_WCHAR_T
#endif
#endif

#define PWS_XML_FILTER_VERSION 1

enum {FI_NORMAL = 0, FI_HISTORY, FI_POLICY, FI_INVALID};

// enum for SAX2 error types
enum {SAX2_WARNING, SAX2_ERROR, SAX2_FATALERROR};

#endif /* __XMLDEFS_H */

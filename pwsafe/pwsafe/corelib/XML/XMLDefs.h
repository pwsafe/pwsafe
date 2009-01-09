/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
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
#ifndef XML_STATIC
#define XML_STATIC
#endif
#ifdef UNICODE
#define XML_UNICODE_WCHAR_T
#define XML_UNICODE
#else
#undef XML_UNICODE_WCHAR_T
#undef XML_UNICODE
#endif
#endif

#if USE_XML_LIBRARY == XERCES
#ifndef XERCES_STATIC_LIBRARY
#define XERCES_STATIC_LIBRARY
#endif
#endif

#define PWS_XML_FILTER_VERSION 1

// enum for SAX2 error types
enum {SAX2_WARNING, SAX2_ERROR, SAX2_FATALERROR};

#endif /* __XMLDEFS_H */

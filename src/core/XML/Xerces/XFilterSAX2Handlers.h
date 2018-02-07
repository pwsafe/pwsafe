/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/*
* This routine processes Filter XML using the STANDARD and UNMODIFIED
* Xerces library V3.1.1 released on April 27, 2010
*
* See http://xerces.apache.org/xerces-c/
*
* Note: This is a cross-platform library and can be linked in as a
* Static library or used as a dynamic library e.g. DLL in Windows.
* To use the static version, the following pre-processor statement
* must be defined: XERCES_STATIC_LIBRARY
*
*/

/*
* NOTE: Xerces characters are ALWAYS in UTF-16 (may or may not be wchar_t 
* depending on platform).
* Non-unicode builds will need convert any results from parsing the XML
* document from UTF-16 to ASCII.
*/

#ifndef __XFILTERSAX2HANDLERS_H
#define __XFILTERSAX2HANDLERS_H

// PWS includes
#include "../../PWSFilters.h"
#include "../../Proxy.h"

// Xerces includes
#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>

class PWSFilters;

XERCES_CPP_NAMESPACE_USE

class XFilterSAX2Handlers : public DefaultHandler
{
public:
  XFilterSAX2Handlers();
  virtual ~XFilterSAX2Handlers();

  // Local variables & functions
  void SetVariables(Asker *pAsker, PWSFilters *mapfilters, const FilterPool fpool, 
                    const bool &bValidation)
  {m_pAsker = pAsker; m_MapXMLFilters = mapfilters, m_FPool = fpool; m_bValidation = bValidation;}
  void SetSchemaVersion(int ischema_version)
  {m_iSchema_Version = ischema_version;}

  PWSFilters *m_MapXMLFilters;  // So as not to confuse with UI & core
  FilterPool m_FPool;
  int m_type;

  // -----------------------------------------------------------------------
  //  Handlers for the SAX ContentHandler interface
  // -----------------------------------------------------------------------
  void startElement(const XMLCh* const uri, const XMLCh* const localname, 
                    const XMLCh* const qname, const Attributes& attrs);
  void characters(const XMLCh* const chars, const XMLSize_t length);
  void ignorableWhitespace(const XMLCh* const chars, const XMLSize_t length);
  void endElement(const XMLCh* const uri,
                  const XMLCh* const localname,
                  const XMLCh* const qname);
  void setDocumentLocator(const Locator *const locator) {m_pLocator = locator;}
  void startDocument();
  bool getIfErrors() {return m_bErrors;}

  // -----------------------------------------------------------------------
  //  Handlers for the SAX ErrorHandler interface
  // -----------------------------------------------------------------------
  void warning(const SAXParseException& exc);
  void error(const SAXParseException& exc);
  void fatalError(const SAXParseException& exc);
  void resetErrors() {m_bErrors = false;}
  stringT getValidationResult() {return m_strValidationResult;}
  stringT getXMLErrors() {return m_strXMLErrors;}

private:
  void FormatError(const SAXParseException& e, const int type);

  // Local variables
  st_filters *cur_filter;
  st_FilterRow *cur_filterentry;
  const Locator *m_pLocator;
  XMLCh * m_pSchema_Version;
  Asker *m_pAsker;

  StringX m_sxElemContent;
  stringT m_strValidationResult;
  stringT m_strXMLErrors;

  int m_fieldlen;
  int m_iXMLVersion, m_iSchemaVersion;
  int m_iSchema_Version;
  bool m_bValidation;
  bool m_bEntryBeingProcessed;
  bool m_bErrors;
  unsigned char m_ctype;
  unsigned char *m_pfield;
};

#endif /* __XFILTERSAX2HANDLERS_H */

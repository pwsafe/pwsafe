/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/*
* This routine processes File XML using the STANDARD and UNMODIFIED
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

#include "../XMLDefs.h"    // Required if testing "USE_XML_LIBRARY"

#if USE_XML_LIBRARY == XERCES

// PWS includes
#include "XFileSAX2Handlers.h"
#include "XFileValidator.h"

#include "../../core.h"

// Xerces includes
#include <xercesc/util/XMLString.hpp>

#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/sax/SAXException.hpp>

#include "./XMLChConverter.h"

using namespace std;

XFileSAX2Handlers::XFileSAX2Handlers()
{
  m_pValidator = new XFileValidator;
}

XFileSAX2Handlers::~XFileSAX2Handlers()
{
  delete m_pValidator;
}

void XFileSAX2Handlers::startDocument( )
{
  m_strXMLErrors = _T("");
  m_bEntryBeingProcessed = false;
}

void XFileSAX2Handlers::startElement(const XMLCh* const /* uri */,
                                     const XMLCh* const /* localname */,
                                     const XMLCh* const qname,
                                     const Attributes& attrs)
{
  USES_XMLCH_STR

  if (m_bValidation) {
    const XMLCh* pwsafe = _A2X("passwordsafe");
    if (XMLString::equals(qname, pwsafe)) {
      // Only interested in the delimiter attribute
      const XMLCh *szValue = attrs.getValue(_A2X("delimiter"));
      if (szValue != nullptr) {
        m_delimiter = szValue[0];
      }
    }
  }

  m_sxElemContent = _T("");

  st_file_element_data edata;
  m_pValidator->GetElementInfo(qname, edata);
  const int icurrent_element = m_bEntryBeingProcessed ? edata.element_entry_code : edata.element_code;
  if (!XMLFileHandlers::ProcessStartElement(icurrent_element))
    return;

  switch (icurrent_element) {
    case XLE_ENTRY:
      {
        const XMLCh *szValue1 = attrs.getValue(_A2X("normal"));
        if (szValue1 != nullptr) {
          m_cur_entry->bforce_normal_entry =
               XMLString::equals(szValue1, _A2X("1")) || XMLString::equals(szValue1, _A2X("true"));
        }
        const XMLCh *szValue2 = attrs.getValue(_A2X("id"));
        if (szValue2 != nullptr) {
          m_cur_entry->id = XMLString::parseInt(szValue2);
        }
      }
      break;
    default:
      break;
  }
  return;
}

void XFileSAX2Handlers::characters(const XMLCh* const chars, const XMLSize_t length)
{
  if (m_bValidation)
    return;

  XMLCh *xmlchData = new XMLCh[length + 1];
  XMLString::copyNString(xmlchData, chars, length);
  xmlchData[length] = L'\0';
  m_sxElemContent += StringX(_X2SX(xmlchData));
  delete [] xmlchData;
}

void XFileSAX2Handlers::ignorableWhitespace(const XMLCh* const chars,
                                           const XMLSize_t length)
{
  if (m_bValidation)
    return;

  XMLCh *xmlchData = new XMLCh[length + 1];
  XMLString::copyNString(xmlchData, chars, length);
  xmlchData[length] = L'\0';
  m_sxElemContent += StringX(_X2SX(xmlchData));
  delete [] xmlchData;
}

void XFileSAX2Handlers::endElement(const XMLCh* const /* uri */,
                                   const XMLCh* const /* localname */,
                                   const XMLCh* const qname)
{
  USES_XMLCH_STR

  if (m_bValidation) {
    if (XMLString::equals(qname, _A2X("entry")))
      m_numEntries++;
    return;
  }

  st_file_element_data edata;
  m_pValidator->GetElementInfo(qname, edata);

  // The rest is only processed in Import mode (not Validation mode)
  const int icurrent_element = m_bEntryBeingProcessed ? edata.element_entry_code : edata.element_code;
  XMLFileHandlers::ProcessEndElement(icurrent_element);
}

void XFileSAX2Handlers::FormatError(const SAXParseException& e, const int type)
{
  stringT FormatString;
  int iLineNumber, iCharacter;

  stringT ErrorMessage = _X2ST(e.getMessage());
  iLineNumber = static_cast<int>(e.getLineNumber());
  iCharacter = static_cast<int>(e.getColumnNumber());

  stringT cs_format, cs_errortype;
  LoadAString(cs_format, IDSC_XERCESSAXGENERROR);
  switch (type) {
    case SAX2_WARNING:
      LoadAString(cs_errortype, IDSC_SAX2WARNING);
      break;
    case SAX2_ERROR:
      LoadAString(cs_errortype, IDSC_SAX2ERROR);
      break;
    case SAX2_FATALERROR:
      LoadAString(cs_errortype, IDSC_SAX2FATALERROR);
      break;
    default:
      assert(0);
  }

  Format(FormatString, cs_format.c_str(),
         cs_errortype.c_str(), iLineNumber, iCharacter, ErrorMessage.c_str());

  m_strValidationResult += FormatString + _T("\r\n");
}

void XFileSAX2Handlers::error(const SAXParseException& e)
{
  FormatError(e, SAX2_ERROR);
  m_bErrors = true;
}

void XFileSAX2Handlers::fatalError(const SAXParseException& e)
{
  FormatError(e, SAX2_FATALERROR);
  m_bErrors = true;
}

void XFileSAX2Handlers::warning(const SAXParseException& e)
{
  FormatError(e, SAX2_WARNING);
}

#endif /* USE_XML_LIBRARY == XERCES */

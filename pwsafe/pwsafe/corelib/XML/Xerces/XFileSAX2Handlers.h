/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/*
* This routine processes Filter XML using the STANDARD and UNMODIFIED
* Xerces library V3.0.0 released on September 29, 2008
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

#ifndef __XFILESAX2HANDLERS_H
#define __XFILESAX2HANDLERS_H

// XML File Import constants - used by Expat and Xerces and will be by MSXML
#include "../XMLFileValidation.h"

#include "XFileValidator.h"

// PWS includes
#include "../../StringX.h"
#include "../../ItemData.h"
#include "../../UUIDGen.h"
#include "../../UnknownField.h"
#include "../../PWScore.h"

// Xerces includes
#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>

// Entry types
enum {NORMAL = 0, ALIAS, SHORTCUT};

// New imported entry
struct pw_entry {
  StringX group;
  StringX title;
  StringX username;
  StringX password;
  StringX url;
  StringX autotype;
  StringX ctime;
  StringX atime;
  StringX xtime;
  StringX xtime_interval;
  StringX pmtime;
  StringX rmtime;
  StringX pwhistory;
  StringX notes;
  StringX uuid;
  PWPolicy pwp;
  UnknownFieldList uhrxl;  // Note: use header format for record unknown fields!
  int entrytype;
  bool bforce_normal_entry;
};

struct pwhistory_entry {
  StringX changed;
  StringX oldpassword;
};

XERCES_CPP_NAMESPACE_USE

class XFileSAX2Handlers : public DefaultHandler
{
public:
  XFileSAX2Handlers();
  virtual ~XFileSAX2Handlers();

  void SetVariables(PWScore *core, const bool &bValidation,
                    const stringT &ImportedPrefix, const TCHAR &delimiter,
                    UUIDList *possible_aliases, UUIDList *possible_shortcuts);

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
  
  inline int getXMLPref(int num)
  {
    assert(num >= 0 && num < NUMPREFSINXML);
    return prefsinXML[num];
  }
  stringT getDefaultAutotypeString() {return m_sDefaultAutotypeString;}
  stringT getDefaultUsername() {return m_sDefaultUsername;}
  stringT getValidationResult() {return m_strValidationResult;}
  stringT getImportErrors() {return m_strImportErrors;}
  int getNumEntries() {return m_numEntries;}
  int getNumIterations() {return m_nITER;}
  int getNumRecordsWithUnknownFields() {return m_nRecordsWithUnknownFields;}
  TCHAR getDelimiter() {return m_delimiter;}
  bool getDatabaseHeaderErrors() {return m_bDatabaseHeaderErrors;}
  bool getRecordHeaderErrors() {return m_bRecordHeaderErrors;}

  UnknownFieldList m_ukhxl;  // For header unknown fields

private:
  void FormatError(const SAXParseException& e, const int type);

  // Local variables
  XFileValidator *m_pValidator;
  PWScore *m_xmlcore;
  UUIDList *m_possible_aliases;
  UUIDList *m_possible_shortcuts;
  pw_entry *cur_entry;
  pwhistory_entry *cur_pwhistory_entry;
  const Locator *m_pLocator;

  StringX m_strElemContent;
  stringT m_ImportedPrefix;
  stringT m_strValidationResult;
  stringT m_strImportErrors;
  int m_nRecordsWithUnknownFields;
  int m_whichtime, m_ipwh;
  int m_fieldlen;
  bool m_bDatabaseHeaderErrors, m_bRecordHeaderErrors;
  bool m_bErrorsFound, m_bErrors;
  bool m_bentrybeingprocessed;
  bool m_bValidation;
  bool m_bheader;
  TCHAR m_delimiter;
  unsigned char m_ctype;
  unsigned char * m_pfield;

  // Preferences possibly stored in database
  // Note: boolean is integer to allow an 'not present' value of '-1'
  int prefsinXML[NUMPREFSINXML];
  stringT m_sDefaultAutotypeString;
  stringT m_sDefaultUsername;
  int m_nITER;
  int m_numEntries;

  static const struct st_file_elements {
    TCHAR *name; st_file_element_data file_element_data;
  } m_file_elements[XLE_ELEMENTS];
};

#endif /* __XFILESAX2HANDLERS_H */

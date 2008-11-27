/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
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

#include "../XMLDefs.h"

#if USE_XML_LIBRARY == XERCES

// PWS includes
#include "XFilterSAX2Handlers.h"

#include "../util.h"
#include "../UUIDGen.h"
#include "../corelib.h"
#include "../PWSFilters.h"
#include "../VerifyFormat.h"
#include "../Match.h"

#include <map>
#include <algorithm>

// Xerces includes
#include <xercesc/util/XMLString.hpp>

#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/sax/SAXException.hpp>

using namespace std;

XFilterSAX2Handlers::XFilterSAX2Handlers()
{
  m_strElemContent.clear();
  m_iXMLVersion = -1;
  m_iSchemaVersion = -1;
  m_bErrors = false;
  m_pAsker = NULL;
}

XFilterSAX2Handlers::~XFilterSAX2Handlers()
{
}

void XFilterSAX2Handlers::startDocument()
{
  m_strImportErrors.clear();
  m_bentrybeingprocessed = false;
}

void XFilterSAX2Handlers::startElement(const XMLCh* const /* uri */,
                                      const XMLCh* const /* localname */,
                                      const XMLCh* const qname,
                                      const Attributes& attrs)
{
  if (m_bValidation && XMLString::equals(qname, L"filters")) {
    if (m_pSchema_Version == NULL) {
      LoadAString(m_strImportErrors, IDSC_MISSING_SCHEMA_VER);
#ifdef UNICODE
      const XMLCh *message = m_strImportErrors.c_str();
#else
      const XMLCh *message = XMLString::transcode(m_strImportErrors.c_str());
#endif
      SAXParseException(message, *m_pLocator);
#ifndef UNICODE
      XMLString::release((XMLCh **)&message);
#endif
      return;
    }

    if (m_iSchemaVersion <= 0) {
      LoadAString(m_strImportErrors, IDSC_INVALID_SCHEMA_VER);
#ifdef UNICODE
      const XMLCh *message = m_strImportErrors.c_str();
#else
      const XMLCh *message = XMLString::transcode(m_strImportErrors.c_str());
#endif
      SAXParseException(message, *m_pLocator);
#ifndef UNICODE
      XMLString::release((XMLCh **)&message);
#endif
      return;
    }

    const XMLCh * xmlchValue = attrs.getValue(L"version");
    if (xmlchValue != NULL) {
      m_iXMLVersion = XMLString::parseInt(xmlchValue);
    }
    return;
  }

  if (m_bValidation)
    return;

  bool bfilter = XMLString::equals(qname, L"filter");
  bool bfilter_entry = XMLString::equals(qname, L"filter_entry");
 
   if (bfilter) {
    cur_filter = new st_filters;
  }

  if (bfilter_entry) {
    cur_filterentry = new st_FilterRow;
    cur_filterentry->Empty();
    cur_filterentry->bFilterActive = true;
    m_bentrybeingprocessed = true;
  }

  if (bfilter || bfilter_entry) {
    // Process the attributes we need.
    if (bfilter) {
      XMLCh * xmlchValue = (XMLCh *)attrs.getValue(L"filtername"); 
      if (xmlchValue != NULL) {
#ifdef UNICODE
        cur_filter->fname = stringT(xmlchValue);
#else
        char *szValue = XMLString::transcode(xmlchValue);
        cur_filter->fname = stringT(szValue);
        XMLString::release(&szValue);
#endif
      }
    }

    if (bfilter_entry) {
      XMLCh * xmlchValue = (XMLCh *)attrs.getValue(L"active"); 
      if (xmlchValue != NULL && XMLString::equals(xmlchValue, L"no"))
        cur_filterentry->bFilterActive = false;
    }
  }

  m_strElemContent = _T("");
}

void XFilterSAX2Handlers::characters(const XMLCh* const chars,
                                    const XMLSize_t length)
{
  if (m_bValidation)
    return;

  XMLCh *xmlchData = new XMLCh[length + 1];
  XMLString::copyNString(xmlchData, chars, length);
  xmlchData[length] = L'\0';
#ifdef UNICODE
  m_strElemContent += StringX(xmlchData);
#else
  char *szData = XMLString::transcode(xmlchData);
  m_strElemContent += StringX(szData);
  XMLString::release(&szData);
#endif
  delete [] xmlchData;
}

void XFilterSAX2Handlers::ignorableWhitespace(const XMLCh* const chars,
                                             const XMLSize_t length)
{
  if (m_bValidation)
    return;

  XMLCh *xmlchData = new XMLCh[length + 1];
  XMLString::copyNString(xmlchData, chars, length);
  xmlchData[length] = L'\0';
#ifdef UNICODE
  m_strElemContent += StringX(xmlchData);
#else
  char *szData = XMLString::transcode(xmlchData);
  m_strElemContent += StringX(szData);
  XMLString::release(&szData);
#endif
  delete [] xmlchData;
}

void XFilterSAX2Handlers::endElement(const XMLCh* const /* uri */,
                                    const XMLCh* const /* localname */,
                                    const XMLCh* const qname)
{
  if (m_bValidation && XMLString::equals(qname, L"filters")) {
    // Check that the XML file version is present and that
    // a. it is less than or equal to the Filter schema version
    // b. it is less than or equal to the version supported by this PWS
    if (m_iXMLVersion < 0) {
      LoadAString(m_strImportErrors, IDSC_MISSING_XML_VER);
#ifdef UNICODE
      const XMLCh *message = m_strImportErrors.c_str();
#else
      const XMLCh *message = XMLString::transcode(m_strImportErrors.c_str());
#endif
      SAXParseException(message, *m_pLocator);
#ifndef UNICODE
      XMLString::release((XMLCh **)&message);
#endif
      return ;
    }
    if (m_iXMLVersion > m_iSchemaVersion) {
      Format(m_strImportErrors,
             IDSC_INVALID_XML_VER1, m_iXMLVersion, m_iSchemaVersion);
#ifdef UNICODE
      const XMLCh *message = m_strImportErrors.c_str();
#else
      const XMLCh *message = XMLString::transcode(m_strImportErrors.c_str());
#endif
      SAXParseException(message, *m_pLocator);
#ifndef UNICODE
      XMLString::release((XMLCh **)&message);
#endif
      return ;
    }
    if (m_iXMLVersion > PWS_XML_FILTER_VERSION) {
      Format(m_strImportErrors, 
             IDSC_INVALID_XML_VER2, m_iXMLVersion, PWS_XML_FILTER_VERSION);
#ifdef UNICODE
      const XMLCh *message = m_strImportErrors.c_str();
#else
      const XMLCh *message = XMLString::transcode(m_strImportErrors.c_str());
#endif
      SAXParseException(message, *m_pLocator);
#ifndef UNICODE
      XMLString::release((XMLCh **)&message);
#endif
      return;
    }
  }

  if (m_bValidation) {
    return;
  }

  if (XMLString::equals(qname, L"filter")) {
    INT_PTR rc = IDYES;
    st_Filterkey fk;
    fk.fpool = m_FPool;
    fk.cs_filtername = cur_filter->fname;
    if (m_MapFilters->find(fk) != m_MapFilters->end()) {
      stringT question;
      Format(question,
             _T("Filter %s already exists in the database, do you wish to replace it with this?"),
             cur_filter->fname);
      if (m_pAsker == NULL || !(*m_pAsker)(question)) {
        m_MapFilters->erase(fk);
      }
    }
    if (rc == IDYES) {
      m_MapFilters->insert(PWSFilters::Pair(fk, *cur_filter));
    }
    delete cur_filter;
    return;
  }

  else if (XMLString::equals(qname, L"filter_entry")) {
    if (m_type == FI_NORMAL) {
      cur_filter->num_Mactive++;
      cur_filter->vMfldata.push_back(*cur_filterentry);
    } else if (m_type == FI_HISTORY) {
      cur_filter->num_Hactive++;
      cur_filter->vHfldata.push_back(*cur_filterentry);
    } else if (m_type == FI_POLICY) {
      cur_filter->num_Pactive++;
      cur_filter->vPfldata.push_back(*cur_filterentry);
    }
    delete cur_filterentry;
  }

  else if (XMLString::equals(qname, L"grouptitle")) {
    m_type = FI_NORMAL;
    cur_filterentry->mtype = PWSMatch::MT_STRING;
    cur_filterentry->ftype = FT_GROUPTITLE;
  }

  else if (XMLString::equals(qname, L"group")) {
    m_type = FI_NORMAL;
    cur_filterentry->mtype = PWSMatch::MT_STRING;
    cur_filterentry->ftype = FT_GROUP;
  }

  else if (XMLString::equals(qname, L"title")) {
    m_type = FI_NORMAL;
    cur_filterentry->mtype = PWSMatch::MT_STRING;
    cur_filterentry->ftype = FT_TITLE;
  }

  else if (XMLString::equals(qname, L"username")) {
    m_type = FI_NORMAL;
    cur_filterentry->mtype = PWSMatch::MT_STRING;
    cur_filterentry->ftype = FT_USER;
  }

  else if (XMLString::equals(qname, L"password")) {
    m_type = FI_NORMAL;
    cur_filterentry->mtype = PWSMatch::MT_PASSWORD;
    cur_filterentry->ftype = FT_PASSWORD;
  }

  else if (XMLString::equals(qname, L"url")) {
    m_type = FI_NORMAL;
    cur_filterentry->mtype = PWSMatch::MT_STRING;
    cur_filterentry->ftype = FT_URL;
  }

  else if (XMLString::equals(qname, L"autotype")) {
    m_type = FI_NORMAL;
    cur_filterentry->mtype = PWSMatch::MT_STRING;
    cur_filterentry->ftype = FT_AUTOTYPE;
  }

  else if (XMLString::equals(qname, L"notes")) {
    m_type = FI_NORMAL;
    cur_filterentry->mtype = PWSMatch::MT_STRING;
    cur_filterentry->ftype = FT_NOTES;
  }

  else if (XMLString::equals(qname, L"create_time")) {
    m_type = FI_NORMAL;
    cur_filterentry->mtype = PWSMatch::MT_DATE;
    cur_filterentry->ftype = FT_CTIME;
  }

  else if (XMLString::equals(qname, L"password_modified_time")) {
    m_type = FI_NORMAL;
    cur_filterentry->mtype = PWSMatch::MT_DATE;
    cur_filterentry->ftype = FT_PMTIME;
  }

  else if (XMLString::equals(qname, L"last_access_time")) {
    m_type = FI_NORMAL;
    cur_filterentry->mtype = PWSMatch::MT_DATE;
    cur_filterentry->ftype = FT_ATIME;
  }

  else if (XMLString::equals(qname, L"expiry_time")) {
    m_type = FI_NORMAL;
    cur_filterentry->mtype = PWSMatch::MT_DATE;
    cur_filterentry->ftype = FT_XTIME;
  }

  else if (XMLString::equals(qname, L"record_modified_time")) {
    m_type = FI_NORMAL;
    cur_filterentry->mtype = PWSMatch::MT_DATE;
    cur_filterentry->ftype = FT_RMTIME;
  }

  else if (XMLString::equals(qname, L"password_expiry_interval")) {
    m_type = FI_NORMAL;
    cur_filterentry->mtype = PWSMatch::MT_INTEGER;
    cur_filterentry->ftype = FT_XTIME_INT;
  }

  else if (XMLString::equals(qname, L"entrytype")) {
    m_type = FI_NORMAL;
    cur_filterentry->mtype = PWSMatch::MT_ENTRYTYPE;
    cur_filterentry->ftype = FT_ENTRYTYPE;
  }

  else if (XMLString::equals(qname, L"unknownfields")) {
    m_type = FI_NORMAL;
    cur_filterentry->ftype = FT_UNKNOWNFIELDS;
  }

  else if (XMLString::equals(qname, L"password_history")) {
    m_type = FI_NORMAL;
    cur_filterentry->mtype = PWSMatch::MT_PWHIST;
    cur_filterentry->ftype = FT_PWHIST;
  }

  else if (XMLString::equals(qname, L"history_present")) {
    m_type = FI_HISTORY;
    cur_filterentry->mtype = PWSMatch::MT_BOOL;
    cur_filterentry->ftype = HT_PRESENT;
  }

  else if (XMLString::equals(qname, L"history_active")) {
    m_type = FI_HISTORY;
    cur_filterentry->mtype = PWSMatch::MT_BOOL;
    cur_filterentry->ftype = HT_ACTIVE;
  }

  else if (XMLString::equals(qname, L"history_number")) {
    m_type = FI_HISTORY;
    cur_filterentry->mtype = PWSMatch::MT_INTEGER;
    cur_filterentry->ftype = HT_NUM;
  }

  else if (XMLString::equals(qname, L"history_maximum")) {
    m_type = FI_HISTORY;
    cur_filterentry->mtype = PWSMatch::MT_INTEGER;
    cur_filterentry->ftype = HT_MAX;
  }

  else if (XMLString::equals(qname, L"history_changedate")) {
    m_type = FI_HISTORY;
    cur_filterentry->mtype = PWSMatch::MT_DATE;
    cur_filterentry->ftype = HT_CHANGEDATE;
  }

  else if (XMLString::equals(qname, L"history_passwords")) {
    m_type = FI_HISTORY;
    cur_filterentry->mtype = PWSMatch::MT_PASSWORD;
    cur_filterentry->ftype = HT_PASSWORDS;
  }

  else if (XMLString::equals(qname, L"password_policy")) {
    m_type = FI_NORMAL;
    cur_filterentry->mtype = PWSMatch::MT_POLICY;
    cur_filterentry->ftype = FT_POLICY;
  }

  else if (XMLString::equals(qname, L"policy_present")) {
    m_type = FI_POLICY;
    cur_filterentry->mtype = PWSMatch::MT_BOOL;
    cur_filterentry->ftype = PT_PRESENT;
  }

  else if (XMLString::equals(qname, L"policy_length")) {
    m_type = FI_POLICY;
    cur_filterentry->mtype = PWSMatch::MT_INTEGER;
    cur_filterentry->ftype = PT_LENGTH;
  }

  else if (XMLString::equals(qname, L"policy_number_lowercase")) {
    m_type = FI_POLICY;
    cur_filterentry->mtype = PWSMatch::MT_INTEGER;
    cur_filterentry->ftype = PT_LOWERCASE;
  }

  else if (XMLString::equals(qname, L"policy_number_uppercase")) {
    m_type = FI_POLICY;
    cur_filterentry->mtype = PWSMatch::MT_INTEGER;
    cur_filterentry->ftype = PT_UPPERCASE;
  }

  else if (XMLString::equals(qname, L"policy_number_digits")) {
    m_type = FI_POLICY;
    cur_filterentry->mtype = PWSMatch::MT_INTEGER;
    cur_filterentry->ftype = PT_DIGITS;
  }

  else if (XMLString::equals(qname, L"policy_number_symbols")) {
    m_type = FI_POLICY;
    cur_filterentry->mtype = PWSMatch::MT_INTEGER;
    cur_filterentry->ftype = PT_SYMBOLS;
  }

  else if (XMLString::equals(qname, L"policy_easyvision")) {
    m_type = FI_POLICY;
    cur_filterentry->mtype = PWSMatch::MT_BOOL;
    cur_filterentry->ftype = PT_EASYVISION;
  }

  else if (XMLString::equals(qname, L"policy_pronounceable")) {
    m_type = FI_POLICY;
    cur_filterentry->mtype = PWSMatch::MT_BOOL;
    cur_filterentry->ftype = PT_PRONOUNCEABLE;
  }

  else if (XMLString::equals(qname, L"policy_hexadecimal")) {
    m_type = FI_POLICY;
    cur_filterentry->mtype = PWSMatch::MT_BOOL;
    cur_filterentry->ftype = PT_HEXADECIMAL;
  }

  else if (XMLString::equals(qname, L"rule")) {
    ToUpper(m_strElemContent);
    if (m_strElemContent == _T("EQ"))
      cur_filterentry->rule = PWSMatch::MR_EQUALS;
    else if (m_strElemContent == _T("NE"))
      cur_filterentry->rule = PWSMatch::MR_NOTEQUAL;
    else if (m_strElemContent == _T("AC"))
      cur_filterentry->rule = PWSMatch::MR_ACTIVE;
    else if (m_strElemContent == _T("IA"))
      cur_filterentry->rule = PWSMatch::MR_INACTIVE;
    else if (m_strElemContent == _T("PR"))
      cur_filterentry->rule = PWSMatch::MR_PRESENT;
    else if (m_strElemContent == _T("NP"))
      cur_filterentry->rule = PWSMatch::MR_NOTPRESENT;
    else if (m_strElemContent == _T("SE"))
      cur_filterentry->rule = PWSMatch::MR_SET;
    else if (m_strElemContent == _T("NS"))
      cur_filterentry->rule = PWSMatch::MR_NOTSET;
    else if (m_strElemContent == _T("IS"))
      cur_filterentry->rule = PWSMatch::MR_IS;
    else if (m_strElemContent == _T("NI"))
      cur_filterentry->rule = PWSMatch::MR_ISNOT;
    else if (m_strElemContent == _T("BE"))
      cur_filterentry->rule = PWSMatch::MR_BEGINS;
    else if (m_strElemContent == _T("NB"))
      cur_filterentry->rule = PWSMatch::MR_NOTBEGIN;
    else if (m_strElemContent == _T("EN"))
      cur_filterentry->rule = PWSMatch::MR_ENDS;
    else if (m_strElemContent == _T("ND"))
      cur_filterentry->rule = PWSMatch::MR_NOTEND;
    else if (m_strElemContent == _T("CO"))
      cur_filterentry->rule = PWSMatch::MR_CONTAINS;
    else if (m_strElemContent == _T("NC"))
      cur_filterentry->rule = PWSMatch::MR_NOTCONTAIN;
    else if (m_strElemContent == _T("BT"))
      cur_filterentry->rule = PWSMatch::MR_BETWEEN;
    else if (m_strElemContent == _T("LT"))
      cur_filterentry->rule = PWSMatch::MR_LT;
    else if (m_strElemContent == _T("LE"))
      cur_filterentry->rule = PWSMatch::MR_LE;
    else if (m_strElemContent == _T("GT"))
      cur_filterentry->rule = PWSMatch::MR_GT;
    else if (m_strElemContent == _T("GE"))
      cur_filterentry->rule = PWSMatch::MR_GE;
    else if (m_strElemContent == _T("BF"))
      cur_filterentry->rule = PWSMatch::MR_BEFORE;
    else if (m_strElemContent == _T("AF"))
      cur_filterentry->rule = PWSMatch::MR_AFTER;
    else if (m_strElemContent == _T("EX"))
      cur_filterentry->rule = PWSMatch::MR_EXPIRED;
    else if (m_strElemContent == _T("WX"))
      cur_filterentry->rule = PWSMatch::MR_WILLEXPIRE;
  }

  else if (XMLString::equals(qname, L"logic")) {
    if (m_strElemContent == _T("or"))
      cur_filterentry->ltype = LC_OR;
    else
      cur_filterentry->ltype = LC_AND;
  }

  else if (XMLString::equals(qname, L"string")) {
    cur_filterentry->fstring = m_strElemContent;
  }

  else if (XMLString::equals(qname, L"case")) {
    cur_filterentry->fcase = _ttoi(m_strElemContent.c_str()) != 0;
  }

  else if (XMLString::equals(qname, L"warn")) {
    cur_filterentry->fnum1 = _ttoi(m_strElemContent.c_str());
  }

  else if (XMLString::equals(qname, L"num1")) {
    cur_filterentry->fnum1 = _ttoi(m_strElemContent.c_str());
  }

  else if (XMLString::equals(qname, L"num2")) {
    cur_filterentry->fnum1 = _ttoi(m_strElemContent.c_str());
  }

  else if (XMLString::equals(qname, L"date1")) {
    time_t t(0);
    if (VerifyXMLDateString(m_strElemContent.c_str(), t) &&
        (t != (time_t)-1))
      cur_filterentry->fdate1 = t;
    else
    cur_filterentry->fdate1 = (time_t)0;
  }

  else if (XMLString::equals(qname, L"date2")) {
    time_t t(0);
    if (VerifyXMLDateString(m_strElemContent.c_str(), t) &&
        (t != (time_t)-1))
      cur_filterentry->fdate2 = t;
    else
      cur_filterentry->fdate1 = (time_t)0;
  }

  else if (XMLString::equals(qname, L"type")) {
    if (m_strElemContent == _T("normal"))
      cur_filterentry->etype = CItemData::ET_NORMAL;
    else if (m_strElemContent == _T("alias"))
      cur_filterentry->etype = CItemData::ET_ALIAS;
    else if (m_strElemContent == _T("shortcut"))
      cur_filterentry->etype = CItemData::ET_SHORTCUT;
    else if (m_strElemContent == _T("aliasbase"))
      cur_filterentry->etype = CItemData::ET_ALIASBASE;
    else if (m_strElemContent == _T("shortcutbase"))
      cur_filterentry->etype = CItemData::ET_SHORTCUTBASE;
  } else if (!(XMLString::equals(qname, L"test") ||
               XMLString::equals(qname, L"filters"))) {
      ASSERT(0);
  }

  return;
}

void XFilterSAX2Handlers::FormatError(const SAXParseException& e, const int type)
{
  TCHAR szFormatString[MAX_PATH * 2] = {0};
  int iLineNumber, iCharacter;

#ifdef UNICODE
  XMLCh *szErrorMessage = (XMLCh *)e.getMessage();
#else
  char *szErrorMessage = XMLString::transcode(e.getMessage());
#endif
  iLineNumber = (int)e.getLineNumber();
  iCharacter = (int)e.getColumnNumber();

  stringT cs_format, cs_errortype;
  LoadAString(cs_format, IDSC_SAXGENERROR);
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

#if (_MSC_VER >= 1400)
  _stprintf_s(szFormatString, MAX_PATH * 2, cs_format.c_str(),
              cs_errortype.c_str(), iLineNumber, iCharacter, szErrorMessage);
#else
  _stprintf(szFormatString, cs_format.c_str(), iLineNumber, iCharacter, szErrorMessage);
#endif

  m_strValidationResult += szFormatString;
#ifndef UNICODE
  XMLString::release(&szErrorMessage);
#endif
}

void XFilterSAX2Handlers::error(const SAXParseException& e)
{
  FormatError(e, SAX2_ERROR);
  m_bErrors = true;
}

void XFilterSAX2Handlers::fatalError(const SAXParseException& e)
{
  FormatError(e, SAX2_FATALERROR);
  m_bErrors = true;
}

void XFilterSAX2Handlers::warning(const SAXParseException& e)
{
  FormatError(e, SAX2_WARNING);
}

#endif /* USE_XML_LIBRARY == XERCES */

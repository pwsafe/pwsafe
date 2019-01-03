/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
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

#include "../XMLDefs.h"    // Required if testing "USE_XML_LIBRARY"

#if USE_XML_LIBRARY == XERCES

// PWS includes
#include "XFilterSAX2Handlers.h"

#include "../../Util.h"
#include "../../core.h"
#include "../../PWSFilters.h"
#include "../../VerifyFormat.h"
#include "../../Match.h"
#include "../../../os/pws_tchar.h"
#include "./XMLChConverter.h"

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
  m_sxElemContent = _T("");
  m_iXMLVersion = -1;
  m_iSchemaVersion = -1;
  m_bErrors = false;
  m_pAsker = nullptr;
}

XFilterSAX2Handlers::~XFilterSAX2Handlers()
{
}

void XFilterSAX2Handlers::startDocument()
{
  m_strXMLErrors = _T("");
  m_bEntryBeingProcessed = false;
}

void XFilterSAX2Handlers::startElement(const XMLCh* const /* uri */,
                                      const XMLCh* const /* localname */,
                                      const XMLCh* const qname,
                                      const Attributes& attrs)
{
  USES_XMLCH_STR

  if (m_bValidation && XMLString::equals(qname, _A2X("filters"))) {
    if (m_pSchema_Version == nullptr) {
      LoadAString(m_strXMLErrors, IDSC_MISSING_SCHEMA_VER);
      const XMLCh *message = _W2X(m_strXMLErrors.c_str());
      SAXParseException(message, *m_pLocator);
      return;
    }

    if (m_iSchemaVersion <= 0) {
      LoadAString(m_strXMLErrors, IDSC_INVALID_SCHEMA_VER);
      const XMLCh *message = _W2X(m_strXMLErrors.c_str());
      SAXParseException(message, *m_pLocator);
      return;
    }

    const XMLCh * xmlchValue = attrs.getValue(_A2X("version"));
    if (xmlchValue != nullptr) {
      m_iXMLVersion = XMLString::parseInt(xmlchValue);
    }
    return;
  }

  if (m_bValidation)
    return;

  bool bfilter = XMLString::equals(qname, _A2X("filter"));
  bool bfilter_entry = XMLString::equals(qname, _A2X("filter_entry"));

   if (bfilter) {
    cur_filter = new st_filters;
  }

  if (bfilter_entry) {
    cur_filterentry = new st_FilterRow;
    cur_filterentry->Empty();
    cur_filterentry->bFilterActive = true;
    m_bEntryBeingProcessed = true;
  }

  if (bfilter || bfilter_entry) {
    // Process the attributes we need.
    if (bfilter) {
      const XMLCh * xmlchValue = attrs.getValue(_A2X("filtername"));
      if (xmlchValue != nullptr) {
        cur_filter->fname = stringT(_X2ST(xmlchValue));
      }
    }

    if (bfilter_entry) {
      const XMLCh * xmlchValue = attrs.getValue(_A2X("active"));
      if (xmlchValue != nullptr && XMLString::equals(xmlchValue, _A2X("no")))
        cur_filterentry->bFilterActive = false;
    }
  }

  m_sxElemContent = _T("");
}

void XFilterSAX2Handlers::characters(const XMLCh* const chars,
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

void XFilterSAX2Handlers::ignorableWhitespace(const XMLCh* const chars,
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

void XFilterSAX2Handlers::endElement(const XMLCh* const /* uri */,
                                    const XMLCh* const /* localname */,
                                    const XMLCh* const qname)
{
  USES_XMLCH_STR

  if (m_bValidation && XMLString::equals(qname, _A2X("filters"))) {
    // Check that the XML file version is present and that
    // a. it is less than or equal to the Filter schema version
    // b. it is less than or equal to the version supported by this PWS
    if (m_iXMLVersion < 0) {
      LoadAString(m_strXMLErrors, IDSC_MISSING_XML_VER);
      const XMLCh *message = _W2X(m_strXMLErrors.c_str());
      SAXParseException(message, *m_pLocator);
      return;
    }
    if (m_iXMLVersion > m_iSchemaVersion) {
      Format(m_strXMLErrors,
             IDSC_INVALID_XML_VER1, m_iXMLVersion, m_iSchemaVersion);
      const XMLCh *message = _W2X(m_strXMLErrors.c_str());
      SAXParseException(message, *m_pLocator);
      return;
    }
    if (m_iXMLVersion > PWS_XML_FILTER_VERSION) {
      Format(m_strXMLErrors,
             IDSC_INVALID_XML_VER2, m_iXMLVersion, PWS_XML_FILTER_VERSION);
      const XMLCh *message = _W2X(m_strXMLErrors.c_str());
      SAXParseException(message, *m_pLocator);
      return;
    }
  }

  if (m_bValidation) {
    return;
  }

  if (XMLString::equals(qname, _A2X("filter"))) {
    bool bAddFilter(true);
    st_Filterkey fk;
    fk.fpool = m_FPool;
    fk.cs_filtername = cur_filter->fname;
    if (m_MapXMLFilters->find(fk) != m_MapXMLFilters->end()) {
      stringT question;
      Format(question, IDSC_FILTEREXISTS, cur_filter->fname.c_str());
      if (m_pAsker == nullptr || (bAddFilter = (*m_pAsker)(question)) == true) {
        m_MapXMLFilters->erase(fk);
      }
    }
    if (bAddFilter) {
      m_MapXMLFilters->insert(PWSFilters::Pair(fk, *cur_filter));
    }
    delete cur_filter;
    return;
  }

  else if (XMLString::equals(qname, _A2X("filter_entry"))) {
    if (cur_filterentry->mtype  == PWSMatch::MT_DATE &&
        cur_filterentry->rule   != PWSMatch::MR_PRESENT &&
        cur_filterentry->rule   != PWSMatch::MR_NOTPRESENT &&
        cur_filterentry->fdate1 == time_t(0) &&
        cur_filterentry->fdate2 == time_t(0))
      cur_filterentry->fdatetype = 1; // Relative Date
    if (m_type == DFTYPE_MAIN) {
      cur_filter->num_Mactive++;
      cur_filter->vMfldata.push_back(*cur_filterentry);
    } else if (m_type == DFTYPE_PWHISTORY) {
      cur_filter->num_Hactive++;
      cur_filter->vHfldata.push_back(*cur_filterentry);
    } else if (m_type == DFTYPE_PWPOLICY) {
      cur_filter->num_Pactive++;
      cur_filter->vPfldata.push_back(*cur_filterentry);
    }
    delete cur_filterentry;
  }

  else if (XMLString::equals(qname, _A2X("grouptitle"))) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_STRING;
    cur_filterentry->ftype = FT_GROUPTITLE;
  }

  else if (XMLString::equals(qname, _A2X("group"))) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_STRING;
    cur_filterentry->ftype = FT_GROUP;
  }

  else if (XMLString::equals(qname, _A2X("title"))) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_STRING;
    cur_filterentry->ftype = FT_TITLE;
  }

  else if (XMLString::equals(qname, _A2X("user"))) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_STRING;
    cur_filterentry->ftype = FT_USER;
  }

  else if (XMLString::equals(qname, _A2X("password"))) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_PASSWORD;
    cur_filterentry->ftype = FT_PASSWORD;
  }

  else if (XMLString::equals(qname, _A2X("notes"))) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_STRING;
    cur_filterentry->ftype = FT_NOTES;
  }

  else if (XMLString::equals(qname, _A2X("autotype"))) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_STRING;
    cur_filterentry->ftype = FT_AUTOTYPE;
  }

  else if (XMLString::equals(qname, _A2X("url"))) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_STRING;
    cur_filterentry->ftype = FT_URL;
  }

  else if (XMLString::equals(qname, _A2X("runcommand"))) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_STRING;
    cur_filterentry->ftype = FT_RUNCMD;
  }

  else if (XMLString::equals(qname, _A2X("dca"))) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_DCA;
    cur_filterentry->ftype = FT_DCA;
  }

  else if (XMLString::equals(qname, _A2X("shiftdca"))) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_SHIFTDCA;
    cur_filterentry->ftype = FT_SHIFTDCA;
  }

  else if (XMLString::equals(qname, _A2X("email"))) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_STRING;
    cur_filterentry->ftype = FT_EMAIL;
  }

  else if (XMLString::equals(qname, _A2X("protected"))) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_BOOL;
    cur_filterentry->ftype = FT_PROTECTED;
  }

  else if (XMLString::equals(qname, _A2X("kbshortcut"))) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_BOOL;
    cur_filterentry->ftype = FT_KBSHORTCUT;
  }

  else if (XMLString::equals(qname, _A2X("symbols"))) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_STRING;
    cur_filterentry->ftype = FT_SYMBOLS;
    cur_filterentry->fstring = PWSUtil::DeDupString(cur_filterentry->fstring);
  }

  else if (XMLString::equals(qname, _A2X("policyname"))) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_STRING;
    cur_filterentry->ftype = FT_POLICYNAME;
  }

  else if (XMLString::equals(qname, _A2X("create_time"))) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_DATE;
    cur_filterentry->ftype = FT_CTIME;
  }

  else if (XMLString::equals(qname, _A2X("password_modified_time"))) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_DATE;
    cur_filterentry->ftype = FT_PMTIME;
  }

  else if (XMLString::equals(qname, _A2X("last_access_time"))) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_DATE;
    cur_filterentry->ftype = FT_ATIME;
  }

  else if (XMLString::equals(qname, _A2X("expiry_time"))) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_DATE;
    cur_filterentry->ftype = FT_XTIME;
  }

  else if (XMLString::equals(qname, _A2X("record_modified_time"))) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_DATE;
    cur_filterentry->ftype = FT_RMTIME;
  }

  else if (XMLString::equals(qname, _A2X("password_expiry_interval"))) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_INTEGER;
    cur_filterentry->ftype = FT_XTIME_INT;
  }

  else if (XMLString::equals(qname, _A2X("password_length"))) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_INTEGER;
    cur_filterentry->ftype = FT_PASSWORDLEN;
  }

  else if (XMLString::equals(qname, _A2X("entrysize"))) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_ENTRYSIZE;
    cur_filterentry->ftype = FT_ENTRYSIZE;
  }

  else if (XMLString::equals(qname, _A2X("entrytype"))) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_ENTRYTYPE;
    cur_filterentry->ftype = FT_ENTRYTYPE;
  }

  else if (XMLString::equals(qname, _A2X("entrystatus"))) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_ENTRYSTATUS;
    cur_filterentry->ftype = FT_ENTRYSTATUS;
  }

  else if (XMLString::equals(qname, _A2X("unknownfields"))) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->ftype = FT_UNKNOWNFIELDS;
  }

  else if (XMLString::equals(qname, _A2X("password_history"))) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_PWHIST;
    cur_filterentry->ftype = FT_PWHIST;
  }

  else if (XMLString::equals(qname, _A2X("history_present"))) {
    m_type = DFTYPE_PWHISTORY;
    cur_filterentry->mtype = PWSMatch::MT_BOOL;
    cur_filterentry->ftype = HT_PRESENT;
  }

  else if (XMLString::equals(qname, _A2X("history_active"))) {
    m_type = DFTYPE_PWHISTORY;
    cur_filterentry->mtype = PWSMatch::MT_BOOL;
    cur_filterentry->ftype = HT_ACTIVE;
  }

  else if (XMLString::equals(qname, _A2X("history_number"))) {
    m_type = DFTYPE_PWHISTORY;
    cur_filterentry->mtype = PWSMatch::MT_INTEGER;
    cur_filterentry->ftype = HT_NUM;
  }

  else if (XMLString::equals(qname, _A2X("history_maximum"))) {
    m_type = DFTYPE_PWHISTORY;
    cur_filterentry->mtype = PWSMatch::MT_INTEGER;
    cur_filterentry->ftype = HT_MAX;
  }

  else if (XMLString::equals(qname, _A2X("history_changedate"))) {
    m_type = DFTYPE_PWHISTORY;
    cur_filterentry->mtype = PWSMatch::MT_DATE;
    cur_filterentry->ftype = HT_CHANGEDATE;
  }

  else if (XMLString::equals(qname, _A2X("history_passwords"))) {
    m_type = DFTYPE_PWHISTORY;
    cur_filterentry->mtype = PWSMatch::MT_PASSWORD;
    cur_filterentry->ftype = HT_PASSWORDS;
  }

  else if (XMLString::equals(qname, _A2X("password_policy"))) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_POLICY;
    cur_filterentry->ftype = FT_POLICY;
  }

  else if (XMLString::equals(qname, _A2X("policy_present"))) {
    m_type = DFTYPE_PWPOLICY;
    cur_filterentry->mtype = PWSMatch::MT_BOOL;
    cur_filterentry->ftype = PT_PRESENT;
  }

  else if (XMLString::equals(qname, _A2X("policy_length"))) {
    m_type = DFTYPE_PWPOLICY;
    cur_filterentry->mtype = PWSMatch::MT_INTEGER;
    cur_filterentry->ftype = PT_LENGTH;
  }

  else if (XMLString::equals(qname, _A2X("policy_number_lowercase"))) {
    m_type = DFTYPE_PWPOLICY;
    cur_filterentry->mtype = PWSMatch::MT_INTEGER;
    cur_filterentry->ftype = PT_LOWERCASE;
  }

  else if (XMLString::equals(qname, _A2X("policy_number_uppercase"))) {
    m_type = DFTYPE_PWPOLICY;
    cur_filterentry->mtype = PWSMatch::MT_INTEGER;
    cur_filterentry->ftype = PT_UPPERCASE;
  }

  else if (XMLString::equals(qname, _A2X("policy_number_digits"))) {
    m_type = DFTYPE_PWPOLICY;
    cur_filterentry->mtype = PWSMatch::MT_INTEGER;
    cur_filterentry->ftype = PT_DIGITS;
  }

  else if (XMLString::equals(qname, _A2X("policy_number_symbols"))) {
    m_type = DFTYPE_PWPOLICY;
    cur_filterentry->mtype = PWSMatch::MT_INTEGER;
    cur_filterentry->ftype = PT_SYMBOLS;
  }

  else if (XMLString::equals(qname, _A2X("policy_easyvision"))) {
    m_type = DFTYPE_PWPOLICY;
    cur_filterentry->mtype = PWSMatch::MT_BOOL;
    cur_filterentry->ftype = PT_EASYVISION;
  }

  else if (XMLString::equals(qname, _A2X("policy_pronounceable"))) {
    m_type = DFTYPE_PWPOLICY;
    cur_filterentry->mtype = PWSMatch::MT_BOOL;
    cur_filterentry->ftype = PT_PRONOUNCEABLE;
  }

  else if (XMLString::equals(qname, _A2X("policy_hexadecimal"))) {
    m_type = DFTYPE_PWPOLICY;
    cur_filterentry->mtype = PWSMatch::MT_BOOL;
    cur_filterentry->ftype = PT_HEXADECIMAL;
  }
  
  else if (XMLString::equals(qname, _A2X("attachment"))) {
    m_type = DFTYPE_MAIN;
    cur_filterentry->mtype = PWSMatch::MT_ATTACHMENT;
    cur_filterentry->ftype = FT_ATTACHMENT;
  }
  
  else if (XMLString::equals(qname, _A2X("attachment_present"))) {
    m_type = DFTYPE_ATTACHMENT;
    cur_filterentry->mtype = PWSMatch::MT_BOOL;
    cur_filterentry->ftype = AT_PRESENT;
  }

  else if (XMLString::equals(qname, _A2X("attachment_title"))) {
    m_type = DFTYPE_ATTACHMENT;
    cur_filterentry->mtype = PWSMatch::MT_STRING;
    cur_filterentry->ftype = AT_TITLE;
  }

  else if (XMLString::equals(qname, _A2X("attachment_mediatype"))) {
    m_type = DFTYPE_ATTACHMENT;
    cur_filterentry->mtype = PWSMatch::MT_STRING;
    cur_filterentry->ftype = AT_MEDIATYPE;
  }

  else if (XMLString::equals(qname, _A2X("attachment_filename"))) {
    m_type = DFTYPE_ATTACHMENT;
    cur_filterentry->mtype = PWSMatch::MT_STRING;
    cur_filterentry->ftype = AT_FILENAME;
  }

  else if (XMLString::equals(qname, _A2X("attachment_filepath"))) {
    m_type = DFTYPE_ATTACHMENT;
    cur_filterentry->mtype = PWSMatch::MT_STRING;
    cur_filterentry->ftype = AT_FILEPATH;
  }

  else if (XMLString::equals(qname, _A2X("attachment_ctime"))) {
    m_type = DFTYPE_ATTACHMENT;
    cur_filterentry->mtype = PWSMatch::MT_DATE;
    cur_filterentry->ftype = AT_CTIME;
  }

  else if (XMLString::equals(qname, _A2X("attachment_filectime"))) {
    m_type = DFTYPE_ATTACHMENT;
    cur_filterentry->mtype = PWSMatch::MT_DATE;
    cur_filterentry->ftype = AT_FILECTIME;
  }

  else if (XMLString::equals(qname, _A2X("attachment_filemtime"))) {
    m_type = DFTYPE_ATTACHMENT;
    cur_filterentry->mtype = PWSMatch::MT_DATE;
    cur_filterentry->ftype = AT_FILEMTIME;
  }

  else if (XMLString::equals(qname, _A2X("attachment_fileatime"))) {
    m_type = DFTYPE_ATTACHMENT;
    cur_filterentry->mtype = PWSMatch::MT_DATE;
    cur_filterentry->ftype = AT_FILEATIME;
  }

  else if (XMLString::equals(qname, _A2X("rule"))) {
    ToUpper(m_sxElemContent);
    cur_filterentry->rule = PWSMatch::GetRule(m_sxElemContent.c_str());
  }

  else if (XMLString::equals(qname, _A2X("logic"))) {
    if (m_sxElemContent == _T("or"))
      cur_filterentry->ltype = LC_OR;
    else
      cur_filterentry->ltype = LC_AND;
  }

  else if (XMLString::equals(qname, _A2X("string"))) {
    cur_filterentry->fstring = m_sxElemContent;
  }

  else if (XMLString::equals(qname, _A2X("case"))) {
    cur_filterentry->fcase = _ttoi(m_sxElemContent.c_str()) != 0;
  }

  else if (XMLString::equals(qname, _A2X("warn"))) {
    cur_filterentry->fnum1 = _ttoi(m_sxElemContent.c_str());
  }

  else if (XMLString::equals(qname, _A2X("num1"))) {
    cur_filterentry->fnum1 = _ttoi(m_sxElemContent.c_str());
  }

  else if (XMLString::equals(qname, _A2X("num2"))) {
    cur_filterentry->fnum2 = _ttoi(m_sxElemContent.c_str());
  }

  else if (XMLString::equals(qname, _A2X("unit"))) {
    cur_filterentry->funit = _ttoi(m_sxElemContent.c_str());
  }

  else if (XMLString::equals(qname, _A2X("date1"))) {
    time_t t(0);
    if (VerifyXMLDateString(m_sxElemContent.c_str(), t) &&
        (t != time_t(-1)))
      cur_filterentry->fdate1 = t;
    else
    cur_filterentry->fdate1 = time_t(0);
  }

  else if (XMLString::equals(qname, _A2X("date2"))) {
    time_t t(0);
    if (VerifyXMLDateString(m_sxElemContent.c_str(), t) &&
        (t != time_t(-1)))
      cur_filterentry->fdate2 = t;
    else
      cur_filterentry->fdate1 = time_t(0);
  }

  else if (XMLString::equals(qname, _A2X("DCA"))) {
    cur_filterentry->fdca = static_cast<short>(_ttoi(m_sxElemContent.c_str()));
  }

  else if (XMLString::equals(qname, _A2X("type"))) {
    if (m_sxElemContent == _T("normal"))
      cur_filterentry->etype = CItemData::ET_NORMAL;
    else if (m_sxElemContent == _T("alias"))
      cur_filterentry->etype = CItemData::ET_ALIAS;
    else if (m_sxElemContent == _T("shortcut"))
      cur_filterentry->etype = CItemData::ET_SHORTCUT;
    else if (m_sxElemContent == _T("aliasbase"))
      cur_filterentry->etype = CItemData::ET_ALIASBASE;
    else if (m_sxElemContent == _T("shortcutbase"))
      cur_filterentry->etype = CItemData::ET_SHORTCUTBASE;
    else
      cur_filterentry->etype = CItemData::ET_INVALID;
  }

  else if (XMLString::equals(qname, _A2X("status"))) {
    if (m_sxElemContent == _T("clean"))
      cur_filterentry->estatus = CItemData::ES_CLEAN;
    else if (m_sxElemContent == _T("added"))
      cur_filterentry->estatus = CItemData::ES_ADDED;
    else if (m_sxElemContent == _T("modified"))
      cur_filterentry->estatus = CItemData::ES_MODIFIED;
    else
      cur_filterentry->estatus = CItemData::ES_INVALID;
  } else if (!(XMLString::equals(qname, _A2X("test")) ||
               XMLString::equals(qname, _A2X("filters")))) {
      ASSERT(0);
  }

  return;
}

void XFilterSAX2Handlers::FormatError(const SAXParseException& e, const int type)
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
  m_strValidationResult += FormatString + L'\n';
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

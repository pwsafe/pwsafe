/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/*
* This routine processes Filter XML using the STANDARD and UNMODIFIED
* Expat library V2.0.1 released on June 5, 2007
*
* See http://expat.sourceforge.net/
*
* Note: This is a cross-platform library and can be linked in as a
* Static library or used as a dynamic library e.g. DLL in Windows.
*/

#include "../XMLDefs.h"

#if USE_XML_LIBRARY == EXPAT

// PWS includes
#include "EFilterHandlers.h"
#include "EFilterValidator.h"

#include "../../util.h"
#include "../../UUIDGen.h"
#include "../../corelib.h"
#include "../../PWSFilters.h"
#include "../../VerifyFormat.h"
#include "../../Match.h"

#include <map>
#include <algorithm>

// Expat includes
#include <expat.h>

using namespace std;

EFilterHandlers::EFilterHandlers()
{
  m_pValidator = new EFilterValidator;
  m_strElemContent.clear();
  m_iXMLVersion = -1;
  m_bErrors = false;
  m_pAsker = NULL;
}

EFilterHandlers::~EFilterHandlers()
{
  delete m_pValidator;
}

void XMLCALL EFilterHandlers::startElement(void *userdata, const XML_Char *name,
                                           const XML_Char **attrs)
{
  m_strElemContent = _T("");

  if (m_bValidation) {
    stringT element_name(name);
    if (!m_pValidator->startElement(element_name)) {
      m_bErrors = true;
      m_iErrorCode = m_pValidator->getErrorCode();
      m_strErrorMessage = m_pValidator->getErrorMsg();
      goto start_errors;
    }

    if (_tcscmp(name, _T("filters")) == 0) {
      bool bversion(false);
      for (int i = 0; attrs[i]; i += 2) {
        if (_tcscmp(attrs[i], _T("version")) == 0) {
          m_iXMLVersion = _ttoi(attrs[i + 1]);
          bversion = true;
        break;
        }
      }
      if (!bversion) {
        // error - it is required!
        m_iErrorCode = XTPEC_FILTERS_VERSION_MISSING;
        XML_StopParser((XML_Parser)userdata, XML_FALSE);
      }
      return;
    }

    if (_tcscmp(name, _T("filter")) == 0) {
      bool bfiltername(false);
      stringT filtername;
      for (int i = 0; attrs[i]; i += 2) {
        if (_tcscmp(attrs[i], _T("filtername")) == 0) {
          filtername = attrs[i + 1];
          bfiltername = true;
        }
      }
      if (!bfiltername) {
        // error - it is required!
        m_iErrorCode = XTPEC_FILTERNAME_MISSING;
        XML_StopParser((XML_Parser)userdata, XML_FALSE);
      } else {
        pair<set<const stringT>::iterator, bool> ret;
        ret = m_unique_filternames.insert(filtername);
        if (ret.second == false) {
          // error - not unique
          m_iErrorCode = XTPEC_NON_UNIQUE_FILTER_NAME;
          XML_StopParser((XML_Parser)userdata, XML_FALSE);
        }
      }
    }
    if (_tcscmp(name, _T("filter_entry")) == 0) {
      for (int i = 0; attrs[i]; i += 2) {
        if (_tcscmp(attrs[i], _T("active")) == 0) {
          if (_tcscmp(attrs[i + 1], _T("no")) == 0)
          break;
        }
      }
    }
    return;
  }

  if (_tcscmp(name, _T("filters")) == 0) {
    m_unique_filternames.clear();
    m_strErrorMessage.clear();
    m_bentrybeingprocessed = false;
  }

  else if (_tcscmp(name, _T("filter")) == 0) {
    cur_filter = new st_filters;
    // Process the attributes we need.
    for (int i = 0; attrs[i]; i += 2) {
      if (_tcscmp(attrs[i], _T("filtername")) == 0) {
        cur_filter->fname = stringT(attrs[i + 1]);
      }
    }
  }

  else if (_tcscmp(name, _T("filter_entry")) == 0) {
    cur_filterentry = new st_FilterRow;
    cur_filterentry->Empty();
    cur_filterentry->bFilterActive = true;
    m_bentrybeingprocessed = true;
    // Process the attributes we need.
    for (int i = 0; attrs[i]; i += 2) {
      if (_tcscmp(attrs[i], _T("active")) == 0) {
        if (_tcscmp(attrs[i + 1], _T("no")) == 0)
          cur_filterentry->bFilterActive = false;
      }
    }
  }

  return;

start_errors:
  // Non validating parser, so we have to tidy up now
  XML_StopParser((XML_Parser)userdata, XML_FALSE);
}

void XMLCALL EFilterHandlers::characterData(void * /* userdata */, const XML_Char *s,
                                            int length)
{
  XML_Char *xmlchData = new XML_Char[length + 1];
#if _MSC_VER >= 1400
  _tcsncpy_s(xmlchData, length + 1, s, length);
#else
  _tcsncpy(xmlchData, s, length);
#endif
  xmlchData[length] = TCHAR('\0');
  m_strElemContent += StringX(xmlchData);
  delete [] xmlchData;
}

void XMLCALL EFilterHandlers::endElement(void * userdata, const XML_Char *name)
{
  if (m_bValidation) {
    stringT element_name(name);
    if (!m_pValidator->endElement(element_name, m_strElemContent)) {
      m_bErrors = true;
      m_iErrorCode = m_pValidator->getErrorCode();
      m_strErrorMessage = m_pValidator->getErrorMsg();
      goto end_errors;
    }
  }

  if (m_bValidation && _tcscmp(name, _T("filters")) == 0) {
    // Check that the XML file version is present and that
    // a. it is less than or equal to the Filter schema version
    // b. it is less than or equal to the version supported by this PWS
    if (m_iXMLVersion < 0) {
      LoadAString(m_strErrorMessage, IDSC_MISSING_XML_VER);
      return ;
    }
    if (m_iXMLVersion > PWS_XML_FILTER_VERSION) {
      Format(m_strErrorMessage,
             IDSC_INVALID_XML_VER2, m_iXMLVersion, PWS_XML_FILTER_VERSION);
      return;
    }
  }

  if (m_bValidation) {
    return;
  }

  st_filter_element_data edata;

  if (_tcscmp(name, _T("filters")) == 0) {
    return;
  }

  else if (_tcscmp(name, _T("filter")) == 0) {
    INT_PTR rc = IDYES;
    st_Filterkey fk;
    fk.fpool = m_FPool;
    fk.cs_filtername = cur_filter->fname;
    if (m_MapFilters->find(fk) != m_MapFilters->end()) {
      stringT question;
      Format(question, IDSC_FILTEREXISTS, cur_filter->fname.c_str());
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

  else if (_tcscmp(name, _T("filter_entry")) == 0) {
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

  else if (m_pValidator->GetElementInfo(name, edata)) {
    m_type = edata.filter_type;
    cur_filterentry->mtype = edata.mt;
    cur_filterentry->ftype = (FieldType)edata.ft;
  }

  else {
    time_t t(0);
    switch(edata.element_code) {
      case XTE_RULE:
        cur_filterentry->rule = m_pValidator->GetMatchRule(m_strElemContent.c_str());
        break;
      case XTE_LOGIC:
        if (m_strElemContent == _T("or"))
          cur_filterentry->ltype = LC_OR;
        else
          cur_filterentry->ltype = LC_AND;
        break;
      case XTE_STRING:
        cur_filterentry->fstring = m_strElemContent;
        break;
      case XTE_CASE:
        cur_filterentry->fcase = _ttoi(m_strElemContent.c_str()) != 0;
        break;
      case XTE_WARN:
      case XTE_NUM1:
        cur_filterentry->fnum1 = _ttoi(m_strElemContent.c_str());
        break;
      case XTE_NUM2:
        cur_filterentry->fnum2 = _ttoi(m_strElemContent.c_str());
        break;
      case XTE_DATE1:
        if (VerifyXMLDateString(m_strElemContent.c_str(), t) &&
            (t != (time_t)-1))
          cur_filterentry->fdate1 = t;
        else
          cur_filterentry->fdate1 = (time_t)0;
        break;
      case XTE_DATE2:
        if (VerifyXMLDateString(m_strElemContent.c_str(), t) &&
            (t != (time_t)-1))
          cur_filterentry->fdate2 = t;
        else
          cur_filterentry->fdate2 = (time_t)0;
        break;
      case XTE_TYPE:
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
        break;
      case XTE_TEST:
        break;
      default:
        ASSERT(0);
        goto end_errors;
    }
  }

  return;

end_errors:
  // Non validating parser, so we have to tidy up now
  XML_StopParser((XML_Parser)userdata, XML_FALSE);
}

#endif /* USE_XML_LIBRARY == EXPAT */

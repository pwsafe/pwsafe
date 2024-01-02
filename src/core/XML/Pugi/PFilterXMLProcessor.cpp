/*
* Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/*
* This routine processes Filter XML using the PUGI
*
* See http://pugixml.org/
*
* Note: An actual version of pugixml library is linked to password safe
* in parallel folder ../../pugixml
*
*/

#if !defined(_WIN32) || defined(__WX__)
// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif
#endif // !defined(_WIN32) || defined(__WX__)
#ifdef _WIN32
#define _(x) _T(x)
#endif // _WIN32

#include "../XMLDefs.h"    // Required if testing "USE_XML_LIBRARY"

#if !defined(USE_XML_LIBRARY) || (!defined(_WIN32) && USE_XML_LIBRARY == MSXML)

// PWS includes
#include "PFilterXMLProcessor.h"

#include "../../StringX.h"
#include "../../core.h"
#include "../../VerifyFormat.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <map>
#include <algorithm>
#include <stdexcept>

#include "os/pws_tchar.h"  // For Linux build not finding _tcslen!

/*!
 * Check pointer before calling compare
 */
static bool SafeCompare(const TCHAR *v1, const TCHAR *v2)
{
  return (v1 != nullptr && v2 != nullptr && stringT(v1) == v2);
}

/*!
 * Constructor. Reporter is only needed when following the call sub-functions no information to the user is given
 */

PFilterXMLProcessor::PFilterXMLProcessor(PWSFilters &mapfilters, const FilterPool fpool,
                                         Asker *pAsker, Reporter *pReporter)
  : m_pAsker(pAsker), m_pReporter(pReporter), m_MapXMLFilters(mapfilters), m_FPool(fpool)
{
  m_strXMLErrors.clear();
}

PFilterXMLProcessor::~PFilterXMLProcessor()
{
}

/*!
 * Build internal tree with content from buffer of file. If File name is given this one is processed.
 */

bool PFilterXMLProcessor::ReadXML(const StringX &strXMLData,
                                  const stringT &strXMLFileName)
{
  pugi::xml_parse_result result;
    
  if(strXMLFileName.empty())
    result = m_doc.load(strXMLData.c_str());
  else
    result = m_doc.load_file(strXMLFileName.c_str());
    
  if (!result) {
    // An XML load error occurred so display the reason
    // Note: "result.description()" returns char* even in Unicode builds.
    stringT sErrorDesc;
    sErrorDesc = pugi::as_wide(result.description());
    Format(m_strXMLErrors, _("XML error:\n%ls\n%ls\noffset approximately at %d"),
           sErrorDesc.c_str(), strXMLFileName.c_str(), result.offset);
    if(m_pReporter)
      (*m_pReporter)(m_strXMLErrors);
    return false;
  } // load failed
    
  return true;
}

/*!
 * Process the data structure on filter. In case of validation no external buffer is changed.
 */

bool PFilterXMLProcessor::Process(const bool &bValidation)
{
  pugi::xml_node Root = m_doc.first_child();
    
  m_bValidation = bValidation;

  if (!Root || !SafeCompare(Root.name(), _T("filters"))) {
    Format(m_strXMLErrors, _("Error in filter XML structure: excpected \"%ls\", found \"%ls\""),
           _T("filters"), Root.name());
    if(m_pReporter)
      (*m_pReporter)(m_strXMLErrors);
    return false;
  }

  int itemp = Root.attribute(_T("version")).as_int(-1);
  if (itemp == -1) {
    Format(m_strXMLErrors, IDSC_MISSING_XML_VER);
    if(m_pReporter)
      (*m_pReporter)(m_strXMLErrors);
    return false;
  }
  if(itemp > PWS_XML_FILTER_VERSION) {
    Format(m_strXMLErrors, IDSC_INVALID_XML_VER1,
           itemp, PWS_XML_FILTER_VERSION);
    if(m_pReporter)
      (*m_pReporter)(m_strXMLErrors);
    return false;
  }
    
  st_filters filter;

  // Iterate over all filter in the buffer/file
  for (pugi::xml_node_iterator it = Root.begin(); it != Root.end(); ++it) {
    if (SafeCompare(it->name(), _T("filter"))) {
      const TCHAR *filterName = it->attribute(_T("filtername")).value();
      if (_tcslen(filterName) == 0) {
        Format(m_strXMLErrors, _("Missing filtername"));
        if(m_pReporter)
          (*m_pReporter)(m_strXMLErrors);
        return false;
      }
      const stringT fname(filterName);
      filter.Empty();
      filter.fname = fname;
      // Handle single filter
      if(! ReadXMLFilter(*it, fname, filter)) {
        // Give Report on filter error when reporter is given
        if(m_pReporter)
          (*m_pReporter)(m_strXMLErrors);
        return false;
      }

    }
    else {
      Format(m_strXMLErrors, _("Unexpected XML tag \"%ls\", expected tag is \"%ls\""),
             it->name(), _T("filter"));
      if(m_pReporter)
        (*m_pReporter)(m_strXMLErrors);
      return false;
    }
  }

  return true;
}

/*!
 * Read single filter from buffer included in <filter filtername="fname"> to </filter>
 * Each filter row is lead in by <filter_entry active=yes|no>
 */

bool PFilterXMLProcessor::ReadXMLFilter(pugi::xml_node &froot, const stringT &fname, st_filters &cur_filter)
{
  int idx = 1;
  FilterType ftype = DFTYPE_INVALID;
  st_FilterRow frow;
    
  for (pugi::xml_node_iterator it = froot.begin(); it != froot.end(); ++it, ++idx) {
    frow.Empty();
    if (SafeCompare(it->name(), _T("filter_entry"))) {
      const TCHAR *isActive = it->attribute(_T("active")).value();
      if (_tcslen(isActive) == 0) {
        Format(m_strXMLErrors, _("Missing active attribute in filter entry %d of filter \'%ls\'"), idx, fname.c_str());
        return false;
      }
      if(SafeCompare(isActive, _T("yes"))) {
        frow.bFilterActive = true;
      }
      else if(SafeCompare(isActive, _T("no"))) {
        frow.bFilterActive = false;
      }
      else {
        Format(m_strXMLErrors, _("Unexpected attribute value \"%ls\" (expected \"yes\" or \"no\") in filter entry %d of \'%ls\'"), idx, fname.c_str());
        return false;
      }
      pugi::xml_node node = *it;
      pugi::xml_node child = node.first_child();
      // The filter entry is child of <filter_entry>
      if(! ReadXMLFilterEntry(child, fname, idx, frow, ftype)) {
        return false;
      }
      // On validation, stop here
      if(m_bValidation) continue;
        
      if(frow.mtype  == PWSMatch::MT_DATE &&
         frow.rule   != PWSMatch::MR_PRESENT &&
         frow.rule   != PWSMatch::MR_NOTPRESENT &&
         frow.fdate1 == time_t(0) &&
         frow.fdate2 == time_t(0))
        frow.fdatetype = 1; // Relative Date
    
      // Add the filter into related vector
      if (ftype == DFTYPE_MAIN) {
        cur_filter.num_Mactive++;
        cur_filter.vMfldata.push_back(frow);
      } else if (ftype == DFTYPE_PWHISTORY) {
        cur_filter.num_Hactive++;
        cur_filter.vHfldata.push_back(frow);
      } else if (ftype == DFTYPE_PWPOLICY) {
        cur_filter.num_Pactive++;
        cur_filter.vPfldata.push_back(frow);
      } else if (ftype == DFTYPE_ATTACHMENT) {
        cur_filter.num_Aactive++;
        cur_filter.vAfldata.push_back(frow);
      } else {
        ASSERT(false);
      }
    }
    else {
      Format(m_strXMLErrors, _("Unexpected XML tag \"%ls\", expected tag is \"%ls\""),
             it->name(), _T("filter_entry"));
      return false;
    }
  }
    
  // Check on filter completion after all filter row read
  std::vector<st_FilterRow>::iterator iter;
  for(iter = cur_filter.vMfldata.begin(); iter != cur_filter.vMfldata.end(); ++iter) {
    st_FilterRow &fldata = *iter;
    if((fldata.ltype != LC_INVALID) &&
       (fldata.ftype != FT_INVALID) &&
       ((fldata.mtype != PWSMatch::MT_INVALID && fldata.rule != PWSMatch::MR_INVALID) ||
        (fldata.ftype == FT_PWHIST && cur_filter.vHfldata.size() > 0) ||
        (fldata.ftype == FT_POLICY && cur_filter.vPfldata.size() > 0) ||
        (fldata.ftype == FT_ATTACHMENT && cur_filter.vAfldata.size() > 0))) {
      fldata.bFilterComplete = true;
    }
    // We could give a warning on not completed filter
  }
    
  for(iter = cur_filter.vHfldata.begin(); iter != cur_filter.vHfldata.end(); ++iter) {
    st_FilterRow &fldata = *iter;
    if((fldata.ltype != LC_INVALID) &&
       (fldata.ftype != FT_INVALID) &&
       (fldata.mtype != PWSMatch::MT_INVALID) &&
       (fldata.rule != PWSMatch::MR_INVALID)) {
      fldata.bFilterComplete = true;
    }
    // We could give a warning on not completed filter
  }
    
  for(iter = cur_filter.vPfldata.begin(); iter != cur_filter.vPfldata.end(); ++iter) {
    st_FilterRow &fldata = *iter;
    if((fldata.ltype != LC_INVALID) &&
       (fldata.ftype != FT_INVALID) &&
       (fldata.mtype != PWSMatch::MT_INVALID) &&
       (fldata.rule != PWSMatch::MR_INVALID)) {
      fldata.bFilterComplete = true;
    }
    // We could give a warning on not completed filter
  }
    
  for(iter = cur_filter.vAfldata.begin(); iter != cur_filter.vAfldata.end(); ++iter) {
    st_FilterRow &fldata = *iter;
    if((fldata.ltype != LC_INVALID) &&
       (fldata.ftype != FT_INVALID) &&
       (fldata.mtype != PWSMatch::MT_INVALID) &&
       (fldata.rule != PWSMatch::MR_INVALID)) {
      fldata.bFilterComplete = true;
    }
    // We could give a warning on not completed filter
  }
    
  // If not validating add the new filter to list of filter
  if(! m_bValidation) {
    bool bAddFilter(true);
    st_Filterkey fk;
    fk.fpool = m_FPool;
    fk.cs_filtername = cur_filter.fname;
    // On double entry ask for overwriting or ignore
    if (m_MapXMLFilters.find(fk) != m_MapXMLFilters.end()) {
      stringT question;
      Format(question, IDSC_FILTEREXISTS, cur_filter.fname.c_str());
      // Only on asker is hand over we might ask, default is overwrite
      if (m_pAsker == nullptr || (bAddFilter = (*m_pAsker)(question)) == true) {
        m_MapXMLFilters.erase(fk);
      }
    }
    if (bAddFilter) {
       m_MapXMLFilters.insert(PWSFilters::Pair(fk, cur_filter));
    }
  }
  return true;
}

/*!
 * Read ony filter entry, where the XML tag gives the field type, group and match type
 */

bool PFilterXMLProcessor::ReadXMLFilterEntry(pugi::xml_node &eroot, const stringT &fname, int idx, st_FilterRow &frow, FilterType &ftype)
{
  const TCHAR *qname = eroot.name();
  if (_tcslen(qname) == 0) {
    Format(m_strXMLErrors, _("Missing XML tag for field type in filter entry %d of \'%ls\'"),
           idx, fname.c_str());
    return false;
  }
    
  if(SafeCompare(qname, _T("grouptitle"))) {
    ftype = DFTYPE_MAIN;
    frow.mtype = PWSMatch::MT_STRING;
    frow.ftype = FT_GROUPTITLE;
  }
  else if (SafeCompare(qname, _T("group"))) {
    ftype = DFTYPE_MAIN;
    frow.mtype = PWSMatch::MT_STRING;
    frow.ftype = FT_GROUP;
  }
  else if (SafeCompare(qname, _T("title"))) {
    ftype = DFTYPE_MAIN;
    frow.mtype = PWSMatch::MT_STRING;
    frow.ftype = FT_TITLE;
  }
  else if (SafeCompare(qname, _T("user"))) {
    ftype = DFTYPE_MAIN;
    frow.mtype = PWSMatch::MT_STRING;
    frow.ftype = FT_USER;
  }
  else if (SafeCompare(qname, _T("password"))) {
    ftype = DFTYPE_MAIN;
    frow.mtype = PWSMatch::MT_PASSWORD;
    frow.ftype = FT_PASSWORD;
  }
  else if (SafeCompare(qname, _T("notes"))) {
    ftype = DFTYPE_MAIN;
    frow.mtype = PWSMatch::MT_STRING;
    frow.ftype = FT_NOTES;
  }
  else if (SafeCompare(qname, _T("autotype"))) {
    ftype = DFTYPE_MAIN;
    frow.mtype = PWSMatch::MT_STRING;
    frow.ftype = FT_AUTOTYPE;
  }
  else if (SafeCompare(qname, _T("url"))) {
    ftype = DFTYPE_MAIN;
    frow.mtype = PWSMatch::MT_STRING;
    frow.ftype = FT_URL;
  }
  else if (SafeCompare(qname, _T("runcommand"))) {
    ftype = DFTYPE_MAIN;
    frow.mtype = PWSMatch::MT_STRING;
    frow.ftype = FT_RUNCMD;
  }
  else if (SafeCompare(qname, _T("dca")) || SafeCompare(qname, _T("DCA"))) {
    ftype = DFTYPE_MAIN;
    frow.mtype = PWSMatch::MT_DCA;
    frow.ftype = FT_DCA;
  }
  else if (SafeCompare(qname, _T("shiftdca")) || SafeCompare(qname, _T("ShiftDCA"))) {
    ftype = DFTYPE_MAIN;
    frow.mtype = PWSMatch::MT_SHIFTDCA;
    frow.ftype = FT_SHIFTDCA;
  }
  else if (SafeCompare(qname, _T("email"))) {
    ftype = DFTYPE_MAIN;
    frow.mtype = PWSMatch::MT_STRING;
    frow.ftype = FT_EMAIL;
  }
  else if (SafeCompare(qname, _T("protected"))) {
    ftype = DFTYPE_MAIN;
    frow.mtype = PWSMatch::MT_BOOL;
    frow.ftype = FT_PROTECTED;
  }
  else if (SafeCompare(qname, _T("kbshortcut"))) {
    ftype = DFTYPE_MAIN;
    frow.mtype = PWSMatch::MT_BOOL;
    frow.ftype = FT_KBSHORTCUT;
  }
  else if (SafeCompare(qname, _T("symbols"))) {
    ftype = DFTYPE_MAIN;
    frow.mtype = PWSMatch::MT_STRING;
    frow.ftype = FT_SYMBOLS;
  }
  else if (SafeCompare(qname, _T("policyname")) || SafeCompare(qname, _T("policy_name"))) {
    ftype = DFTYPE_MAIN;
    frow.mtype = PWSMatch::MT_STRING;
    frow.ftype = FT_POLICYNAME;
  }
  else if (SafeCompare(qname, _T("create_time"))) {
    ftype = DFTYPE_MAIN;
    frow.mtype = PWSMatch::MT_DATE;
    frow.ftype = FT_CTIME;
  }
  else if (SafeCompare(qname, _T("password_modified_time"))) {
    ftype = DFTYPE_MAIN;
    frow.mtype = PWSMatch::MT_DATE;
    frow.ftype = FT_PMTIME;
  }
  else if (SafeCompare(qname, _T("last_access_time"))) {
    ftype = DFTYPE_MAIN;
    frow.mtype = PWSMatch::MT_DATE;
    frow.ftype = FT_ATIME;
  }
  else if (SafeCompare(qname, _T("expiry_time"))) {
    ftype = DFTYPE_MAIN;
    frow.mtype = PWSMatch::MT_DATE;
    frow.ftype = FT_XTIME;
  }
  else if (SafeCompare(qname, _T("record_modified_time"))) {
    ftype = DFTYPE_MAIN;
    frow.mtype = PWSMatch::MT_DATE;
    frow.ftype = FT_RMTIME;
  }
  else if (SafeCompare(qname, _T("password_expiry_interval"))) {
    ftype = DFTYPE_MAIN;
    frow.mtype = PWSMatch::MT_INTEGER;
    frow.ftype = FT_XTIME_INT;
  }
  else if (SafeCompare(qname, _T("password_length"))) {
    ftype = DFTYPE_MAIN;
    frow.mtype = PWSMatch::MT_INTEGER;
    frow.ftype = FT_PASSWORDLEN;
  }
  else if (SafeCompare(qname, _T("entrysize"))) {
    ftype = DFTYPE_MAIN;
    frow.mtype = PWSMatch::MT_ENTRYSIZE;
    frow.ftype = FT_ENTRYSIZE;
  }
  else if (SafeCompare(qname, _T("entrytype"))) {
    ftype = DFTYPE_MAIN;
    frow.mtype = PWSMatch::MT_ENTRYTYPE;
    frow.ftype = FT_ENTRYTYPE;
  }
  else if (SafeCompare(qname, _T("entrystatus"))) {
    ftype = DFTYPE_MAIN;
    frow.mtype = PWSMatch::MT_ENTRYSTATUS;
    frow.ftype = FT_ENTRYSTATUS;
  }
  else if (SafeCompare(qname, _T("unknownfields"))) {
    ftype = DFTYPE_MAIN;
    frow.mtype = PWSMatch::MT_BOOL;
    frow.ftype = FT_UNKNOWNFIELDS;
  }
  else if (SafeCompare(qname, _T("password_history"))) {
    ftype = DFTYPE_MAIN;
    frow.mtype = PWSMatch::MT_PWHIST;
    frow.ftype = FT_PWHIST;
  }
  else if (SafeCompare(qname, _T("history_present"))) {
    ftype = DFTYPE_PWHISTORY;
    frow.mtype = PWSMatch::MT_BOOL;
    frow.ftype = HT_PRESENT;
  }
  else if (SafeCompare(qname, _T("history_active"))) {
    ftype = DFTYPE_PWHISTORY;
    frow.mtype = PWSMatch::MT_BOOL;
    frow.ftype = HT_ACTIVE;
  }
  else if (SafeCompare(qname, _T("history_number"))) {
    ftype = DFTYPE_PWHISTORY;
    frow.mtype = PWSMatch::MT_INTEGER;
    frow.ftype = HT_NUM;
  }
  else if (SafeCompare(qname, _T("history_maximum"))) {
    ftype = DFTYPE_PWHISTORY;
    frow.mtype = PWSMatch::MT_INTEGER;
    frow.ftype = HT_MAX;
  }
  else if (SafeCompare(qname, _T("history_changedate"))) {
    ftype = DFTYPE_PWHISTORY;
    frow.mtype = PWSMatch::MT_DATE;
    frow.ftype = HT_CHANGEDATE;
  }
  else if (SafeCompare(qname, _T("history_passwords"))) {
    ftype = DFTYPE_PWHISTORY;
    frow.mtype = PWSMatch::MT_PASSWORD;
    frow.ftype = HT_PASSWORDS;
  }
  else if (SafeCompare(qname, _T("password_policy"))) {
    ftype = DFTYPE_MAIN;
    frow.mtype = PWSMatch::MT_POLICY;
    frow.ftype = FT_POLICY;
  }
  else if (SafeCompare(qname, _T("policy_present"))) {
    ftype = DFTYPE_PWPOLICY;
    frow.mtype = PWSMatch::MT_BOOL;
    frow.ftype = PT_PRESENT;
  }
  else if (SafeCompare(qname, _T("policy_length"))) {
    ftype = DFTYPE_PWPOLICY;
    frow.mtype = PWSMatch::MT_INTEGER;
    frow.ftype = PT_LENGTH;
  }
  else if (SafeCompare(qname, _T("policy_number_lowercase"))) {
    ftype = DFTYPE_PWPOLICY;
    frow.mtype = PWSMatch::MT_INTEGER;
    frow.ftype = PT_LOWERCASE;
  }
  else if (SafeCompare(qname, _T("policy_number_uppercase"))) {
    ftype = DFTYPE_PWPOLICY;
    frow.mtype = PWSMatch::MT_INTEGER;
    frow.ftype = PT_UPPERCASE;
  }
  else if (SafeCompare(qname, _T("policy_number_digits"))) {
    ftype = DFTYPE_PWPOLICY;
    frow.mtype = PWSMatch::MT_INTEGER;
    frow.ftype = PT_DIGITS;
  }
  else if (SafeCompare(qname, _T("policy_number_symbols"))) {
    ftype = DFTYPE_PWPOLICY;
    frow.mtype = PWSMatch::MT_INTEGER;
    frow.ftype = PT_SYMBOLS;
  }
  else if (SafeCompare(qname, _T("policy_easyvision"))) {
    ftype = DFTYPE_PWPOLICY;
    frow.mtype = PWSMatch::MT_BOOL;
    frow.ftype = PT_EASYVISION;
  }
  else if (SafeCompare(qname, _T("policy_pronounceable"))) {
    ftype = DFTYPE_PWPOLICY;
    frow.mtype = PWSMatch::MT_BOOL;
    frow.ftype = PT_PRONOUNCEABLE;
  }
  else if (SafeCompare(qname, _T("policy_hexadecimal"))) {
    ftype = DFTYPE_PWPOLICY;
    frow.mtype = PWSMatch::MT_BOOL;
    frow.ftype = PT_HEXADECIMAL;
  }
  else if (SafeCompare(qname, _T("attachment"))) {
    ftype = DFTYPE_MAIN;
    frow.mtype = PWSMatch::MT_ATTACHMENT;
    frow.ftype = FT_ATTACHMENT;
  }
  else if (SafeCompare(qname, _T("attachment_present"))) {
    ftype = DFTYPE_ATTACHMENT;
    frow.mtype = PWSMatch::MT_BOOL;
    frow.ftype = AT_PRESENT;
  }
  else if (SafeCompare(qname, _T("attachment_title"))) {
    ftype = DFTYPE_ATTACHMENT;
    frow.mtype = PWSMatch::MT_STRING;
    frow.ftype = AT_TITLE;
  }
  else if (SafeCompare(qname, _T("attachment_mediatype"))) {
    ftype = DFTYPE_ATTACHMENT;
    frow.mtype = PWSMatch::MT_STRING;
    frow.ftype = AT_MEDIATYPE;
  }
  else if (SafeCompare(qname, _T("attachment_filename"))) {
    ftype = DFTYPE_ATTACHMENT;
    frow.mtype = PWSMatch::MT_STRING;
    frow.ftype = AT_FILENAME;
  }
  else if (SafeCompare(qname, _T("attachment_filepath"))) {
    ftype = DFTYPE_ATTACHMENT;
    frow.mtype = PWSMatch::MT_STRING;
    frow.ftype = AT_FILEPATH;
  }
  else if (SafeCompare(qname, _T("attachment_ctime"))) {
    ftype = DFTYPE_ATTACHMENT;
    frow.mtype = PWSMatch::MT_DATE;
    frow.ftype = AT_CTIME;
  }
  else if (SafeCompare(qname, _T("attachment_filectime"))) {
    ftype = DFTYPE_ATTACHMENT;
    frow.mtype = PWSMatch::MT_DATE;
    frow.ftype = AT_FILECTIME;
  }
  else if (SafeCompare(qname, _T("attachment_filemtime"))) {
    ftype = DFTYPE_ATTACHMENT;
    frow.mtype = PWSMatch::MT_DATE;
    frow.ftype = AT_FILEMTIME;
  }
  else if (SafeCompare(qname, _T("attachment_fileatime"))) {
    ftype = DFTYPE_ATTACHMENT;
    frow.mtype = PWSMatch::MT_DATE;
    frow.ftype = AT_FILEATIME;
  }
  else {
    Format(m_strXMLErrors, _("Unknown XML tag \"%ls\" for field type in filter entry %d of \'%ls\'"),
           qname, idx, fname.c_str());
    return false;
  }
    
  bool bLogic = false, bRule = false;
  // Each field must include <logic>or|and</logic> and optional <rule>..</rule> (not needed for
  // reference to history, policy or attachement), optional <test></test> include the tested values
  for (pugi::xml_node_iterator it = eroot.begin(); it != eroot.end(); ++it) {
    if (SafeCompare(it->name(), _T("rule"))) {
      pugi::xml_node node = *it;
      if (node != nullptr) {
        const TCHAR *val = node.child_value();
        if (_tcslen(val) == 0) {
          Format(m_strXMLErrors, _("Missing rule in filter entry %d of filter \'%ls\'"), idx, fname.c_str());
          return false;
        }
        frow.rule = PWSMatch::GetRule(val);
        if(frow.rule == PWSMatch::MR_INVALID) {
          Format(m_strXMLErrors, _("Unknown rule \"%ls\" in filter entry %d of filter \'%ls\'"), val, idx, fname.c_str());
          return false;
        }
        bRule = true;
      }
    }
    else if (SafeCompare(it->name(), _T("logic"))) {
      pugi::xml_node node = *it;
      if (node != nullptr) {
        const TCHAR *val = node.child_value();
        if (_tcslen(val) == 0) {
          Format(m_strXMLErrors, _("Missing logic in filter entry %d of filter \'%ls\'"), idx, fname.c_str());
          return false;
        }
        if(SafeCompare(val, _T("or"))) {
          frow.ltype = LC_OR;
        }
        else if(SafeCompare(val, _T("and"))) {
          frow.ltype = LC_AND;
        }
        else {
          Format(m_strXMLErrors, _("Unknown logic \"%ls\" in filter entry %d of filter \'%ls\'"), val, idx, fname.c_str());
          return false;
        }
        bLogic = true;
      }
    }
    else if (SafeCompare(it->name(), _T("test"))) {
        pugi::xml_node node = *it;
        // Fetch values to be compared against
        if(! ReadXMLFilterTest(node, fname, idx, frow)) {
          // Error string is filled inside of called function
          return false;
        }
        if(frow.ftype == FT_SYMBOLS) {
          frow.fstring = PWSUtil::DeDupString(frow.fstring);
        }
    }
    else {
      Format(m_strXMLErrors, _("Unknown XML tag \"%ls\" in field type in filter entry %d of \'%ls\'"),
             it->name(), idx, fname.c_str());
      return false;
    }
  }
    
  // Check on present <logic> and <rule>
  if(!bLogic ||
     (frow.ftype != FT_PWHIST && frow.ftype != FT_POLICY  && frow.ftype != FT_ATTACHMENT && !bRule)) {
    Format(m_strXMLErrors, _("Missing logic or rule XML tag in filter entry %d of \'%ls\'"),
           idx, fname.c_str());
    return false;
  }
    
  return true;
}

/*!
 * Process values between <test> and </test>
 */

bool PFilterXMLProcessor::ReadXMLFilterTest(pugi::xml_node &troot, const stringT &fname, int idx, st_FilterRow &frow)
{
  for (pugi::xml_node_iterator it = troot.begin(); it != troot.end(); ++it, ++idx) {
    pugi::xml_node node = *it;
    const TCHAR *val = node.child_value();
    if (SafeCompare(it->name(), _T("string"))) {
      // The string can be build up by several child values <![CDATA[...]]><![CDATA[...]]>
      for (pugi::xml_node i = node.first_child(); i; i = i.next_sibling()) {
        const TCHAR *value = i.value();
        frow.fstring += value;
      }
    }
    else if (SafeCompare(it->name(), _T("case"))) {
      frow.fcase = SafeCompare(val, _T("1")) ? true : false;
    }
    else if (SafeCompare(it->name(), _T("warn"))) {
      frow.fnum1 = _ttoi(val);
    }
    else if (SafeCompare(it->name(), _T("num1"))) {
      frow.fnum1 = _ttoi(val);
    }
    else if (SafeCompare(it->name(), _T("num2"))) {
      frow.fnum2 = _ttoi(val);
    }
    else if (SafeCompare(it->name(), _T("unit"))) {
      frow.funit = _ttoi(val);
    }
    else if (SafeCompare(it->name(), _T("date1"))) {
      time_t t(0);
      if (VerifyXMLDateString(val, t) && (t != time_t(-1)))
        frow.fdate1 = t;
      else
        frow.fdate1 = time_t(0);
    }
    else if (SafeCompare(it->name(), _T("date2"))) {
      time_t t(0);
      if (VerifyXMLDateString(val, t) && (t != time_t(-1)))
        frow.fdate2 = t;
      else
        frow.fdate2 = time_t(0);
    }
    else if (SafeCompare(it->name(), _T("DCA")) || SafeCompare(it->name(), _T("dca")) || SafeCompare(it->name(), _T("shiftdca"))) {
      frow.fdca = static_cast<short>(_ttoi(val));
    }
    else if (SafeCompare(it->name(), _T("type"))) {
      if(SafeCompare(val, _T("normal"))) {
        frow.etype = CItemData::ET_NORMAL;
      }
      else if(SafeCompare(val, _T("alias"))) {
        frow.etype = CItemData::ET_ALIAS;
      }
      else if(SafeCompare(val, _T("shortcut"))) {
        frow.etype = CItemData::ET_SHORTCUT;
      }
      else if(SafeCompare(val, _T("aliasbase"))) {
        frow.etype = CItemData::ET_ALIASBASE;
      }
      else if(SafeCompare(val, _T("shortcutbase"))) {
        frow.etype = CItemData::ET_SHORTCUTBASE;
      }
      else {
        frow.etype = CItemData::ET_INVALID;
        // We could give a warning on unknown value
      }
    }
    else if (SafeCompare(it->name(), _T("status"))) {
      if(SafeCompare(val, _T("clean"))) {
        frow.estatus = CItemData::ES_CLEAN;
      }
      else if(SafeCompare(val, _T("added"))) {
        frow.estatus = CItemData::ES_ADDED;
      }
      else if(SafeCompare(val, _T("modified"))) {
        frow.estatus = CItemData::ES_MODIFIED;
      }
      else {
        frow.estatus = CItemData::ES_INVALID;
        // We could give a warning on unknown value
      }
    }
    else {
      Format(m_strXMLErrors, _("Unknown XML tag \"%ls\" in test in filter entry %d of \'%ls\'"),
             it->name(), idx, fname.c_str());
      return false;
    }
  }
  return true;
}

#endif /* !defined(USE_XML_LIBRARY) || (!defined(_WIN32) && USE_XML_LIBRARY == MSXML) */

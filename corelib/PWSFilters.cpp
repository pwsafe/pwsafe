/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "PWSFilters.h"
#include "PWHistory.h"
#include "PWSprefs.h"
#include "Match.h"
#include "UUIDGen.h"
#include "corelib.h"
#include "PWScore.h"
#include "StringX.h"
#include "os/file.h"
#include "os/dir.h"

#include "XML/XMLDefs.h"

#if   USE_XML_LIBRARY == EXPAT
#include "XML/Expat/EFilterXMLProcessor.h"
#elif USE_XML_LIBRARY == MSXML
#include "XML/MSXML/MFilterXMLProcessor.h"
#elif USE_XML_LIBRARY == XERCES
#include "XML/Xerces/XFilterXMLProcessor.h"
#endif

#define PWS_XML_FILTER_VERSION 1

#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <algorithm>
#include <map>

using namespace std;

// hide w_char/char differences where possible:
#ifdef UNICODE
typedef std::wifstream ifstreamT;
typedef std::wofstream ofstreamT;
#else
typedef std::ifstream ifstreamT;
typedef std::ofstream ofstreamT;
#endif
typedef std::vector<stringT>::const_iterator vciter;
typedef std::vector<stringT>::iterator viter;

// These are in the same order as "enum MatchRule" in Match.h
static const char * szentry[] = {"normal", 
                                 "aliasbase", "alias", 
                                 "shortcutbase", "shortcut"};

static void GetFilterTestXML(const st_FilterRow &st_fldata,
                             ostringstream &oss, bool bFile)
{
  CUTF8Conv utf8conv;
  const unsigned char *utf8 = NULL;
  int utf8Len = 0;

  const char *sztab1, *sztab2, *sztab3, *sztab4, *szendl;
  if (bFile) {
    sztab1 = "\t";
    sztab2 = "\t\t";
    sztab3 = "\t\t\t";
    sztab4 = "\t\t\t\t";
    szendl = "\n";
  } else {
    sztab1 = sztab2 = sztab3 = sztab4 = "\0";
    szendl = "\0";
  }

  if (st_fldata.mtype != PWSMatch::MT_BOOL)
    oss << sztab3 << "<test>" << szendl;

  switch (st_fldata.mtype) {
    case PWSMatch::MT_STRING:
      // Even if rule == 'present'/'not present', need to put 'string' & 'case' XML
      // elements to make schema work, since W3C Schema V1.0 does NOT support 
      // conditional processing :-(
      oss << sztab4 << "<string>";
      if (!st_fldata.fstring.empty()) { // string empty if 'present' or 'not present'
        utf8conv.ToUTF8(st_fldata.fstring, utf8, utf8Len);
        oss << utf8;
      }
      oss << "</string>" << szendl;
      oss << sztab4 << "<case>" << st_fldata.fcase 
          << "</case>" << szendl;
      break;
    case PWSMatch::MT_PASSWORD:
      utf8conv.ToUTF8(st_fldata.fstring, utf8, utf8Len);
      oss << sztab4 << "<string>" << utf8
                                              << "</string>" << szendl;
      oss << sztab4 << "<case>" << st_fldata.fcase 
                                              << "</case>" << szendl;
      oss << sztab4 << "<warn>" << st_fldata.fcase 
                                              << "</warn>" << szendl;
      break;
    case PWSMatch::MT_INTEGER:
      oss << sztab4 << "<num1>" << st_fldata.fcase 
                                              << "</num1>" << endl;
      oss << sztab4 << "<num2>" << st_fldata.fcase 
                                              << "</num2>" << endl;
      break;
    case PWSMatch::MT_DATE:
      {
      const StringX tmp1 = PWSUtil::ConvertToDateTimeString(st_fldata.fdate1, TMC_XML);
      utf8conv.ToUTF8(tmp1.substr(0, 10), utf8, utf8Len);
      oss << sztab4 << "<date1>" << utf8
                                              << "</date1>" << szendl;
      const StringX tmp2 = PWSUtil::ConvertToDateTimeString(st_fldata.fdate2, TMC_XML);
      utf8conv.ToUTF8(tmp2.substr(0, 10), utf8, utf8Len);
      oss << sztab4 << "<date2>" << utf8
                                              << "</date2>" << szendl;
      }
      break;
    case PWSMatch::MT_ENTRYTYPE:
      oss << sztab4 <<  "<type>" << szentry[st_fldata.etype]
                                              << "</type>" << szendl;
      break;
    case PWSMatch::MT_BOOL:
      break;
    default:
      ASSERT(0);
  }
  if (st_fldata.mtype != PWSMatch::MT_BOOL)
    oss << sztab3 << "</test>" << szendl;
}

static string GetFilterXML(const st_filters &filters, bool bWithFormatting)
{
  ostringstream oss; // ALWAYS a string of chars, never wchar_t!

  CUTF8Conv utf8conv;
  const unsigned char *utf8 = NULL;
  int utf8Len = 0;
  const char *sztab1, *sztab2, *sztab3, *sztab4, *szendl;
  if (bWithFormatting) {
    sztab1 = "\t";
    sztab2 = "\t\t";
    sztab3 = "\t\t\t";
    sztab4 = "\t\t\t\t";
    szendl = "\n";
  } else {
    sztab1 = sztab2 = sztab3 = sztab4 = "\0";
    szendl = "\0";
  }

  utf8conv.ToUTF8(filters.fname.c_str(), utf8, utf8Len);
  oss << sztab1 << "<filter filtername=\"" << reinterpret_cast<const char *>(utf8) 
      << "\">" << szendl;

  std::vector<st_FilterRow>::const_iterator Flt_citer;
  for (Flt_citer = filters.vMfldata.begin(); 
       Flt_citer != filters.vMfldata.end(); Flt_citer++) {
    const st_FilterRow &st_fldata = *Flt_citer;

    if (!st_fldata.bFilterComplete)
      continue;

    oss << sztab2 << "<filter_entry active=\"";
    if (st_fldata.bFilterActive)
      oss << "yes";
    else
      oss << "no";
    oss << "\">" << szendl;

    const int ft = (int)st_fldata.ftype;
    const char *pszfieldtype = {"\0"};
    switch (ft) {
      case FT_GROUPTITLE:
        pszfieldtype = "grouptitle";
        break;
      case FT_GROUP:
        pszfieldtype = "group";
        break;
      case FT_TITLE:
        pszfieldtype = "title";
        break;
      case FT_USER:
        pszfieldtype = "user";
        break;
      case FT_NOTES:
        pszfieldtype = "notes";
        break;
      case FT_URL:
        pszfieldtype = "url";
        break;
      case FT_AUTOTYPE:
        pszfieldtype = "autotype";
        break;
      case FT_PASSWORD:
        pszfieldtype = "password";
        break;
      case FT_CTIME:
        pszfieldtype = "create_time";
        break;
      case FT_PMTIME:
        pszfieldtype = "password_modified_time";
        break;
      case FT_ATIME:
        pszfieldtype = "last_access_time";
        break;
      case FT_XTIME:
        pszfieldtype = "expiry_time";
        break;
      case FT_RMTIME:
        pszfieldtype = "record_modified_time";
        break;
      case FT_PWHIST:
        pszfieldtype = "password_history";
        break;
      case FT_POLICY:
        pszfieldtype = "password_policy";
        break;
      case FT_XTIME_INT:
        pszfieldtype = "password_expiry_interval";
        break;
      case FT_UNKNOWNFIELDS:
        pszfieldtype = "unknownfields";
        break;
      case FT_ENTRYTYPE:
        pszfieldtype = "entrytype";
        break;
      default:
        ASSERT(0);
    }

    oss << sztab3 << "<" << pszfieldtype << ">" << szendl;
 
    PWSMatch::MatchRule mr = st_fldata.rule;
    if (mr >= PWSMatch::MR_LAST)
      mr = PWSMatch::MR_INVALID;

    const LogicConnect lgc = st_fldata.ltype;

    if (ft != FT_PWHIST && ft != FT_POLICY) {
      oss << sztab4 << "<rule>" << PWSMatch::GetRuleString(mr)
                                     << "</rule>" << szendl;

      oss << sztab4 << "<logic>" << (lgc != LC_AND ? "or" : "and")
                                     << "</logic>" << szendl;

      GetFilterTestXML(st_fldata, oss, bWithFormatting);
    } else
      oss << sztab4 << "<logic>" << (lgc != LC_AND ? "or" : "and")
                                     << "</logic>" << szendl;

    oss << sztab3 << "</" << pszfieldtype << ">" << szendl;
    oss << sztab2 << "</filter_entry>" << szendl;
  }

  for (Flt_citer = filters.vHfldata.begin(); 
       Flt_citer != filters.vHfldata.end(); Flt_citer++) {
    const st_FilterRow &st_fldata = *Flt_citer;

    if (!st_fldata.bFilterComplete)
      continue;

    oss << sztab2 << "<filter_entry active=\"";
    if (st_fldata.bFilterActive)
      oss << "yes";
    else
      oss << "no";
    oss << "\">" << szendl;

    const int ft = (int)st_fldata.ftype;
    const char *pszfieldtype = {"\0"};
    switch (ft) {
      case HT_PRESENT:
        pszfieldtype = "history_present";
        break;
      case HT_ACTIVE:
        pszfieldtype = "history_active";
        break;
      case HT_NUM:
        pszfieldtype = "history_number";
        break;
      case HT_MAX:
        pszfieldtype = "history_maximum";
        break;
      case HT_CHANGEDATE:
        pszfieldtype = "history_changedate";
        break;
      case HT_PASSWORDS:
        pszfieldtype = "history_passwords";
        break;
      default:
        ASSERT(0);
    }

    oss << sztab3 << "<" << pszfieldtype << ">" << szendl;
 
    PWSMatch::MatchRule mr = st_fldata.rule;
    if (mr >= PWSMatch::MR_LAST)
      mr = PWSMatch::MR_INVALID;

    oss << sztab4 << "<rule>" << PWSMatch::GetRuleString(mr)
                                   << "</rule>" << szendl;

    const LogicConnect lgc = st_fldata.ltype;
    oss << sztab4 << "<logic>" << (lgc != LC_AND ? "or" : "and")
                                   << "</logic>" << szendl;

    GetFilterTestXML(st_fldata, oss, bWithFormatting);

    oss << sztab3 << "</" << pszfieldtype << ">" << szendl;
    oss << sztab2 << "</filter_entry>" << szendl;
  }

  for (Flt_citer = filters.vPfldata.begin(); 
       Flt_citer != filters.vPfldata.end(); Flt_citer++) {
    const st_FilterRow &st_fldata = *Flt_citer;

    if (!st_fldata.bFilterComplete)
      continue;

    oss << sztab2 << "<filter_entry active=\"";
    if (st_fldata.bFilterActive)
      oss << "yes";
    else
      oss << "no";
    oss << "\">" << szendl;

    const int ft = (int)st_fldata.ftype;
    const char *pszfieldtype = {"\0"};
    switch (ft) {
      case PT_PRESENT:
        pszfieldtype = "policy_present";
        break;
      case PT_LENGTH:
        pszfieldtype = "policy_length";
        break;
      case PT_LOWERCASE:
        pszfieldtype = "policy_number_lowercase";
        break;
      case PT_UPPERCASE:
        pszfieldtype = "policy_number_uppercase";
        break;
      case PT_DIGITS:
        pszfieldtype = "policy_number_digits";
        break;
      case PT_SYMBOLS:
        pszfieldtype = "policy_number_symbols";
        break;
      case PT_EASYVISION:
        pszfieldtype = "policy_easyvision";
        break;
      case PT_PRONOUNCEABLE:
        pszfieldtype = "policy_pronounceable";
        break;
      case PT_HEXADECIMAL:
        pszfieldtype = "policy_hexadecimal";
        break;
        default:
        ASSERT(0);
    }

    oss << sztab3 << "<" << pszfieldtype << ">" << szendl;
 
    PWSMatch::MatchRule mr = st_fldata.rule;
    if (mr >= PWSMatch::MR_LAST)
      mr = PWSMatch::MR_INVALID;

    oss << sztab4 << "<rule>" << PWSMatch::GetRuleString(mr)
                                   << "</rule>" << szendl;

    const LogicConnect lgc = st_fldata.ltype;
    oss << sztab4 << "<logic>" << (lgc != LC_AND ? "or" : "and")
                                   << "</logic>" << szendl;

    GetFilterTestXML(st_fldata, oss, bWithFormatting);

    oss << sztab3 << "</" << pszfieldtype << ">" << szendl;
    oss << sztab2 << "</filter_entry>" << szendl;
  }

  oss << sztab1 << "</filter>" << szendl;
  oss << szendl;

  return oss.str();
}

struct XMLFilterWriterToString {
  XMLFilterWriterToString(ostream &os, bool bWithFormatting) :
  m_os(os), m_bWithFormatting(bWithFormatting)
  {}
  // operator
  void operator()(pair<const st_Filterkey, st_filters> p)
  {
    string xml = GetFilterXML(p.second, m_bWithFormatting);
    m_os << xml.c_str();
  }
private:
  ostream &m_os;
  bool m_bWithFormatting;
};

int PWSFilters::WriteFilterXMLFile(const StringX &filename,
                                   const PWSfile::HeaderRecord hdr,
                                   const StringX &currentfile)
{
  ofstream of(filename.c_str());
  if (!of)
    return PWScore::CANT_OPEN_FILE;
  else
    return WriteFilterXMLFile(of, hdr, currentfile, true);
}

int PWSFilters::WriteFilterXMLFile(ostream &os,
                                   const PWSfile::HeaderRecord hdr,
                                   const StringX &currentfile,
                                   const bool bWithFormatting)
{
  string str_hdr = GetFilterXMLHeader(currentfile, hdr);
  os << str_hdr;

  XMLFilterWriterToString put_filterxml(os, bWithFormatting);
  for_each(this->begin(), this->end(), put_filterxml);

  os << "</filters>";

  return PWScore::SUCCESS;
}

std::string PWSFilters::GetFilterXMLHeader(const StringX &currentfile,
                                           const PWSfile::HeaderRecord &hdr)
{
  CUTF8Conv utf8conv;
  const unsigned char *utf8 = NULL;
  int utf8Len = 0;

  ostringstream oss;
  StringX tmp;
  stringT cs_tmp;
  time_t time_now;

  time(&time_now);
  const StringX now = PWSUtil::ConvertToDateTimeString(time_now, TMC_XML);

  oss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
  oss << endl;

  oss << "<filters version=\"";
  oss << PWS_XML_FILTER_VERSION;
  oss << "\"" << endl;
  
  if (!currentfile.empty()) {
    cs_tmp = currentfile.c_str();
    Replace(cs_tmp, stringT(_T("&")), stringT(_T("&amp;")));

    utf8conv.ToUTF8(cs_tmp.c_str(), utf8, utf8Len);
    oss << "Database=\"";
    oss << reinterpret_cast<const char *>(utf8);
    oss << "\"" << endl;
    utf8conv.ToUTF8(now, utf8, utf8Len);
    oss << "ExportTimeStamp=\"";
    oss << reinterpret_cast<const char *>(utf8);
    oss << "\"" << endl;
    oss << "FromDatabaseFormat=\"";
    oss << hdr.m_nCurrentMajorVersion
        << "." << setw(2) << setfill('0')
        << hdr.m_nCurrentMinorVersion;
    oss << "\"" << endl;
    if (!hdr.m_lastsavedby.empty() || !hdr.m_lastsavedon.empty()) {
      stringT wls(_T(""));
      Format(wls,
             _T("%s on %s"),
             hdr.m_lastsavedby.c_str(), hdr.m_lastsavedon.c_str());
      utf8conv.ToUTF8(wls.c_str(), utf8, utf8Len);
      oss << "WhoSaved=\"";
      oss << reinterpret_cast<const char *>(utf8);
      oss << "\"" << endl;
    }
    if (!hdr.m_whatlastsaved.empty()) {
      utf8conv.ToUTF8(hdr.m_whatlastsaved, utf8, utf8Len);
      oss << "WhatSaved=\"";
      oss << reinterpret_cast<const char *>(utf8);
      oss << "\"" << endl;
    }
    if (hdr.m_whenlastsaved != 0) {
      StringX wls = PWSUtil::ConvertToDateTimeString(hdr.m_whenlastsaved,
                                                     TMC_XML);
      utf8conv.ToUTF8(wls.c_str(), utf8, utf8Len);
      oss << "WhenLastSaved=\"";
      oss << reinterpret_cast<const char *>(utf8);
      oss << "\"" << endl;
    }

    CUUIDGen huuid(hdr.m_file_uuid_array, true); // true to print canonically

    oss << "Database_uuid=\"" << huuid << "\"" << endl;
  }
  oss << "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"" << endl;
  oss << "xsi:noNamespaceSchemaLocation=\"pwsafe_filter.xsd\">" << endl;
  oss << endl;

  return oss.str().c_str();
}

#if !defined(USE_XML_LIBRARY) || (!defined(_WIN32) && USE_XML_LIBRARY == MSXML)
// Don't support importing XML from non-Windows using Microsoft XML libraries
int PWSFilters::ImportFilterXMLFile(const FilterPool, 
                                    const StringX &, 
                                    const stringT &, 
                                    const stringT &,
                                    stringT &,
                                    Asker *)
{
  return PWScore::UNIMPLEMENTED;
}
#else
int PWSFilters::ImportFilterXMLFile(const FilterPool fpool,
                                    const StringX &strXMLData,
                                    const stringT &strXMLFileName,
                                    const stringT &strXSDFileName,
                                    stringT &strErrors,
                                    Asker *pAsker)
{
#if   USE_XML_LIBRARY == EXPAT
  EFilterXMLProcessor fXML(*this, fpool, pAsker);
#elif USE_XML_LIBRARY == MSXML
  MFilterXMLProcessor fXML(*this, fpool, pAsker);
#elif USE_XML_LIBRARY == XERCES
  XFilterXMLProcessor fXML(*this, fpool, pAsker);
#endif
  bool status, validation;

  strErrors = _T("");

  validation = true;
  if (strXMLFileName.empty())
    status = fXML.Process(validation, strXMLData, _T(""), strXSDFileName);
  else
    status = fXML.Process(validation, _T(""), strXMLFileName, strXSDFileName);

  strErrors = fXML.getResultText();
  if (!status) {
    return PWScore::XML_FAILED_VALIDATION;
  }

  validation = false;
  if (strXMLFileName.empty())
    status = fXML.Process(validation, strXMLData, _T(""), strXSDFileName);
  else
    status = fXML.Process(validation, _T(""), strXMLFileName, strXSDFileName);

    strErrors = fXML.getResultText();
  if (!status) {
    return PWScore::XML_FAILED_IMPORT;
  }

  // By definition - all imported filters are complete!
  // Now set this.
  PWSFilters::iterator mf_iter;
  for (mf_iter = this->begin(); mf_iter != this->end(); mf_iter++) {
    st_filters &filters = mf_iter->second;
    for_each(filters.vMfldata.begin(), filters.vMfldata.end(),
             mem_fun_ref(&st_FilterRow::SetFilterComplete));
    for_each(filters.vHfldata.begin(), filters.vHfldata.end(),
             mem_fun_ref(&st_FilterRow::SetFilterComplete));
    for_each(filters.vPfldata.begin(), filters.vPfldata.end(),
             mem_fun_ref(&st_FilterRow::SetFilterComplete));
  }
  return PWScore::SUCCESS;
}
#endif

stringT PWSFilters::GetFilterDescription(const st_FilterRow &st_fldata)
{
  // Get the description of the current criterion to display on the static text
  stringT cs_rule, cs1, cs2, cs_and, cs_criteria;
  LoadAString(cs_rule, PWSMatch::GetRule(st_fldata.rule));
  TrimRight(cs_rule, _T("\t"));
  PWSMatch::GetMatchType(st_fldata.mtype,
                         st_fldata.fnum1, st_fldata.fnum2,
                         st_fldata.fdate1, st_fldata.fdate2,
                         st_fldata.fstring.c_str(), st_fldata.fcase,
                         st_fldata.etype,
                         st_fldata.rule == PWSMatch::MR_BETWEEN,
                         cs1, cs2);
  switch (st_fldata.mtype) {
    case PWSMatch::MT_PASSWORD:
      if (st_fldata.rule == PWSMatch::MR_EXPIRED) {
        Format(cs_criteria, _T("%s"), cs_rule.c_str());
        break;
      } else if (st_fldata.rule == PWSMatch::MR_WILLEXPIRE) {
        Format(cs_criteria, _T("%s %s"), cs_rule.c_str(), cs1.c_str());
        break;
      }
      // Note: purpose drop through to standard 'string' processing
    case PWSMatch::MT_STRING:
      if (st_fldata.rule == PWSMatch::MR_PRESENT ||
          st_fldata.rule == PWSMatch::MR_NOTPRESENT)
        Format(cs_criteria, _T("%s"), cs_rule.c_str());
      else {
        stringT cs_delim(_T(""));
        if (cs1.find(_T(" ")) != stringT::npos)
          cs_delim = _T("'");
        Format(cs_criteria, _T("%s %s%s%s %s"), 
               cs_rule.c_str(), cs_delim.c_str(), 
               cs1.c_str(), cs_delim.c_str(), cs2.c_str());
      }
      break;
    case PWSMatch::MT_INTEGER:
    case PWSMatch::MT_DATE:
      if (st_fldata.rule == PWSMatch::MR_PRESENT ||
          st_fldata.rule == PWSMatch::MR_NOTPRESENT)
        Format(cs_criteria, _T("%s"), cs_rule.c_str());
      else
      if (st_fldata.rule == PWSMatch::MR_BETWEEN) {  // Date or Integer only
        LoadAString(cs_and, IDSC_AND);
        Format(cs_criteria, _T("%s %s %s %s"), 
               cs_rule.c_str(), cs1.c_str(), cs_and.c_str(), cs2.c_str());
      } else
        Format(cs_criteria, _T("%s %s"), cs_rule.c_str(), cs1.c_str());
      break;
    case PWSMatch::MT_PWHIST:
      LoadAString(cs_criteria, IDSC_SEEPWHISTORYFILTERS);
      break;
    case PWSMatch::MT_POLICY:
      LoadAString(cs_criteria, IDSC_SEEPWPOLICYFILTERS);
      break;
    case PWSMatch::MT_BOOL:
      cs_criteria = cs_rule;
      break;
    case PWSMatch::MT_ENTRYTYPE:
      Format(cs_criteria, _T("%s %s"), cs_rule.c_str(), cs1.c_str());
      break;
    default:
      ASSERT(0);
  }
  return cs_criteria;
}

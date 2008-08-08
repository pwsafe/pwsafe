/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "PWSFilters.h"
#include "PWHistory.h"
#include "pwsprefs.h"
#include "match.h"
#include "UUIDGen.h"
#include "PWSXMLFilters.h"
#include "SAXFilters.h"
#include "corelib.h"
#include "PWScore.h"

#include <fstream>
#include <iostream>
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

  char *sztab, *szendl;
  if (bFile) {
    sztab = "\t";
    szendl = "\n";
  } else {
    sztab = "\0";
    szendl = "\0";
  }

  if (st_fldata.mtype != PWSMatch::MT_BOOL)
    oss << sztab << sztab << sztab << "<test>" << szendl;

  switch (st_fldata.mtype) {
    case PWSMatch::MT_STRING:
      if (!st_fldata.fstring.IsEmpty()) { // string empty if 'present' or 'not present'
        utf8conv.ToUTF8(st_fldata.fstring, utf8, utf8Len);
        oss << sztab << sztab << sztab << sztab << "<string>" << utf8
            << "</string>" << szendl;
        oss << sztab << sztab << sztab << sztab << "<case>" << st_fldata.fcase 
            << "</case>" << szendl;
      }
      break;
    case PWSMatch::MT_PASSWORD:
      utf8conv.ToUTF8(st_fldata.fstring, utf8, utf8Len);
      oss << sztab << sztab << sztab << sztab << "<string>" << utf8
                                              << "</string>" << szendl;
      oss << sztab << sztab << sztab << sztab << "<case>" << st_fldata.fcase 
                                              << "</case>" << szendl;
      oss << sztab << sztab << sztab << sztab << "<warn>" << st_fldata.fcase 
                                              << "</warn>" << szendl;
      break;
    case PWSMatch::MT_INTEGER:
      oss << sztab << sztab << sztab << sztab << "<num1>" << st_fldata.fcase 
                                              << "</num1>" << endl;
      oss << sztab << sztab << sztab << sztab << "<num2>" << st_fldata.fcase 
                                              << "</num2>" << endl;
      break;
    case PWSMatch::MT_DATE:
      {
      const CMyString tmp1 = PWSUtil::ConvertToDateTimeString(st_fldata.fdate1, TMC_XML);
      utf8conv.ToUTF8(tmp1.Left(10), utf8, utf8Len);
      oss << sztab << sztab << sztab << sztab << "<date1>" << utf8
                                              << "</date1>" << szendl;
      const CMyString tmp2 = PWSUtil::ConvertToDateTimeString(st_fldata.fdate2, TMC_XML);
      utf8conv.ToUTF8(tmp2.Left(10), utf8, utf8Len);
      oss << sztab << sztab << sztab << sztab << "<date2>" << utf8
                                              << "</date2>" << szendl;
      }
      break;
    case PWSMatch::MT_ENTRYTYPE:
      oss << sztab << sztab << sztab << sztab <<  "<type>" << szentry[st_fldata.etype]
                                              << "</type>" << szendl;
      break;
    case PWSMatch::MT_BOOL:
      break;
    default:
      ASSERT(0);
  }
  if (st_fldata.mtype != PWSMatch::MT_BOOL)
    oss << sztab << sztab << sztab << "</test>" << szendl;
}

static string GetFilterXML(const st_filters &filters, bool bFile)
{
  ostringstream oss; // ALWAYS a string of chars, never wchar_t!

  CMyString tmp;
  CUTF8Conv utf8conv;
  const unsigned char *utf8 = NULL;
  int utf8Len = 0;
  char *sztab, *szendl;
  if (bFile) {
    sztab = "\t";
    szendl = "\n";
  } else {
    sztab = "\0";
    szendl = "\0";
  }

  utf8conv.ToUTF8(filters.fname, utf8, utf8Len);
  oss << "<filter filtername=\"" << reinterpret_cast<const char *>(utf8) 
      << "\">" << szendl;

  std::vector<st_FilterRow>::const_iterator Flt_citer;
  for (Flt_citer = filters.vMfldata.begin(); 
       Flt_citer != filters.vMfldata.end(); Flt_citer++) {
    const st_FilterRow &st_fldata = *Flt_citer;

    if (!st_fldata.bFilterComplete)
      continue;

    oss << sztab << "<filter_entry active=\"";
    if (st_fldata.bFilterActive)
      oss << "yes";
    else
      oss << "no";
    oss << "\">" << szendl;

    const int ft = (int)st_fldata.ftype;
    char *pszfieldtype = {"\0"};
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

    oss << sztab << sztab <<"<" << pszfieldtype << ">" << szendl;
 
    PWSMatch::MatchRule mr = st_fldata.rule;
    if (mr >= PWSMatch::MR_LAST)
      mr = PWSMatch::MR_INVALID;

    const LogicConnect lgc = st_fldata.ltype;

    if (ft != FT_PWHIST && ft != FT_POLICY) {
      oss << sztab << sztab << sztab << "<rule>" << PWSMatch::GetRuleString(mr)
                                     << "</rule>" << szendl;

      oss << sztab << sztab << sztab << "<logic>" << (lgc != LC_AND ? "or" : "and")
                                     << "</logic>" << szendl;

      GetFilterTestXML(st_fldata, oss, bFile);
    } else
      oss << sztab << sztab << sztab << "<logic>" << (lgc != LC_AND ? "or" : "and")
                                     << "</logic>" << szendl;

    oss << sztab << sztab << "</" << pszfieldtype << ">" << szendl;
    oss << sztab << "</filter_entry>" << szendl;
    oss << szendl;
  }

  for (Flt_citer = filters.vHfldata.begin(); 
       Flt_citer != filters.vHfldata.end(); Flt_citer++) {
    const st_FilterRow &st_fldata = *Flt_citer;

    if (!st_fldata.bFilterComplete)
      continue;

    oss << sztab << "<filter_entry active=\"";
    if (st_fldata.bFilterActive)
      oss << "yes";
    else
      oss << "no";
    oss << "\">" << szendl;

    const int ft = (int)st_fldata.ftype;
    char *pszfieldtype = {"\0"};
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

    oss << sztab << sztab << "<" << pszfieldtype << ">" << szendl;
 
    PWSMatch::MatchRule mr = st_fldata.rule;
    if (mr >= PWSMatch::MR_LAST)
      mr = PWSMatch::MR_INVALID;

    oss << sztab << sztab << sztab << "<rule>" << PWSMatch::GetRuleString(mr)
                                   << "</rule>" << szendl;

    const LogicConnect lgc = st_fldata.ltype;
    oss << sztab << sztab << sztab << "<logic>" << (lgc != LC_AND ? "or" : "and")
                                   << "</logic>" << szendl;

    GetFilterTestXML(st_fldata, oss, bFile);

    oss << sztab << sztab << "</" << pszfieldtype << ">" << szendl;
    oss << sztab << "</filter_entry>" << szendl;
    oss << szendl;
  }

  for (Flt_citer = filters.vPfldata.begin(); 
       Flt_citer != filters.vPfldata.end(); Flt_citer++) {
    const st_FilterRow &st_fldata = *Flt_citer;

    if (!st_fldata.bFilterComplete)
      continue;

    oss << sztab << "<filter_entry active=\"";
    if (st_fldata.bFilterActive)
      oss << "yes";
    else
      oss << "no";
    oss << "\">" << szendl;

    const int ft = (int)st_fldata.ftype;
    char *pszfieldtype = {"\0"};
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

    oss << sztab << sztab << "<" << pszfieldtype << ">" << szendl;
 
    PWSMatch::MatchRule mr = st_fldata.rule;
    if (mr >= PWSMatch::MR_LAST)
      mr = PWSMatch::MR_INVALID;

    oss << sztab << sztab << sztab << "<rule>" << PWSMatch::GetRuleString(mr)
                                   << "</rule>" << szendl;

    const LogicConnect lgc = st_fldata.ltype;
    oss << sztab << sztab << sztab << "<logic>" << (lgc != LC_AND ? "or" : "and")
                                   << "</logic>" << szendl;

    GetFilterTestXML(st_fldata, oss, bFile);

    oss << sztab << sztab << "</" << pszfieldtype << ">" << szendl;
    oss << sztab << "</filter_entry>" << szendl;
  }

  oss << "</filter>" << szendl;
  oss << szendl;

  return oss.str();
}

struct XMLFilterWriterToFile {
  XMLFilterWriterToFile(ofstream &ofs) :
  m_of(ofs)
  {}
  // operator
  void operator()(pair<const CString, st_filters> p)
  {
    string xml = GetFilterXML(p.second, true);
    m_of.write(xml.c_str(),
               static_cast<streamsize>(xml.length()));
  }

private:
  ostream &m_of;
};

struct XMLFilterWriterToString {
  XMLFilterWriterToString(ostream &os) :
  m_os(os)
  {}
  // operator
  void operator()(pair<const CString, st_filters> p)
  {
    string xml = GetFilterXML(p.second, false);
    m_os << xml.c_str();
  }
private:
  ostream &m_os;
};

int PWSFilters::WriteFilterXMLFile(const CMyString &filename,
                                   const PWSfile::HeaderRecord hdr,
                                   const CMyString &currentfile)
{
  ofstream of(filename);
  if (!of)
    return PWScore::CANT_OPEN_FILE;
  else
    return WriteFilterXMLFile(of, hdr, currentfile);
}

int PWSFilters::WriteFilterXMLFile(ostream &os,
                                   const PWSfile::HeaderRecord hdr,
                                   const CMyString &currentfile)
{
  string str_hdr = GetFilterXMLHeader(currentfile, hdr);
  os << str_hdr;

  XMLFilterWriterToString put_filterxml(os);

  for_each(this->begin(), this->end(), put_filterxml);

  os << "</filters>";

  return PWScore::SUCCESS;
}

std::string PWSFilters::GetFilterXMLHeader(const CMyString &currentfile,
                                           const PWSfile::HeaderRecord &hdr)
{
  CUTF8Conv utf8conv;
  const unsigned char *utf8 = NULL;
  int utf8Len = 0;

  ostringstream oss;
  CMyString tmp;
  CString cs_tmp;
  time_t time_now;

  time(&time_now);
  const CMyString now = PWSUtil::ConvertToDateTimeString(time_now, TMC_XML);

  oss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
  oss << endl;
  tmp.Format(_T("%d"), PWS_XML_FILTER_VERSION);
  utf8conv.ToUTF8(tmp, utf8, utf8Len);

  oss << "<filters version=\"";
  oss << reinterpret_cast<const char *>(utf8);
  oss << "\"" << endl;
  
  if (!currentfile.IsEmpty()) {
    tmp = currentfile;
    tmp.Replace(_T("&"), _T("&amp;"));

    utf8conv.ToUTF8(tmp, utf8, utf8Len);
    oss << "Database=\"";
    oss << reinterpret_cast<const char *>(utf8);
    oss << "\"" << endl;
    utf8conv.ToUTF8(now, utf8, utf8Len);
    oss << "ExportTimeStamp=\"";
    oss << reinterpret_cast<const char *>(utf8);
    oss << "\"" << endl;
    cs_tmp.Format(_T("%d.%02d"), hdr.m_nCurrentMajorVersion, hdr.m_nCurrentMinorVersion);
    utf8conv.ToUTF8(cs_tmp, utf8, utf8Len);
    oss << "FromDatabaseFormat=\"";
    oss << reinterpret_cast<const char *>(utf8);
    oss << "\"" << endl;
    if (!hdr.m_lastsavedby.IsEmpty() || !hdr.m_lastsavedon.IsEmpty()) {
      CString wls(_T(""));
      wls.Format(_T("%s on %s"), hdr.m_lastsavedby, hdr.m_lastsavedon);
      utf8conv.ToUTF8(wls, utf8, utf8Len);
      oss << "WhoSaved=\"";
      oss << reinterpret_cast<const char *>(utf8);
      oss << "\"" << endl;
    }
    if (!hdr.m_whatlastsaved.IsEmpty()) {
      utf8conv.ToUTF8(hdr.m_whatlastsaved, utf8, utf8Len);
      oss << "WhatSaved=\"";
      oss << reinterpret_cast<const char *>(utf8);
      oss << "\"" << endl;
    }
    if (hdr.m_whenlastsaved != 0) {
      CString wls = CString(PWSUtil::ConvertToDateTimeString(hdr.m_whenlastsaved,
                            TMC_XML));
      utf8conv.ToUTF8(wls, utf8, utf8Len);
      oss << "WhenLastSaved=\"";
      oss << reinterpret_cast<const char *>(utf8);
      oss << "\"" << endl;
    }

    uuid_str_WH_t hdr_uuid_buffer;
    CUUIDGen::GetUUIDStr(hdr.m_file_uuid_array, hdr_uuid_buffer);

    oss << "Database_uuid=\"" << hdr_uuid_buffer << "\"" << endl;
  }
  oss << "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"" << endl;
  oss << "xsi:noNamespaceSchemaLocation=\"pwsafe_filter.xsd\">" << endl;
  oss << endl;

  return oss.str().c_str();
}

int PWSFilters::ImportFilterXMLFile(const CString &strXMLData,
                                    const CString &strXMLFileName,
                                    const CString &strXSDFileName, CString &strErrors)
{
  PWSXMLFilters fXML(*this);
  bool status, validation;

  strErrors = _T("");

  validation = true;
  if (strXMLFileName.IsEmpty())
    status = fXML.XMLFilterProcess(validation, strXMLData, _T(""), strXSDFileName);
  else
    status = fXML.XMLFilterProcess(validation, _T(""), strXMLFileName, strXSDFileName);

    strErrors = fXML.m_strResultText;
  if (!status) {
    return PWScore::XML_FAILED_VALIDATION;
  }

  validation = false;
  if (strXMLFileName.IsEmpty())
    status = fXML.XMLFilterProcess(validation, strXMLData, _T(""), strXSDFileName);
  else
    status = fXML.XMLFilterProcess(validation, _T(""), strXMLFileName, strXSDFileName);

    strErrors = fXML.m_strResultText;
  if (!status) {
    return PWScore::XML_FAILED_IMPORT;
  }

  // By definition - all imported filters are complete!
  // Now set this.
  PWSFilters::iterator mf_iter;
  for (mf_iter = this->begin(); mf_iter != this->end(); mf_iter++) {
    st_filters &filters = mf_iter->second;
    std::vector<st_FilterRow>::iterator Flt_iter;
    for (Flt_iter = filters.vMfldata.begin(); 
         Flt_iter != filters.vMfldata.end(); Flt_iter++) {
      Flt_iter->bFilterComplete = true;
    }
    for (Flt_iter = filters.vHfldata.begin(); 
         Flt_iter != filters.vHfldata.end(); Flt_iter++) {
      Flt_iter->bFilterComplete = true;
    }
    for (Flt_iter = filters.vPfldata.begin(); 
         Flt_iter != filters.vPfldata.end(); Flt_iter++) {
      Flt_iter->bFilterComplete = true;
    }
  }
  return PWScore::SUCCESS;
}

CString PWSFilters::GetFilterDescription(const st_FilterRow &st_fldata)
{
  // Get the description of the current criterion to display on the static text
  CString cs_rule, cs1, cs2, cs_and, cs_criteria;
  cs_rule.LoadString(PWSMatch::GetRule(st_fldata.rule));
  cs_rule.TrimRight(_T('\t'));
  PWSMatch::GetMatchType(st_fldata.mtype,
                         st_fldata.fnum1, st_fldata.fnum2,
                         st_fldata.fdate1, st_fldata.fdate2,
                         st_fldata.fstring, st_fldata.fcase,
                         st_fldata.etype,
                         st_fldata.rule == PWSMatch::MR_BETWEEN,
                         cs1, cs2);
  switch (st_fldata.mtype) {
    case PWSMatch::MT_PASSWORD:
      if (st_fldata.rule == PWSMatch::MR_EXPIRED) {
        cs_criteria.Format(_T("%s"), cs_rule);
        break;
      } else if (st_fldata.rule == PWSMatch::MR_WILLEXPIRE) {
        cs_criteria.Format(_T("%s %s"), cs_rule, cs1);
        break;
      }
      // Note: purpose drop through to standard 'string' processing
    case PWSMatch::MT_STRING:
      if (st_fldata.rule == PWSMatch::MR_PRESENT ||
          st_fldata.rule == PWSMatch::MR_NOTPRESENT)
        cs_criteria.Format(_T("%s"), cs_rule);
      else {
        CString cs_delim(_T(""));
        if (cs1.Find(_T(' ')) != -1)
          cs_delim = _T("'");
        cs_criteria.Format(_T("%s %s%s%s %s"), 
                          cs_rule, cs_delim, cs1, cs_delim, cs2);
      }
      break;
    case PWSMatch::MT_INTEGER:
    case PWSMatch::MT_DATE:
      if (st_fldata.rule == PWSMatch::MR_PRESENT ||
          st_fldata.rule == PWSMatch::MR_NOTPRESENT)
        cs_criteria.Format(_T("%s"), cs_rule);
      else
      if (st_fldata.rule == PWSMatch::MR_BETWEEN) {  // Date or Integer only
        cs_and.LoadString(IDSC_AND);
        cs_criteria.Format(_T("%s %s %s %s"), cs_rule, cs1, cs_and, cs2);
      } else
        cs_criteria.Format(_T("%s %s"), cs_rule, cs1);
      break;
    case PWSMatch::MT_PWHIST:
      cs_criteria.LoadString(IDSC_SEEPWHISTORYFILTERS);
      break;
    case PWSMatch::MT_POLICY:
      cs_criteria.LoadString(IDSC_SEEPWPOLICYFILTERS);
      break;
    case PWSMatch::MT_BOOL:
      cs_criteria.Format(_T("%s"), cs_rule);
      break;
    case PWSMatch::MT_ENTRYTYPE:
      cs_criteria.Format(_T("%s %s"), cs_rule, cs1);
      break;
    default:
      ASSERT(0);
  }
  return cs_criteria;
}

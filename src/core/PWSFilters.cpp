/*
* Copyright (c) 2003-2015 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#include "PWSFilters.h"
#include "PWSfileHeader.h"
#include "PWHistory.h"
#include "PWSprefs.h"
#include "core.h"
#include "PWScore.h"
#include "StringX.h"
#include "Util.h"

#include "os/file.h"
#include "os/dir.h"

#include "XML/XMLDefs.h"  // Required if testing "USE_XML_LIBRARY"

#if USE_XML_LIBRARY == MSXML
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
#include <functional>
#include <algorithm>
#include <map>

using namespace std;
using pws_os::CUUID;

typedef std::vector<stringT>::const_iterator vciter;
typedef std::vector<stringT>::iterator viter;

// These are in the same order as "enum EntryType" in ItemData.h
static const char * szentry[] = {"normal",
                                 "aliasbase", "alias", 
                                 "shortcutbase", "shortcut"};

// These are in the same order as "enum EntryStatus" in ItemData.h
static const char * szstatus[] = {"clean", "added", "modified", 
                                  "deleted"};

static void GetFilterTestXML(const st_FilterRow &st_fldata,
                             ostringstream &oss, bool bFile)
{
  CUTF8Conv utf8conv;
  const unsigned char *utf8 = NULL;
  size_t utf8Len = 0;

  const char *sztab4, *sztab5, *szendl;
  if (bFile) {
    sztab4 = "\t\t\t\t";
    sztab5 = "\t\t\t\t\t";
    szendl = "\n";
  } else {
    sztab4 = sztab5 = "\0";
    szendl = "\0";
  }

  if (st_fldata.mtype != PWSMatch::MT_BOOL)
    oss << sztab4 << "<test>" << szendl;

  switch (st_fldata.mtype) {
    case PWSMatch::MT_STRING:
      // Even if rule == 'present'/'not present', need to put 'string' & 'case' XML
      // elements to make schema work, since W3C Schema V1.0 does NOT support 
      // conditional processing :-(
      // 'string' needs special processing to place within CDATA XML construct
      if (!st_fldata.fstring.empty()) { // string empty if 'present' or 'not present'
        PWSUtil::WriteXMLField(oss, "string", st_fldata.fstring, utf8conv, sztab5);
      } else {
        oss << sztab5 << "<string></string>" << szendl;
      }
      oss << sztab5 << "<case>" << st_fldata.fcase 
          << "</case>" << szendl;
      break;
    case PWSMatch::MT_PASSWORD:
      // 'string' needs special processing to place within CDATA XML construct
      PWSUtil::WriteXMLField(oss, "string", st_fldata.fstring, utf8conv, sztab5);
      oss << sztab5 << "<case>" << st_fldata.fcase 
                                              << "</case>" << szendl;
      oss << sztab5 << "<warn>" << st_fldata.fcase 
                                              << "</warn>" << szendl;
      break;
    case PWSMatch::MT_INTEGER:
      oss << sztab5 << "<num1>" << st_fldata.fnum1 
                                              << "</num1>" << endl;
      oss << sztab5 << "<num2>" << st_fldata.fnum2 
                                              << "</num2>" << endl;
      break;
    case PWSMatch::MT_DATE:
    {
      if (st_fldata.fdatetype == 0 /* DTYPE_ABS */) {
        const StringX tmp1 = PWSUtil::ConvertToDateTimeString(st_fldata.fdate1, PWSUtil::TMC_XML);
        utf8conv.ToUTF8(tmp1.substr(0, 10), utf8, utf8Len);
        oss << sztab5 << "<date1>" << utf8
                                                << "</date1>" << szendl;
        const StringX tmp2 = PWSUtil::ConvertToDateTimeString(st_fldata.fdate2, PWSUtil::TMC_XML);
        utf8conv.ToUTF8(tmp2.substr(0, 10), utf8, utf8Len);
        oss << sztab5 << "<date2>" << utf8
                                                << "</date2>" << szendl;
      } else {
        oss << sztab5 << "<num1>" << st_fldata.fnum1 
                                                << "</num1>" << endl;
        oss << sztab5 << "<num2>" << st_fldata.fnum2 
                                                << "</num2>" << endl;
      }
      break;
    }
    case PWSMatch::MT_ENTRYTYPE:
    {
      // Get index for string values
      int index(0);
      switch (st_fldata.etype) {
        case CItemData::ET_NORMAL:       index = 0; break;
        case CItemData::ET_ALIASBASE:    index = 1; break;
        case CItemData::ET_ALIAS:        index = 2; break;
        case CItemData::ET_SHORTCUTBASE: index = 3; break;
        case CItemData::ET_SHORTCUT:     index = 4; break;
        default:
          ASSERT(0);
      }
      oss << sztab5 << "<type>" << szentry[index]
                                              << "</type>" << szendl;
      break;
    }
    case PWSMatch::MT_DCA:
      oss << sztab5 << "<dca>" << st_fldata.fdca 
                                              << "</dca>" << szendl;
      break;
    case PWSMatch::MT_SHIFTDCA:
      oss << sztab5 << "<shiftdca>" << st_fldata.fdca 
                                              << "</shiftdca>" << szendl;
      break;
    case PWSMatch::MT_ENTRYSTATUS:
    {
      // Get index for string values
      int index(0);
      switch (st_fldata.estatus) {
        case CItemData::ES_CLEAN:    index = 0; break;
        case CItemData::ES_ADDED:    index = 1; break;
        case CItemData::ES_MODIFIED: index = 2; break;
        case CItemData::ES_DELETED:  index = 3; break;
        default:
          ASSERT(0);
      }
      oss << sztab5 << "<status>" << szstatus[index]
                                              << "</status>" << szendl;
      break;
    }
    case PWSMatch::MT_ENTRYSIZE:
      oss << sztab5 << "<num1>" << st_fldata.fnum1 
                                              << "</num1>" << endl;
      oss << sztab5 << "<num2>" << st_fldata.fnum2 
                                              << "</num2>" << endl;
      oss << sztab5 << "<unit>" << st_fldata.funit 
                                              << "</unit>" << endl;
      break;
    case PWSMatch::MT_BOOL:
      break;
    default:
      ASSERT(0);
  }
  if (st_fldata.mtype != PWSMatch::MT_BOOL)
    oss << sztab4 << "</test>" << szendl;
}

static string GetFilterXML(const st_filters &filters, bool bWithFormatting)
{
  ostringstream oss; // ALWAYS a string of chars, never wchar_t!

  CUTF8Conv utf8conv;
  const unsigned char *utf8 = NULL;
  size_t utf8Len = 0;
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

  // Filter name is an element attribute and so must be a quoted string.
  // Convert any embedded quotes to &quot;
  stringT fname = filters.fname;
  const stringT from = _T("\""), to = _T("&quot;");
  Replace(fname, from, to);

  utf8conv.ToUTF8(fname.c_str(), utf8, utf8Len);
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

    const int ft = static_cast<int>(st_fldata.ftype);
    const char *pszfieldtype = {"\0"};

    // These are the entry names exported and must be recognised by the associated schema
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
      case FT_PASSWORD:
        pszfieldtype = "password";
        break;
      case FT_URL:
        pszfieldtype = "url";
        break;
      case FT_AUTOTYPE:
        pszfieldtype = "autotype";
        break;
      case FT_RUNCMD:
        pszfieldtype = "runcommand";
        break;
      case FT_DCA:
        pszfieldtype = "DCA";
        break;
      case FT_SHIFTDCA:
        pszfieldtype = "ShiftDCA";
        break;
      case FT_EMAIL:
        pszfieldtype = "email";
        break;
      case FT_PROTECTED:
        pszfieldtype = "protected";
        break;
      case FT_SYMBOLS:
        pszfieldtype = "symbols";
        break;
      case FT_POLICYNAME:
        pszfieldtype = "policy_name";
        break;
      case FT_KBSHORTCUT:
        pszfieldtype = "kbshortcut";
        break;
      // Time fields
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
      case FT_XTIME_INT:
        pszfieldtype = "password_expiry_interval";
        break;
      // History & Policy
      case FT_PWHIST:
        pszfieldtype = "password_history";
        break;
      case FT_POLICY:
        pszfieldtype = "password_policy";
        break;
      // Other!
      case FT_PASSWORDLEN:
        pszfieldtype = "password_length";
        break;
      case FT_UNKNOWNFIELDS:
        pszfieldtype = "unknownfields";
        break;
      case FT_ENTRYSIZE:
        pszfieldtype = "entrysize";
        break;
      case FT_ENTRYTYPE:
        pszfieldtype = "entrytype";
        break;
      case FT_ENTRYSTATUS:
        pszfieldtype = "entrystatus";
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

    const int ft = static_cast<int>(st_fldata.ftype);
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

    const int ft = static_cast<int>(st_fldata.ftype);
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
  XMLFilterWriterToString(coStringXStream &os, bool bWithFormatting) :
  m_os(os), m_bWithFormatting(bWithFormatting)
  {}
  // operator
  void operator()(pair<const st_Filterkey, st_filters> p)
  {
    string xml = GetFilterXML(p.second, m_bWithFormatting);
    m_os << xml.c_str();
  }
private:
  XMLFilterWriterToString& operator=(const XMLFilterWriterToString&); // Do not implement
  coStringXStream &m_os;
  bool m_bWithFormatting;
};

int PWSFilters::WriteFilterXMLFile(const StringX &filename,
                                   const PWSfileHeader &hdr,
                                   const StringX &currentfile)
{
  FILE *xmlfile = pws_os::FOpen(filename.c_str(), _T("wt"));
  if (xmlfile == NULL)
    return PWScore::CANT_OPEN_FILE;

  coStringXStream oss;
  int irc = WriteFilterXMLFile(oss, hdr, currentfile, true);

  // Write it out to the file, clear string stream, close file
  fwrite(oss.str().c_str(), 1, oss.str().length(), xmlfile);
  oss.str("");
  fclose(xmlfile);

  return irc;
}

int PWSFilters::WriteFilterXMLFile(coStringXStream &oss,
                                   const PWSfileHeader &hdr,
                                   const StringX &currentfile,
                                   const bool bWithFormatting)
{
  string str_hdr = GetFilterXMLHeader(currentfile, hdr);
  oss << str_hdr;

  XMLFilterWriterToString put_filterxml(oss, bWithFormatting);
  for_each(this->begin(), this->end(), put_filterxml);

  oss << "</filters>";

  return PWScore::SUCCESS;
}

std::string PWSFilters::GetFilterXMLHeader(const StringX &currentfile,
                                           const PWSfileHeader &hdr)
{
  CUTF8Conv utf8conv;
  const unsigned char *utf8 = NULL;
  size_t utf8Len = 0;

  ostringstream oss;
  stringT cs_tmp;
  time_t time_now;

  time(&time_now);
  const StringX now = PWSUtil::ConvertToDateTimeString(time_now, PWSUtil::TMC_XML);

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
             L"%ls on %ls",
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
                                                     PWSUtil::TMC_XML);
      utf8conv.ToUTF8(wls.c_str(), utf8, utf8Len);
      oss << "WhenLastSaved=\"";
      oss << reinterpret_cast<const char *>(utf8);
      oss << "\"" << endl;
    }

    CUUID huuid(*hdr.m_file_uuid.GetARep(), true); // true to print canonically

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
#if USE_XML_LIBRARY == MSXML
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

  strErrors = fXML.getXMLErrors();
  if (!status) {
    return PWScore::XML_FAILED_VALIDATION;
  }

  validation = false;
  if (strXMLFileName.empty())
    status = fXML.Process(validation, strXMLData, _T(""), strXSDFileName);
  else
    status = fXML.Process(validation, _T(""), strXMLFileName, strXSDFileName);

  strErrors = fXML.getXMLErrors();
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
  stringT cs_rule, cs1, cs2, cs_and, cs_criteria, cs_unit(_T(" B"));
  LoadAString(cs_rule, PWSMatch::GetRule(st_fldata.rule));
  TrimRight(cs_rule, _T("\t"));
  PWSMatch::GetMatchType(st_fldata.mtype,
                         st_fldata.fnum1, st_fldata.fnum2,
                         st_fldata.fdate1, st_fldata.fdate2, st_fldata.fdatetype,
                         st_fldata.fstring.c_str(), st_fldata.fcase,
                         st_fldata.fdca, st_fldata.etype, st_fldata.estatus,
                         st_fldata.funit,
                         st_fldata.rule == PWSMatch::MR_BETWEEN,
                         cs1, cs2);
  switch (st_fldata.mtype) {
    case PWSMatch::MT_PASSWORD:
      if (st_fldata.rule == PWSMatch::MR_EXPIRED) {
        Format(cs_criteria, L"%ls", cs_rule.c_str());
        break;
      } else if (st_fldata.rule == PWSMatch::MR_WILLEXPIRE) {
        Format(cs_criteria, L"%ls %ls", cs_rule.c_str(), cs1.c_str());
        break;
      }
      // Note: purpose drop through to standard 'string' processing
    case PWSMatch::MT_STRING:
      if (st_fldata.rule == PWSMatch::MR_PRESENT ||
          st_fldata.rule == PWSMatch::MR_NOTPRESENT)
        Format(cs_criteria, L"%ls", cs_rule.c_str());
      else {
        stringT cs_delim(L"");
        if (cs1.find(L" ") != stringT::npos)
          cs_delim = L"'";
        Format(cs_criteria, L"%ls %ls%ls%ls %ls", 
               cs_rule.c_str(), cs_delim.c_str(), 
               cs1.c_str(), cs_delim.c_str(), cs2.c_str());
      }
      break;
    case PWSMatch::MT_INTEGER:
    case PWSMatch::MT_ENTRYSIZE:
    case PWSMatch::MT_DATE:
      if (st_fldata.rule == PWSMatch::MR_PRESENT ||
          st_fldata.rule == PWSMatch::MR_NOTPRESENT)
        Format(cs_criteria, L"%ls", cs_rule.c_str());
      else
      if (st_fldata.rule == PWSMatch::MR_BETWEEN) {  // Date or Integer only
        LoadAString(cs_and, IDSC_AND);
        Format(cs_criteria, L"%ls %ls %ls %ls", 
               cs_rule.c_str(), cs1.c_str(), cs_and.c_str(), cs2.c_str());
      } else {
        Format(cs_criteria, L"%ls %ls", cs_rule.c_str(), cs1.c_str());
      }
      if (st_fldata.mtype == PWSMatch::MT_DATE &&
          st_fldata.fdatetype == 1 /* Relative */) {
        stringT cs_temp;
        LoadAString(cs_temp, IDSC_RELATIVE);
        cs_criteria += cs_temp;
      }
      if (st_fldata.mtype == PWSMatch::MT_ENTRYSIZE) {
        switch (st_fldata.funit) {
          case 0:
            cs_unit = _T(" B");
            break;
          case 1:
            cs_unit = _T(" KB");
            break;
          case 2:
            cs_unit = _T(" MB");
            break;
          default:
            ASSERT(0);
            break;
        }
        cs_criteria = cs_criteria + cs_unit;
      }
      break;
    case PWSMatch::MT_PWHIST:
      LoadAString(cs_criteria, IDSC_SEEPWHISTORYFILTERS);
      break;
    case PWSMatch::MT_POLICY:
      LoadAString(cs_criteria, IDSC_SEEPWPOLICYFILTERS);
      break;
    case PWSMatch::MT_ATTACHMENT:
      LoadAString(cs_criteria, IDSC_SEEATTACHMENTFILTERS);
      break;
    case PWSMatch::MT_BOOL:
      cs_criteria = cs_rule;
      break;
    case PWSMatch::MT_DCA:
    case PWSMatch::MT_SHIFTDCA:
      Format(cs_criteria, L"%ls %ls", cs_rule.c_str(), cs1.c_str());
      break;
    case PWSMatch::MT_ENTRYTYPE:
      Format(cs_criteria, L"%ls %ls", cs_rule.c_str(), cs1.c_str());
      break;
    case PWSMatch::MT_ENTRYSTATUS:
      Format(cs_criteria, L"%ls %ls", cs_rule.c_str(), cs1.c_str());
      break;
    case PWSMatch::MT_MEDIATYPE:
      Format(cs_criteria, L"%ls %ls", cs_rule.c_str(), cs1.c_str());
      break;
    default:
      ASSERT(0);
  }
  return cs_criteria;
}

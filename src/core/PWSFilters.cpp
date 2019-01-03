/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
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
  const unsigned char *utf8 = nullptr;
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
    case PWSMatch::MT_MEDIATYPE:
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
    case PWSMatch::MT_ATTACHMENT:
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
  const unsigned char *utf8 = nullptr;
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

      // History, Policy & Attachments
      case FT_PWHIST:
        pszfieldtype = "password_history";
        break;
      case FT_POLICY:
        pszfieldtype = "password_policy";
        break;
      case FT_ATTACHMENT:
        pszfieldtype = "attachment";
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

    if (ft != FT_PWHIST && ft != FT_POLICY && ft != FT_ATTACHMENT) {
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

  for (Flt_citer = filters.vAfldata.begin();
       Flt_citer != filters.vAfldata.end(); Flt_citer++) {
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
      case AT_PRESENT:
        pszfieldtype = "attachment_present";
        break;
      case AT_TITLE:
        pszfieldtype = "attachment_title";
        break;
      case AT_MEDIATYPE:
        pszfieldtype = "attachment_mediatype";
        break;
      case AT_FILENAME:
        pszfieldtype = "attachment_filename";
        break;
      case AT_FILEPATH:
        pszfieldtype = "attachment_filepath";
        break;
      case AT_CTIME:
        pszfieldtype = "attachment_ctime";
        break;
      case AT_FILECTIME:
        pszfieldtype = "attachment_filectime";
        break;
      case AT_FILEMTIME:
        pszfieldtype = "attachment_filemtime";
        break;
      case AT_FILEATIME:
        pszfieldtype = "attachment_fileatime";
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
  void operator()(const pair<const st_Filterkey, st_filters> &p)
  {
    string xml = GetFilterXML(p.second, m_bWithFormatting);
    m_os << xml.c_str();
  }

private:
  XMLFilterWriterToString& operator=(const XMLFilterWriterToString&) = delete;
  coStringXStream &m_os;
  bool m_bWithFormatting;
};

int PWSFilters::WriteFilterXMLFile(const StringX &filename,
                                   const PWSfileHeader &hdr,
                                   const StringX &currentfile)
{
  FILE *xmlfile = pws_os::FOpen(filename.c_str(), _T("wt"));
  if (xmlfile == nullptr)
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
  const unsigned char *utf8 = nullptr;
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
    for_each(filters.vAfldata.begin(), filters.vAfldata.end(),
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
        if (cs1.find(L' ') != stringT::npos)
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

static inline bool group_pred(const vfiltergroup& v1, const vfiltergroup& v2)
{
  return v1.size() < v2.size();
}

PWSFilterManager::PWSFilterManager()
{
  // setup predefined filters:
  {
    LoadAString(m_expirefilter.fname, IDSC_EXPIREPASSWORDS);

    st_FilterRow fr;

    fr.bFilterComplete = true;
    fr.ftype = FT_XTIME;
    fr.mtype = PWSMatch::MT_DATE;
    fr.rule = PWSMatch::MR_NOTEQUAL;
    fr.ltype = LC_OR;

    fr.fdate1 = 0;
    m_expirefilter.vMfldata.push_back(fr);
    m_expirefilter.num_Mactive = (int)m_expirefilter.vMfldata.size();
  }

  {
    LoadAString(m_unsavedfilter.fname, IDSC_NONSAVEDCHANGES);

    st_FilterRow fr;

    fr.bFilterComplete = true;
    fr.ftype = FT_ENTRYSTATUS;
    fr.mtype = PWSMatch::MT_ENTRYSTATUS;
    fr.rule = PWSMatch::MR_IS;
    fr.ltype = LC_OR;

    fr.estatus = CItemData::ES_ADDED;
    m_unsavedfilter.vMfldata.push_back(fr);
    fr.estatus = CItemData::ES_MODIFIED;
    m_unsavedfilter.vMfldata.push_back(fr);
    m_unsavedfilter.num_Mactive = (int)m_unsavedfilter.vMfldata.size();
  }

  {
    LoadAString(m_lastfoundfilter.fname, IDSC_FOUNDENTRIESFILTER);

    // Actual values not needed as matching done by UUID
    st_FilterRow fr;

    fr.bFilterComplete = true;
    fr.ftype = FT_INVALID;
    fr.mtype = PWSMatch::MT_INVALID;
    fr.rule = PWSMatch::MR_INVALID;
    fr.ltype = LC_OR;

    fr.fdate1 = 0;
    m_lastfoundfilter.vMfldata.push_back(fr);
    m_lastfoundfilter.num_Mactive = (int)m_lastfoundfilter.vMfldata.size();
  }

  m_bFindFilterActive = false;
}

void PWSFilterManager::CreateGroups()
{
  int i(0);
  vfiltergroup group;
  vfiltergroups groups;

  // Do the main filters
  for (auto iter = m_currentfilter.vMfldata.begin();
       iter != m_currentfilter.vMfldata.end(); iter++) {
    const st_FilterRow &st_fldata = *iter;

    if (st_fldata.bFilterActive) {
      if (st_fldata.ltype == LC_OR && !group.empty()) {
        // This active filter is in a new group!
        groups.push_back(group);
        group.clear();
      }

      group.push_back(i);

      if (st_fldata.ftype == FT_PWHIST) {
        // Add a number of 'dummy' entries to increase the length of this group
        // Reduce by one as we have already included main FT_PWHIST entry
        for (int j = 0; j < m_currentfilter.num_Hactive - 1; j++) {
          group.push_back(-1);
         }
      } else if (st_fldata.ftype == FT_POLICY) {
        // Add a number of 'dummy' entries to increase the length of this group
        // Reduce by one as we have already included main FT_POLICY entry
        for (int j = 0; j < m_currentfilter.num_Pactive - 1; j++) {
          group.push_back(-1);
        }
      } else if (st_fldata.ftype == FT_ATTACHMENT) {
        // Add a number of 'dummy' entries to increase the length of this group
        // Reduce by one as we have already included main FT_ATTACHMENT entry
        for (int j = 0; j < m_currentfilter.num_Aactive - 1; j++) {
          group.push_back(-1);
        }
      }
    } // st_fldata.bFilterActive
    i++;
  } // iterate over m_currentfilter.vMfldata
  if (!group.empty())
    groups.push_back(group);

  if (!groups.empty()) {
    // Sort them so the smallest group is first
    std::sort(groups.begin(), groups.end(), group_pred);

    // And save
    m_vMflgroups = groups;
  } else
    m_vMflgroups.clear();

  // Now do the History filters
  i = 0;
  group.clear();
  groups.clear();
  for (auto iter = m_currentfilter.vHfldata.begin();
       iter != m_currentfilter.vHfldata.end(); iter++) {
    const st_FilterRow &st_fldata = *iter;

    if (st_fldata.bFilterActive) {
      if (st_fldata.ltype == LC_OR && !group.empty()) {
        // Next active is in a new group!
        groups.push_back(group);
        group.clear();
      }
      group.push_back(i);
    }
    i++;
  }
  if (!group.empty())
    groups.push_back(group);

  if (!groups.empty()) {
    // Sort them so the smallest group is first
    std::sort(groups.begin(), groups.end(), group_pred);

    // And save
    m_vHflgroups = groups;
  } else
    m_vHflgroups.clear();

  // Now do the Policy filters
  i = 0;
  group.clear();
  groups.clear();
  for (auto iter = m_currentfilter.vPfldata.begin();
       iter != m_currentfilter.vPfldata.end(); iter++) {
    const st_FilterRow &st_fldata = *iter;

    if (st_fldata.bFilterActive) {
      if (st_fldata.ltype == LC_OR && !group.empty()) {
        // Next active is in a new group!
        groups.push_back(group);
        group.clear();
      }
      group.push_back(i);
    }
    i++;
  }
  if (!group.empty())
    groups.push_back(group);

  if (!groups.empty()) {
    // Sort them so the smallest group is first
    std::sort(groups.begin(), groups.end(), group_pred);

    // And save
    m_vPflgroups = groups;
  } else
    m_vPflgroups.clear();

  // Now do the Attachment filters
  i = 0;
  group.clear();
  groups.clear();
  for (auto iter = m_currentfilter.vAfldata.begin();
       iter != m_currentfilter.vAfldata.end(); iter++) {
    const st_FilterRow &st_fldata = *iter;

    if (st_fldata.bFilterActive) {
      if (st_fldata.ltype == LC_OR && !group.empty()) {
        // Next active is in a new group!
        groups.push_back(group);
        group.clear();
      }
      group.push_back(i);
    }
    i++;
  }
  if (!group.empty())
    groups.push_back(group);

  if (!groups.empty()) {
    // Sort them so the smallest group is first
    std::sort(groups.begin(), groups.end(), group_pred);

    // And save
    m_vAflgroups = groups;
  } else
    m_vAflgroups.clear();
}

void PWSFilterManager::SetFilterFindEntries(std::vector<pws_os::CUUID> *pvFoundUUIDs)
{
  if (pvFoundUUIDs == nullptr)
    m_vFltrFoundUUIDs.clear();
  else
    m_vFltrFoundUUIDs = *pvFoundUUIDs;
}

bool PWSFilterManager::PassesFiltering(const CItemData &ci, const PWScore &core)
{
  bool thistest_rc;
  bool bValue(false);
  bool bFilterForStatus(false), bFilterForType(false);
  const CItemData *pci;

  if (!m_currentfilter.IsActive())
    return true;

  if (m_bFindFilterActive) {
    return (std::find(m_vFltrFoundUUIDs.begin(), m_vFltrFoundUUIDs.end(),
                      ci.GetUUID()) != m_vFltrFoundUUIDs.end());
  }

  const CItemData::EntryType entrytype = ci.GetEntryType();

  for (auto groups_iter = m_vMflgroups.begin();
       groups_iter != m_vMflgroups.end(); groups_iter++) {
    const vfiltergroup &group = *groups_iter;

    int tests(0);
    bool thisgroup_rc = false;
    for (auto iter = group.begin();
         iter != group.end(); iter++) {
      const int &num = *iter;
      if (num == -1) // Padding to ensure group size is correct for FT_PWHIST & FT_POLICY
        continue;

      const st_FilterRow &st_fldata = m_currentfilter.vMfldata.at(num);
      thistest_rc = false;

      PWSMatch::MatchType mt(PWSMatch::MT_INVALID);
      const FieldType ft = st_fldata.ftype;
      const auto ifunction = (int)st_fldata.rule;

      switch (ft) {
        case FT_GROUPTITLE:
        case FT_GROUP:
        case FT_TITLE:
        case FT_USER:
        case FT_NOTES:
        case FT_URL:
        case FT_AUTOTYPE:
        case FT_RUNCMD:
        case FT_EMAIL:
        case FT_SYMBOLS:
        case FT_POLICYNAME:
          mt = PWSMatch::MT_STRING;
          break;
        case FT_PASSWORD:
          mt = PWSMatch::MT_PASSWORD;
          break;
        case FT_DCA:
          mt = PWSMatch::MT_DCA;
          break;
        case FT_SHIFTDCA:
          mt = PWSMatch::MT_SHIFTDCA;
          break;
        case FT_CTIME:
        case FT_PMTIME:
        case FT_ATIME:
        case FT_XTIME:
        case FT_RMTIME:
          mt = PWSMatch::MT_DATE;
          break;
        case FT_PWHIST:
          mt = PWSMatch::MT_PWHIST;
          break;
        case FT_POLICY:
          mt = PWSMatch::MT_POLICY;
          break;
        case FT_XTIME_INT:
          mt = PWSMatch::MT_INTEGER;
          break;
        case FT_KBSHORTCUT:
          bValue = !ci.GetKBShortcut().empty();
          mt = PWSMatch::MT_BOOL;
          break;
        case FT_UNKNOWNFIELDS:
          bValue = ci.NumberUnknownFields() > 0;
          mt = PWSMatch::MT_BOOL;
          break;
        case FT_PROTECTED:
          bValue = ci.IsProtected();
          mt = PWSMatch::MT_BOOL;
          break;
        case FT_PASSWORDLEN:
          mt = PWSMatch::MT_INTEGER;
          break;
        case FT_ENTRYTYPE:
          mt = PWSMatch::MT_ENTRYTYPE;
          break;
        case FT_ENTRYSTATUS:
          mt = PWSMatch::MT_ENTRYSTATUS;
          break;
        case FT_ENTRYSIZE:
          mt = PWSMatch::MT_ENTRYSIZE;
          break;
        case FT_ATTACHMENT:
          mt = PWSMatch::MT_ATTACHMENT;
          break;
        default:
          ASSERT(0);
      }

      if (ft == FT_ENTRYSTATUS) {
        bFilterForStatus = true;
      }

      if (ft == FT_ENTRYTYPE) {
        bFilterForType = true;
      }

      pci = &ci;

      if (ft == FT_PASSWORD && entrytype == CItemData::ET_ALIAS) {
        pci = core.GetBaseEntry(pci); // This is an alias
      }

      if (entrytype == CItemData::ET_SHORTCUT && !bFilterForStatus && !bFilterForType) {
        // Only include shortcuts if the filter is on the group, title or user fields
        // Note: "GROUPTITLE = 0x00", "GROUP = 0x02", "TITLE = 0x03", "USER = 0x04"
        //   "UUID = 0x01" but no filter is implemented against this field
        // The following is a simple single test rather than testing against every value
        if (ft > FT_USER) {
          pci = core.GetBaseEntry(pci); // This is an shortcut
        }
      }

      switch (mt) {
        case PWSMatch::MT_PASSWORD:
          if (ifunction == PWSMatch::MR_EXPIRED) {
            // Special Password "string" case
            thistest_rc = pci->IsExpired();
            tests++;
            break;
          } else if (ifunction == PWSMatch::MR_WILLEXPIRE) {
            // Special Password "string" case
            thistest_rc = pci->WillExpire(st_fldata.fnum1);
            tests++;
            break;
          }
          // Note: purpose drop through to standard 'string' processing
        case PWSMatch::MT_STRING:
          thistest_rc = pci->Matches(st_fldata.fstring.c_str(), (int)ft,
                                 st_fldata.fcase ? -ifunction : ifunction);
          tests++;
          break;
        case PWSMatch::MT_INTEGER:
          thistest_rc = pci->Matches(st_fldata.fnum1, st_fldata.fnum2,
                                     (int)ft, ifunction);
          tests++;
          break;
        case PWSMatch::MT_DATE:
        {
          time_t t1(st_fldata.fdate1), t2(st_fldata.fdate2);
          if (st_fldata.fdatetype == 1 /* Relative */) {
            time_t now;
            time(&now);
            t1 = now + (st_fldata.fnum1 * 86400);
            if (ifunction == PWSMatch::MR_BETWEEN)
              t2 = now + (st_fldata.fnum2 * 86400);
          }
          thistest_rc = pci->MatchesTime(t1, t2,
                                     (int)ft, ifunction);
          tests++;
          break;
        }
        case PWSMatch::MT_PWHIST:
          if (m_currentfilter.num_Hactive != 0) {
            thistest_rc = PassesPWHFiltering(pci);
            tests++;
          }
          break;
        case PWSMatch::MT_POLICY:
          if (m_currentfilter.num_Pactive != 0) {
            thistest_rc = PassesPWPFiltering(pci);
            tests++;
          }
          break;
        case PWSMatch::MT_BOOL:
          thistest_rc = PWSMatch::Match(bValue, ifunction);
          tests++;
          break;
        case PWSMatch::MT_ENTRYTYPE:
          thistest_rc = pci->Matches(st_fldata.etype, ifunction);
          tests++;
          break;
        case PWSMatch::MT_DCA:
        case PWSMatch::MT_SHIFTDCA:
          thistest_rc = pci->Matches(st_fldata.fdca, ifunction, mt == PWSMatch::MT_SHIFTDCA);
          tests++;
          break;
        case PWSMatch::MT_ENTRYSTATUS:
          thistest_rc = pci->Matches(st_fldata.estatus, ifunction);
          tests++;
          break;
        case PWSMatch::MT_ENTRYSIZE:
          thistest_rc = pci->Matches(st_fldata.fnum1, st_fldata.fnum2,
                                     (int)ft, ifunction);
          tests++;
          break;
        case PWSMatch::MT_ATTACHMENT:
          if (m_currentfilter.num_Aactive != 0) {
            thistest_rc = PassesAttFiltering(pci, core);
            tests++;
          }
          break;
        default:
          ASSERT(0);
      }

      if (tests <= 1)
        thisgroup_rc = thistest_rc;
      else {
        //Within groups, tests are always "AND" connected
        thisgroup_rc = thistest_rc && thisgroup_rc;
      }
    }
    // This group of tests completed -
    //   if 'thisgroup_rc == true', leave now; else go on to next group
    if (thisgroup_rc)
      return true;
  }

  // We finished all the groups and haven't found one that is true - exclude entry.
  return false;
}

bool PWSFilterManager::PassesEmptyGroupFiltering(const StringX &sxGroup)
{
  bool thistest_rc;

  if (!m_currentfilter.IsActive())
    return true;

  for (auto groups_iter = m_vMflgroups.begin();
       groups_iter != m_vMflgroups.end(); groups_iter++) {
    const vfiltergroup &group = *groups_iter;

    int tests(0);
    bool thisgroup_rc = false;
    for (auto iter = group.begin();
         iter != group.end(); iter++) {
      const int &num = *iter;
      if (num == -1) // Padding to ensure group size is correct for FT_PWHIST & FT_POLICY
        continue;

      const st_FilterRow &st_fldata = m_currentfilter.vMfldata.at(num);
      thistest_rc = false;

      const FieldType ft = m_currentfilter.vMfldata[num].ftype;
      const auto ifunction = (int)st_fldata.rule;

      // We are only testing the group value and, as an empty group, it must be present
      if (ft != FT_GROUP || ifunction == PWSMatch::MR_PRESENT || ifunction == PWSMatch::MR_NOTPRESENT) {
        continue;
      }

      thistest_rc = PWSMatch::Match(st_fldata.fstring, sxGroup,
                                    st_fldata.fcase ? -ifunction : ifunction);
      tests++;   

      if (tests <= 1)
        thisgroup_rc = thistest_rc;
      else {
        //Within groups, tests are always "AND" connected
        thisgroup_rc = thistest_rc && thisgroup_rc;
      }
    }

    // This group of tests completed -
    //   if 'thisgroup_rc == true', leave now; else go on to next group
    if (thisgroup_rc)
      return true;
  }

  // We finished all the groups and haven't found one that is true - exclude entry.
  return false;
}

bool PWSFilterManager::PassesPWHFiltering(const CItemData *pci) const
{
  bool thistest_rc, bPresent;
  bool bValue(false);
  int iValue(0);

  size_t pwh_max, err_num;
  PWHistList pwhistlist;

  bool status = CreatePWHistoryList(pci->GetPWHistory(),
                                    pwh_max, err_num,
                                    pwhistlist, PWSUtil::TMC_EXPORT_IMPORT);

  bPresent = pwh_max > 0 || !pwhistlist.empty();

  for (auto group_iter = m_vHflgroups.begin();
       group_iter != m_vHflgroups.end(); group_iter++) {
    const vfiltergroup &group = *group_iter;

    int tests(0);
    bool thisgroup_rc = false;
    for (auto num_iter = group.begin();
         num_iter != group.end(); num_iter++) {
      const int &num = *num_iter;
      if (num == -1) // Padding for FT_PWHIST & FT_POLICY - shouldn't happen here
        continue;

      const st_FilterRow &st_fldata = m_currentfilter.vHfldata.at(num);
      thistest_rc = false;

      PWSMatch::MatchType mt(PWSMatch::MT_INVALID);
      const FieldType ft = st_fldata.ftype;

      switch (ft) {
        case HT_PRESENT:
          bValue = bPresent;
          mt = PWSMatch::MT_BOOL;
          break;
        case HT_ACTIVE:
          bValue = status;
          mt = PWSMatch::MT_BOOL;
          break;
        case HT_NUM:
          iValue = (int)pwhistlist.size();
          mt = PWSMatch::MT_INTEGER;
          break;
        case HT_MAX:
          iValue = (int)pwh_max;
          mt = PWSMatch::MT_INTEGER;
          break;
        case HT_CHANGEDATE:
          mt = PWSMatch::MT_DATE;
          break;
        case HT_PASSWORDS:
          mt = PWSMatch::MT_STRING;
          break;
        default:
          ASSERT(0);
      }

      const auto ifunction = (int)st_fldata.rule;
      switch (mt) {
        case PWSMatch::MT_STRING:
          for (auto pwshe_iter = pwhistlist.begin(); pwshe_iter != pwhistlist.end(); pwshe_iter++) {
            PWHistEntry pwshe = *pwshe_iter;
            thistest_rc = PWSMatch::Match(st_fldata.fstring, pwshe.password,
                                          st_fldata.fcase ? -ifunction : ifunction);
            tests++;
            if (thistest_rc)
              break;
          }
          break;
        case PWSMatch::MT_INTEGER:
          thistest_rc = PWSMatch::Match(st_fldata.fnum1, st_fldata.fnum2,
                                        iValue, ifunction);
          tests++;
          break;
        case PWSMatch::MT_DATE:
          for (auto pwshe_iter = pwhistlist.begin(); pwshe_iter != pwhistlist.end(); pwshe_iter++) {
            const PWHistEntry pwshe = *pwshe_iter;
            // Following throws away hours/min/sec from changetime, for proper date comparison
            time_t changetime = pwshe.changetttdate - (pwshe.changetttdate % (24*60*60));
            thistest_rc = PWSMatch::Match(st_fldata.fdate1, st_fldata.fdate2,
                                          changetime, ifunction);
            tests++;
            if (thistest_rc)
              break;
          }
          break;
        case PWSMatch::MT_BOOL:
          thistest_rc = PWSMatch::Match(bValue, ifunction);
          tests++;
          break;
        default:
          ASSERT(0);
      }

      if (tests <= 1)
        thisgroup_rc = thistest_rc;
      else {
        //Within groups, tests are always "AND" connected
        thisgroup_rc = thistest_rc && thisgroup_rc;
      }
    }
    // This group of tests completed -
    //   if 'thisgroup_rc == true', leave now; else go on to next group
    if (thisgroup_rc)
      return true;
  }

  // We finished all the groups and haven't found one that is true - exclude entry.
  return false;
}

bool PWSFilterManager::PassesPWPFiltering(const CItemData *pci) const
{
  bool thistest_rc, bPresent;
  bool bValue(false);
  int iValue(0);

  PWPolicy pwp;

  pci->GetPWPolicy(pwp);
  bPresent = pwp.flags != 0;

  for (auto group_iter = m_vPflgroups.begin();
       group_iter != m_vPflgroups.end(); group_iter++) {
    const vfiltergroup &group = *group_iter;

    int tests(0);
    bool thisgroup_rc = false;
    for (auto num_iter = group.begin();
         num_iter != group.end(); num_iter++) {
      const int &num = *num_iter;
      if (num == -1) // Padding for FT_PWHIST & FT_POLICY - shouldn't happen here
        continue;

      const st_FilterRow &st_fldata = m_currentfilter.vPfldata.at(num);
      thistest_rc = false;

      PWSMatch::MatchType mt(PWSMatch::MT_INVALID);
      const FieldType ft = st_fldata.ftype;

      switch (ft) {
        case PT_PRESENT:
          bValue = bPresent;
          mt = PWSMatch::MT_BOOL;
          break;
        case PT_LENGTH:
          iValue = pwp.length;
          mt = PWSMatch::MT_INTEGER;
          break;
        case PT_LOWERCASE:
          iValue = pwp.lowerminlength;
          mt = PWSMatch::MT_INTEGER;
          break;
        case PT_UPPERCASE:
          iValue = pwp.upperminlength;
          mt = PWSMatch::MT_INTEGER;
          break;
        case PT_DIGITS:
          iValue = pwp.digitminlength;
          mt = PWSMatch::MT_INTEGER;
          break;
        case PT_SYMBOLS:
          iValue = pwp.symbolminlength;
          mt = PWSMatch::MT_INTEGER;
          break;
        case PT_HEXADECIMAL:
          bValue = (pwp.flags & PWPolicy::UseHexDigits) ==
                       PWPolicy::UseHexDigits;
          mt = PWSMatch::MT_BOOL;
          break;
        case PT_EASYVISION:
          bValue = (pwp.flags & PWPolicy::UseEasyVision) ==
                       PWPolicy::UseEasyVision;
          mt = PWSMatch::MT_BOOL;
          break;
        case PT_PRONOUNCEABLE:
          bValue = (pwp.flags & PWPolicy::MakePronounceable) ==
                       PWPolicy::MakePronounceable;
          mt = PWSMatch::MT_BOOL;
          break;
        default:
          ASSERT(0);
      }

      const auto ifunction = (int)st_fldata.rule;
      switch (mt) {
        case PWSMatch::MT_INTEGER:
          thistest_rc = PWSMatch::Match(st_fldata.fnum1, st_fldata.fnum2,
                                        iValue, ifunction);
          tests++;
          break;
        case PWSMatch::MT_BOOL:
          thistest_rc = PWSMatch::Match(bValue, ifunction);
          tests++;
          break;
        default:
          ASSERT(0);
      }

      if (tests <= 1)
        thisgroup_rc = thistest_rc;
      else {
        //Within groups, tests are always "AND" connected
        thisgroup_rc = thistest_rc && thisgroup_rc;
      }
    }
    // This group of tests completed -
    //   if 'thisgroup_rc == true', leave now; else go on to next group
    if (thisgroup_rc)
      return true;
  }

  // We finished all the groups and haven't found one that is true - exclude entry.
  return false;
}

bool PWSFilterManager::PassesAttFiltering(const CItemData *pci, const PWScore &core) const
{
  bool thistest_rc;
  const bool bPresent = pci->HasAttRef();
  bool bValue(false);

  
  for (auto group_iter = m_vAflgroups.begin();
       group_iter != m_vAflgroups.end(); group_iter++) {
    const vfiltergroup &group = *group_iter;

    int tests(0);
    bool thisgroup_rc = false;
    for (auto num_iter = group.begin();
      num_iter != group.end(); num_iter++) {
      const int &num = *num_iter;
      if (num == -1) // Padding for FT_PWHIST & FT_POLICY - shouldn't happen here
        continue;

      const st_FilterRow &st_fldata = m_currentfilter.vAfldata.at(num);
      thistest_rc = false;

      PWSMatch::MatchType mt(PWSMatch::MT_INVALID);
      const FieldType ft = st_fldata.ftype;

      switch (ft) {
        case AT_PRESENT:
          bValue = bPresent;
          mt = PWSMatch::MT_BOOL;
          break;
        case AT_TITLE:
        case AT_FILENAME:
        case AT_FILEPATH:
        case AT_MEDIATYPE:
          mt = PWSMatch::MT_STRING;
          break;
        case AT_CTIME:
        case AT_FILECTIME:
        case AT_FILEMTIME:
        case AT_FILEATIME:
          mt = PWSMatch::MT_DATE;
          break;
        default:
          ASSERT(0);
      }

      const auto ifunction = (int)st_fldata.rule;
      switch (mt) {
        case PWSMatch::MT_BOOL:
          thistest_rc = PWSMatch::Match(bValue, ifunction);
          tests++;
          break;
        case PWSMatch::MT_STRING:
          if (bPresent) {
            const CItemAtt &att = core.GetAtt(pci->GetAttUUID());

            thistest_rc = att.Matches(st_fldata.fstring.c_str(), (int)ft,
              st_fldata.fcase ? -ifunction : ifunction);
          } else {
            thistest_rc = false;
          }
          tests++;
          break;
        case PWSMatch::MT_DATE:
          if (bPresent) {
            const CItemAtt &att = core.GetAtt(pci->GetAttUUID());

            time_t t1(st_fldata.fdate1), t2(st_fldata.fdate2);
            if (st_fldata.fdatetype == 1 /* Relative */) {
              time_t now;
              time(&now);
              t1 = now + (st_fldata.fnum1 * 86400);
              if (ifunction == PWSMatch::MR_BETWEEN)
                t2 = now + (st_fldata.fnum2 * 86400);
            }
            thistest_rc = att.Matches(t1, t2, (int)ft, ifunction);
          } else {
            thistest_rc = false;
          }
          tests++;
          break;
        default:
          ASSERT(0);
      }

      if (tests <= 1)
        thisgroup_rc = thistest_rc;
      else {
        //Within groups, tests are always "AND" connected
        thisgroup_rc = thistest_rc && thisgroup_rc;
      }
    }
    // This group of tests completed -
    //   if 'thisgroup_rc == true', leave now; else go on to next group
    if (thisgroup_rc)
      return true;
  }

  // We finished all the groups and haven't found one that is true - exclude entry.
  return false;
}

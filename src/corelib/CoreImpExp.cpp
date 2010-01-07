/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// file CoreImpExp.cpp
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------
// Import/Export PWScore member functions
//-----------------------------------------------------------------
#include "PWScore.h"
#include "corelib.h"
#include "PWSprefs.h"
#include "Util.h"
#include "UUIDGen.h"
#include "SysInfo.h"
#include "UTF8Conv.h"
#include "Report.h"
#include "VerifyFormat.h"
#include "PWSfileV3.h" // XXX cleanup with dynamic_cast
#include "StringXStream.h"

#include "XML/XMLDefs.h"  // Required if testing "USE_XML_LIBRARY"

#include "os/typedefs.h"
#include "os/dir.h"
#include "os/debug.h"
#include "os/file.h"
#include "os/mem.h"

#if USE_XML_LIBRARY == EXPAT
#include "XML/Expat/EFileXMLProcessor.h"
#elif USE_XML_LIBRARY == MSXML
#include "XML/MSXML/MFileXMLProcessor.h"
#elif USE_XML_LIBRARY == XERCES
#include "XML/Xerces/XFileXMLProcessor.h"
#endif

#include <fstream> // for WritePlaintextFile
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <algorithm>
#include <set>

using namespace std;

// hide w_char/char differences where possible:
#ifdef UNICODE
typedef std::wifstream ifstreamT;
typedef std::wofstream ofstreamT;
#else
typedef std::ifstream ifstreamT;
typedef std::ofstream ofstreamT;
#endif
typedef std::vector<stringT>::iterator viter;

struct ExportTester {
  ExportTester(const stringT &subgroup_name,
               const int &subgroup_object, const int &subgroup_function)
  :  m_subgroup_name(subgroup_name), m_subgroup_object(subgroup_object),
  m_subgroup_function(subgroup_function)
  {}

  // operator for ItemList
  bool operator()(pair<CUUIDGen, CItemData> p)
  {return operator()(p.second);}

  // operator for OrderedItemList
  bool operator()(const CItemData &item)
  {
    return item.Matches(m_subgroup_name,
                        m_subgroup_object, m_subgroup_function);
  }

private:
  const stringT &m_subgroup_name;
  const int &m_subgroup_object;
  const int &m_subgroup_function;
};

int PWScore::TestForExport(const stringT &subgroup_name,
                           const int &subgroup_object,
                           const int &subgroup_function,
                           const OrderedItemList *il)
{
  // Check if any pass restricting criteria
  if (!subgroup_name.empty()) {
    bool bAnyMatch(false);
    if (il != NULL) {
      if (find_if(il->begin(), il->end(),
                  ExportTester(subgroup_name,
                               subgroup_object,
                               subgroup_function)) != il->end())
        bAnyMatch = true;
    } else {
      if (find_if(m_pwlist.begin(), m_pwlist.end(),
                  ExportTester(subgroup_name, 
                               subgroup_object, 
                               subgroup_function)) != m_pwlist.end())
        bAnyMatch = true;
    }

    if (!bAnyMatch)
      return NO_ENTRIES_EXPORTED;
  }
  return SUCCESS;
}

inline bool bittest(const CItemData::FieldBits &bsFields,
                    const CItemData::FieldType &ft,
                    const bool &bIncluded)
{
  return bsFields.test(ft) ? bIncluded : !bIncluded;
}

StringX PWScore::BuildHeader(const CItemData::FieldBits &bsFields, const bool bIncluded)
{
  // User chose fields, build custom header
  // Header fields MUST be in the same order as actual fields written
  // See CItemData::GetPlaintext for TextExport
  StringX hdr(_T("")), cs_temp;
  if (bittest(bsFields, CItemData::GROUP, bIncluded) && 
      bittest(bsFields, CItemData::TITLE, bIncluded)) {
    LoadAString(cs_temp, IDSC_EXPHDRGROUPTITLE);
    hdr += cs_temp;
  } else if (bittest(bsFields, CItemData::GROUP, bIncluded)) {
    LoadAString(cs_temp, IDSC_EXPHDRGROUP);
    hdr += cs_temp;
  } else if (bittest(bsFields, CItemData::TITLE, bIncluded)) {
    LoadAString(cs_temp, IDSC_EXPHDRTITLE);
    hdr += cs_temp;
  }
  if (bittest(bsFields, CItemData::USER, bIncluded)) {
    LoadAString(cs_temp, IDSC_EXPHDRUSERNAME);
    hdr += cs_temp;
  }
  if (bittest(bsFields, CItemData::PASSWORD, bIncluded)) {
    LoadAString(cs_temp, IDSC_EXPHDRPASSWORD);
    hdr += cs_temp;
  }
  if (bittest(bsFields, CItemData::URL, bIncluded)) {
    LoadAString(cs_temp, IDSC_EXPHDRURL);
    hdr += cs_temp;
  }
  if (bittest(bsFields, CItemData::AUTOTYPE, bIncluded)) {
    LoadAString(cs_temp, IDSC_EXPHDRAUTOTYPE);
    hdr += cs_temp;
  }
  if (bittest(bsFields, CItemData::CTIME, bIncluded)) {
    LoadAString(cs_temp, IDSC_EXPHDRCTIME);
    hdr += cs_temp;
  }
  if (bittest(bsFields, CItemData::PMTIME, bIncluded)) {
    LoadAString(cs_temp, IDSC_EXPHDRPMTIME);
    hdr += cs_temp;
  }
  if (bittest(bsFields, CItemData::ATIME, bIncluded)) {
    LoadAString(cs_temp, IDSC_EXPHDRATIME);
    hdr += cs_temp;
  }
  if (bittest(bsFields, CItemData::XTIME, bIncluded)) {
    LoadAString(cs_temp, IDSC_EXPHDRXTIME);
    hdr += cs_temp;
  }
  if (bittest(bsFields, CItemData::XTIME_INT, bIncluded)) {
    LoadAString(cs_temp, IDSC_EXPHDRXTIMEINT);
    hdr += cs_temp;
  }
  if (bittest(bsFields, CItemData::RMTIME, bIncluded)) {
    LoadAString(cs_temp, IDSC_EXPHDRRMTIME);
    hdr += cs_temp;
  }
  if (bittest(bsFields, CItemData::POLICY, bIncluded)) {
    LoadAString(cs_temp, IDSC_EXPHDRPWPOLICY);
    hdr += cs_temp;
  }
  if (bittest(bsFields, CItemData::PWHIST, bIncluded)) {
    LoadAString(cs_temp, IDSC_EXPHDRPWHISTORY);
    hdr += cs_temp;
  }
  if (bittest(bsFields, CItemData::RUNCMD, bIncluded)) {
    LoadAString(cs_temp, IDSC_EXPHDRRUNCOMMAND);
    hdr += cs_temp;
  }
  if (bittest(bsFields, CItemData::DCA, bIncluded)) {
    LoadAString(cs_temp, IDSC_EXPHDRDCA);
    hdr += cs_temp;
  }
  if (bittest(bsFields, CItemData::EMAIL, bIncluded)) {
    LoadAString(cs_temp, IDSC_EXPHDREMAIL);
    hdr += cs_temp;
  }
  if (bittest(bsFields, CItemData::NOTES, bIncluded)) {
    LoadAString(cs_temp, IDSC_EXPHDRNOTES);
    hdr += cs_temp;
  }
  int hdr_len = hdr.length();
  if (hdr_len > 0) {
    if (hdr[hdr.length() - 1] == _T('\t')) {
      hdr_len--;
      hdr = hdr.substr(0, hdr_len);
    }
  }
  return hdr;
}

struct PutText {
  PutText(const stringT &subgroup_name,
          const int &subgroup_object, const int &subgroup_function,
          const CItemData::FieldBits &bsFields,
          const TCHAR &delimiter, ofstream &ofs, PWScore *pcore) :
  m_subgroup_name(subgroup_name), m_subgroup_object(subgroup_object),
  m_subgroup_function(subgroup_function), m_bsFields(bsFields),
  m_delimiter(delimiter), m_ofs(ofs), m_pcore(pcore)
  {}

  // operator for ItemList
  void operator()(pair<CUUIDGen, CItemData> p)
  {operator()(p.second);}

  // operator for OrderedItemList
  void operator()(const CItemData &item)
  {
    if (m_subgroup_name.empty() || 
        item.Matches(m_subgroup_name, m_subgroup_object,
        m_subgroup_function)) {
      CItemData *pcibase(NULL);
      if (item.IsAlias()) {
        uuid_array_t base_uuid, item_uuid;
        item.GetUUID(item_uuid);
        m_pcore->GetAliasBaseUUID(item_uuid, base_uuid);
        ItemListIter iter;
        iter = m_pcore->Find(base_uuid);
        if (iter !=  m_pcore->GetEntryEndIter())
          pcibase = &iter->second;
      }
      if (item.IsShortcut()) {
        uuid_array_t base_uuid, item_uuid;
        item.GetUUID(item_uuid);
        m_pcore->GetShortcutBaseUUID(item_uuid, base_uuid);
        ItemListIter iter;
        iter = m_pcore->Find(base_uuid);
        if (iter !=  m_pcore->GetEntryEndIter())
          pcibase = &iter->second;
      }
      const StringX line = item.GetPlaintext(TCHAR('\t'),
                                             m_bsFields, m_delimiter, pcibase);
      if (!line.empty()) {
        CUTF8Conv conv; // can't make a member, as no copy c'tor!
        const unsigned char *utf8;
        int utf8Len;
        if (conv.ToUTF8(line, utf8, utf8Len)) {
          m_ofs.write(reinterpret_cast<const char *>(utf8), utf8Len);
          m_ofs << endl;
        } else {
          ASSERT(0);
        }
      } // !line.IsEmpty()
    } // we've a match
  }

private:
  const stringT &m_subgroup_name;
  const int &m_subgroup_object;
  const int &m_subgroup_function;
  const CItemData::FieldBits &m_bsFields;
  const TCHAR &m_delimiter;
  ofstream &m_ofs;
  PWScore *m_pcore;
};

int PWScore::WritePlaintextFile(const StringX &filename,
                                const CItemData::FieldBits &bsFields,
                                const stringT &subgroup_name,
                                const int &subgroup_object,
                                const int &subgroup_function,
                                const TCHAR &delimiter, const OrderedItemList *il)
{
  // Check if anything to do! 
  if (bsFields.count() == 0)
    return NO_ENTRIES_EXPORTED;

  // Although the MFC UI prevents the user selecting export of an
  // empty database, other UIs might not, so:
  if ((il != NULL && il->size() == 0) ||
      (il == NULL && m_pwlist.size() == 0))
    return NO_ENTRIES_EXPORTED;
 
  CUTF8Conv conv;
#ifdef UNICODE
  int fnamelen;
  const unsigned char *fname = NULL;
  conv.ToUTF8(filename, fname, fnamelen); 
#else
  const char *fname = filename.c_str();
#endif

  ofstream ofs(reinterpret_cast<const char *>(fname));

  if (!ofs)
    return CANT_OPEN_FILE;

  StringX hdr(_T(""));
  const unsigned char *utf8 = NULL;
  int utf8Len = 0;

  if (bsFields.count() == bsFields.size()) {
    // all fields to be exported, use pre-built header
    StringX exphdr;
    LoadAString(exphdr, IDSC_EXPORTHEADER);
    conv.ToUTF8(exphdr.c_str(), utf8, utf8Len);
    ofs.write(reinterpret_cast<const char *>(utf8), utf8Len);
    ofs << endl;
  } else {
    hdr = BuildHeader(bsFields, true);
    conv.ToUTF8(hdr, utf8, utf8Len);
    ofs.write(reinterpret_cast<const char *>(utf8), utf8Len);
    ofs << endl;
  }

  PutText put_text(subgroup_name, subgroup_object, subgroup_function,
                   bsFields, delimiter, ofs, this);

  if (il != NULL) {
    for_each(il->begin(), il->end(), put_text);
  } else {
    for_each(m_pwlist.begin(), m_pwlist.end(), put_text);
  }

  ofs.close();

  return SUCCESS;
}

struct XMLRecordWriter {
  XMLRecordWriter(const stringT &subgroup_name,
                  const int subgroup_object, const int subgroup_function,
                  const CItemData::FieldBits &bsFields,
                  TCHAR delimiter, ofstream &ofs, PWScore *pcore) :
  m_subgroup_name(subgroup_name), m_subgroup_object(subgroup_object),
  m_subgroup_function(subgroup_function), m_bsFields(bsFields),
  m_delimiter(delimiter), m_of(ofs), m_id(0), m_pcore(pcore)
  {}

  // operator for ItemList
  void operator()(pair<CUUIDGen, CItemData> p)
  {operator()(p.second);}

  // operator for OrderedItemList
  void operator()(const CItemData &item)
  {
    m_id++;
    if (m_subgroup_name.empty() ||
        item.Matches(m_subgroup_name,
                     m_subgroup_object, m_subgroup_function)) {
      CItemData *pcibase(NULL);
      bool bforce_normal_entry(false);
      if (item.IsNormal()) {
        //  Check password doesn't incorrectly imply alias or shortcut entry
        StringX pswd;
        pswd = item.GetPassword();
        int num_colons = Replace(pswd, _T(':'), _T(';')) + 1;
        if ((pswd[0] == _T('[')) &&
            (pswd[pswd.length() - 1] == _T(']')) &&
            num_colons <= 3) {
          bforce_normal_entry = true;
        }
      }
      if (item.IsAlias()) {
        uuid_array_t base_uuid, item_uuid;
        item.GetUUID(item_uuid);
        m_pcore->GetAliasBaseUUID(item_uuid, base_uuid);
        ItemListIter iter;
        iter = m_pcore->Find(base_uuid);
        if (iter != m_pcore->GetEntryEndIter())
          pcibase = &iter->second;
      }
      if (item.IsShortcut()) {
        uuid_array_t base_uuid, item_uuid;
        item.GetUUID(item_uuid);
        m_pcore->GetShortcutBaseUUID(item_uuid, base_uuid);
        ItemListIter iter;
        iter = m_pcore->Find(base_uuid);
        if (iter != m_pcore->GetEntryEndIter())
          pcibase = &iter->second;
      }
      string xml = item.GetXML(m_id, m_bsFields, m_delimiter, pcibase, bforce_normal_entry);
      m_of.write(xml.c_str(),
                 static_cast<streamsize>(xml.length()));
    }
  }

private:
  const stringT &m_subgroup_name;
  const int m_subgroup_object;
  const int m_subgroup_function;
  const CItemData::FieldBits &m_bsFields;
  TCHAR m_delimiter;
  ofstream &m_of;
  unsigned m_id;
  PWScore *m_pcore;
};

int PWScore::WriteXMLFile(const StringX &filename,
                          const CItemData::FieldBits &bsFields,
                          const stringT &subgroup_name,
                          const int &subgroup_object, const int &subgroup_function,
                          const TCHAR &delimiter, const OrderedItemList *il,
                          const bool &bFilterActive)
{
  // Although the MFC UI prevents the user selecting export of an
  // empty database, other UIs might not, so:
  if ((il != NULL && il->size() == 0) ||
      (il == NULL && m_pwlist.size() == 0))
    return NO_ENTRIES_EXPORTED;

#ifdef UNICODE
  const unsigned char *fname = NULL;
  CUTF8Conv conv;
  int fnamelen;
  conv.ToUTF8(filename, fname, fnamelen); 
#else
  const char *fname = filename.c_str();
#endif

  ofstream ofs(reinterpret_cast<const char *>(fname));

  if (!ofs)
    return CANT_OPEN_FILE;

  oStringXStream oss_xml;
  StringX pwh, tmp;
  stringT cs_temp;
  time_t time_now;

  time(&time_now);
  const StringX now = PWSUtil::ConvertToDateTimeString(time_now, TMC_XML);

  ofs << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
  ofs << "<?xml-stylesheet type=\"text/xsl\" href=\"pwsafe.xsl\"?>" << endl;
  ofs << endl;
  ofs << "<passwordsafe" << endl;
  tmp = m_currfile;
  Replace(tmp, StringX(_T("&")), StringX(_T("&amp;")));

  CUTF8Conv utf8conv;
  const unsigned char *utf8 = NULL;
  int utf8Len = 0;
  StringX delStr;
  delStr += delimiter;
  utf8conv.ToUTF8(delStr, utf8, utf8Len);
  ofs << "delimiter=\"";
  ofs.write(reinterpret_cast<const char *>(utf8), utf8Len);
  ofs << "\"" << endl;
  utf8conv.ToUTF8(tmp, utf8, utf8Len);
  ofs << "Database=\"";
  ofs.write(reinterpret_cast<const char *>(utf8), utf8Len);
  ofs << "\"" << endl;
  utf8conv.ToUTF8(now, utf8, utf8Len);
  ofs << "ExportTimeStamp=\"";
  ofs.write(reinterpret_cast<const char *>(utf8), utf8Len);
  ofs << "\"" << endl;
  ofs << "FromDatabaseFormat=\"";
  ostringstream osv; // take advantage of UTF-8 == ascii for version string
  osv << m_hdr.m_nCurrentMajorVersion
      << "." << setw(2) << setfill('0')
      << m_hdr.m_nCurrentMinorVersion;
  ofs.write(osv.str().c_str(), osv.str().length());
  ofs << "\"" << endl;
  if (!m_hdr.m_lastsavedby.empty() || !m_hdr.m_lastsavedon.empty()) {
    oStringXStream oss;
    oss << m_hdr.m_lastsavedby << _T(" on ") << m_hdr.m_lastsavedon;
    utf8conv.ToUTF8(oss.str(), utf8, utf8Len);
    ofs << "WhoSaved=\"";
    ofs.write(reinterpret_cast<const char *>(utf8), utf8Len);
    ofs << "\"" << endl;
  }
  if (!m_hdr.m_whatlastsaved.empty()) {
    utf8conv.ToUTF8(m_hdr.m_whatlastsaved, utf8, utf8Len);
    ofs << "WhatSaved=\"";
    ofs.write(reinterpret_cast<const char *>(utf8), utf8Len);
    ofs << "\"" << endl;
  }
  if (m_hdr.m_whenlastsaved != 0) {
    StringX wls = PWSUtil::ConvertToDateTimeString(m_hdr.m_whenlastsaved,
                                                   TMC_XML);
    utf8conv.ToUTF8(wls.c_str(), utf8, utf8Len);
    ofs << "WhenLastSaved=\"";
    ofs.write(reinterpret_cast<const char *>(utf8), utf8Len);
    ofs << "\"" << endl;
  }

  CUUIDGen huuid(m_hdr.m_file_uuid_array, true); // true to print canoncally

  ofs << "Database_uuid=\"" << huuid << "\"" << endl;
  ofs << "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"" << endl;
  ofs << "xsi:noNamespaceSchemaLocation=\"pwsafe.xsd\">" << endl;
  ofs << endl;

  if (m_hdr.m_nITER > MIN_HASH_ITERATIONS) {
    ofs << "\t<NumberHashIterations>" << m_hdr.m_nITER << "</NumberHashIterations>";
    ofs << endl;
  }

  // write out preferences stored in database
  LoadAString(cs_temp, IDSC_XMLEXP_PREFERENCES);
  oss_xml << _T(" <!-- ") << cs_temp << _T(" --> ");
  conv.ToUTF8(oss_xml.str(), utf8, utf8Len);
  ofs.write(reinterpret_cast<const char *>(utf8), utf8Len);
  ofs << endl;
  oss_xml.str(_T(""));  // Clear buffer for next user

  stringT prefs = PWSprefs::GetInstance()->GetXMLPreferences();
  utf8conv.ToUTF8(prefs.c_str(), utf8, utf8Len);
  ofs.write(reinterpret_cast<const char *>(utf8), utf8Len);

  if (m_UHFL.size() > 0) {
    ofs << "\t<unknownheaderfields>" << endl;
    UnknownFieldList::const_iterator vi_IterUHFE;
    for (vi_IterUHFE = m_UHFL.begin();
         vi_IterUHFE != m_UHFL.end();
         vi_IterUHFE++) {
      UnknownFieldEntry unkhfe = *vi_IterUHFE;
      if (unkhfe.st_length == 0)
        continue;

      unsigned char *pmem = unkhfe.uc_pUField;

      // UNK_HEX_REP will represent unknown values
      // as hexadecimal, rather than base64 encoding.
      // Easier to debug.
#ifndef UNK_HEX_REP
      tmp = PWSUtil::Base64Encode(pmem, unkhfe.st_length).c_str();
#else
      tmp.clear();
      unsigned char c;
      for (unsigned int i = 0; i < unkhfe.st_length; i++) {
        c = *pmem++;
        Format(cs_temp, _T("%02x"), c);
        tmp += cs_temp;
      }
#endif
      utf8conv.ToUTF8(tmp, utf8, utf8Len);
      ofs << "\t\t<field ftype=\"" << int(unkhfe.uc_Type) << "\">";
      ofs.write(reinterpret_cast<const char *>(utf8), utf8Len);
      ofs << "</field>" << endl;
    }
    ofs << "\t</unknownheaderfields>" << endl;  
  }

  bool bStartComment(false);
  if (bFilterActive) {
    if (!bStartComment) {
      bStartComment = true;
      ofs << " <!-- " << endl;
    }
    LoadAString(cs_temp, IDSC_XMLEXP_FILTERACTIVE);
    oss_xml << _T("     ") << cs_temp;
    conv.ToUTF8(oss_xml.str(), utf8, utf8Len);
    ofs.write(reinterpret_cast<const char *>(utf8), utf8Len);
    ofs << endl;
    oss_xml.str(_T(""));  // Clear buffer for next user
  }

  if (!subgroup_name.empty() || bsFields.count() != bsFields.size()) {
    if (!bStartComment) {
      bStartComment = true;
      ofs << " <!-- " << endl;
    }
    // Some restrictions - put in a comment to that effect
    LoadAString(cs_temp, IDSC_XMLEXP_FLDRESTRICT);
    oss_xml << _T("     ") << cs_temp;
    conv.ToUTF8(oss_xml.str(), utf8, utf8Len);
    ofs.write(reinterpret_cast<const char *>(utf8), utf8Len);
    ofs << endl;
    oss_xml.str(_T(""));  // Clear buffer for next user

    if (!subgroup_name.empty()) {
      stringT cs_object, cs_function, cs_case(_T(""));
      int iObject(IDSC_UNKNOWNOBJECT);
      switch (subgroup_object) {
        case CItemData::GROUP:
          iObject = IDSC_EXPHDRGROUP;
          break;
        case CItemData::GROUPTITLE:
          iObject = IDSC_EXPHDRGROUPTITLE;
          break;
        case CItemData::TITLE:
          iObject = IDSC_EXPHDRTITLE;
          break;
        case CItemData::USER:
          iObject = IDSC_EXPHDRUSERNAME;
          break;
        case CItemData::URL:
          iObject = IDSC_EXPHDRURL;
          break;
        case CItemData::NOTES:
          iObject = IDSC_EXPHDRNOTES;
          break;
        default:
          ASSERT(0);
      }
      LoadAString(cs_object, iObject);
      int object_len = cs_object.length();
      if (cs_object[object_len - 1] == _T('\t')) {
        object_len--;
        cs_object = cs_object.substr(0, object_len);
      }

      int iCase(IDSC_CASE_INSENSITIVE);
      if (subgroup_function < 0) {
        iCase = IDSC_CASE_SENSITIVE;
      }
      LoadAString(cs_case, iCase);

      int iFunction(IDSC_UNKNOWNFUNCTION);
      switch (subgroup_function) {
        case  PWSMatch::MR_EQUALS:
        case -PWSMatch::MR_EQUALS:
          iFunction = IDSC_EQUALS;
          break;
        case  PWSMatch::MR_NOTEQUAL:
        case -PWSMatch::MR_NOTEQUAL:
          iFunction = IDSC_DOESNOTEQUAL;
          break;
        case  PWSMatch::MR_BEGINS:
        case -PWSMatch::MR_BEGINS:
          iFunction = IDSC_BEGINSWITH;
          break;
        case  PWSMatch::MR_NOTBEGIN:
        case -PWSMatch::MR_NOTBEGIN:
          iFunction = IDSC_DOESNOTBEGINSWITH;
          break;
        case  PWSMatch::MR_ENDS:
        case -PWSMatch::MR_ENDS:
          iFunction = IDSC_ENDSWITH;
          break;
        case  PWSMatch::MR_NOTEND:
        case -PWSMatch::MR_NOTEND:
          iFunction = IDSC_DOESNOTENDWITH;
          break;
        case  PWSMatch::MR_CONTAINS:
        case -PWSMatch::MR_CONTAINS:
          iFunction = IDSC_CONTAINS;
          break;
        case  PWSMatch::MR_NOTCONTAIN:
        case -PWSMatch::MR_NOTCONTAIN:
          iFunction = IDSC_DOESNOTCONTAIN;
          break;
        default:
          ASSERT(0);
      }
      LoadAString(cs_function, iFunction);

      LoadAString(cs_temp, IDSC_XMLEXP_SUBSETACTIVE);
      if (!bStartComment) {
        bStartComment = true;
        ofs << " <!-- " << endl;
      }
      oss_xml << _T("     ") << cs_temp;
      conv.ToUTF8(oss_xml.str(), utf8, utf8Len);
      ofs.write(reinterpret_cast<const char *>(utf8), utf8Len);
      ofs << endl;
      oss_xml.str(_T(""));  // Clear buffer for next user

      oss_xml << _T("     ") << _T(" '") << cs_object     << _T("' ")
                             << _T(" '") << cs_function   << _T("' ")
                             << _T(" '") << subgroup_name << _T("' ")
                             << cs_case;
      utf8conv.ToUTF8(oss_xml.str(), utf8, utf8Len);
      ofs.write(reinterpret_cast<const char *>(utf8), utf8Len);
      ofs << endl;
      oss_xml.str(_T(""));  // Clear buffer for next user
    }

    if (bsFields.count() != bsFields.size()) {
      if (!bStartComment) {
        bStartComment = true;
        ofs << " <!-- " << endl;
      }
      StringX hdr;
      LoadAString(cs_temp, IDSC_XMLEXP_SUBSETFIELDS);
      oss_xml << _T("     ") << cs_temp;
      conv.ToUTF8(oss_xml.str(), utf8, utf8Len);
      ofs.write(reinterpret_cast<const char *>(utf8), utf8Len);
      ofs << endl;
      oss_xml.str(_T(""));  // Clear buffer for next user

      hdr = BuildHeader(bsFields, false);
      int found = hdr.find(_T("\t"));
	    while (found >= 0) {
		    if (found >= 0) {
			    hdr.replace(found, 1, _T(", "));
		    }
		    found = hdr.find(_T("\t"));
	    }
      hdr = _T("     ") + hdr;
      conv.ToUTF8(hdr, utf8, utf8Len);
      ofs.write(reinterpret_cast<const char *>(utf8), utf8Len);
      ofs << endl;
    }
  }

  if (bStartComment) {
    bStartComment = false;
    ofs << " --> " << endl;
  }
  ofs << endl;

  XMLRecordWriter put_xml(subgroup_name, subgroup_object, subgroup_function,
                          bsFields, delimiter, ofs, this);

  if (il != NULL) {
    for_each(il->begin(), il->end(), put_xml);
  } else {
    for_each(m_pwlist.begin(), m_pwlist.end(), put_xml);
  }

  ofs << "</passwordsafe>" << endl;
  ofs.close();

 return SUCCESS;
}

#if !defined(USE_XML_LIBRARY) || (!defined(_WIN32) && USE_XML_LIBRARY == MSXML)
// Don't support importing XML on non-Windows platforms using Microsoft XML libraries
int PWScore::ImportXMLFile(const stringT &, const stringT &,
                           const stringT &, const bool &,
                           stringT &, int &, int &,
                           bool &, bool &, 
                           CReport &)
{
  return UNIMPLEMENTED;
}
#else
int PWScore::ImportXMLFile(const stringT &ImportedPrefix, const stringT &strXMLFileName,
                           const stringT &strXSDFileName, const bool &bImportPSWDsOnly,
                           stringT &strErrors, int &numValidated, int &numImported,
                           bool &bBadUnknownFileFields, bool &bBadUnknownRecordFields,
                           CReport &rpt)
{
  UUIDList possible_aliases, possible_shortcuts;
  MultiCommands *pmulticmds = MultiCommands::Create(this);

#if   USE_XML_LIBRARY == EXPAT
  EFileXMLProcessor iXML(this, &possible_aliases, &possible_shortcuts, pmulticmds);
#elif USE_XML_LIBRARY == MSXML
  MFileXMLProcessor iXML(this, &possible_aliases, &possible_shortcuts, pmulticmds);
#elif USE_XML_LIBRARY == XERCES
  XFileXMLProcessor iXML(this, &possible_aliases, &possible_shortcuts, pmulticmds);
#endif

  bool status, validation;
  int nITER(0);
  int nRecordsWithUnknownFields;
  UnknownFieldList uhfl;
  bool bEmptyDB = (GetNumEntries() == 0);

  strErrors = _T("");

  // Expat is not a validating parser - but we now do it ourselves!
  validation = true;
  status = iXML.Process(validation, ImportedPrefix, strXMLFileName,
                        strXSDFileName, bImportPSWDsOnly, 
                        nITER, nRecordsWithUnknownFields, uhfl);
  strErrors = iXML.getResultText();
  if (!status) {
    return XML_FAILED_VALIDATION;
  }
  numValidated = iXML.getNumEntriesValidated();

  validation = false;
  status = iXML.Process(validation, ImportedPrefix, strXMLFileName,
                        strXSDFileName, bImportPSWDsOnly,
                        nITER, nRecordsWithUnknownFields, uhfl);
  strErrors = iXML.getResultText();
  if (!status) {
    delete pmulticmds;
    return XML_FAILED_IMPORT;
  }

  numImported = iXML.getNnumEntriesImported();
  bBadUnknownFileFields = iXML.getIfDatabaseHeaderErrors();
  bBadUnknownRecordFields = iXML.getIfRecordHeaderErrors();
  m_nRecordsWithUnknownFields += nRecordsWithUnknownFields;
  // Only add header unknown fields or change number of iterations
  // if the database was empty to start with
  if (bEmptyDB) {
    m_hdr.m_nITER = nITER;
    if (uhfl.empty())
      m_UHFL.clear();
    else {
      m_UHFL = uhfl;
    }
  }
  uhfl.clear();

  Command *pcmdA = AddDependentEntriesCommand::Create(this, possible_aliases, &rpt, 
                                                      CItemData::ET_ALIAS,
                                                      CItemData::PASSWORD);
  pcmdA->SetNoGUINotify();
  pmulticmds->Add(pcmdA);
  Command *pcmdS = AddDependentEntriesCommand::Create(this, possible_shortcuts, &rpt, 
                                                      CItemData::ET_SHORTCUT,
                                                      CItemData::PASSWORD);
  pcmdS->SetNoGUINotify();
  pmulticmds->Add(pcmdS);
  Execute(pmulticmds);

  possible_aliases.clear();
  possible_shortcuts.clear();

  if (numImported > 0)
    SetDBChanged(true);

  return SUCCESS;
}
#endif

static void ReportInvalidField(CReport &rpt, const string &value, int numlines)
{
  CUTF8Conv conv;
  StringX vx;
  conv.FromUTF8((const unsigned char *)value.c_str(), value.length(), vx);
  stringT csError;
  Format(csError, IDSC_IMPORTINVALIDFIELD, numlines, vx.c_str());
  rpt.WriteLine(csError);
}

int PWScore::ImportPlaintextFile(const StringX &ImportedPrefix,
                                 const StringX &filename,
                                 const TCHAR &fieldSeparator, const TCHAR &delimiter,
                                 const bool &bImportPSWDsOnly,
                                 stringT &strError,
                                 int &numImported, int &numSkipped,
                                 CReport &rpt)
{
  stringT csError;
#ifdef UNICODE
  const unsigned char *fname = NULL;
  CUTF8Conv conv;
  int fnamelen;
  conv.ToUTF8(filename, fname, fnamelen); 
#else
  const char *fname = filename.c_str();
#endif
  // following's a stream of chars, as the header row's straight ASCII, and
  // we need to handle rest as utf-8
  ifstream ifs(reinterpret_cast<const char *>(fname));

  if (!ifs)
    return CANT_OPEN_FILE;

  numImported = numSkipped = 0;

  int numlines = 0;

  CItemData ci_temp;
  vector<string> vs_Header;
  stringT cs_hdr;
  LoadAString(cs_hdr, IDSC_EXPORTHEADER);
  const unsigned char *hdr;
  int hdrlen;
  conv.ToUTF8(cs_hdr.c_str(), hdr, hdrlen);
  const string s_hdr((const char *)hdr);
  const char pTab[] = "\t";
  char pSeps[] = " ";

  // Order of fields determined in CItemData::GetPlaintext()
  enum Fields {GROUPTITLE, USER, PASSWORD, URL, AUTOTYPE,
               CTIME, PMTIME, ATIME, XTIME, XTIME_INT, RMTIME,
               POLICY, HISTORY, RUNCMD, DCA, EMAIL, NOTES, 
               NUMFIELDS};

  int i_Offset[NUMFIELDS];
  for (int i = 0; i < NUMFIELDS; i++)
    i_Offset[i] = -1;

  pSeps[0] = (const char)fieldSeparator;

  // Capture individual column titles:
  string::size_type to = 0, from;
  do {
    from = s_hdr.find_first_not_of(pTab, to);
    if (from == string::npos)
      break;
    to = s_hdr.find_first_of(pTab, from);
    vs_Header.push_back(s_hdr.substr(from,
                                     ((to == string::npos) ?
                                      string::npos : to - from)));
  } while (to != string::npos);

  // Following fails if a field was added in enum but not in
  // IDSC_EXPORTHEADER, or vice versa.
  ASSERT(vs_Header.size() == NUMFIELDS);

  string s_title, linebuf;

  // Get title record
  if (!getline(ifs, s_title, '\n')) {
    LoadAString(strError, IDSC_IMPORTNOHEADER);
    rpt.WriteLine(strError);
    return SUCCESS;  // not even a title record! - succeeded but none imported!
  }

  // Capture individual column titles from s_title:
  // Set i_Offset[field] to column in which field is found in text file,
  // or leave at -1 if absent from text.
  unsigned num_found = 0;
  int itoken = 0;

  to = 0;
  do {
    from = s_title.find_first_not_of(pSeps, to);
    if (from == string::npos)
      break;
    to = s_title.find_first_of(pSeps, from);
    string token = s_title.substr(from,
                                  ((to == string::npos) ?
                                   string::npos : to - from));
    vector<string>::iterator it(std::find(vs_Header.begin(), vs_Header.end(), token));
    if (it != vs_Header.end()) {
      i_Offset[it - vs_Header.begin()] = itoken;
      num_found++;
    }
    itoken++;
  } while (to != string::npos);

  if (num_found == 0) {
    LoadAString(strError, IDSC_IMPORTNOCOLS);
    rpt.WriteLine(strError);
    return FAILURE;
  }

  // These are "must haves"!
  if (bImportPSWDsOnly &&
      (i_Offset[PASSWORD] == -1 || i_Offset[GROUPTITLE] == -1 ||
       i_Offset[USER] == -1)) {
    LoadAString(strError, IDSC_IMPORTPSWDNOCOLS);
    rpt.WriteLine(strError);
    return FAILURE;
  } else
  if (i_Offset[PASSWORD] == -1 || i_Offset[GROUPTITLE] == -1) {
    LoadAString(strError, IDSC_IMPORTMISSINGCOLS);
    rpt.WriteLine(strError);
    return FAILURE;
  }

  if (num_found < vs_Header.size()) {
    Format(csError, IDSC_IMPORTHDR, num_found);
    rpt.WriteLine(csError);
    LoadAString(csError, bImportPSWDsOnly ? IDSC_IMPORTKNOWNHDRS2 : IDSC_IMPORTKNOWNHDRS);
    rpt.WriteLine(csError, bImportPSWDsOnly);
    for (int i = 0; i < NUMFIELDS; i++) {
      if (i_Offset[i] >= 0) {
        const string &sHdr = vs_Header.at(i);
        StringX sh2;
        conv.FromUTF8((const unsigned char *)sHdr.c_str(), sHdr.length(), sh2);
        Format(csError, _T(" %s,"), sh2.c_str());
        rpt.WriteLine(csError, false);
      }
    }
    rpt.WriteLine();
    rpt.WriteLine();
  }

  bool bMaintainDateTimeStamps = PWSprefs::GetInstance()->
              GetPref(PWSprefs::MaintainDateTimeStamps);
  bool bIntoEmpty = m_pwlist.size() == 0;

  UUIDList possible_aliases, possible_shortcuts;

  MultiCommands *pmulticmds = MultiCommands::Create(this);
  Command *pcmd1 = UpdateGUICommand::Create(this, Command::WN_UNDO,
                                            Command::GUI_UNDO_IMPORT);
  pmulticmds->Add(pcmd1);

  // Finished parsing header, go get the data!
  for (;;) {
    // read a single line.
    if (!getline(ifs, linebuf, '\n')) break;
    numlines++;

    // remove MS-DOS linebreaks, if needed.
    if (!linebuf.empty() && *(linebuf.end() - 1) == '\r') {
      linebuf.resize(linebuf.size() - 1);
    }

    // skip blank lines
    if (linebuf.empty()) {
      Format(csError, IDSC_IMPORTEMPTYLINESKIPPED, numlines);
      rpt.WriteLine(csError);
      numSkipped++;
      continue;
    }

    // convert linebuf from UTF-8 to stringX
    StringX slinebuf;
    if (!conv.FromUTF8((const unsigned char *)linebuf.c_str(), linebuf.length(), slinebuf)) {
      // XXX add an appropriate error message
      numSkipped++;
      continue;
    }

    // tokenize into separate elements
    itoken = 0;
    vector<stringT> tokens;
    for (size_t startpos = 0;
         startpos < slinebuf.size(); 
         /* startpos advanced in body */) {
      size_t nextchar = slinebuf.find_first_of(fieldSeparator, startpos);
      if (nextchar == stringT::npos)
        nextchar = slinebuf.size();
      if (nextchar > 0) {
        if (itoken != i_Offset[NOTES]) {
          const StringX tsx(slinebuf.substr(startpos, nextchar - startpos));
          tokens.push_back(tsx.c_str());
        } else { // Notes field
          // Notes may be double-quoted, and
          // if they are, they may span more than one line.
          stringT note(slinebuf.substr(startpos).c_str());
          size_t first_quote = note.find_first_of('\"');
          size_t last_quote = note.find_last_of('\"');
          if (first_quote == last_quote && first_quote != string::npos) {
            //there was exactly one quote, meaning that we've a multi-line Note
            bool noteClosed = false;
            do {
              if (!getline(ifs, linebuf, '\n')) {
                Format(csError, IDSC_IMPMISSINGQUOTE, numlines);
                rpt.WriteLine(csError);
                ifs.close(); // file ends before note closes
                return (numImported > 0) ? SUCCESS : INVALID_FORMAT;
              }
              numlines++;
              // remove MS-DOS linebreaks, if needed.
              if (!linebuf.empty() && *(linebuf.end() - 1) == TCHAR('\r')) {
                linebuf.resize(linebuf.size() - 1);
              }
              note += _T("\r\n");
              if (!conv.FromUTF8((const unsigned char *)linebuf.c_str(), linebuf.length(),
                                 slinebuf)) {
                // XXX add an appropriate error message
                numSkipped++;
                continue;
              }
              note += slinebuf.c_str();
              size_t fq = linebuf.find_first_of('\"');
              size_t lq = linebuf.find_last_of('\"');
              noteClosed = (fq == lq && fq != string::npos);
            } while (!noteClosed);
          } // multiline note processed
          tokens.push_back(note);
          break;
        } // Notes handling
      } // nextchar > 0
      startpos = nextchar + 1; // too complex for for statement
      itoken++;
    } // tokenization for loop

    // Sanity check
    if (tokens.size() < num_found) {
      Format(csError, IDSC_IMPORTLINESKIPPED, numlines, tokens.size(), num_found);
      rpt.WriteLine(csError);
      numSkipped++;
      continue;
    }

    const TCHAR *tc_whitespace = _T(" \t\r\n\f\v");
    // Make fields that are *only* whitespace = empty
    viter tokenIter;
    for (tokenIter = tokens.begin(); tokenIter != tokens.end(); tokenIter++) {
      const vector<stringT>::size_type len = tokenIter->length();

      // Don't bother if already empty
      if (len == 0)
        continue;

      // Dequote if: value big enough to have opening and closing quotes
      // (len >=2) and the first and last characters are doublequotes.
      // UNLESS there's at least one quote in the text itself
      if (len > 1 && (*tokenIter)[0] == _T('\"') && (*tokenIter)[len - 1] == _T('\"')) {
        const stringT dequoted = tokenIter->substr(1, len - 2);
        if (dequoted.find_first_of(_T('\"')) == stringT::npos)
          tokenIter->assign(dequoted);
      }

      // Empty field if purely whitespace
      if (tokenIter->find_first_not_of(tc_whitespace) == stringT::npos) {
        tokenIter->clear();
      }
    } // loop over tokens

    if ((size_t)i_Offset[PASSWORD] >= tokens.size() ||
        tokens[i_Offset[PASSWORD]].empty()) {
      Format(csError, IDSC_IMPORTNOPASSWORD, numlines);
      rpt.WriteLine(csError);
      numSkipped++;
      continue;
    }

    if (bImportPSWDsOnly) {
      StringX sxgroup(_T("")), sxtitle, sxuser;
      const stringT &grouptitle = tokens[i_Offset[GROUPTITLE]];
      stringT entrytitle;
      size_t lastdot = grouptitle.find_last_of(TCHAR('.'));
      if (lastdot != string::npos) {
        sxgroup = grouptitle.substr(0, lastdot).c_str();
        sxtitle = grouptitle.substr(lastdot + 1).c_str();
      } else {
        sxtitle = grouptitle.c_str();
      }
      sxuser = tokens[i_Offset[USER]].c_str();
      ItemListIter iter = Find(sxgroup, sxtitle, sxuser);
      if (iter == m_pwlist.end()) {
        stringT cs_online;
        LoadAString(cs_online, IDSC_IMPORT_ON_LINE);
        Format(csError, IDSC_IMPORTRECNOTFOUND, cs_online.c_str(), numlines, 
               sxgroup.c_str(), sxtitle.c_str(), sxuser.c_str());
        rpt.WriteLine(csError);
        numSkipped++;
      } else {
        CItemData *pci = &iter->second;
        Command *pcmd = UpdatePasswordCommand::Create(this, *pci,
                                                      tokens[i_Offset[PASSWORD]].c_str());
        pcmd->SetNoGUINotify();
        pmulticmds->Add(pcmd);
        if (bMaintainDateTimeStamps) {
          pci->SetATime();
        }
        numImported++;
      }
      continue;
    }

    // Start initializing the new record.
    ci_temp.Clear();
    ci_temp.CreateUUID();
    if (i_Offset[USER] >= 0 && tokens.size() > (size_t)i_Offset[USER])
      ci_temp.SetUser(tokens[i_Offset[USER]].c_str());
    StringX csPassword = tokens[i_Offset[PASSWORD]].c_str();
    if (i_Offset[PASSWORD] >= 0)
      ci_temp.SetPassword(csPassword);

    // The group and title field are concatenated.
    // If the title field has periods, then they have been changed to the delimiter
    const stringT &grouptitle = tokens[i_Offset[GROUPTITLE]];
    stringT entrytitle;
    size_t lastdot = grouptitle.find_last_of(TCHAR('.'));
    if (lastdot != string::npos) {
      StringX newgroup(ImportedPrefix.empty() ?
                         _T("") : ImportedPrefix + _T("."));
      newgroup += grouptitle.substr(0, lastdot).c_str();
      ci_temp.SetGroup(newgroup);
      entrytitle = grouptitle.substr(lastdot + 1);
    } else {
      ci_temp.SetGroup(ImportedPrefix);
      entrytitle = grouptitle;
    }

    std::replace(entrytitle.begin(), entrytitle.end(), delimiter, TCHAR('.'));
    if (entrytitle.empty()) {
      Format(csError, IDSC_IMPORTNOTITLE, numlines);
      rpt.WriteLine(csError);
      numSkipped++;
      continue;
    }

    ci_temp.SetTitle(entrytitle.c_str());

    // Now make sure it is unique
    const StringX group = ci_temp.GetGroup();
    const StringX title = ci_temp.GetTitle();
    const StringX user = ci_temp.GetUser();
    StringX newtitle = GetUniqueTitle(group, title, user, IDSC_IMPORTNUMBER);
    ci_temp.SetTitle(newtitle);
    if (newtitle != title) {
      if (group.empty())
        Format(csError, IDSC_IMPORTCONFLICTS2, numlines,
               title.c_str(), user.c_str(), newtitle.c_str());
      else
        Format(csError, IDSC_IMPORTCONFLICTS1, numlines,
               group.c_str(), title.c_str(), user.c_str(), newtitle.c_str());
      rpt.WriteLine(csError);
    }

    if (i_Offset[URL] >= 0 && tokens.size() > (size_t)i_Offset[URL])
      ci_temp.SetURL(tokens[i_Offset[URL]].c_str());
    if (i_Offset[AUTOTYPE] >= 0 && tokens.size() > (size_t)i_Offset[AUTOTYPE])
      ci_temp.SetAutoType(tokens[i_Offset[AUTOTYPE]].c_str());
    if (i_Offset[CTIME] >= 0 && tokens.size() > (size_t)i_Offset[CTIME])
      if (!ci_temp.SetCTime(tokens[i_Offset[CTIME]].c_str()))
        ReportInvalidField(rpt, vs_Header.at(CTIME), numlines);
    if (i_Offset[PMTIME] >= 0 && tokens.size() > (size_t)i_Offset[PMTIME])
      if (!ci_temp.SetPMTime(tokens[i_Offset[PMTIME]].c_str()))
        ReportInvalidField(rpt, vs_Header.at(PMTIME), numlines);
    if (i_Offset[ATIME] >= 0 && tokens.size() > (size_t)i_Offset[ATIME])
      if (!ci_temp.SetATime(tokens[i_Offset[ATIME]].c_str()))
        ReportInvalidField(rpt, vs_Header.at(ATIME), numlines);
    if (i_Offset[XTIME] >= 0 && tokens.size() > (size_t)i_Offset[XTIME])
      if (!ci_temp.SetXTime(tokens[i_Offset[XTIME]].c_str()))
        ReportInvalidField(rpt, vs_Header.at(XTIME), numlines);
    if (i_Offset[XTIME_INT] >= 0 && tokens.size() > (size_t)i_Offset[XTIME_INT])
      if (!ci_temp.SetXTimeInt(tokens[i_Offset[XTIME_INT]].c_str()))
        ReportInvalidField(rpt, vs_Header.at(XTIME_INT), numlines);
    if (i_Offset[RMTIME] >= 0 && tokens.size() > (size_t)i_Offset[RMTIME])
      if (!ci_temp.SetRMTime(tokens[i_Offset[RMTIME]].c_str()))
        ReportInvalidField(rpt, vs_Header.at(RMTIME), numlines);
    if (i_Offset[POLICY] >= 0 && tokens.size() > (size_t)i_Offset[POLICY])
      if (!ci_temp.SetPWPolicy(tokens[i_Offset[POLICY]].c_str()))
        ReportInvalidField(rpt, vs_Header.at(POLICY), numlines);
    if (i_Offset[HISTORY] >= 0 && tokens.size() > (size_t)i_Offset[HISTORY]) {
      StringX newPWHistory;
      stringT strPWHErrors;
      Format(csError, IDSC_IMPINVALIDPWH, numlines);
      switch (VerifyImportPWHistoryString(tokens[i_Offset[HISTORY]].c_str(),
                                          newPWHistory, strPWHErrors)) {
        case PWH_OK:
          ci_temp.SetPWHistory(newPWHistory.c_str());
          break;
        case PWH_IGNORE:
          break;
        case PWH_INVALID_HDR:
        case PWH_INVALID_STATUS:
        case PWH_INVALID_NUM:
        case PWH_INVALID_DATETIME:
        case PWH_INVALID_PSWD_LENGTH:
        case PWH_TOO_SHORT:
        case PWH_TOO_LONG:
        case PWH_INVALID_CHARACTER:
        default:
          rpt.WriteLine(csError, false);
          rpt.WriteLine(strPWHErrors, false);
          LoadAString(csError, IDSC_PWHISTORYSKIPPED);
          rpt.WriteLine(csError);
          break;
      }
    }
    if (i_Offset[RUNCMD] >= 0 && tokens.size() > (size_t)i_Offset[RUNCMD])
      ci_temp.SetRunCommand(tokens[i_Offset[RUNCMD]].c_str());
    if (i_Offset[DCA] >= 0 && tokens.size() > (size_t)i_Offset[DCA])
      ci_temp.SetDCA(tokens[i_Offset[DCA]].c_str());
    if (i_Offset[EMAIL] >= 0 && tokens.size() > (size_t)i_Offset[EMAIL])
      ci_temp.SetEmail(tokens[i_Offset[EMAIL]].c_str());

    // The notes field begins and ends with a double-quote, with
    // replacement of delimiter by CR-LF.
    if (i_Offset[NOTES] >= 0 && tokens.size() > (size_t)i_Offset[NOTES]) {
      stringT quotedNotes = tokens[i_Offset[NOTES]];
      if (!quotedNotes.empty()) {
        if (*quotedNotes.begin() == TCHAR('\"') &&
            *(quotedNotes.end() - 1) == TCHAR('\"')) {
          quotedNotes = quotedNotes.substr(1, quotedNotes.size() - 2);
        }
        size_t from = 0, pos;
        stringT fixedNotes;
        while (string::npos != (pos = quotedNotes.find(delimiter, from))) {
          fixedNotes += quotedNotes.substr(from, (pos - from));
          fixedNotes += _T("\r\n");
          from = pos + 1;
        }
        fixedNotes += quotedNotes.substr(from);
        ci_temp.SetNotes(fixedNotes.c_str());
      }
    }

    if (Replace(csPassword, _T(':'), _T(';')) <= 2) {
      uuid_array_t temp_uuid;
      ci_temp.GetUUID(temp_uuid);
      if (csPassword.substr(0, 2) == _T("[[") &&
          csPassword.substr(csPassword.length() - 2) == _T("]]")) {
        possible_aliases.push_back(temp_uuid);
      }
      if (csPassword.substr(0, 2) == _T("[~") &&
          csPassword.substr(csPassword.length() - 2) == _T("~]")) {
        possible_shortcuts.push_back(temp_uuid);
      }
    }

    if (!bIntoEmpty) {
      ci_temp.SetStatus(CItemData::ES_ADDED);
    }

    // Get GUI to populate its field
    if (m_pfcnGUIUpdateEntry != NULL) {
      m_pfcnGUIUpdateEntry(ci_temp);
    }

    // Add to commands to execute
    Command *pcmd = AddEntryCommand::Create(this, ci_temp);
    pcmd->SetNoGUINotify();
    pmulticmds->Add(pcmd);
    numImported++;
  } // file processing for (;;) loop
  ifs.close();

  Command *pcmdA = AddDependentEntriesCommand::Create(this,
                                                      possible_aliases, &rpt, 
                                                      CItemData::ET_ALIAS,
                                                      CItemData::PASSWORD);
  pcmdA->SetNoGUINotify();
  pmulticmds->Add(pcmdA);
  Command *pcmdS = AddDependentEntriesCommand::Create(this,
                                                      possible_shortcuts, &rpt, 
                                                      CItemData::ET_SHORTCUT,
                                                      CItemData::PASSWORD);
  pcmdS->SetNoGUINotify();
  pmulticmds->Add(pcmdS);
  Command *pcmd2 = UpdateGUICommand::Create(this, Command::WN_REDO,
                                            Command::GUI_REDO_IMPORT);
  pmulticmds->Add(pcmd2);
  Execute(pmulticmds);

  possible_aliases.clear();
  possible_shortcuts.clear();

  if (numImported > 0)
    SetDBChanged(true);

  return SUCCESS;
}

/*
Thought this might be useful to others...
I made the mistake of using another password safe for a while...
Glad I came back before it was too late, but I still needed to bring in those passwords.

The format of the source file is from doing an export to TXT file in keepass.
I tested it using my password DB from KeePass.

There are two small things: if you have a line that is enclosed by square brackets in the
notes, it will stop processing.  Also, it adds a single, extra newline character to any notes
that is imports.  Both are pretty easy things to live with.

--jah
*/

int
PWScore::ImportKeePassTextFile(const StringX &filename)
{
  static const TCHAR *ImportedPrefix = { _T("ImportedKeePass") };
#ifdef UNICODE
  CUTF8Conv conv;
  const unsigned char *fname = NULL;
  int fnamelen;
  conv.ToUTF8(filename, fname, fnamelen); 
#else
  const char *fname = filename.c_str();
#endif
  ifstreamT ifs(reinterpret_cast<const char *>(fname));

  if (!ifs) {
    return CANT_OPEN_FILE;
  }

  stringT linebuf;
  stringT group, title, user, passwd, notes;

  // read a single line.
  if (!getline(ifs, linebuf, TCHAR('\n')) || linebuf.empty()) {
    return INVALID_FORMAT;
  }

  MultiCommands *pmulticmds = MultiCommands::Create(this);

  // the first line of the keepass text file contains a few garbage characters
  linebuf = linebuf.erase(0, linebuf.find(_T("[")));

  stringT::size_type pos = stringT::npos;
  for (;;) {
    if (!ifs)
      break;
    notes.erase();

    // this line should always be a title contained in []'s
    if (*(linebuf.begin()) != '[' || *(linebuf.end() - 1) != TCHAR(']')) {
      return INVALID_FORMAT;
    }

    // set the title: line pattern: [<group>]
    title = linebuf.substr(linebuf.find(_T("[")) + 1, linebuf.rfind(_T("]")) - 1).c_str();

    // set the group: line pattern: Group: <user>
    if (!getline(ifs, linebuf, TCHAR('\n')) ||
        (pos = linebuf.find(_T("Group: "))) == stringT::npos) {
      return INVALID_FORMAT;
    }
    group = ImportedPrefix;
    if (!linebuf.empty()) {
      group.append(_T("."));
      group.append(linebuf.substr(pos + 7));
    }

    // set the user: line pattern: UserName: <user>
    if (!getline(ifs, linebuf, TCHAR('\n')) ||
        (pos = linebuf.find(_T("UserName: "))) == stringT::npos) {
      return INVALID_FORMAT;
    }
    user = linebuf.substr(pos + 10);

    // set the url: line pattern: URL: <url>
    if (!getline(ifs, linebuf, TCHAR('\n')) ||
        (pos = linebuf.find(_T("URL: "))) == stringT::npos) {
      return INVALID_FORMAT;
    }
    if (!linebuf.substr(pos + 5).empty()) {
      notes.append(linebuf.substr(pos + 5));
      notes.append(_T("\r\n\r\n"));
    }

    // set the password: line pattern: Password: <passwd>
    if (!getline(ifs, linebuf, TCHAR('\n')) ||
        (pos = linebuf.find(_T("Password: "))) == stringT::npos) {
      return INVALID_FORMAT;
    }
    passwd = linebuf.substr(pos + 10);

    // set the first line of notes: line pattern: Notes: <notes>
    if (!getline(ifs, linebuf, TCHAR('\n')) ||
        (pos = linebuf.find(_T("Notes: "))) == stringT::npos) {
      return INVALID_FORMAT;
    }
    notes.append(linebuf.substr(pos + 7));

    // read in any remaining new notes and set up the next record
    for (;;) {
      // see if we hit the end of the file
      if (!getline(ifs, linebuf, TCHAR('\n'))) {
        break;
      }

      // see if we hit a new record
      if (linebuf.find(_T("[")) == 0 && linebuf.rfind(_T("]")) == linebuf.length() - 1) {
        break;
      }

      notes.append(_T("\r\n"));
      notes.append(linebuf);
    }

    // Create & append the new record.
    CItemData ci_temp;
    ci_temp.CreateUUID();
    ci_temp.SetTitle(title.empty() ? group.c_str() : title.c_str());
    ci_temp.SetGroup(group.c_str());
    ci_temp.SetUser(user.empty() ? _T(" ") : user.c_str());
    ci_temp.SetPassword(passwd.empty() ? _T(" ") : passwd.c_str());
    ci_temp.SetNotes(notes.empty() ? _T("") : notes.c_str());
    ci_temp.SetStatus(CItemData::ES_ADDED);

    if (m_pfcnGUIUpdateEntry != NULL) {
      m_pfcnGUIUpdateEntry(ci_temp);
    }
    Command *pcmd = AddEntryCommand::Create(this, ci_temp);
    pcmd->SetNoGUINotify();
    pmulticmds->Add(pcmd);
  }
  ifs.close();

  Execute(pmulticmds);

  // TODO: maybe return an error if the full end of the file was not reached?

  SetDBChanged(true);
  return SUCCESS;
}

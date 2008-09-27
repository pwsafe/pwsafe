/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// file PWScore.cpp
//-----------------------------------------------------------------------------

#include "os/typedefs.h"
#include "os/dir.h"
#include "os/mem.h"
#include "PWScore.h"
#include "corelib.h"
#include "BlowFish.h"
#include "PWSprefs.h"
#include "PWSrand.h"
#include "Util.h"
#include "PWSXML.h"
#include "UUIDGen.h"
#include "SysInfo.h"
#include "UTF8Conv.h"
#include "Report.h"
#include "VerifyFormat.h"
#include "PWSfileV3.h" // XXX cleanup with dynamic_cast
#include "StringXStream.h"

#include <shellapi.h>
#include <shlwapi.h>
#include <fstream> // for WritePlaintextFile
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <algorithm>

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

unsigned char PWScore::m_session_key[20];
unsigned char PWScore::m_session_salt[20];
unsigned char PWScore::m_session_initialized = false;

PWScore::PWScore() : m_currfile(_T("")), m_changed(false),
                     m_usedefuser(false), m_defusername(_T("")),
                     m_ReadFileVersion(PWSfile::UNKNOWN_VERSION),
                     m_passkey(NULL), m_passkey_len(0),
                     m_IsReadOnly(false), m_nRecordsWithUnknownFields(0),
                     m_pfcnNotifyListModified(NULL), m_NotifyInstance(NULL),
                     m_bNotify(false)
{
  // following should ideally be wrapped in a mutex
  if (!PWScore::m_session_initialized) {
    PWScore::m_session_initialized = true;
    CItemData::SetSessionKey(); // per-session initialization
    pws_os::mlock(m_session_key, sizeof(m_session_key));
    PWSrand::GetInstance()->GetRandomData(m_session_key, sizeof(m_session_key));
    PWSrand::GetInstance()->GetRandomData(m_session_salt,
                                          sizeof(m_session_salt));
  }
  m_lockFileHandle = INVALID_HANDLE_VALUE;
  m_lockFileHandle2 = INVALID_HANDLE_VALUE;
  m_LockCount = 0;
}

PWScore::~PWScore()
{
  // do NOT trash m_session_*, as there may be other cores around
  // relying on it. Trashing the ciphertext encrypted with it is enough
  if (m_passkey_len > 0) {
    trashMemory(m_passkey, ((m_passkey_len + 7)/8)*8);
    delete[] m_passkey;
  }
  if (!m_UHFL.empty()) {
    m_UHFL.clear();
  }
}

void PWScore::SetApplicationNameAndVersion(const CString &appName,
                                           DWORD dwMajorMinor)
{
  int nMajor = HIWORD(dwMajorMinor);
  int nMinor = LOWORD(dwMajorMinor);
  m_AppNameAndVersion.Format(_T("%s V%d.%02d"), appName, nMajor, nMinor);
}

void PWScore::AddEntry(const uuid_array_t &uuid, const CItemData &item)
{
  m_changed = true;
  ASSERT(m_pwlist.find(uuid) == m_pwlist.end());
  m_pwlist[uuid] = item;
  NotifyListModified();
}

void PWScore::ClearData(void)
{
  if (m_passkey_len > 0) {
    trashMemory(m_passkey, ((m_passkey_len + 7)/8)*8);
    delete[] m_passkey;
    m_passkey_len = 0;
  }
  //Composed of ciphertext, so doesn't need to be overwritten
  m_pwlist.clear();

  // Clear out out dependents mappings
  m_base2aliases_mmap.clear();
  m_alias2base_map.clear();
  m_base2shortcuts_mmap.clear();
  m_shortcut2base_map.clear();

  // Clear out unknown fields
  m_UHFL.clear();

  // Clear out database filters
  m_MapFilters.clear();

  NotifyListModified();
}

void PWScore::ReInit(bool bNewFile)
{
  // Now reset all values as if created from new
  m_changed = false;
  m_usedefuser = false;
  m_defusername = _T("");
  if (bNewFile)
    m_ReadFileVersion = PWSfile::NEWFILE;
  else
    m_ReadFileVersion = PWSfile::UNKNOWN_VERSION;
  if (m_passkey_len > 0) {
    trashMemory(m_passkey, ((m_passkey_len + 7)/8)*8);
    delete[] m_passkey;
    m_passkey = NULL;
    m_passkey_len = 0;
  }
  m_nRecordsWithUnknownFields = 0;
  m_UHFL.clear();
  NotifyListModified();
}

void PWScore::NewFile(const StringX &passkey)
{
  ClearData();
  SetPassKey(passkey);
  m_changed = false;
  // default username is a per-database preference - wipe clean
  // for new database:
  m_usedefuser = false;
  m_defusername = _T("");
  NotifyListModified();
}

// functor object type for for_each:
struct RecordWriter {
  RecordWriter(PWSfile *out, PWScore *core) : m_out(out), m_core(core) {}
  void operator()(pair<CUUIDGen, CItemData> p)
  {
    StringX savePassword, uuid_str;
    savePassword = p.second.GetPassword();
    if (p.second.IsAlias()) {
      uuid_array_t item_uuid, base_uuid;
      p.second.GetUUID(item_uuid);
      m_core->GetAliasBaseUUID(item_uuid, base_uuid);

      uuid_str_NH_t base_uuid_buffer;
      CUUIDGen::GetUUIDStr(base_uuid, base_uuid_buffer);
      StringX uuid_str(_T("[["));
      uuid_str += (TCHAR *)base_uuid_buffer;
      uuid_str += _T("]]");
      p.second.SetPassword(uuid_str);
    } else if (p.second.IsShortcut()) {
        uuid_array_t item_uuid, base_uuid;
        p.second.GetUUID(item_uuid);
        m_core->GetShortcutBaseUUID(item_uuid, base_uuid);

        uuid_str_NH_t base_uuid_buffer;
        CUUIDGen::GetUUIDStr(base_uuid, base_uuid_buffer);
        StringX uuid_str(_T("[~"));
        uuid_str += (TCHAR *)base_uuid_buffer;
        uuid_str += _T("~]");
    p.second.SetPassword(uuid_str);
      }
      m_out->WriteRecord(p.second);
      p.second.SetPassword(savePassword);
  }

private:
  PWSfile *m_out;
  PWScore *m_core;
};

int PWScore::WriteFile(const StringX &filename, PWSfile::VERSION version)
{
  int status;
  PWSfile *out = PWSfile::MakePWSfile(filename, version,
                                      PWSfile::Write, status);

  if (status != PWSfile::SUCCESS) {
    delete out;
    return status;
  }

  m_hdr.m_prefString = PWSprefs::GetInstance()->Store();
  m_hdr.m_whatlastsaved = m_AppNameAndVersion;

  out->SetHeader(m_hdr);

  // Give PWSfileV3 the unknown headers to write out
  // XXX cleanup gross dynamic_cast
  PWSfileV3 *out3 = dynamic_cast<PWSfileV3 *>(out);
  if (out3 != NULL) {
    out3->SetUnknownHeaderFields(m_UHFL);
    out3->SetFilters(m_MapFilters); // Give it the filters to write out
  }
  status = out->Open(GetPassKey());

  if (status != PWSfile::SUCCESS) {
    delete out;
    return CANT_OPEN_FILE;
  }

  RecordWriter write_record(out, this);
  for_each(m_pwlist.begin(), m_pwlist.end(), write_record);

  m_hdr = out->GetHeader(); // update time saved, etc.

  out->Close();
  delete out;

  m_changed = false;
  m_ReadFileVersion = version; // needed when saving a V17 as V20 1st time [871893]

  return SUCCESS;
}

struct PutText {
  PutText(const CString &subgroup_name,
          const int subgroup_object, const int subgroup_function,
          const CItemData::FieldBits &bsFields,
          TCHAR delimiter, ofstream &ofs, PWScore *core) :
  m_subgroup_name(subgroup_name), m_subgroup_object(subgroup_object),
  m_subgroup_function(subgroup_function), m_bsFields(bsFields),
  m_delimiter(delimiter), m_ofs(ofs), m_core(core)
  {}
  // operator for ItemList
  void operator()(pair<CUUIDGen, CItemData> p)
  {operator()(p.second);}
  // operator for OrderedItemList
  void operator()(const CItemData &item)
  {
    if (m_subgroup_name.IsEmpty() || 
        item.Matches(m_subgroup_name, m_subgroup_object,
        m_subgroup_function)) {
      CItemData *cibase(NULL);
      if (item.IsAlias()) {
        uuid_array_t base_uuid, item_uuid;
        item.GetUUID(item_uuid);
        m_core->GetAliasBaseUUID(item_uuid, base_uuid);
        ItemListIter iter;
        iter = m_core->Find(base_uuid);
        if (iter !=  m_core->GetEntryEndIter())
          cibase = &iter->second;
      }
      if (item.IsShortcut()) {
        uuid_array_t base_uuid, item_uuid;
        item.GetUUID(item_uuid);
        m_core->GetShortcutBaseUUID(item_uuid, base_uuid);
        ItemListIter iter;
        iter = m_core->Find(base_uuid);
        if (iter !=  m_core->GetEntryEndIter())
          cibase = &iter->second;
      }
      const StringX line = item.GetPlaintext(TCHAR('\t'),
                                             m_bsFields, m_delimiter, cibase);
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
  const CString &m_subgroup_name;
  const int m_subgroup_object;
  const int m_subgroup_function;
  const CItemData::FieldBits &m_bsFields;
  TCHAR m_delimiter;  ofstream &m_ofs;
  PWScore *m_core;
};

int PWScore::WritePlaintextFile(const StringX &filename,
                                const CItemData::FieldBits &bsFields,
                                const CString &subgroup_name,
                                const int &subgroup_object,
                                const int &subgroup_function,
                                TCHAR &delimiter, const OrderedItemList *il)
{
  // Check if anything to do! 
  if (bsFields.count() == 0)
    return SUCCESS;

  ofstream ofs(filename.c_str());
  if (!ofs)
    return CANT_OPEN_FILE;

  StringX hdr(_T(""));
  // Following for writing header to an ofstream. Ugh.
  CUTF8Conv conv; const unsigned char *utf8; int utf8Len;


  if ( bsFields.count() == bsFields.size()) {
    // all fields to be exported, use pre-built header
    CString exphdr;
    exphdr.LoadString(IDSC_EXPORTHEADER);
    conv.ToUTF8(LPCTSTR(exphdr), utf8, utf8Len);
    ofs.write(reinterpret_cast<const char *>(utf8), utf8Len);
    ofs << endl;
  } else {
    // user chose fields, build custom header
    CString cs_temp;
    if (bsFields.test(CItemData::GROUP) && bsFields.test(CItemData::TITLE)) {
      cs_temp.LoadString(IDSC_EXPHDRGROUPTITLE);
      hdr += cs_temp;
    } else if (bsFields.test(CItemData::GROUP)) {
      cs_temp.LoadString(IDSC_EXPHDRGROUP);
      hdr += cs_temp;
    } else if (bsFields.test(CItemData::TITLE)) {
      cs_temp.LoadString(IDSC_EXPHDRTITLE);
      hdr += cs_temp;
    }
    if (bsFields.test(CItemData::USER)) {
      cs_temp.LoadString(IDSC_EXPHDRUSERNAME);
      hdr += cs_temp;
    }
    if (bsFields.test(CItemData::PASSWORD)) {
      cs_temp.LoadString(IDSC_EXPHDRPASSWORD);
      hdr += cs_temp;
    }
    if (bsFields.test(CItemData::URL)) {
      cs_temp.LoadString(IDSC_EXPHDRURL);
      hdr += cs_temp;
    }
    if (bsFields.test(CItemData::AUTOTYPE)) {
      cs_temp.LoadString(IDSC_EXPHDRAUTOTYPE);
      hdr += cs_temp;
    }
    if (bsFields.test(CItemData::CTIME)) {
      cs_temp.LoadString(IDSC_EXPHDRCTIME);
      hdr += cs_temp;
    }
    if (bsFields.test(CItemData::PMTIME)) {
      cs_temp.LoadString(IDSC_EXPHDRPMTIME);
      hdr += cs_temp;
    }
    if (bsFields.test(CItemData::ATIME)) {
      cs_temp.LoadString(IDSC_EXPHDRATIME);
      hdr += cs_temp;
    }
    if (bsFields.test(CItemData::XTIME)) {
      cs_temp.LoadString(IDSC_EXPHDRXTIME);
      hdr += cs_temp;
    }
    if (bsFields.test(CItemData::XTIME_INT)) {
      cs_temp.LoadString(IDSC_EXPHDRXTIMEINT);
      hdr += cs_temp;
    }
    if (bsFields.test(CItemData::RMTIME)) {
      cs_temp.LoadString(IDSC_EXPHDRRMTIME);
      hdr += cs_temp;
    }
    if (bsFields.test(CItemData::PWHIST)) {
      cs_temp.LoadString(IDSC_EXPHDRPWHISTORY);
      hdr += cs_temp;
    }
    if (bsFields.test(CItemData::POLICY)) {
      cs_temp.LoadString(IDSC_EXPHDRPWPOLICY);
      hdr += cs_temp;
    }
    if (bsFields.test(CItemData::NOTES)) {
      cs_temp.LoadString(IDSC_EXPHDRNOTES);
      hdr += cs_temp;
    }

    int hdr_len = hdr.length();
    if (hdr[hdr.length() - 1] == _T('\t'))
      hdr_len--;
    hdr = hdr.substr(0, hdr_len);
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
  XMLRecordWriter(const CString &subgroup_name,
                  const int subgroup_object, const int subgroup_function,
                  const CItemData::FieldBits &bsFields,
                  TCHAR delimiter, ofstream &ofs, PWScore *core) :
  m_subgroup_name(subgroup_name), m_subgroup_object(subgroup_object),
  m_subgroup_function(subgroup_function), m_bsFields(bsFields),
  m_delimiter(delimiter), m_of(ofs), m_id(0), m_core(core)
  {}
  // operator for ItemList
  void operator()(pair<CUUIDGen, CItemData> p)
  {operator()(p.second);}
  // operator for OrderedItemList
  void operator()(const CItemData &item)
  {
    m_id++;
    if (m_subgroup_name.IsEmpty() ||
        item.Matches(m_subgroup_name,
        m_subgroup_object, m_subgroup_function)) {
      CItemData *cibase(NULL);
      if (item.IsAlias()) {
        uuid_array_t base_uuid, item_uuid;
        item.GetUUID(item_uuid);
        m_core->GetAliasBaseUUID(item_uuid, base_uuid);
        ItemListIter iter;
        iter = m_core->Find(base_uuid);
        if (iter != m_core->GetEntryEndIter())
          cibase = &iter->second;
      }
      if (item.IsShortcut()) {
        uuid_array_t base_uuid, item_uuid;
        item.GetUUID(item_uuid);
        m_core->GetShortcutBaseUUID(item_uuid, base_uuid);
        ItemListIter iter;
        iter = m_core->Find(base_uuid);
        if (iter != m_core->GetEntryEndIter())
          cibase = &iter->second;
      }
      string xml = item.GetXML(m_id, m_bsFields, m_delimiter, cibase);
      m_of.write(xml.c_str(),
                 static_cast<streamsize>(xml.length()));
    }
  }

private:
  const CString &m_subgroup_name;
  const int m_subgroup_object;
  const int m_subgroup_function;
  const CItemData::FieldBits &m_bsFields;
  TCHAR m_delimiter;
  ofstream &m_of;
  unsigned m_id;
  PWScore *m_core;
};

int PWScore::WriteXMLFile(const StringX &filename,
                          const CItemData::FieldBits &bsFields,
                          const CString &subgroup_name,
                          const int &subgroup_object, const int &subgroup_function,
                          const TCHAR delimiter, const OrderedItemList *il)
{
  ofstream of(filename.c_str());
  CUTF8Conv utf8conv;
  const unsigned char *utf8 = NULL;
  int utf8Len = 0;

  if (!of)
    return CANT_OPEN_FILE;

  StringX pwh, tmp;
  CString cs_tmp;
  time_t time_now;

  time(&time_now);
  const StringX now = PWSUtil::ConvertToDateTimeString(time_now, TMC_XML);

  of << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
  of << "<?xml-stylesheet type=\"text/xsl\" href=\"pwsafe.xsl\"?>" << endl;
  of << endl;
  of << "<passwordsafe" << endl;
  tmp = m_currfile;
  Replace(tmp, _T("&"), _T("&amp;"));

  StringX delStr;
  delStr += delimiter;
  utf8conv.ToUTF8(delStr, utf8, utf8Len);
  of << "delimiter=\"";
  of.write(reinterpret_cast<const char *>(utf8), utf8Len);
  of << "\"" << endl;
  utf8conv.ToUTF8(tmp, utf8, utf8Len);
  of << "Database=\"";
  of.write(reinterpret_cast<const char *>(utf8), utf8Len);
  of << "\"" << endl;
  utf8conv.ToUTF8(now, utf8, utf8Len);
  of << "ExportTimeStamp=\"";
  of.write(reinterpret_cast<const char *>(utf8), utf8Len);
  of << "\"" << endl;
  of << "FromDatabaseFormat=\"";
  ostringstream osv; // take advantage of UTF-8 == ascii for version string
  osv << m_hdr.m_nCurrentMajorVersion
      << "." << setw(2) << setfill('0')
      << m_hdr.m_nCurrentMinorVersion;
  of.write(osv.str().c_str(), osv.str().length());
  of << "\"" << endl;
  if (!m_hdr.m_lastsavedby.empty() || !m_hdr.m_lastsavedon.empty()) {
    oStringXStream oss;
    oss << m_hdr.m_lastsavedby << _T(" on ") << m_hdr.m_lastsavedon;
    utf8conv.ToUTF8(oss.str(), utf8, utf8Len);
    of << "WhoSaved=\"";
    of.write(reinterpret_cast<const char *>(utf8), utf8Len);
    of << "\"" << endl;
  }
  if (!m_hdr.m_whatlastsaved.empty()) {
    utf8conv.ToUTF8(m_hdr.m_whatlastsaved, utf8, utf8Len);
    of << "WhatSaved=\"";
    of.write(reinterpret_cast<const char *>(utf8), utf8Len);
    of << "\"" << endl;
  }
  if (m_hdr.m_whenlastsaved != 0) {
    StringX wls = PWSUtil::ConvertToDateTimeString(m_hdr.m_whenlastsaved,
                                                   TMC_XML);
    utf8conv.ToUTF8(wls.c_str(), utf8, utf8Len);
    of << "WhenLastSaved=\"";
    of.write(reinterpret_cast<const char *>(utf8), utf8Len);
    of << "\"" << endl;
  }

  uuid_str_WH_t hdr_uuid_buffer;
  CUUIDGen::GetUUIDStr(m_hdr.m_file_uuid_array, hdr_uuid_buffer);

  of << "Database_uuid=\"" << hdr_uuid_buffer << "\"" << endl;
  of << "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"" << endl;
  of << "xsi:noNamespaceSchemaLocation=\"pwsafe.xsd\">" << endl;
  of << endl;

  if (m_hdr.m_nITER > MIN_HASH_ITERATIONS) {
    of << "\t<NumberHashIterations>" << m_hdr.m_nITER << "</NumberHashIterations>";
    of << endl;
  }

  // write out preferences stored in database
  CString prefs = PWSprefs::GetInstance()->GetXMLPreferences();
  utf8conv.ToUTF8(LPCTSTR(prefs), utf8, utf8Len);
  of.write(reinterpret_cast<const char *>(utf8), utf8Len);

  if (m_UHFL.size() > 0) {
    of << "\t<unknownheaderfields>" << endl;
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
      tmp = PWSUtil::Base64Encode(pmem, unkhfe.st_length);
#else
      tmp.clear();
      unsigned char c;
      for (unsigned int i = 0; i < unkhfe.st_length; i++) {
        c = *pmem++;
        cs_tmp.Format(_T("%02x"), c);
        tmp += cs_tmp;
      }
#endif
      utf8conv.ToUTF8(tmp, utf8, utf8Len);
      of << "\t\t<field ftype=\"" << int(unkhfe.uc_Type) << "\">";
      of.write(reinterpret_cast<const char *>(utf8), utf8Len);
      of << "</field>" << endl;
    }
    of << "\t</unknownheaderfields>" << endl;  
  }

  if (bsFields.count() != bsFields.size()) {
    // Some restrictions - put in a comment to that effect
    of << "<!-- Export of data was restricted to certain fields by the user -->"
      << endl;
    of << endl;
  }

  XMLRecordWriter put_xml(subgroup_name, subgroup_object, subgroup_function,
                          bsFields, delimiter, of, this);

  if (il != NULL) {
    for_each(il->begin(), il->end(), put_xml);
  } else {
    for_each(m_pwlist.begin(), m_pwlist.end(), put_xml);
  }

  of << "</passwordsafe>" << endl;
  of.close();

  return SUCCESS;
}

int PWScore::ImportXMLFile(const CString &ImportedPrefix, const CString &strXMLFileName,
                           const CString &strXSDFileName, CString &strErrors,
                           int &numValidated, int &numImported,
                           bool &bBadUnknownFileFields, bool &bBadUnknownRecordFields,
                           CReport &rpt)
{
  UUIDList possible_aliases, possible_shortcuts;
  PWSXML iXML(this, &possible_aliases, &possible_shortcuts);
  bool status, validation;
  int nITER(0);
  int nRecordsWithUnknownFields;
  UnknownFieldList uhfl;
  bool bEmptyDB = (GetNumEntries() == 0);

  strErrors = _T("");

  validation = true;
  status = iXML.XMLProcess(validation, ImportedPrefix, strXMLFileName,
                           strXSDFileName, nITER, nRecordsWithUnknownFields, uhfl);
  strErrors = iXML.m_strResultText;
  if (!status) {
    return XML_FAILED_VALIDATION;
  }

  numValidated = iXML.m_numEntriesValidated;

  validation = false;
  status = iXML.XMLProcess(validation, ImportedPrefix, strXMLFileName,
                           strXSDFileName, nITER, nRecordsWithUnknownFields, uhfl);
  strErrors = iXML.m_strResultText;
  if (!status) {
    return XML_FAILED_IMPORT;
  }

  numImported = iXML.m_numEntriesImported;
  bBadUnknownFileFields = iXML.m_bDatabaseHeaderErrors;
  bBadUnknownRecordFields = iXML.m_bRecordHeaderErrors;
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

  AddDependentEntries(possible_aliases, &rpt, CItemData::ET_ALIAS, CItemData::PASSWORD);
  AddDependentEntries(possible_shortcuts, &rpt, CItemData::ET_SHORTCUT, CItemData::PASSWORD);
  possible_aliases.clear();
  possible_shortcuts.clear();

  m_changed = true;
  return SUCCESS;
}

int PWScore::ImportPlaintextFile(const StringX &ImportedPrefix,
                                 const StringX &filename, CString &strError,
                                 TCHAR fieldSeparator, TCHAR delimiter,
                                 int &numImported, int &numSkipped,
                                 CReport &rpt)
{
  CString csError;
  ifstreamT ifs(filename.c_str());

  if (!ifs)
    return CANT_OPEN_FILE;

  numImported = numSkipped = 0;

  int numlines = 0;

  CItemData temp;
  vector<stringT> vs_Header;
  CString cs_hdr;
  cs_hdr.LoadString(IDSC_EXPORTHEADER);
  const stringT s_hdr(cs_hdr);
  const TCHAR pTab[] = _T("\t");
  TCHAR pSeps[] = _T(" ");
  TCHAR *pTemp;

  // Order of fields determined in CItemData::GetPlaintext()
  enum Fields {GROUPTITLE, USER, PASSWORD, URL, AUTOTYPE,
               CTIME, PMTIME, ATIME, XTIME, XTIME_INT, RMTIME,
               POLICY, HISTORY, NOTES, NUMFIELDS};

  int i_Offset[NUMFIELDS];
  for (int i = 0; i < NUMFIELDS; i++)
    i_Offset[i] = -1;

  // Duplicate as c_str is R-O and strtok modifies the string
  pTemp = _tcsdup(s_hdr.c_str());
  if (pTemp == NULL) {
    strError.LoadString(IDSC_IMPORTFAILURE);
    rpt.WriteLine(strError);
    return FAILURE;
  }

  pSeps[0] = fieldSeparator;
#if _MSC_VER >= 1400
  // Capture individual column titles:
  TCHAR *next_token;
  TCHAR *token = _tcstok_s(pTemp, pTab, &next_token);
  while(token) {
    vs_Header.push_back(token);
    token = _tcstok_s(NULL, pTab, &next_token);
  }
#else
  // Capture individual column titles:
  TCHAR *token = _tcstok(pTemp, pTab);
  while(token) {
    vs_Header.push_back(token);
    token = _tcstok(NULL, pTab);
  }
#endif
  // Following fails if a field was added in enum but not in
  // IDSC_EXPORTHEADER, or vice versa.
  ASSERT(vs_Header.size() == NUMFIELDS);

  free(pTemp);

  stringT s_title, linebuf;

  // Get title record
  if (!getline(ifs, s_title, TCHAR('\n'))) {
    strError.LoadString(IDSC_IMPORTNOHEADER);
    rpt.WriteLine(strError);
    return SUCCESS;  // not even a title record! - succeeded but none imported!
  }

  // Duplicate as c_str is R-O and strtok modifies the string
  pTemp = _tcsdup(s_title.c_str());
  if (pTemp == NULL) {
    strError.LoadString(IDSC_IMPORTFAILURE);
    rpt.WriteLine(strError);
    return FAILURE;
  }

  unsigned num_found = 0;
  int itoken = 0;

  // Capture individual column titles:
  // Set i_Offset[field] to column in which field is found in text file,
  // or leave at -1 if absent from text.
#if _MSC_VER >= 1400
  token = _tcstok_s(pTemp, pSeps, &next_token);
  while(token) {
    vciter it(std::find(vs_Header.begin(), vs_Header.end(), token));
    if (it != vs_Header.end()) {
      i_Offset[it - vs_Header.begin()] = itoken;
      num_found++;
    }
    token = _tcstok_s(NULL, pSeps, &next_token);
    itoken++;
  }
#else
  token = _tcstok(pTemp, pSeps);
  while(token) {
    vciter it(std::find(vs_Header.begin(), vs_Header.end(), token));
    if (it != vs_Header.end()) {
      i_Offset[it - vs_Header.begin()] = itoken;
      num_found++;
    }
    token = _tcstok(NULL, pSeps);
    itoken++;
  }
#endif

  free(pTemp);
  if (num_found == 0) {
    strError.LoadString(IDSC_IMPORTNOCOLS);
    rpt.WriteLine(strError);
    return FAILURE;
  }

  // These are "must haves"!
  if (i_Offset[PASSWORD] == -1 || i_Offset[GROUPTITLE] == -1) {
    strError.LoadString(IDSC_IMPORTMISSINGCOLS);
    rpt.WriteLine(strError);
    return FAILURE;
  }

  if (num_found < vs_Header.size()) {
    csError.Format(IDSC_IMPORTHDR, num_found);
    rpt.WriteLine(csError);
    csError.LoadString(IDSC_IMPORTKNOWNHDRS);
    rpt.WriteLine(csError, false);
    for (int i = 0; i < NUMFIELDS; i++) {
      if (i_Offset[i] >= 0) {
        const stringT &sHdr = vs_Header.at(i);
        csError.Format(_T(" %s,"), sHdr.c_str());
        rpt.WriteLine(csError, false);
      }
    }
    rpt.WriteLine();
  }

  UUIDList possible_aliases, possible_shortcuts;

  // Finished parsing header, go get the data!
  for (;;) {
    // read a single line.
    if (!getline(ifs, linebuf, TCHAR('\n'))) break;
    numlines++;

    // remove MS-DOS linebreaks, if needed.
    if (!linebuf.empty() && *(linebuf.end() - 1) == TCHAR('\r')) {
      linebuf.resize(linebuf.size() - 1);
    }

    // skip blank lines
    if (linebuf.empty()) {
      csError.Format(IDSC_IMPORTEMPTYLINESKIPPED, numlines);
      rpt.WriteLine(csError);
      numSkipped++;
      continue;
    }

    // tokenize into separate elements
    itoken = 0;
    vector<stringT> tokens;
    for (size_t startpos = 0;
         startpos < linebuf.size(); 
         /* startpos advanced in body */) {
      size_t nextchar = linebuf.find_first_of(fieldSeparator, startpos);
      if (nextchar == string::npos)
        nextchar = linebuf.size();
      if (nextchar > 0)
        if (itoken != i_Offset[NOTES]) {
          tokens.push_back(linebuf.substr(startpos, nextchar - startpos));
        } else { // Notes field
          // Notes may be double-quoted, and
          // if they are, they may span more than one line.
          stringT note(linebuf.substr(startpos));
          size_t first_quote = note.find_first_of('\"');
          size_t last_quote = note.find_last_of('\"');
          if (first_quote == last_quote && first_quote != string::npos) {
            //there was exactly one quote, meaning that we've a multi-line Note
            bool noteClosed = false;
            do {
              if (!getline(ifs, linebuf, TCHAR('\n'))) {
                csError.Format(IDSC_IMPMISSINGQUOTE, numlines);
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
              note += linebuf;
              size_t fq = linebuf.find_first_of(TCHAR('\"'));
              size_t lq = linebuf.find_last_of(TCHAR('\"'));
              noteClosed = (fq == lq && fq != string::npos);
            } while (!noteClosed);
          } // multiline note processed
          tokens.push_back(note);
          break;
        } // Notes handling
      startpos = nextchar + 1; // too complex for for statement
      itoken++;
    } // tokenization for loop

    // Sanity check
    if (tokens.size() < num_found) {
      csError.Format(IDSC_IMPORTLINESKIPPED, numlines);
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

      pTemp = _tcsdup(tokenIter->c_str());

      // Dequote if: value big enough to have opening and closing quotes
      // (len >=2) and the first and last characters are doublequotes.
      // UNLESS there's at least one quote in the text itself
      if (len > 1 && pTemp[0] == _T('\"') && pTemp[len - 1] == _T('\"')) {
        const stringT dequoted = tokenIter->substr(1, len - 2);
        if (dequoted.find_first_of(_T('\"')) == stringT::npos)
          tokenIter->assign(dequoted);
      }

      // Empty field if purely whitespace
      if (tokenIter->find_first_not_of(tc_whitespace) == stringT::npos) {
        tokenIter->clear();
      }

      free(pTemp);
    } // loop over tokens

    if ((size_t)i_Offset[PASSWORD] >= tokens.size() ||
        tokens[i_Offset[PASSWORD]].empty()) {
      csError.Format(IDSC_IMPORTNOPASSWORD, numlines);
      rpt.WriteLine(csError);
      numSkipped++;
      continue;
    }

    // Start initializing the new record.
    temp.Clear();
    temp.CreateUUID();
    if (i_Offset[USER] >= 0 && tokens.size() > (size_t)i_Offset[USER])
      temp.SetUser(tokens[i_Offset[USER]].c_str());
    StringX csPassword = tokens[i_Offset[PASSWORD]].c_str();
    if (i_Offset[PASSWORD] >= 0)
      temp.SetPassword(csPassword);

    // The group and title field are concatenated.
    // If the title field has periods, then they have been changed to the delimiter
    const stringT &grouptitle = tokens[i_Offset[GROUPTITLE]];
    stringT entrytitle;
    size_t lastdot = grouptitle.find_last_of(TCHAR('.'));
    if (lastdot != string::npos) {
      StringX newgroup(ImportedPrefix.empty() ?
                         _T("") : ImportedPrefix + _T("."));
      newgroup += grouptitle.substr(0, lastdot).c_str();
      temp.SetGroup(newgroup);
      entrytitle = grouptitle.substr(lastdot + 1);
    } else {
      temp.SetGroup(ImportedPrefix);
      entrytitle = grouptitle;
    }

    std::replace(entrytitle.begin(), entrytitle.end(), delimiter, TCHAR('.'));
    if (entrytitle.empty()) {
      csError.Format(IDSC_IMPORTNOTITLE, numlines);
      rpt.WriteLine(csError);
      numSkipped++;
      continue;
    }

    temp.SetTitle(entrytitle.c_str());

    // Now make sure it is unique
    const StringX group = temp.GetGroup();
    const StringX title = temp.GetTitle();
    const StringX user = temp.GetUser();
    StringX newtitle = GetUniqueTitle(group, title, user, IDSC_IMPORTNUMBER);
    temp.SetTitle(newtitle);
    if (newtitle != title) {
      if (group.empty())
        csError.Format(IDSC_IMPORTCONFLICTS2, numlines,
                       title.c_str(), user.c_str(), newtitle.c_str());
      else
        csError.Format(IDSC_IMPORTCONFLICTS1, numlines,
                       group.c_str(), title.c_str(), user.c_str(), newtitle.c_str());
      rpt.WriteLine(csError);
    }

    if (i_Offset[URL] >= 0 && tokens.size() > (size_t)i_Offset[URL])
      temp.SetURL(tokens[i_Offset[URL]].c_str());
    if (i_Offset[AUTOTYPE] >= 0 && tokens.size() > (size_t)i_Offset[AUTOTYPE])
      temp.SetAutoType(tokens[i_Offset[AUTOTYPE]].c_str());
    if (i_Offset[CTIME] >= 0 && tokens.size() > (size_t)i_Offset[CTIME])
      if (!temp.SetCTime(tokens[i_Offset[CTIME]].c_str())) {
        const stringT &time_value = vs_Header.at(CTIME);
        csError.Format(IDSC_IMPORTINVALIDFIELD, numlines, time_value.c_str());
        rpt.WriteLine(csError);
      }
    if (i_Offset[PMTIME] >= 0 && tokens.size() > (size_t)i_Offset[PMTIME])
      if (!temp.SetPMTime(tokens[i_Offset[PMTIME]].c_str())) {
        const stringT &time_value = vs_Header.at(PMTIME);
        csError.Format(IDSC_IMPORTINVALIDFIELD, numlines, time_value.c_str());
        rpt.WriteLine(csError);
      }
    if (i_Offset[ATIME] >= 0 && tokens.size() > (size_t)i_Offset[ATIME])
      if (!temp.SetATime(tokens[i_Offset[ATIME]].c_str())) {
        const stringT &time_value = vs_Header.at(ATIME);
        csError.Format(IDSC_IMPORTINVALIDFIELD, numlines, time_value.c_str());
        rpt.WriteLine(csError);
      }
    if (i_Offset[XTIME] >= 0 && tokens.size() > (size_t)i_Offset[XTIME])
      if (!temp.SetXTime(tokens[i_Offset[XTIME]].c_str())) {
        const stringT &time_value = vs_Header.at(XTIME);
        csError.Format(IDSC_IMPORTINVALIDFIELD, numlines, time_value.c_str());
        rpt.WriteLine(csError);
      }
    if (i_Offset[XTIME_INT] >= 0 && tokens.size() > (size_t)i_Offset[XTIME_INT])
      if (!temp.SetXTimeInt(tokens[i_Offset[XTIME_INT]].c_str())) {
        const stringT &int_value = vs_Header.at(XTIME_INT);
        csError.Format(IDSC_IMPORTINVALIDFIELD, numlines, int_value.c_str());
        rpt.WriteLine(csError);
      }
    if (i_Offset[RMTIME] >= 0 && tokens.size() > (size_t)i_Offset[RMTIME])
      if (!temp.SetRMTime(tokens[i_Offset[RMTIME]].c_str())) {
        const stringT &time_value = vs_Header.at(RMTIME);
        csError.Format(IDSC_IMPORTINVALIDFIELD, numlines, time_value.c_str());
        rpt.WriteLine(csError);
      }
    if (i_Offset[POLICY] >= 0 && tokens.size() > (size_t)i_Offset[POLICY])
      if (!temp.SetPWPolicy(tokens[i_Offset[POLICY]].c_str())) {
        const stringT &dword_value = vs_Header.at(POLICY);
        csError.Format(IDSC_IMPORTINVALIDFIELD, numlines, dword_value.c_str());
        rpt.WriteLine(csError);
      }
    if (i_Offset[HISTORY] >= 0 && tokens.size() > (size_t)i_Offset[HISTORY]) {
      StringX newPWHistory;
      CString strPWHErrors;
      csError.Format(IDSC_IMPINVALIDPWH, numlines);
      switch (VerifyImportPWHistoryString(tokens[i_Offset[HISTORY]].c_str(),
                                          newPWHistory, strPWHErrors)) {
        case PWH_OK:
          temp.SetPWHistory(newPWHistory.c_str());
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
          csError.LoadString(IDSC_PWHISTORYSKIPPED);
          rpt.WriteLine(csError);
          break;
      }
    }

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
        temp.SetNotes(fixedNotes.c_str());
      }
    }

    if (Replace(csPassword, _T(':'), _T(';')) <= 2) {
      uuid_array_t temp_uuid;
      temp.GetUUID(temp_uuid);
      if (csPassword.substr(0, 2) == _T("[[") &&
          csPassword.substr(csPassword.length() - 2) == _T("]]")) {
        possible_aliases.push_back(temp_uuid);
      }
      if (csPassword.substr(0, 2) == _T("[~") &&
          csPassword.substr(csPassword.length() - 2) == _T("~]")) {
        possible_shortcuts.push_back(temp_uuid);
      }
    }

    AddEntry(temp);
    numImported++;
  } // file processing for (;;) loop
  ifs.close();

  AddDependentEntries(possible_aliases, &rpt, CItemData::ET_ALIAS, CItemData::PASSWORD);
  AddDependentEntries(possible_shortcuts, &rpt, CItemData::ET_SHORTCUT, CItemData::PASSWORD);
  possible_aliases.clear();
  possible_shortcuts.clear();
  m_changed = true;
  return SUCCESS;
}

int PWScore::CheckPassword(const StringX &filename, const StringX &passkey)
{
  int status;

  if (!filename.empty())
    status = PWSfile::CheckPassword(filename, passkey, m_ReadFileVersion);
  else { // can happen if tries to export b4 save
    unsigned int t_passkey_len = passkey.length();
    if (t_passkey_len != m_passkey_len) // trivial test
      return WRONG_PASSWORD;
    int BlockLength = ((m_passkey_len + 7)/8)*8;
    unsigned char *t_passkey = new unsigned char[BlockLength];
    LPCTSTR plaintext = LPCTSTR(passkey.c_str());
    EncryptPassword((const unsigned char *)plaintext, t_passkey_len, t_passkey);
    if (memcmp(t_passkey, m_passkey, BlockLength) == 0)
      status = PWSfile::SUCCESS;
    else
      status = PWSfile::WRONG_PASSWORD;
    delete[] t_passkey;
  }

  switch (status) {
    case PWSfile::SUCCESS:
      return SUCCESS;
    case PWSfile::CANT_OPEN_FILE:
      return CANT_OPEN_FILE;
    case PWSfile::WRONG_PASSWORD:
      return WRONG_PASSWORD;
    default:
      ASSERT(0);
      return status; // should never happen
  }
}
#define MRE_FS _T("\xbb")

int PWScore::ReadFile(const StringX &a_filename,
                      const StringX &a_passkey)
{
  int status;
  PWSfile *in = PWSfile::MakePWSfile(a_filename, m_ReadFileVersion,
                                     PWSfile::Read, status);

  if (status != PWSfile::SUCCESS) {
    delete in;
    return status;
  }

  status = in->Open(a_passkey);

  // in the old times we could open even 1.x files
  // for compatibility reasons, we open them again, to see if this is really a "1.x" file
  if ((m_ReadFileVersion == PWSfile::V20) && (status == PWSfile::WRONG_VERSION)) {
    PWSfile::VERSION tmp_version;  // only for getting compatible to "1.x" files
    tmp_version = m_ReadFileVersion;
    m_ReadFileVersion = PWSfile::V17;
    in->SetCurVersion(PWSfile::V17);
    status = in->Open(a_passkey);
    if (status != PWSfile::SUCCESS) {
      m_ReadFileVersion = tmp_version;
    }
  }

  if (status != PWSfile::SUCCESS) {
    delete in;
    return CANT_OPEN_FILE;
  }
  if (m_ReadFileVersion == PWSfile::UNKNOWN_VERSION) {
    delete in;
    return UNKNOWN_VERSION;
  }

  m_hdr = in->GetHeader();
  m_OrigDisplayStatus = m_hdr.m_displaystatus; // for WasDisplayStatusChanged

  // Get pref string and tree display status & who saved when
  // all possibly empty!
  PWSprefs::GetInstance()->Load(m_hdr.m_prefString);

  // prepare handling of pre-2.0 DEFUSERCHR conversion
  if (m_ReadFileVersion == PWSfile::V17) {
    in->SetDefUsername(m_defusername);
    m_hdr.m_nCurrentMajorVersion = PWSfile::V17;
    m_hdr.m_nCurrentMinorVersion = 0;
  } else {
    // for 2.0 & later...
    in->SetDefUsername(PWSprefs::GetInstance()->
      GetPref(PWSprefs::DefaultUsername));
  }

  ClearData(); //Before overwriting old data, but after opening the file...
  SetPassKey(a_passkey); // so user won't be prompted for saves

  CItemData temp;
  uuid_array_t base_uuid, temp_uuid;
  StringX csMyPassword, cs_possibleUUID;
  bool go = true;
#ifdef DEMO
  bool limited = false;
#endif

  PWSfileV3 *in3 = dynamic_cast<PWSfileV3 *>(in); // XXX cleanup
  if (in3 != NULL  && !in3->GetFilters().empty())
    m_MapFilters = in3->GetFilters();

  UUIDList possible_aliases, possible_shortcuts;
  do {
    temp.Clear(); // Rather than creating a new one each time.
    status = in->ReadRecord(temp);
    switch (status) {
      case PWSfile::FAILURE:
      {
        // Show a useful (?) error message - better than
        // silently losing data (but not by much)
        // Best if title intact. What to do if not?

        CString cs_msg;
        CString cs_caption(MAKEINTRESOURCE(IDSC_READ_ERROR));
        cs_msg.Format(IDSC_ENCODING_PROBLEM, temp.GetTitle().c_str());
        MessageBox(NULL, cs_msg, cs_caption, MB_OK);
      }
      // deliberate fall-through
      case PWSfile::SUCCESS:
        uuid_array_t uuid;
        temp.GetUUID(uuid);
        /*
         * If, for some reason, we're reading in a uuid that we already have
         * we will change the uuid, rather than overwrite an entry.
         * This is to protect the user from possible bugs that break
         * the uniqueness requirement of uuids.
         */
         if (m_pwlist.find(uuid) != m_pwlist.end()) {
#ifdef DEBUG
           TRACE(_T("Non-Unique uuid detected:\n"));
           CItemData::FieldBits bf;
           bf.flip();
           StringX dump = temp.GetPlaintext(TCHAR(':'), bf, TCHAR('-'), NULL);
           TRACE(_T("%s\n"), dump);
#endif
           temp.CreateUUID(); // replace duplicated uuid
           temp.GetUUID(uuid); // refresh uuid_array
         }
         // following is duplicated in Validate() - need to refactor
         csMyPassword = temp.GetPassword();
         cs_possibleUUID = csMyPassword.substr(2, 32);  // Extract possible uuid
         ToLower(cs_possibleUUID);
         if (((csMyPassword.substr(0, 2) == _T("[[") &&
               csMyPassword.substr(csMyPassword.length() - 2) == _T("]]")) ||
              (csMyPassword.substr(0, 2) == _T("[~") &&
               csMyPassword.substr(csMyPassword.length() - 2) == _T("~]"))) &&
               csMyPassword.length() == 36 &&
             cs_possibleUUID.find_first_not_of(_T("0123456789abcdef")) == StringX::npos) {
           // _stscanf_s always outputs to an "int" using %x even though
           // target is only 1.  Read into larger buffer to prevent data being
           // overwritten and then copy to where we want it!
           unsigned char temp_uuid_array[sizeof(uuid_array_t) + sizeof(int)];
           int nscanned = 0;
           const TCHAR *lpszuuid = csMyPassword.c_str();
           for (unsigned i = 0; i < sizeof(uuid_array_t); i++) {
#if _MSC_VER >= 1400
             nscanned += _stscanf_s(lpszuuid, _T("%02x"), &temp_uuid_array[i]);
#else
             nscanned += _stscanf(lpszuuid, _T("%02x"), &temp_uuid_array[i]);
#endif
             lpszuuid += 2;
           }
           memcpy(base_uuid, temp_uuid_array, sizeof(uuid_array_t));
           temp.GetUUID(temp_uuid);
           if (csMyPassword.substr(1, 1) == _T("[")) {
             m_alias2base_map[temp_uuid] = base_uuid;
             possible_aliases.push_back(temp_uuid);
           } else {
             m_shortcut2base_map[temp_uuid] = base_uuid;
             possible_shortcuts.push_back(temp_uuid);
           }
         }
#ifdef DEMO
         if (m_pwlist.size() < MAXDEMO) {
           m_pwlist[uuid] = temp;
         } else {
           limited = true;
         }
#else
         m_pwlist[uuid] = temp;
#endif
         break;
      case PWSfile::END_OF_FILE:
        go = false;
        break;
    } // switch
  } while (go);

  m_nRecordsWithUnknownFields = in->GetNumRecordsWithUnknownFields();
  in->GetUnknownHeaderFields(m_UHFL);
  int closeStatus = in->Close(); // in V3 this checks integrity
#ifdef DEMO
  if (closeStatus == PWSfile::SUCCESS && limited)
    closeStatus = PWScore::LIMIT_REACHED; // if integrity OK but LIMIT_REACHED, return latter
#endif
  delete in;

  AddDependentEntries(possible_aliases, NULL, CItemData::ET_ALIAS, CItemData::UUID);
  AddDependentEntries(possible_shortcuts, NULL, CItemData::ET_SHORTCUT, CItemData::UUID);
  possible_aliases.clear();
  possible_shortcuts.clear();
  NotifyListModified();

  return closeStatus;
}

int PWScore::RenameFile(const StringX &oldname, const StringX &newname)
{
  return PWSfile::RenameFile(oldname, newname);
}

bool PWScore::BackupCurFile(int maxNumIncBackups, int backupSuffix,
                            const CString &userBackupPrefix,
                            const CString &userBackupDir)
{
  CString cs_temp, cs_newfile;
  // Get location for intermediate backup
  if (userBackupDir.IsEmpty()) { // directory same as database's
    // Get directory containing database
    cs_temp = CString(m_currfile.c_str());
    TCHAR *lpszTemp = cs_temp.GetBuffer(_MAX_PATH);
    PathRemoveFileSpec(lpszTemp);
    cs_temp.ReleaseBuffer();
    cs_temp += _T("\\");
  } else {
    cs_temp = userBackupDir;
  }

  // generate prefix of intermediate backup file name
  if (userBackupPrefix.IsEmpty()) {
    const stringT path(m_currfile.c_str());
    stringT drv, dir, name, ext;

    pws_os::splitpath(path, drv, dir, name, ext);
    cs_temp += name.c_str();
  } else {
    cs_temp += userBackupPrefix;
  }

  // Add on suffix
  switch (backupSuffix) { // case values from order in listbox.
    case 1: // YYYYMMDD_HHMMSS suffix
    {
      time_t now;
      time(&now);
      StringX cs_datetime = PWSUtil::ConvertToDateTimeString(now,
                                                             TMC_EXPORT_IMPORT);
      cs_temp += _T("_");
      StringX nf = StringX(cs_temp) + cs_datetime.substr(0, 4) +  // YYYY
        cs_datetime.substr(5,2) +  // MM
        cs_datetime.substr(8,2) +  // DD
        StringX(_T("_")) +
        cs_datetime.substr(11,2) +  // HH
        cs_datetime.substr(14,2) +  // MM
        cs_datetime.substr(17,2);   // SS
      cs_newfile = nf.c_str();
      break;
    }
    case 2: // _nnn suffix
      if (GetIncBackupFileName(cs_temp, maxNumIncBackups, cs_newfile) == FALSE)
        return false;
      break;
    case 0: // no suffix
    default:
      cs_newfile = cs_temp;
      break;
  }

  cs_newfile +=  _T(".ibak");

  // Now copy file and create any intervening directories as necessary & automatically
  TCHAR szSource[_MAX_PATH];
  TCHAR szDestination[_MAX_PATH];

  const TCHAR *lpsz_current = m_currfile.c_str();
  TCHAR *lpsz_new = cs_newfile.GetBuffer(_MAX_PATH);
#if _MSC_VER >= 1400
  _tcscpy_s(szSource, _MAX_PATH, lpsz_current);
  _tcscpy_s(szDestination, _MAX_PATH, lpsz_new);
#else
  _tcscpy(szSource, lpsz_current);
  _tcscpy(szDestination, lpsz_new);
#endif
  cs_newfile.ReleaseBuffer();

  // Must end with double NULL
  szSource[m_currfile.length() + 1] = TCHAR('\0');
  szDestination[cs_newfile.GetLength() + 1] = TCHAR('\0');

  SHFILEOPSTRUCT sfop;
  memset(&sfop, 0, sizeof(sfop));
  sfop.hwnd = GetActiveWindow();
  sfop.wFunc = FO_COPY;
  sfop.pFrom = szSource;
  sfop.pTo = szDestination;
  sfop.fFlags = FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR | FOF_SILENT;

  if (SHFileOperation(&sfop) != 0) {
    return false;
  }
  return true;
  // renames CurFile to CurFile~
  // CString newname(GetCurFile());
  // newname += TCHAR('~');
  // return PWSfile::RenameFile(GetCurFile(), newname);
}

void PWScore::ChangePassword(const StringX &newPassword)
{
  SetPassKey(newPassword);
  m_changed = true;
}

// functor object type for find_if:

struct FieldsMatch {
  bool operator()(pair<CUUIDGen, CItemData> p) {
    const CItemData &item = p.second;
    return (m_group == item.GetGroup() &&
            m_title == item.GetTitle() &&
            m_user == item.GetUser());
  }
  FieldsMatch(const StringX &a_group, const StringX &a_title,
              const StringX &a_user) :
  m_group(a_group), m_title(a_title), m_user(a_user) {}

private:
  const StringX &m_group;
  const StringX &m_title;
  const StringX &m_user;
};

// Finds stuff based on title, group & user fields only
ItemListIter PWScore::Find(const StringX &a_group,const StringX &a_title,
                           const StringX &a_user)
{
  FieldsMatch fields_match(a_group, a_title, a_user);

  ItemListIter retval = find_if(m_pwlist.begin(), m_pwlist.end(),
                                fields_match);
  return retval;
}

struct TitleMatch {
  bool operator()(pair<CUUIDGen, CItemData> p) {
    const CItemData &item = p.second;
    return (m_title == item.GetTitle());
  }
  TitleMatch(const StringX &a_title) :
    m_title(a_title) {}

private:
  const StringX &m_title;
};

ItemListIter PWScore::GetUniqueBase(const StringX &a_title, bool &bMultiple)
{
  ItemListIter retval(m_pwlist.end());
  int num(0);
  TitleMatch TitleMatch(a_title);

  ItemListIter found(m_pwlist.begin());
  do {
    found = find_if(found, m_pwlist.end(), TitleMatch);
    if (found != m_pwlist.end()) {
      num++;
      if (num == 1) {
        // Save first
        retval = found;
      } else {
        // More than 1, set to end()
        retval = m_pwlist.end();
        break;
      }
      found++;
    } else
      break;
  } while (found != m_pwlist.end());

  // It is 1 if only 1, but 0 if none & 2 if more than 1 (we just stopped at the second)
  bMultiple = (num > 1);
  return retval;
}

struct GroupTitle_TitleUserMatch {
  bool operator()(pair<CUUIDGen, CItemData> p) {
    const CItemData &item = p.second;
    return ((m_gt == item.GetGroup() && m_tu == item.GetTitle()) ||
            (m_gt == item.GetTitle() && m_tu == item.GetUser()));
  }
  GroupTitle_TitleUserMatch(const StringX &a_grouptitle,
                            const StringX &a_titleuser) :
                            m_gt(a_grouptitle),  m_tu(a_titleuser) {}

private:
  const StringX &m_gt;
  const StringX &m_tu;
};

ItemListIter PWScore::GetUniqueBase(const StringX &grouptitle, 
                                    const StringX &titleuser, bool &bMultiple)
{
  ItemListIter retval(m_pwlist.end());
  int num(0);
  GroupTitle_TitleUserMatch GroupTitle_TitleUserMatch(grouptitle, titleuser);

  ItemListIter found(m_pwlist.begin());
  do {
    found = find_if(found, m_pwlist.end(), GroupTitle_TitleUserMatch);
    if (found != m_pwlist.end()) {
      num++;
      if (num == 1) {
        // Save first
        retval = found;
      } else {
        // More than 1, set to end()
        retval = m_pwlist.end();
        break;
      }
      found++;
    } else
      break;
  } while (found != m_pwlist.end());

  // It is 1 if only 1, but 0 if none & 2 if more than 1 (we just stopped at the second)
  bMultiple = (num > 1);
  return retval;
}

void PWScore::EncryptPassword(const unsigned char *plaintext, int len,
                              unsigned char *ciphertext) const
{
  // ciphertext is ((len +7)/8)*8 bytes long
  BlowFish *Algorithm = BlowFish::MakeBlowFish(m_session_key,
                                               sizeof(m_session_key),
                                               m_session_salt,
                                               sizeof(m_session_salt));
  int BlockLength = ((len + 7)/8)*8;
  unsigned char curblock[8];

  for (int x = 0; x< BlockLength; x += 8) {
    int i;
    if ((len == 0) ||
      ((len%8 != 0) && (len - x < 8))) {
        //This is for an uneven last block
        memset(curblock, 0, 8);
        for (i = 0; i < len %8; i++)
          curblock[i] = plaintext[x + i];
    } else
      for (i = 0; i < 8; i++)
        curblock[i] = plaintext[x + i];
    Algorithm->Encrypt(curblock, curblock);
    memcpy(ciphertext + x, curblock, 8);
  }
  trashMemory(curblock, 8);
  delete Algorithm;
}

void PWScore::SetPassKey(const StringX &new_passkey)
{
  // if changing, clear old
  if (m_passkey_len > 0) {
    trashMemory(m_passkey, ((m_passkey_len + 7)/8)*8);
    delete[] m_passkey;
  }

  m_passkey_len = new_passkey.length() * sizeof(TCHAR);

  int BlockLength = ((m_passkey_len + 7)/8)*8;
  m_passkey = new unsigned char[BlockLength];
  LPCTSTR plaintext = LPCTSTR(new_passkey.c_str());
  EncryptPassword((const unsigned char *)plaintext, m_passkey_len, m_passkey);
}

StringX PWScore::GetPassKey() const
{
  StringX retval(_T(""));
  if (m_passkey_len > 0) {
    const unsigned int BS = BlowFish::BLOCKSIZE;
    unsigned int BlockLength = ((m_passkey_len + (BS-1))/BS)*BS;
    BlowFish *Algorithm = BlowFish::MakeBlowFish(m_session_key,
                                                 sizeof(m_session_key),
                                                 m_session_salt,
                                                 sizeof(m_session_salt));
    unsigned char curblock[BS];

    for (unsigned int x = 0; x < BlockLength; x += BS) {
      unsigned int i;
      for (i = 0; i < BS; i++)
        curblock[i] = m_passkey[x + i];

      Algorithm->Decrypt(curblock, curblock);
      for (i = 0; i < BS; i += sizeof(TCHAR))
        if (x + i < m_passkey_len)
          retval += *((TCHAR*)(curblock + i));
    }
    trashMemory(curblock, sizeof(curblock));
    delete Algorithm;
  }
  return retval;
}

void PWScore::SetDisplayStatus(const std::vector<bool> &s)
{ 
  // DON'T set m_changed!
  // Application should use WasDisplayStatusChanged()
  // to determine if state has changed.
  // This allows app to silently save without nagging user
  m_hdr.m_displaystatus = s;
}

const vector<bool> &PWScore::GetDisplayStatus() const
{
  return m_hdr.m_displaystatus;
}

bool PWScore::WasDisplayStatusChanged() const
{
  // m_OrigDisplayStatus is set while reading file.
  // m_hdr.m_displaystatus may be changed via SetDisplayStatus
  return m_hdr.m_displaystatus != m_OrigDisplayStatus;
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
  ifstreamT ifs(filename.c_str());

  if (!ifs) {
    return CANT_OPEN_FILE;
  }

  stringT linebuf;

  stringT group;
  stringT title;
  stringT user;
  stringT passwd;
  stringT notes;

  // read a single line.
  if (!getline(ifs, linebuf, TCHAR('\n')) || linebuf.empty()) {
    return INVALID_FORMAT;
  }

  // the first line of the keepass text file contains a few garbage characters
  linebuf = linebuf.erase(0, linebuf.find(_T("[")));

  size_t pos = static_cast<size_t>(-1);
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
    if (!getline(ifs, linebuf, TCHAR('\n')) || (pos = linebuf.find(_T("Group: "))) == -1) {
      return INVALID_FORMAT;
    }
    group = ImportedPrefix;
    if (!linebuf.empty()) {
      group.append(_T("."));
      group.append(linebuf.substr(pos + 7));
    }

    // set the user: line pattern: UserName: <user>
    if (!getline(ifs, linebuf, TCHAR('\n')) || (pos = linebuf.find(_T("UserName: "))) == -1) {
      return INVALID_FORMAT;
    }
    user = linebuf.substr(pos + 10);

    // set the url: line pattern: URL: <url>
    if (!getline(ifs, linebuf, TCHAR('\n')) || (pos = linebuf.find(_T("URL: "))) == -1) {
      return INVALID_FORMAT;
    }
    if (!linebuf.substr(pos + 5).empty()) {
      notes.append(linebuf.substr(pos + 5));
      notes.append(_T("\r\n\r\n"));
    }

    // set the password: line pattern: Password: <passwd>
    if (!getline(ifs, linebuf, TCHAR('\n')) || (pos = linebuf.find(_T("Password: "))) == -1) {
      return INVALID_FORMAT;
    }
    passwd = linebuf.substr(pos + 10);

    // set the first line of notes: line pattern: Notes: <notes>
    if (!getline(ifs, linebuf, TCHAR('\n')) || (pos = linebuf.find(_T("Notes: "))) == -1) {
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
    CItemData temp;
    temp.CreateUUID();
    temp.SetTitle(title.empty() ? group.c_str() : title.c_str());
    temp.SetGroup(group.c_str());
    temp.SetUser(user.empty() ? _T(" ") : user.c_str());
    temp.SetPassword(passwd.empty() ? _T(" ") : passwd.c_str());
    temp.SetNotes(notes.empty() ? _T("") : notes.c_str());

    AddEntry(temp);
  }
  ifs.close();

  // TODO: maybe return an error if the full end of the file was not reached?

  m_changed = true;
  return SUCCESS;
}

// GetUniqueGroups - Creates an array of all group names, with no duplicates.
void PWScore::GetUniqueGroups(CStringArray &aryGroups) const
{
  aryGroups.RemoveAll();

  ItemListConstIter iter;

  for (iter = m_pwlist.begin(); iter != m_pwlist.end(); iter++ ) {
    const CItemData &ci = iter->second;
    const CString strThisGroup = ci.GetGroup().c_str();
    // Is this group already in the list?
    bool bAlreadyInList=false;
    for (int igrp = 0; igrp < aryGroups.GetSize(); igrp++) {
      if (aryGroups[igrp] == strThisGroup) {
        bAlreadyInList = true;
        break;
      }
    }
    if (!bAlreadyInList) aryGroups.Add(strThisGroup);
  }
}

void PWScore::CopyPWList(const ItemList &in)
{
  m_pwlist = in;
  m_changed = true;
}

bool
PWScore::Validate(CString &status)
{
  // Check uuid is valid
  // Check PWH is valid
  // Check alias password has corresponding base entry
  // Check shortcut password has corresponding base entry
  // Note that with m_pwlist implemented as a map keyed
  // on uuids, each entry is guaranteed to have
  // a unique uuid. The uniqueness invariant
  // should be enforced elsewhere (upon read/import).

  uuid_array_t uuid_array, base_uuid, temp_uuid;
  int n = -1;
  unsigned num_PWH_fixed = 0;
  unsigned num_uuid_fixed = 0;
  unsigned num_alias_warnings, num_shortcuts_warnings;

  CReport rpt;
  CString cs_Error, cs_temp;
  cs_temp.LoadString(IDSC_RPTVALIDATE);
  rpt.StartReport(cs_temp, GetCurFile().c_str());

  TRACE(_T("%s : Start validation\n"), PWSUtil::GetTimeStamp());

  UUIDList possible_aliases, possible_shortcuts;
  ItemListIter iter;
  for (iter = m_pwlist.begin(); iter != m_pwlist.end(); iter++) {
    CItemData &ci = iter->second;
    ci.GetUUID(uuid_array);
    n++;
    if (uuid_array[0] == 0x00) {
      CItemData fixedItem(ci);
      num_uuid_fixed += fixedItem.ValidateUUID(m_hdr.m_nCurrentMajorVersion,
        m_hdr.m_nCurrentMinorVersion,
        uuid_array);
      cs_Error.Format(IDSC_VALIDATEUUID,
                      ci.GetGroup().c_str(), ci.GetTitle().c_str(), ci.GetUser().c_str());
      rpt.WriteLine(cs_Error);

      m_pwlist.erase(iter); // erasing item in mid-iteration!
      AddEntry(fixedItem);
    }
    if (ci.ValidatePWHistory() != 0) {
      cs_Error.Format(IDSC_VALIDATEPWH,
                      ci.GetGroup().c_str(), ci.GetTitle().c_str(), ci.GetUser().c_str());
      rpt.WriteLine(cs_Error);
      num_PWH_fixed++;
    }
    StringX csMyPassword = ci.GetPassword();
    StringX cs_possibleUUID = csMyPassword.substr(2, 32);  // Extract possible uuid
         ToLower(cs_possibleUUID);
         if (((csMyPassword.substr(0,2) == _T("[[") &&
               csMyPassword.substr(csMyPassword.length() - 2) == _T("]]")) ||
              (csMyPassword.substr(0, 2) == _T("[~") &&
               csMyPassword.substr(csMyPassword.length() - 2) == _T("~]"))) &&
               csMyPassword.length() == 36 &&
             cs_possibleUUID.find_first_not_of(_T("0123456789abcdef")) == StringX::npos) {
      // _stscanf_s always outputs to an "int" using %x even though
      // target is only 1.  Read into larger buffer to prevent data being
      // overwritten and then copy to where we want it!
      unsigned char temp_uuid_array[sizeof(uuid_array_t) + sizeof(int)];
      int nscanned = 0;
      const TCHAR *lpszuuid = csMyPassword.c_str();
      for (unsigned i = 0; i < sizeof(uuid_array_t); i++) {
#if _MSC_VER >= 1400
        nscanned += _stscanf_s(lpszuuid, _T("%02x"), &temp_uuid_array[i]);
#else
        nscanned += _stscanf(lpszuuid, _T("%02x"), &temp_uuid_array[i]);
#endif
        lpszuuid += 2;
      }
      memcpy(base_uuid, temp_uuid_array, sizeof(uuid_array_t));
      ci.GetUUID(temp_uuid);
      if (csMyPassword.substr(0, 2) == _T("[[")) {
        m_alias2base_map[temp_uuid] = base_uuid;
        possible_aliases.push_back(temp_uuid);
      } else {
        m_shortcut2base_map[temp_uuid] = base_uuid;
        possible_shortcuts.push_back(temp_uuid);
      }
    }
  } // iteration over m_pwlist

  num_alias_warnings = AddDependentEntries(possible_aliases, &rpt, CItemData::ET_ALIAS, CItemData::UUID);
  num_shortcuts_warnings = AddDependentEntries(possible_shortcuts, &rpt, CItemData::ET_ALIAS, CItemData::UUID);
  possible_aliases.clear();
  possible_shortcuts.clear();

  TRACE(_T("%s : End validation. %d entries processed\n"), 
    PWSUtil::GetTimeStamp(), n + 1);
  rpt.EndReport();

  if ((num_uuid_fixed + num_PWH_fixed + num_alias_warnings + num_shortcuts_warnings) > 0) {
    status.Format(IDSC_NUMPROCESSED,
                  n + 1, num_uuid_fixed, num_PWH_fixed, num_alias_warnings, num_shortcuts_warnings);
    SetChanged(true);
    return true;
  } else {
    return false;
  }
}

BOOL PWScore::GetIncBackupFileName(const CString &cs_filenamebase,
                                   int i_maxnumincbackups, CString &cs_newname)
{
  CString cs_filenamemask(cs_filenamebase), cs_filename, cs_ibak_number;
  CFileFind finder;
  BOOL bWorking, brc(TRUE);
  int num_found(0), n;
  std::vector<int> file_nums;

  cs_filenamemask += _T("_???.ibak");

  bWorking = finder.FindFile(cs_filenamemask);
  while (bWorking) {
    bWorking = finder.FindNextFile();
    num_found++;
    cs_filename = finder.GetFileName();
    cs_ibak_number = cs_filename.Mid(cs_filename.GetLength() - 8, 3);
    if (cs_ibak_number.SpanIncluding(CString(_T("0123456789"))) != cs_ibak_number)
      continue;
    n = _ttoi(cs_ibak_number);
    file_nums.push_back(n);
  }

  if (num_found == 0) {
    cs_newname = cs_filenamebase + _T("_001");
    return brc;
  }

  sort(file_nums.begin(), file_nums.end());

  int nnn = file_nums.back();
  nnn++;
  if (nnn > 999) nnn = 1;

  cs_newname.Format(_T("%s_%03d"), cs_filenamebase, nnn);

  int i = 0;
  while (num_found >= i_maxnumincbackups) {
    nnn = file_nums.at(i);
    cs_filename.Format(_T("%s_%03d.ibak"), cs_filenamebase, nnn);
    i++;
    num_found--;
    if (DeleteFile(cs_filename) == FALSE) {
      TRACE(_T("DeleteFile(%s)"), cs_filename);
      continue;
    }
  }

  return brc;
}

void PWScore::ClearFileUUID()
{
  memset(m_hdr.m_file_uuid_array, 0x00, sizeof(m_hdr.m_file_uuid_array));
}

void PWScore::SetFileUUID(uuid_array_t &file_uuid_array)
{
  memcpy(m_hdr.m_file_uuid_array, file_uuid_array,
    sizeof(m_hdr.m_file_uuid_array));
}

void PWScore::GetFileUUID(uuid_array_t &file_uuid_array) const
{
  memcpy(file_uuid_array, m_hdr.m_file_uuid_array, sizeof(file_uuid_array));
}

StringX PWScore::GetUniqueTitle(const StringX &path, const StringX &title,
                                const StringX &user, const int IDS_MESSAGE)
{
  StringX new_title(title);
  if (Find(path, title, user) != m_pwlist.end()) {
    // Find a unique "Title"
    ItemListConstIter listpos;
    int i = 0;
    CString s_copy;
    do {
      i++;
      s_copy.Format(IDS_MESSAGE, i);
      new_title = title + LPCTSTR(s_copy);
      listpos = Find(path, new_title, user);
    } while (listpos != m_pwlist.end());
  }
  return new_title;
}

void PWScore::AddDependentEntry(const uuid_array_t &base_uuid, const uuid_array_t &entry_uuid, 
                                const CItemData::EntryType type)
{
  ItemMMap *pmmap;
  ItemMap *pmap;
  if (type == CItemData::ET_ALIAS) {
    pmap = &m_alias2base_map;
    pmmap = &m_base2aliases_mmap;
  } else if (type == CItemData::ET_SHORTCUT) {
    pmap = &m_shortcut2base_map;
    pmmap = &m_base2shortcuts_mmap;
  } else
    return;

  ItemListIter iter = m_pwlist.find(base_uuid);
  ASSERT(iter != m_pwlist.end());

  if (type == CItemData::ET_ALIAS) {
    // Mark base entry as a base entry - must be a normal entry or already an alias base
    ASSERT(iter->second.IsNormal() || iter->second.IsAliasBase());
    iter->second.SetAliasBase();
  } else if (type == CItemData::ET_SHORTCUT) {
    // Mark base entry as a base entry - must be a normal entry or already a shortcut base
    ASSERT(iter->second.IsNormal() || iter->second.IsShortcutBase());
    iter->second.SetShortcutBase();
  }

  // Add to both the base->type multimap and the type->base map
  pmmap->insert(ItemMMap_Pair(base_uuid, entry_uuid));
  pmap->insert(ItemMap_Pair(entry_uuid, base_uuid));
}

void PWScore::RemoveDependentEntry(const uuid_array_t &base_uuid, const uuid_array_t &entry_uuid,
                                   const CItemData::EntryType type)
{
  ItemMMap *pmmap;
  ItemMap *pmap;
  if (type == CItemData::ET_ALIAS) {
    pmap = &m_alias2base_map;
    pmmap = &m_base2aliases_mmap;
  } else if (type == CItemData::ET_SHORTCUT) {
    pmap = &m_shortcut2base_map;
    pmmap = &m_base2shortcuts_mmap;
  } else
    return;

  // Remove from entry -> base map
  pmap->erase(entry_uuid);

  // Remove from base -> entry multimap
  ItemMMapIter mmiter;
  ItemMMapIter mmlastElement;

  mmiter = pmmap->find(base_uuid);
  if (mmiter == pmmap->end())
    return;

  mmlastElement = pmmap->upper_bound(base_uuid);
  uuid_array_t mmiter_uuid;

  for ( ; mmiter != mmlastElement; mmiter++) {
    mmiter->second.GetUUID(mmiter_uuid);
    if (memcmp(entry_uuid, mmiter_uuid, sizeof(uuid_array_t)) == 0) {
      pmmap->erase(mmiter);
      break;
    }
  }

  // Reset base entry to normal if it has no more aliases
  if (pmmap->find(base_uuid) == pmmap->end()) {
    ItemListIter iter = m_pwlist.find(base_uuid);
    if (iter != m_pwlist.end())
      iter->second.SetNormal();
  }
}

void PWScore::RemoveAllDependentEntries(const uuid_array_t &base_uuid, 
                                        const CItemData::EntryType type)
{
  ItemMMap *pmmap;
  ItemMap *pmap;
  if (type == CItemData::ET_ALIAS) {
    pmap = &m_alias2base_map;
    pmmap = &m_base2aliases_mmap;
  } else if (type == CItemData::ET_SHORTCUT) {
    pmap = &m_shortcut2base_map;
    pmmap = &m_base2shortcuts_mmap;
  } else
    return;

  // Remove from entry -> base map for each entry
  ItemMMapIter itr;
  ItemMMapIter lastElement;

  itr = pmmap->find(base_uuid);
  if (itr == pmmap->end())
    return;

  lastElement = pmmap->upper_bound(base_uuid);
  uuid_array_t itr_uuid;

  for ( ; itr != lastElement; itr++) {
    itr->second.GetUUID(itr_uuid);
    // Remove from entry -> base map
    pmap->erase(itr_uuid);
  }

  // Remove from base -> entry multimap
  pmmap->erase(base_uuid);

  // Reset base entry to normal
  ItemListIter iter = m_pwlist.find(base_uuid);
  if (iter != m_pwlist.end())
    iter->second.SetNormal();
}

void PWScore::MoveDependentEntries(const uuid_array_t &from_baseuuid,
                                   const uuid_array_t &to_baseuuid, 
                                   const CItemData::EntryType type)
{
  ItemMMap *pmmap;
  ItemMap *pmap;
  if (type == CItemData::ET_ALIAS) {
    pmap = &m_alias2base_map;
    pmmap = &m_base2aliases_mmap;
  } else if (type == CItemData::ET_SHORTCUT) {
    pmap = &m_shortcut2base_map;
    pmmap = &m_base2shortcuts_mmap;
  } else
    return;

  ItemMMapIter from_itr;
  ItemMMapIter lastfromElement;

  from_itr = pmmap->find(from_baseuuid);
  if (from_itr == pmmap->end())
    return;

  lastfromElement = pmmap->upper_bound(from_baseuuid);

  for ( ; from_itr != lastfromElement; from_itr++) {
    // Add to new base in base -> entry multimap
    pmmap->insert(ItemMMap_Pair(to_baseuuid, from_itr->second));
    // Remove from entry -> base map
    pmap->erase(from_itr->second);
    // Add to entry -> base map (new base)
    pmmap->insert(ItemMap_Pair(from_itr->second, to_baseuuid));    
  }

  // Now delete all old base entries
  pmmap->erase(from_baseuuid);
}

int PWScore::AddDependentEntries(UUIDList &dependentlist, CReport *rpt,
                                 const CItemData::EntryType type, const int &iVia)
{
  // When called during validation of a database  - *rpt is valid
  // When called during the opening of a database or during drag & drop
  //   - *rpt is NULL and no report generated

  // type is either CItemData::ET_ALIAS or CItemData::ET_SHORTCUT

  // If iVia == CItemData::UUID, the password was "[[uuidstr]]" or "[~uuidstr~]" of the
  //   associated base entry
  // If iVia == CItemData::PASSWORD, the password is expected to be in the full format 
  // [g:t:u], where g and/or u may be empty.

  ItemMap *pmap;
  ItemMMap *pmmap;
  if (type == CItemData::ET_ALIAS) {
    pmap = &m_alias2base_map;
    pmmap = &m_base2aliases_mmap;
  } else if (type == CItemData::ET_SHORTCUT) {
    pmap = &m_shortcut2base_map;
    pmmap = &m_base2shortcuts_mmap;
  } else
    return -1;

  int num_warnings(0);

  if (!dependentlist.empty()) {
    UUIDListIter paiter;
    ItemListIter iter;
    StringX csPwdGroup, csPwdTitle, csPwdUser, tmp;
    uuid_array_t base_uuid, entry_uuid;
    bool bwarnings(false);
    CString strError;

    for (paiter = dependentlist.begin();
         paiter != dependentlist.end(); paiter++) {
      iter = m_pwlist.find(*paiter);
      if (iter == m_pwlist.end())
        return num_warnings;

      CItemData *curitem = &iter->second;
      curitem->GetUUID(entry_uuid);
      GetDependentEntryBaseUUID(entry_uuid, base_uuid, type);

      // Delete it - we will put it back if it is an alias/shortcut
      pmap->erase(entry_uuid);

      if (iVia == CItemData::UUID) {
        iter = m_pwlist.find(base_uuid);
      } else {
        tmp = curitem->GetPassword();
        // Remove leading '[['/'[~' & trailing ']]'/'~]'
        tmp = tmp.substr(2, tmp.length() - 4);
        csPwdGroup = tmp.substr(0, tmp.find_first_of(_T(":")));
        // Skip over 'group:'
        tmp = tmp.substr(csPwdGroup.length() + 1);
        csPwdTitle = tmp.substr(0, tmp.find_first_of(_T(":")));
        // Skip over 'title:'
        csPwdUser = tmp.substr(csPwdTitle.length() + 1);
        iter = Find(csPwdGroup, csPwdTitle, csPwdUser);
      }

      if (iter != m_pwlist.end()) {
        const CItemData::EntryType type2 = iter->second.GetEntryType();
        if (type == CItemData::ET_SHORTCUT) {
          // Adding shortcuts -> Base must be normal or already a shortcut base
          if (type2 != CItemData::ET_NORMAL && type2 != CItemData::ET_SHORTCUTBASE) {
            // Bad news!
            if (rpt != NULL) {
              if (!bwarnings) {
                bwarnings = true;
                strError.LoadString(IDSC_IMPORTWARNINGHDR);
                rpt->WriteLine(strError);
              }
              CString cs_type;
              cs_type.LoadString(IDSC_SHORTCUT);
              strError.Format(IDSC_IMPORTWARNING3, cs_type,
                              curitem->GetGroup().c_str(), curitem->GetTitle().c_str(), 
                              curitem->GetUser().c_str(), cs_type);
              rpt->WriteLine(strError);
            }
            // Invalid - delete!
            RemoveEntryAt(m_pwlist.find(entry_uuid));
            continue;
          } 
        }
        if (type == CItemData::ET_ALIAS) {
          // Adding Aliases -> Base must be normal or already a alias base
          if (type2 != CItemData::ET_NORMAL && type2 != CItemData::ET_ALIASBASE) {
            // Bad news!
            if (rpt != NULL) {
              if (!bwarnings) {
                bwarnings = true;
                strError.LoadString(IDSC_IMPORTWARNINGHDR);
                rpt->WriteLine(strError);
              }
              CString cs_type;
              cs_type.LoadString(IDSC_ALIAS);
              strError.Format(IDSC_IMPORTWARNING3, cs_type,
                              curitem->GetGroup().c_str(), curitem->GetTitle().c_str(), 
                              curitem->GetUser().c_str(), cs_type);
              rpt->WriteLine(strError);
            }
            // Invalid - delete!
            RemoveEntryAt(m_pwlist.find(entry_uuid));
            continue;
          }
          if (type2 == CItemData::ET_ALIAS) {
            // This is an alias too!  Not allowed!  Make new one point to original base
            // Note: this may be random as who knows the order of reading records?
            uuid_array_t temp_uuid;
            iter->second.GetUUID(temp_uuid);
            GetDependentEntryBaseUUID(temp_uuid, base_uuid, type);
            if (rpt != NULL) {
              if (!bwarnings) {
                bwarnings = true;
                strError.LoadString(IDSC_IMPORTWARNINGHDR);
                rpt->WriteLine(strError);
              }
              strError.Format(IDSC_IMPORTWARNING1, curitem->GetGroup().c_str(),
                              curitem->GetTitle().c_str(), curitem->GetUser().c_str());
              rpt->WriteLine(strError);
              strError.LoadString(IDSC_IMPORTWARNING1A);
              rpt->WriteLine(strError);
            }
            curitem->SetAlias();
            num_warnings++;
          }
        }
        iter->second.GetUUID(base_uuid);
        if (type == CItemData::ET_ALIAS) {
          iter->second.SetAliasBase();
        } else
        if (type == CItemData::ET_SHORTCUT) {
          iter->second.SetShortcutBase();
        }

        pmmap->insert(ItemMMap_Pair(base_uuid, entry_uuid));
        pmap->insert(ItemMap_Pair(entry_uuid, base_uuid));
        if (type == CItemData::ET_ALIAS) {
          curitem->SetPassword(_T("[Alias]"));
          curitem->SetAlias();
        } else
        if (type == CItemData::ET_SHORTCUT) {
          curitem->SetPassword(_T("[Shortcut]"));
          curitem->SetShortcut();
        }
      } else {
        // Specified base does not exist!
        if (rpt != NULL) {
          if (!bwarnings) {
            bwarnings = true;
            strError.LoadString(IDSC_IMPORTWARNINGHDR);
            rpt->WriteLine(strError);
          }
          strError.Format(IDSC_IMPORTWARNING2, curitem->GetGroup().c_str(),
                          curitem->GetTitle().c_str(), curitem->GetUser().c_str());
          rpt->WriteLine(strError);
          strError.LoadString(IDSC_IMPORTWARNING2A);
          rpt->WriteLine(strError);
        }
        if (type == CItemData::ET_SHORTCUT)
          RemoveEntryAt(m_pwlist.find(entry_uuid)); // Can't keep invalid shortcut
        else
          curitem->SetNormal(); // but can make invalid alias a normal entry

        num_warnings++;
      }
    }
  }
  return num_warnings;
}

void PWScore::ResetAllAliasPasswords(const uuid_array_t &base_uuid)
{
  // Alias ONLY - no shortcut version needed
  ItemMMapIter itr;
  ItemMMapIter lastElement;
  ItemListIter base_itr, alias_itr;
  uuid_array_t alias_uuid;
  StringX csBasePassword;

  itr = m_base2aliases_mmap.find(base_uuid);
  if (itr == m_base2aliases_mmap.end())
    return;

  base_itr = m_pwlist.find(base_uuid);
  if (base_itr != m_pwlist.end()) {
    csBasePassword = base_itr->second.GetPassword();
  } else {
    m_base2aliases_mmap.erase(base_uuid);
    return;
  }

  lastElement = m_base2aliases_mmap.upper_bound(base_uuid);

  for ( ; itr != lastElement; itr++) {
    itr->second.GetUUID(alias_uuid);
    alias_itr = m_pwlist.find(alias_uuid);
    if (alias_itr != m_pwlist.end()) {
      alias_itr->second.SetPassword(csBasePassword);
      alias_itr->second.SetNormal();
    }
  }
  m_base2aliases_mmap.erase(base_uuid);
}

void PWScore::GetAllDependentEntries(const uuid_array_t &base_uuid, UUIDList &tlist, const CItemData::EntryType type)
{
  ItemMMapIter itr;
  ItemMMapIter lastElement;
  uuid_array_t uuid;

  ItemMMap *pmmap;
  if (type == CItemData::ET_ALIAS)
    pmmap = &m_base2aliases_mmap;
  else if (type == CItemData::ET_SHORTCUT)
    pmmap = &m_base2shortcuts_mmap;
  else
    return;

  itr = pmmap->find(base_uuid);
  if (itr == pmmap->end())
    return;

  lastElement = pmmap->upper_bound(base_uuid);

  for ( ; itr != lastElement; itr++) {
    itr->second.GetUUID(uuid);
    tlist.push_back(uuid);
  }
}

bool PWScore::GetBaseEntry(const StringX &passwd, GetBaseEntryPL &pl)
{
  // pl.ibasedata is:
  //  +n: password contains (n-1) colons and base entry found (n = 1, 2 or 3)
  //   0: password not in alias format
  //  -n: password contains (n-1) colons but either no base entry found or no unique entry found (n = 1, 2 or 3)

  // "bMultipleEntriesFound" is set if no "unique" base entry could be found and is only valid if n = -1 or -2.

  pl.bMultipleEntriesFound = false;
  memset(pl.base_uuid, 0x00, sizeof(uuid_array_t));

  StringX Password(passwd);

  int num_colonsP1 = Replace(Password, _T(':'), _T(';')) + 1;
  if ((Password[0] == _T('[')) &&
      (Password[Password.length() - 1] == _T(']')) &&
      num_colonsP1 <= 3) {
    StringX tmp;
    ItemListIter iter;
    switch (num_colonsP1) {
      case 1:
        // [X] - OK if unique entry [g:X:u], [g:X:], [:X:u] or [:X:] exists for any value of g or u
        pl.csPwdTitle = Password.substr(1, Password.length() - 2);  // Skip over '[' & ']'
        iter = GetUniqueBase(pl.csPwdTitle, pl.bMultipleEntriesFound);
        if (iter != m_pwlist.end()) {
          // Fill in the fields found during search
          pl.csPwdGroup = iter->second.GetGroup();
          pl.csPwdUser = iter->second.GetUser();
        }
        break;
      case 2:
        // [X:Y] - OK if unique entry [X:Y:u] or [g:X:Y] exists for any value of g or u
        pl.csPwdUser = _T("");
        tmp = Password.substr(1, Password.length() - 2);  // Skip over '[' & ']'
        pl.csPwdGroup = tmp.substr(0, tmp.find_first_of(_T(":")));
        pl.csPwdTitle = tmp.substr(pl.csPwdGroup.length() + 1);  // Skip over 'group:'
        iter = GetUniqueBase(pl.csPwdGroup, pl.csPwdTitle, pl.bMultipleEntriesFound);
        if (iter != m_pwlist.end()) {
          // Fill in the fields found during search
          pl.csPwdGroup = iter->second.GetGroup();
          pl.csPwdTitle = iter->second.GetTitle();
          pl.csPwdUser = iter->second.GetUser();
        }
        break;
      case 3:
        // [X:Y:Z], [X:Y:], [:Y:Z], [:Y:] (title cannot be empty)
        tmp = Password.substr(1, Password.length() - 2);  // Skip over '[' & ']'
        pl.csPwdGroup = tmp.substr(0, tmp.find_first_of(_T(":")));
        tmp = tmp.substr(pl.csPwdGroup.length() + 1);  // Skip over 'group:'
        pl.csPwdTitle = tmp.substr(0, tmp.find_first_of(_T(":")));    // Skip over 'title:'
        pl.csPwdUser = tmp.substr(pl.csPwdTitle.length() + 1);
        iter = Find(pl.csPwdGroup, pl.csPwdTitle, pl.csPwdUser);
        break;
      default:
        ASSERT(0);
    }
    if (iter != m_pwlist.end()) {
      pl.TargetType = iter->second.GetEntryType();
      if (pl.InputType == CItemData::ET_ALIAS && pl.TargetType == CItemData::ET_ALIAS) {
        // Check if base is already an alias, if so, set this entry -> real base entry
        uuid_array_t temp_uuid;
        iter->second.GetUUID(temp_uuid);
        GetAliasBaseUUID(temp_uuid, pl.base_uuid);
      } else {
        // This may not be a valid combination of source+target entries - sorted out by caller
        iter->second.GetUUID(pl.base_uuid);
      }
      // Valid and found
      pl.ibasedata = num_colonsP1;
      return true;
    }
    // Valid but either exact [g:t:u] not found or
    //  no or multiple entries satisfy [x] or [x:y]
    pl.ibasedata  = -num_colonsP1;
    return true;
  }
  pl.ibasedata = 0; // invalid password format for an alias
  return false;
}

bool PWScore::GetDependentEntryBaseUUID(const uuid_array_t &entry_uuid, 
                                        uuid_array_t &base_uuid, 
                                        const CItemData::EntryType type)
{
  memset(base_uuid, 0x00, sizeof(uuid_array_t));

  ItemMap *pmap;
  if (type == CItemData::ET_ALIAS)
    pmap = &m_alias2base_map;
  else if (type == CItemData::ET_SHORTCUT)
    pmap = &m_shortcut2base_map;
  else
    return false;

  ItemMapIter iter = pmap->find(entry_uuid);
  if (iter != pmap->end()) {
    iter->second.GetUUID(base_uuid);
    return true;
  } else {
    return false;
  }
}

bool PWScore::RegisterOnListModified(void (*pfcn) (LPARAM), LPARAM instance)
{
  if (m_pfcnNotifyListModified != NULL)
    return false;

  m_pfcnNotifyListModified = pfcn;
  m_NotifyInstance = instance;
  m_bNotify = true;
  return true;
}

void PWScore::UnRegisterOnListModified()
{
  m_pfcnNotifyListModified = NULL;
  m_NotifyInstance = NULL;
  m_bNotify = false;
}

void PWScore::NotifyListModified()
{
  if (!m_bNotify || m_pfcnNotifyListModified == NULL)
    return;

  m_pfcnNotifyListModified(m_NotifyInstance);
}

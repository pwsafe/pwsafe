/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
// file PWScore.cpp
//-----------------------------------------------------------------------------

#include "PWScore.h"
#include "corelib.h"
#include "BlowFish.h"
#include "PWSprefs.h"
#include "PWSrand.h"
#include "Util.h"
#include "PWSXML.h"
#include "UUIDGen.h"
#include "SysInfo.h"

#include <shellapi.h>
#include <shlwapi.h>

#include <fstream> // for WritePlaintextFile
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
using namespace std;

// hide w_char/char differences where possible:
#ifdef UNICODE
typedef std::vector<std::wstring>::const_iterator vciter;
typedef std::wstring stringT;
typedef std::wifstream ifstreamT;
typedef std::wofstream ofstreamT;
#else
typedef std::vector<std::string>::const_iterator vciter;
typedef std::string stringT;
typedef std::ifstream ifstreamT;
typedef std::ofstream ofstreamT;
#endif


unsigned char PWScore::m_session_key[20];
unsigned char PWScore::m_session_salt[20];
unsigned char PWScore::m_session_initialized = false;
CString PWScore::m_hdr;

PWScore::PWScore() : m_currfile(_T("")), m_changed(false),
                     m_usedefuser(false), m_defusername(_T("")),
                     m_ReadFileVersion(PWSfile::UNKNOWN_VERSION),
                     m_passkey(NULL), m_passkey_len(0),
                     m_IsReadOnly(false), m_nRecordsWithUnknownFields(0)
{
  if (!PWScore::m_session_initialized) {
	CItemData::SetSessionKey(); // per-session initialization
    PWSrand::GetInstance()->GetRandomData(m_session_key, sizeof(m_session_key) );
    PWSrand::GetInstance()->GetRandomData(m_session_salt, sizeof(m_session_salt) );

	PWScore::m_session_initialized = true;
  }
  m_lockFileHandle = INVALID_HANDLE_VALUE;
  m_LockCount = 0;
}

PWScore::~PWScore()
{
  if (m_passkey_len > 0) {
    trashMemory(m_passkey, ((m_passkey_len + 7)/8)*8);
    delete[] m_passkey;
  }
  if (!m_UHFL.empty()) {
    m_UHFL.clear();
  }
}

void
PWScore::ClearData(void)
{
  if (m_passkey_len > 0) {
    trashMemory(m_passkey, ((m_passkey_len + 7)/8)*8);
    delete[] m_passkey;
    m_passkey_len = 0;
  }
  //Composed of ciphertext, so doesn't need to be overwritten
  m_pwlist.RemoveAll();
}
void
PWScore::ReInit(void)
{
  // Now reset all values as if created from new
  m_currfile = _T("");
  m_changed = false;
  m_usedefuser = false;
  m_defusername = _T("");
  m_ReadFileVersion = PWSfile::UNKNOWN_VERSION;
  m_passkey = NULL;
  m_passkey_len = 0;
  m_nRecordsWithUnknownFields = 0;
}

void
PWScore::NewFile(const CMyString &passkey)
{
   ClearData();
   SetPassKey(passkey);
   m_changed = false;
   // default username is a per-database preference - wipe clean
   // for new database:
   m_usedefuser = false;
   m_defusername = _T("");
}

int
PWScore::WriteFile(const CMyString &filename, PWSfile::VERSION version)
{
  int status;
  PWSfile *out = PWSfile::MakePWSfile(filename, version,
                                      PWSfile::Write, status);

  if (status != PWSfile::SUCCESS) {
    delete out;
    return status;
  }

  // Re-use file's UUID and number of hash iterations
  out->SetFileUUID(m_file_uuid_array);
  out->SetFileHashIterations(m_nITER);

  // Give PWSfileV3 the unknown headers to write out
  out->SetUnknownHeaderFields(m_UHFL);

  // preferences are kept in header, which is written in OpenWriteFile,
  // so we need to update the prefernce string here
  out->SetPrefString(PWSprefs::GetInstance()->Store());

  // Tree Display Status is kept in header
  out->SetDisplayStatus(m_displaystatus);

  // Who last saved which is kept in header
  const SysInfo *si = SysInfo::GetInstance();
  const CString user = si->GetCurrentUser();
  const CString host = si->GetCurrentHost();
  out->SetUserHost(user, host);

  // What last saved which is kept in  header
  out->SetApplicationVersion(m_dwMajorMinor);

  status = out->Open(GetPassKey());

  if (status != PWSfile::SUCCESS) {
    delete out;
    return CANT_OPEN_FILE;
  }

  CItemData temp;
  POSITION listPos = m_pwlist.GetHeadPosition();
  while (listPos != NULL) {
    temp = m_pwlist.GetAt(listPos);
    out->WriteRecord(temp);
    m_pwlist.GetNext(listPos);
  }
  m_nCurrentMajorVersion = out->GetCurrentMajorVersion();
  m_nCurrentMinorVersion = out->GetCurrentMinorVersion();
  m_wholastsaved = out->GetWhoLastSaved();
  m_whenlastsaved = out->GetWhenLastSaved();
  m_whatlastsaved = out->GetWhatLastSaved();
  out->GetFileUUID(m_file_uuid_array);

  out->Close();
  delete out;

  m_changed = false;
  m_ReadFileVersion = version; // needed when saving a V17 as V20 1st time [871893]

  return SUCCESS;
}

int
PWScore::WritePlaintextFile(const CMyString &filename,
                            const CItemData::FieldBits &bsFields,
                            const CString &subgroup_name,
                            const int &subgroup_object, const int &subgroup_function,
                            TCHAR &delimiter, const ItemList *il)
{
  // Check if anything to do! 
  if (bsFields.count() == 0)
    return SUCCESS;

  ofstreamT ofs(filename);

  if (!ofs)
    return CANT_OPEN_FILE;

	if ( bsFields.count() == bsFields.size()) {
	  if (m_hdr.IsEmpty())
	    m_hdr.LoadString(IDSC_EXPORTHEADER);
	  ofs << LPCTSTR(m_hdr) << endl;
	} else {
		CString hdr = _T(""), cs_temp;
		if (bsFields.test(CItemData::GROUP)) {
			cs_temp.LoadString(IDSC_EXPHDRGROUPTITLE);
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
		if (bsFields.test(CItemData::LTIME)) {
			cs_temp.LoadString(IDSC_EXPHDRLTIME);
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
		if (bsFields.test(CItemData::NOTES)) {
			cs_temp.LoadString(IDCS_EXPHDRNOTES);
			hdr += cs_temp;
		}

		int hdr_len = hdr.GetLength();
		if (hdr.Right(1) == _T("\t"))
			hdr_len--;

		ofs << LPCTSTR(hdr.Left(hdr_len)) << endl;
  }

  CItemData temp;
  const ItemList &pwlist = (il == NULL) ? m_pwlist : *il;

  POSITION listPos = pwlist.GetHeadPosition();

  while (listPos != NULL) {
    temp = pwlist.GetAt(listPos);

    if (subgroup_name.IsEmpty() || 
        temp.WantEntry(subgroup_name, subgroup_object, subgroup_function) == TRUE) {
      const CMyString line = temp.GetPlaintext(TCHAR('\t'), bsFields, delimiter);
      if (!line.IsEmpty())
          ofs << LPCTSTR(line) << endl;
    }

    pwlist.GetNext(listPos);
  }
  ofs.close();

  return SUCCESS;
}

static void WriteXMLTime(ofstreamT &of, int indent, const TCHAR *name, time_t t)
{
    int i;
    const CString tmp = PWSUtil::ConvertToDateTimeString(t, TMC_XML);

    for (i = 0; i < indent; i++) of << _T("\t");
    of << _T("<") << name << _T(">") << endl;
    for (i = 0; i <= indent; i++) of << _T("\t");
    of << _T("<date>") << LPCTSTR(tmp.Left(10)) << _T("</date>") << endl;
    for (i = 0; i <= indent; i++) of << _T("\t");
    of << _T("<time>") << LPCTSTR(tmp.Right(8)) << _T("</time>") << endl;
    for (i = 0; i < indent; i++) of << _T("\t");
    of << _T("</") << name << _T(">") << endl;
}

int
PWScore::WriteXMLFile(const CMyString &filename,
                      const CItemData::FieldBits &bsFields,
                      const CString &subgroup_name,
                      const int &subgroup_object, const int &subgroup_function,
                      const TCHAR delimiter, const ItemList *il)
{
	ofstreamT of(filename);

	if (!of)
		return CANT_OPEN_FILE;

	CMyString pwh, tmp;
	CString cs_tmp;
	uuid_array_t uuid_array;

	TCHAR buffer[8];
	time_t time_now;
	int id = 1;

	const ItemList &pwlist = (il == NULL) ? m_pwlist : *il;
	POSITION listPos = pwlist.GetHeadPosition();

	time(&time_now);
	const CMyString now = PWSUtil::ConvertToDateTimeString(time_now, TMC_XML);

	CString wls(_T(""));
	if (!m_wholastsaved.IsEmpty()) {
		int ulen; 
		TCHAR *lpszWLS = m_wholastsaved.GetBuffer(wls.GetLength() + 1);
#if _MSC_VER >= 1400
		int iread = _stscanf_s(lpszWLS, _T("%4x"), &ulen);
#else
		int iread = _stscanf(lpszWLS, _T("%4x"), &ulen);
#endif
		m_wholastsaved.ReleaseBuffer();
		ASSERT(iread == 1);
		wls.Format(_T("%s on %s"), m_wholastsaved.Mid(4, ulen), m_wholastsaved.Mid(ulen + 4));
	}
	of << _T("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>") << endl;
	of << _T("<?xml-stylesheet type=\"text/xsl\" href=\"pwsafe.xsl\"?>") << endl;
	of << endl;
	of << _T("<passwordsafe") << endl;
	tmp = m_currfile;
	tmp.Replace(_T("&"), _T("&amp;"));
	of << _T("delimiter=\"") << delimiter << _T("\"") << endl;
	of << _T("Database=\"") << LPCTSTR(tmp) << _T("\"") << endl;
	of << _T("ExportTimeStamp=\"") << LPCTSTR(now) << _T("\"") << endl;
	cs_tmp.Format(_T("%d.%02d"), m_nCurrentMajorVersion, m_nCurrentMinorVersion);
	of << _T("FromDatabaseFormat=\"") << LPCTSTR(cs_tmp) << _T("\"") << endl;
	if (!m_wholastsaved.IsEmpty())
      of << _T("WhoSaved=\"") << LPCTSTR(wls) << _T("\"") << endl;
	if (!m_whatlastsaved.IsEmpty())
      of << _T("WhatSaved=\"") << LPCTSTR(m_whatlastsaved) << _T("\"") << endl;

  if (m_whenlastsaved.GetLength() == 8) {
	  long t;
	  TCHAR *lpszWLS = m_whenlastsaved.GetBuffer(9);
#if _MSC_VER >= 1400
	  int iread = _stscanf_s(lpszWLS, _T("%8x"), &t);
#else
	  int iread = _stscanf(lpszWLS, _T("%8x"), &t);
#endif
	  m_whenlastsaved.ReleaseBuffer();
	  ASSERT(iread == 1);
    wls = CString(PWSUtil::ConvertToDateTimeString((time_t) t, TMC_XML));
    of << _T("WhenLastSaved=\"") << LPCTSTR(wls) << _T("\"") << endl;
  }

  TCHAR uuid_buffer[37];
#if _MSC_VER >= 1400
	_stprintf_s(uuid_buffer, 37,
                  _T("%02x%02x%02x%02x-%02x%02x-%0x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x"),
                  m_file_uuid_array[0],  m_file_uuid_array[1],
                  m_file_uuid_array[2],  m_file_uuid_array[3],
                  m_file_uuid_array[4],  m_file_uuid_array[5],
                  m_file_uuid_array[6],  m_file_uuid_array[7],
                  m_file_uuid_array[8],  m_file_uuid_array[9],
                  m_file_uuid_array[10], m_file_uuid_array[11],
                  m_file_uuid_array[12], m_file_uuid_array[13],
                  m_file_uuid_array[14], m_file_uuid_array[15]);
#else
  _stprintf(uuid_buffer,
                  _T("%02x%02x%02x%02x-%02x%02x-%0x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x"), 
                  m_file_uuid_array[0],  m_file_uuid_array[1],
                  m_file_uuid_array[2],  m_file_uuid_array[3],
                  m_file_uuid_array[4],  m_file_uuid_array[5],
                  m_file_uuid_array[6],  m_file_uuid_array[7],
                  m_file_uuid_array[8],  m_file_uuid_array[9],
                  m_file_uuid_array[10], m_file_uuid_array[11],
                  m_file_uuid_array[12], m_file_uuid_array[13],
                  m_file_uuid_array[14], m_file_uuid_array[15]);
#endif
  uuid_buffer[36] = TCHAR('\0');
  of << _T("Database_uuid=\"") << uuid_buffer << _T("\"") << endl;
  of << _T("xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"") << endl;
	of << _T("xsi:noNamespaceSchemaLocation=\"pwsafe.xsd\">") << endl;
	of << endl;

  if (m_nITER > MIN_HASH_ITERATIONS) {
      cs_tmp.Format(_T("%d"), m_nITER);
      of << _T("\t<NumberHashIterations>") << LPCTSTR(cs_tmp) << _T("</NumberHashIterations>") << endl;
  }

  if (m_UHFL.size() > 0) {
    of << _T("\t<unknownheaderfields>") << endl;
    UnknownFieldList::const_iterator vi_IterUHFE;
    for (vi_IterUHFE = m_UHFL.begin();
         vi_IterUHFE != m_UHFL.end();
         vi_IterUHFE++) {
      UnknownFieldEntry unkhfe = *vi_IterUHFE;
      if (unkhfe.st_length == 0)
        continue;
#if _MSC_VER >= 1400
		  _itot_s( (int)unkhfe.uc_Type, buffer, 8, 10 );
#else
		  _itot( (int)unkhfe.uc_Type, buffer, 10 );
#endif
      unsigned char * pmem;
      pmem = unkhfe.uc_pUField;

      // UNK_HEX_REP will represent unknown values
      // as hexadecimal, rather than base64 encoding.
      // Easier to debug.
#ifndef UNK_HEX_REP
      tmp = (CMyString)PWSUtil::Base64Encode(pmem, unkhfe.st_length);
#else
      tmp.Empty();
      unsigned char c;
      for (unsigned int i = 0; i < unkhfe.st_length; i++) {
        c = *pmem++;
        cs_tmp.Format(_T("%02x"), c);
        tmp += CMyString(cs_tmp);
      }
#endif
      of << _T("\t\t<field ftype=\"") << buffer << _T("\">") <<  LPCTSTR(tmp) << _T("</field>") << endl;
    }
    of << _T("\t</unknownheaderfields>") << endl;  
  }

  if (bsFields.count() != bsFields.size()) {
    // Some restrictions - put in a comment to that effect
    of << _T("<!-- Export of data was restricted to certain fields by the user -->") << endl;
    of << endl;
  }

	while (listPos != NULL) {
		CItemData temp = pwlist.GetAt(listPos);
#if _MSC_VER >= 1400
		_itot_s( id, buffer, 8, 10 );
#else
		_itot( id, buffer, 10 );
#endif

    if (!subgroup_name.IsEmpty() &&
        temp.WantEntry(subgroup_name, subgroup_object, subgroup_function) == FALSE)
      goto skip_entry;

		// TODO: need to handle entity escaping of values.
    of << _T("\t<entry id=\"") << buffer << _T("\">") << endl;

    tmp = temp.GetGroup();
		if (bsFields.test(CItemData::GROUP) && !tmp.IsEmpty())
        of << _T("\t\t<group><![CDATA[") << LPCTSTR(tmp)
           << _T("]]></group>") << endl;

		// Title mandatory (see pwsafe.xsd)
		tmp = temp.GetTitle();
		of <<_T("\t\t<title><![CDATA[") << LPCTSTR(tmp)
       << _T("]]></title>") << endl;

    tmp = temp.GetUser();
		if (bsFields.test(CItemData::USER) && !tmp.IsEmpty())
        of << _T("\t\t<username><![CDATA[") << LPCTSTR(tmp)
           << _T("]]></username>") << endl;

		tmp = temp.GetPassword();
		// Password mandatory (see pwsafe.xsd)
		of << _T("\t\t<password><![CDATA[") << LPCTSTR(tmp)
       << _T("]]></password>") << endl;

    tmp = temp.GetURL();
		if (bsFields.test(CItemData::URL) && !tmp.IsEmpty())
        of << _T("\t\t<url><![CDATA[") << LPCTSTR(tmp)
           << _T("]]></url>") << endl;

		tmp = temp.GetAutoType();
		if (bsFields.test(CItemData::AUTOTYPE) && !tmp.IsEmpty())
        of << _T("\t\t<autotype><![CDATA[") << LPCTSTR(tmp)
           << _T("]]></autotype>") << endl;

    tmp = temp.GetNotes();
		if (bsFields.test(CItemData::NOTES) && !tmp.IsEmpty())
        of << _T("\t\t<notes><![CDATA[") << LPCTSTR(tmp)
           << _T("]]></notes>") << endl;

		temp.GetUUID(uuid_array);
#if _MSC_VER >= 1400
		_stprintf_s(uuid_buffer, 33,
                    _T("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"), 
                    uuid_array[0],  uuid_array[1],  uuid_array[2],  uuid_array[3],
                    uuid_array[4],  uuid_array[5],  uuid_array[6],  uuid_array[7],
                    uuid_array[8],  uuid_array[9],  uuid_array[10], uuid_array[11],
                    uuid_array[12], uuid_array[13], uuid_array[14], uuid_array[15]);
#else
    _stprintf(uuid_buffer,
                    _T("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"), 
                    uuid_array[0],  uuid_array[1],  uuid_array[2],  uuid_array[3],
                    uuid_array[4],  uuid_array[5],  uuid_array[6],  uuid_array[7],
                    uuid_array[8],  uuid_array[9],  uuid_array[10], uuid_array[11],
                    uuid_array[12], uuid_array[13], uuid_array[14], uuid_array[15]);
#endif
        uuid_buffer[32] = TCHAR('\0');
        of << _T("\t\t<uuid><![CDATA[") << uuid_buffer << _T("]]></uuid>") << endl;

        time_t t;

        temp.GetCTime(t);
        if (bsFields.test(CItemData::CTIME) && (long)t != 0)
            WriteXMLTime(of, 2, _T("ctime"), t);

        temp.GetATime(t);
        if (bsFields.test(CItemData::ATIME) && (long)t != 0)
            WriteXMLTime(of, 2, _T("atime"), t);

        temp.GetLTime(t);
        if (bsFields.test(CItemData::LTIME) && (long)t != 0)
            WriteXMLTime(of, 2, _T("ltime"), t);

        temp.GetPMTime(t);
        if (bsFields.test(CItemData::PMTIME) && (long)t != 0)
            WriteXMLTime(of, 2, _T("pmtime"), t);

        temp.GetRMTime(t);
        if (bsFields.test(CItemData::RMTIME) && (long)t != 0)
            WriteXMLTime(of, 2, _T("rmtime"), t);

        if (bsFields.test(CItemData::PWHIST)) {
          BOOL pwh_status;
          size_t pwh_max, pwh_num;
          PWHistList PWHistList;
          temp.CreatePWHistoryList(pwh_status, pwh_max, pwh_num,
                                 &PWHistList, TMC_XML);
          if (pwh_status == TRUE || pwh_max > 0 || pwh_num > 0) {
            of << _T("\t\t<pwhistory>") << endl;
#if _MSC_VER >= 1400
            _stprintf_s(buffer, 3, _T("%1d"), pwh_status);
            of << _T("\t\t\t<status>") << buffer << _T("</status>") << endl;

            _stprintf_s(buffer, 3, _T("%2d"), pwh_max);
            of << _T("\t\t\t<max>") << buffer << _T("</max>") << endl;

            _stprintf_s(buffer, 3, _T("%2d"), pwh_num);
            of << _T("\t\t\t<num>") << buffer << _T("</num>") << endl;
#else
            _stprintf(buffer, _T("%1d"), pwh_status);
            of << _T("\t\t\t<status>") << buffer << _T("</status>") << endl;

            _stprintf(buffer, _T("%2d"), pwh_max);
            of << _T("\t\t\t<max>") << buffer << _T("</max>") << endl;

            _stprintf(buffer, _T("%2d"), pwh_num);
            of << _T("\t\t\t<num>") << buffer << _T("</num>") << endl;
#endif
            if (!PWHistList.empty()) {
              of << _T("\t\t\t<history_entries>") << endl;
              int num = 1;
              PWHistList::iterator iter;
              for (iter = PWHistList.begin(); iter != PWHistList.end();
                   iter++) {
#if _MSC_VER >= 1400
                _itot_s( num, buffer, 8, 10 );
#else
                _itot( num, buffer, 10 );
#endif
                of << _T("\t\t\t\t<history_entry num=\"") << buffer << _T("\">") << endl;
                const PWHistEntry pwshe = *iter;
                of << _T("\t\t\t\t\t<changed>") << endl;
                of << _T("\t\t\t\t\t\t<date>")
                   << LPCTSTR(pwshe.changedate.Left(10))
                   << _T("</date>") << endl;
                of << _T("\t\t\t\t\t\t<time>")
                   << LPCTSTR(pwshe.changedate.Right(8))
                   << _T("</time>") << endl;
                of << _T("\t\t\t\t\t</changed>") << endl;
                of << _T("\t\t\t\t\t<oldpassword><![CDATA[")
                   << LPCTSTR(pwshe.password)
                   << _T("]]></oldpassword>") << endl;
                of << _T("\t\t\t\t</history_entry>") << endl;

                num++;
              } // for
              of << _T("\t\t\t</history_entries>") << endl;
            } // if !empty
            of << _T("\t\t</pwhistory>") << endl;
          }
        }

        if (temp.NumberUnknownFields() > 0) {
          of << _T("\t\t<unknownrecordfields>") << endl;
          for (unsigned int i = 0; i != temp.NumberUnknownFields(); i++) {
            unsigned int length = 0;
            unsigned char type;
            unsigned char *pdata(NULL);
            temp.GetUnknownField(type, length, pdata, i);
            if (length == 0)
              continue;
#if _MSC_VER >= 1400
		        _itot_s( (int)type, buffer, 8, 10 );
#else
		        _itot( (int)type, buffer, 10 );
#endif
      // UNK_HEX_REP will represent unknown values
      // as hexadecimal, rather than base64 encoding.
      // Easier to debug.
#ifndef UNK_HEX_REP
            tmp = (CMyString)PWSUtil::Base64Encode(pdata, length);
#else
            tmp.Empty();
            unsigned char * pdata2(pdata);
            unsigned char c;
            for (int j = 0; j < (int)length; j++) {
              c = *pdata2++;
              cs_tmp.Format(_T("%02x"), c);
              tmp += CMyString(cs_tmp);
            }
#endif
            of << _T("\t\t\t<field ftype=\"") << buffer << _T("\">") <<  LPCTSTR(tmp) << _T("</field>") << endl;
            trashMemory(pdata, length);
            delete[] pdata;
          } // iteration over unknown fields
          of << _T("\t\t</unknownrecordfields>") << endl;  
        } // if there are unknown fields

        of << _T("\t</entry>") << endl;
        of << endl;

skip_entry:
        pwlist.GetNext(listPos);
        id++;
    }
    of << _T("</passwordsafe>") << endl;
    of.close();

    return SUCCESS;
}

int
PWScore::ImportXMLFile(const CString &ImportedPrefix, const CString &strXMLFileName,
                       const CString &strXSDFileName, CString &strErrors,
                       int &numValidated, int &numImported,
                       bool &bBadUnknownFileFields, bool &bBadUnknownRecordFields)
{
    PWSXML *iXML;
    bool status, validation;
    int nITER;
    int nRecordsWithUnknownFields;
    UnknownFieldList uhfl;
    bool bEmptyDB = (GetNumEntries() == 0);

    iXML = new PWSXML;
    strErrors = _T("");

    validation = true;
    status = iXML->XMLProcess(validation, ImportedPrefix, strXMLFileName, strXSDFileName,
                              nITER, nRecordsWithUnknownFields, uhfl);
    strErrors = iXML->m_strResultText;
    if (!status) {
        delete iXML;
        return XML_FAILED_VALIDATION;
    }

    numValidated = iXML->m_numEntriesValidated;

    iXML->SetCore(this);
    validation = false;
    status = iXML->XMLProcess(validation, ImportedPrefix, strXMLFileName, strXSDFileName,
                              nITER, nRecordsWithUnknownFields, uhfl);
    strErrors = iXML->m_strResultText;
    if (!status) {
        delete iXML;
        return XML_FAILED_IMPORT;
    }

    numImported = iXML->m_numEntriesImported;
    bBadUnknownFileFields = iXML->m_bDatabaseHeaderErrors;
    bBadUnknownRecordFields = iXML->m_bRecordHeaderErrors;
    m_nRecordsWithUnknownFields += nRecordsWithUnknownFields;
    // Only add header unknown fields or change number of iterations
    // if the database was empty to start with
    if (bEmptyDB) {
      m_nITER = nITER;
      if (uhfl.empty())
        m_UHFL.clear();
      else {
        m_UHFL = uhfl;
      }
    }
    uhfl.clear();

    delete iXML;
    m_changed = true;
    return SUCCESS;
}

int
PWScore::ImportPlaintextFile(const CMyString &ImportedPrefix,
                             const CMyString &filename, CString &strErrors,
                             TCHAR fieldSeparator, TCHAR delimiter,
                             int &numImported, int &numSkipped)
{
    ifstreamT ifs(filename);

  if (!ifs)
    return CANT_OPEN_FILE;

  numImported = numSkipped = 0;
  strErrors = _T("");

  if (m_hdr.IsEmpty())
    m_hdr.LoadString(IDSC_EXPORTHEADER);

  int numlines = 0;

  CItemData temp;
  CString buffer;
  vector<stringT> vs_Header;
  const stringT s_hdr(m_hdr);
  const TCHAR pTab[] = _T("\t");
  TCHAR pSeps[] = _T(" ");
  TCHAR *pTemp;

  // Order of fields determined in CItemData::GetPlaintext()
  enum Fields {GROUPTITLE, USER, PASSWORD, URL, AUTOTYPE,
               CTIME, PMTIME, ATIME, LTIME, RMTIME,
               HISTORY, NOTES, NUMFIELDS};
  int i_Offset[NUMFIELDS];
  for (int i = 0; i < NUMFIELDS; i++)
      i_Offset[i] = -1;

  // Duplicate as c_str is R-O and strtok modifies the string
  pTemp = _tcsdup(s_hdr.c_str());
  if (pTemp == NULL) {
    strErrors.LoadString(IDSC_IMPORTFAILURE);
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
  free(pTemp);

  stringT s_title, linebuf;

  // Get title record
  if (!getline(ifs, s_title, TCHAR('\n')))
     return SUCCESS;  // not even a title record! - succeeded but none imported!

  // Duplicate as c_str is R-O and strtok modifies the string
  pTemp = _tcsdup(s_title.c_str());
  if (pTemp == NULL) {
    strErrors.LoadString(IDSC_IMPORTFAILURE);
    return FAILURE;
  }

  unsigned num_found = 0;
  int itoken = 0;

#if _MSC_VER >= 1400
  // Capture individual column titles:
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
  // Capture individual column titles:
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
      strErrors.LoadString(IDSC_IMPORTNOCOLS);
      return FAILURE;
  }

  // These are "must haves"!
  if (i_Offset[PASSWORD] == -1 || i_Offset[GROUPTITLE] == -1) {
      strErrors.LoadString(IDSC_IMPORTMISSINGCOLS);
      return FAILURE;
  }

  if (num_found < vs_Header.size())
      strErrors.Format(IDSC_IMPORTHDR, num_found, vs_Header.size() - num_found);

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
    if (linebuf.empty())
        continue;

    // tokenize into separate elements
    itoken = 0;
    vector<stringT> tokens;
    for (size_t startpos = 0; ; ) {
      size_t nextchar = linebuf.find_first_of(fieldSeparator, startpos);
      if (nextchar >= 0 && i_Offset[itoken] != NOTES) {
        tokens.push_back(linebuf.substr(startpos, nextchar - startpos));
        startpos = nextchar + 1;
      } else {
        // Here for the Notes field. Notes may be double-quoted, and
        // if they are, they may span more than one line.
        stringT note(linebuf.substr(startpos));
        size_t first_quote = note.find_first_of('\"');
        size_t last_quote = note.find_last_of('\"');
        if (first_quote == last_quote && first_quote != string::npos) {
          //there was exactly one quote, meaning that we've a multi-line Note
          bool noteClosed = false;
          do {
            if (!getline(ifs, linebuf, TCHAR('\n'))) {
              buffer.Format(IDSC_IMPMISSINGQUOTE, numlines);
              strErrors += buffer;
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
      }
      itoken++;
    }

    // Sanity check
    if (tokens.size() < num_found) {
        numSkipped++;
        continue;
    }
    // Start initializing the new record.
    temp.Clear();
    temp.CreateUUID();
    if (i_Offset[USER] >= 0)
        temp.SetUser(CMyString(tokens[i_Offset[USER]].c_str()));
    if (i_Offset[PASSWORD] >= 0)
        temp.SetPassword(CMyString(tokens[i_Offset[PASSWORD]].c_str()));

    // The group and title field are concatenated.
    // If the title field has periods, then they have been changed to the delimiter
    const stringT &grouptitle = tokens[i_Offset[GROUPTITLE]];
    stringT entrytitle;
    size_t lastdot = grouptitle.find_last_of(TCHAR('.'));
    if (lastdot != string::npos) {
      CMyString newgroup(ImportedPrefix.IsEmpty() ?
                         _T("") : ImportedPrefix + _T("."));
      newgroup += grouptitle.substr(0, lastdot).c_str();
      temp.SetGroup(newgroup);
      entrytitle = grouptitle.substr(lastdot + 1);
    } else {
      temp.SetGroup(ImportedPrefix);
      entrytitle = grouptitle;
    }
    std::replace(entrytitle.begin(), entrytitle.end(), delimiter, TCHAR('.'));
    temp.SetTitle(CMyString(entrytitle.c_str()));

    if (i_Offset[URL] >= 0)
        temp.SetURL(tokens[i_Offset[URL]].c_str());
    if (i_Offset[AUTOTYPE] >= 0)
        temp.SetAutoType(tokens[i_Offset[AUTOTYPE]].c_str());
    if (i_Offset[CTIME] >= 0)
        temp.SetCTime(tokens[i_Offset[CTIME]].c_str());
    if (i_Offset[PMTIME] >= 0)
        temp.SetPMTime(tokens[i_Offset[PMTIME]].c_str());
    if (i_Offset[ATIME] >= 0)
        temp.SetATime(tokens[i_Offset[ATIME]].c_str());
    if (i_Offset[LTIME] >= 0)
        temp.SetLTime(tokens[i_Offset[LTIME]].c_str());
    if (i_Offset[RMTIME] >= 0)
        temp.SetRMTime(tokens[i_Offset[RMTIME]].c_str());
    if (i_Offset[HISTORY] >= 0) {
        CMyString newPWHistory;
        CString strPWHErrors;
	    buffer.Format(IDSC_IMPINVALIDPWH, numlines);
	    switch (PWSUtil::VerifyImportPWHistoryString(tokens[i_Offset[HISTORY]].c_str(),
                      newPWHistory, strPWHErrors)) {
		    case PWH_OK:
			    temp.SetPWHistory(newPWHistory);
			    buffer.Empty();
			    break;
		    case PWH_IGNORE:
			    buffer.Empty();
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
			    buffer += strPWHErrors;
			    break;
	    }
    }
    strErrors += buffer;

    // The notes field begins and ends with a double-quote, with
    // replacement of delimiter by CR-LF.
    if (i_Offset[NOTES] >= 0) {
        stringT quotedNotes = tokens[i_Offset[NOTES]];
        if (!quotedNotes.empty()) {
          if (*quotedNotes.begin() == TCHAR('\"') &&
            *(quotedNotes.end() - 1) == TCHAR('\"')) {
              quotedNotes = quotedNotes.substr(1, quotedNotes.size() - 2);
          }
            size_t pos;
            const TCHAR *CRLF = _T("\r\n");
            const stringT crlf (CRLF, _tcslen(CRLF) * sizeof(TCHAR));
            while (string::npos != (pos = quotedNotes.find(delimiter)))
              quotedNotes.replace(pos, 1, crlf);

            temp.SetNotes(CMyString(quotedNotes.c_str()));
        }
    }

    AddEntryToTail(temp);
    numImported++;
  } // file processing for (;;) loop
  ifs.close();

  m_changed = true;
  return SUCCESS;
}

int PWScore::CheckPassword(const CMyString &filename, CMyString& passkey)
{
    int status;

    if (!filename.IsEmpty())
        status = PWSfile::CheckPassword(filename, passkey, m_ReadFileVersion);
    else { // can happen if tries to export b4 save
        unsigned int t_passkey_len = passkey.GetLength();
        if (t_passkey_len != m_passkey_len) // trivial test
            return WRONG_PASSWORD;
        int BlockLength = ((m_passkey_len + 7)/8)*8;
        unsigned char *t_passkey = new unsigned char[BlockLength];
        LPCTSTR plaintext = LPCTSTR(passkey);
        EncryptPassword((const unsigned char *)plaintext, t_passkey_len,
                        t_passkey);
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

int
PWScore::ReadFile(const CMyString &a_filename,
                  const CMyString &a_passkey)
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
        PWSfile::VERSION tmp_version;	// only for getting compatible to "1.x" files
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

    // Get file's UUID and number of hash iterations - in case we
    // rewrite file
    in->GetFileUUID(m_file_uuid_array);
    m_nITER = in->GetFileHashIterations();

    // Get pref string and tree display status & who saved when
    // all possibly empty!
    PWSprefs::GetInstance()->Load(in->GetPrefString());

    // prepare handling of pre-2.0 DEFUSERCHR conversion
    if (m_ReadFileVersion == PWSfile::V17) {
        in->SetDefUsername(m_defusername);
        m_nCurrentMajorVersion = PWSfile::V17;
        m_nCurrentMinorVersion = 0;
    } else {
        // for 2.0 & later...
        in->SetDefUsername(PWSprefs::GetInstance()->
                              GetPref(PWSprefs::DefUserName));
        m_nCurrentMajorVersion = in->GetCurrentMajorVersion();
        m_nCurrentMinorVersion = in->GetCurrentMinorVersion();
    }

    m_displaystatus = in->GetDisplayStatus();
    m_whenlastsaved = in->GetWhenLastSaved();
    m_wholastsaved = in->GetWhoLastSaved();
    m_whatlastsaved = in->GetWhatLastSaved();

    ClearData(); //Before overwriting old data, but after opening the file...

    SetPassKey(a_passkey); // so user won't be prompted for saves

    CItemData temp;

    status = in->ReadRecord(temp);
#ifndef DEMO
    while (status == PWSfile::SUCCESS) {
        m_pwlist.AddTail(temp);
        temp.Clear(); // Rather than creating a new one each time.
        status = in->ReadRecord(temp);
    }
    m_nRecordsWithUnknownFields = in->GetNumRecordsWithUnknownFields();
    status = in->Close(); // in V3 this checks integrity
#else // DEMO
    unsigned long numRead = 0;
    while (status == PWSfile::SUCCESS) {
        if (++numRead <= MAXDEMO) { // don't add to list more than MAXDEMO
            m_pwlist.AddTail(temp);
        }
        temp.Clear(); // Rather than creating a new one each time.
        status = in->ReadRecord(temp);
    }
    status = in->Close(); // in V3 this checks integrity
    // if integrity OK but LIMIT_REACHED, return latter
    if (status == PWSfile::SUCCESS && numRead > MAXDEMO)
        status = LIMIT_REACHED;
#endif // DEMO
    in->GetUnknownHeaderFields(m_UHFL);
    delete in;
    return status;
}

int PWScore::RenameFile(const CMyString &oldname, const CMyString &newname)
{
    return PWSfile::RenameFile(oldname, newname);
}

bool PWScore::BackupCurFile(int maxNumIncBackups, int backupSuffix,
                            const CString &userBackupPrefix, const CString &userBackupDir)
{
    CString cs_temp, cs_newfile;
    // Get location for intermediate backup
    if (userBackupDir.IsEmpty()) { // directory same as database's
        // Get directory containing database
        cs_temp = CString(m_currfile);
        TCHAR *lpszTemp = cs_temp.GetBuffer(_MAX_PATH);
        PathRemoveFileSpec(lpszTemp);
        cs_temp.ReleaseBuffer();
        cs_temp += _T("\\");
    } else {
        cs_temp = userBackupDir;
    }

    // generate prefix of intermediate backup file name
    if (userBackupPrefix.IsEmpty()) {
        TCHAR fname[_MAX_FNAME];

#if _MSC_VER >= 1400
        _tsplitpath_s( m_currfile, NULL, 0, NULL, 0, fname, _MAX_FNAME, NULL, 0 );
#else
        _tsplitpath( m_currfile, NULL, NULL, fname, NULL );
#endif
        cs_temp += CString(fname);
    } else {
        cs_temp += userBackupPrefix;
    }

    // Add on suffix
    switch (backupSuffix) { // case values from order in listbox.
        case 1: // YYYYMMDD_HHMMSS suffix
        {
            time_t now;
            time(&now);
            CString cs_datetime = (CString)PWSUtil::ConvertToDateTimeString(now,
                                                                            TMC_EXPORT_IMPORT);
            cs_temp += _T("_");
            cs_newfile = cs_temp + cs_datetime.Left(4) +	// YYYY
                cs_datetime.Mid(5,2) +	// MM
                cs_datetime.Mid(8,2) +	// DD
                _T("_") +
                cs_datetime.Mid(11,2) +	// HH
                cs_datetime.Mid(14,2) +	// MM
                cs_datetime.Mid(17,2);	// SS
        }
        break;
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

    TCHAR *lpsz_current = m_currfile.GetBuffer(_MAX_PATH);
    TCHAR *lpsz_new = cs_newfile.GetBuffer(_MAX_PATH);
#if _MSC_VER >= 1400
    _tcscpy_s(szSource, _MAX_PATH, lpsz_current);
    _tcscpy_s(szDestination, _MAX_PATH, lpsz_new);
#else
    _tcscpy(szSource, lpsz_current);
    _tcscpy(szDestination, lpsz_new);
#endif
    m_currfile.ReleaseBuffer();
    cs_newfile.ReleaseBuffer();

    // Must end with double NULL
    szSource[m_currfile.GetLength() + 1] = TCHAR('\0');
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

void PWScore::ChangePassword(const CMyString &newPassword)
{
    SetPassKey(newPassword);
    m_changed = true;
}

// Finds stuff based on title & user fields only
POSITION
PWScore::Find(const CMyString &a_group,const CMyString &a_title,
              const CMyString &a_user) const
{
    POSITION listPos = m_pwlist.GetHeadPosition();
    CMyString group, title, user;

    while (listPos != NULL) {
        const CItemData &item = m_pwlist.GetAt(listPos);
        group = item.GetGroup();
        title = item.GetTitle();
        user = item.GetUser();
        if (group == a_group && title == a_title && user == a_user)
            break;
        m_pwlist.GetNext(listPos);
    }

    return listPos;
}

POSITION
PWScore::Find(const uuid_array_t &uuid) const
{
    POSITION listPos = m_pwlist.GetHeadPosition();
    uuid_array_t pw_uuidEntry;

    while (listPos != NULL) {
        const CItemData &item = m_pwlist.GetAt(listPos);
        item.GetUUID(pw_uuidEntry);
        if (memcmp(pw_uuidEntry, uuid, sizeof(uuid_array_t)) == 0)
            break;
        m_pwlist.GetNext(listPos);
    }
    return listPos;
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

    for (int x=0;x<BlockLength;x+=8) {
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

void PWScore::SetPassKey(const CMyString &new_passkey)
{
    // if changing, clear old
    if (m_passkey_len > 0) {
        trashMemory(m_passkey, ((m_passkey_len + 7)/8)*8);
        delete[] m_passkey;
    }

    m_passkey_len = new_passkey.GetLength() * sizeof(TCHAR);

    int BlockLength = ((m_passkey_len + 7)/8)*8;
    m_passkey = new unsigned char[BlockLength];
    LPCTSTR plaintext = LPCTSTR(new_passkey);
    EncryptPassword((const unsigned char *)plaintext, m_passkey_len,
                    m_passkey);
}

CMyString PWScore::GetPassKey() const
{
    CMyString retval(_T(""));
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
PWScore::ImportKeePassTextFile(const CMyString &filename)
{
    static const TCHAR *ImportedPrefix = { _T("ImportedKeePass") };
    ifstreamT ifs(filename);

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

        AddEntryToTail(temp);
    }
    ifs.close();

    // TODO: maybe return an error if the full end of the file was not reached?

    m_changed = true;
    return SUCCESS;
}

// GetUniqueGroups - Creates an array of all group names, with no duplicates.
void PWScore::GetUniqueGroups(CStringArray &aryGroups)
{
    aryGroups.RemoveAll();
    POSITION listPos = GetFirstEntryPosition();
    while (listPos != NULL) {
        CItemData &ci = GetEntryAt(listPos);
        CString strThisGroup = ci.GetGroup();
        // Is this group already in the list?
        bool bAlreadyInList=false;
        for(int igrp=0; igrp<aryGroups.GetSize(); igrp++) {
            if(aryGroups[igrp] == strThisGroup) {
                bAlreadyInList = true;
                break;
            }
        }
        if(!bAlreadyInList) aryGroups.Add(strThisGroup);
        GetNextEntry(listPos);
    }
}

void PWScore::SetDisplayStatus(TCHAR *p_char_displaystatus, const int length)
{
    m_displaystatus = CString(p_char_displaystatus, length);
}

void PWScore::CopyPWList(const ItemList &in)
{
    // Clear output
    m_pwlist.RemoveAll();
    // Get head of input
    POSITION listPos = in.GetHeadPosition();
    // Copy them across in order
    while (listPos != NULL) {
        const CItemData ci = in.GetAt(listPos);
        m_pwlist.AddTail(ci);
        in.GetNext(listPos);
    }
    m_changed = true;
}

// The following structure needed for remembering details of uuids to
// ensure they are unique
struct st_uuids {
    DWORD dw_uuidA;
    DWORD dw_uuidB;
    DWORD dw_uuidC;
    DWORD dw_uuidD;
    POSITION nPos;
};

bool
PWScore::Validate(CString &status)
{
    // Check uuid is valid
    // Check uuids are unique
    // Check PWH is valid
    uuid_array_t uuid_array;
    int n = -1;
    unsigned num_PWH_fixed = 0;
    unsigned num_uuid_fixed = 0;
    unsigned num_uuid_notunique = 0;

    TRACE(_T("%s : Start validation\n"), PWSUtil::GetTimeStamp());
    st_uuids *uuids = new st_uuids [GetNumEntries() + 1];
    const unsigned short nMajor = GetCurrentMajorVersion();
    const unsigned short nMinor = GetCurrentMinorVersion();

    POSITION listPos = GetFirstEntryPosition();
    while (listPos != NULL) {
        CItemData &ci = GetEntryAt(listPos);
        ci.GetUUID(uuid_array);
        n++;
        if (uuid_array[0] == 0x00)
            num_uuid_fixed += ci.ValidateUUID(nMajor, nMinor, uuid_array);
#if _MSC_VER >= 1400
        memcpy_s(&uuids[n].dw_uuidA, 16, uuid_array, 16);
#else
        memcpy(&uuids[n].dw_uuidA, uuid_array, 16);
#endif
        uuids[n].nPos = listPos;
        num_PWH_fixed += ci.ValidatePWHistory();
        GetNextEntry(listPos);
    } // while

    // Curently brute force O(n^2)
    // Sorting & searching would be O(N*logN)
    // Best to use a hash or set and test for membership O(1)
    for (int i = 0; i < n - 1; i++) {
        for (int j = i + 1; j < n; j++) {
            if (uuids[i].dw_uuidA == uuids[j].dw_uuidA &&
                uuids[i].dw_uuidB == uuids[j].dw_uuidB &&
                uuids[i].dw_uuidC == uuids[j].dw_uuidC && 
                uuids[i].dw_uuidD == uuids[j].dw_uuidD) {
                CItemData &ci = GetEntryAt(uuids[j].nPos);
                ci.CreateUUID();
                ci.GetUUID(uuid_array);
#if _MSC_VER >= 1400
                memcpy_s(&uuids[j].dw_uuidA, 16, uuid_array, 16);
#else
                memcpy(&uuids[j].dw_uuidA, uuid_array, 16);
#endif
                num_uuid_notunique++;
            }
        }
    }
    delete[] uuids;
    TRACE(_T("%s : End validation. %d entries processed\n"), PWSUtil::GetTimeStamp(), n + 1);
    if ((num_uuid_fixed + num_uuid_notunique + num_PWH_fixed) > 0) {
        status.Format(IDSC_NUMPROCESSED,
                      n + 1, num_uuid_fixed, num_uuid_notunique, num_PWH_fixed);
        SetChanged(true);
        return true;
    } else {
        return false;
    }
}

BOOL
PWScore::GetIncBackupFileName(const CString &cs_filenamebase,
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
  memset(m_file_uuid_array, 0x00, sizeof(m_file_uuid_array));
}
  
void PWScore::SetFileUUID(uuid_array_t &file_uuid_array)
{
  memcpy(m_file_uuid_array, file_uuid_array, sizeof(m_file_uuid_array));
}

void PWScore::GetFileUUID(uuid_array_t &file_uuid_array)
{
  memcpy(file_uuid_array, m_file_uuid_array, sizeof(file_uuid_array));
}

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

#include <bitset>

unsigned char PWScore::m_session_key[20];
unsigned char PWScore::m_session_salt[20];
unsigned char PWScore::m_session_initialized = false;
CString PWScore::m_hdr;

PWScore::PWScore() : m_currfile(_T("")), m_changed(false),
                     m_usedefuser(false), m_defusername(_T("")),
                     m_ReadFileVersion(PWSfile::UNKNOWN_VERSION),
                     m_passkey(NULL), m_passkey_len(0)
{
  if (!PWScore::m_session_initialized) {
	CItemData::SetSessionKey(); // per-session initialization
    PWSrand::GetInstance()->GetRandomData(m_session_key, sizeof(m_session_key) );
    PWSrand::GetInstance()->GetRandomData(m_session_salt, sizeof(m_session_salt) );

	PWScore::m_session_initialized = true;
  }
}

PWScore::~PWScore()
{
  if (m_passkey_len > 0) {
    trashMemory(m_passkey, ((m_passkey_len + 7)/8)*8);
    delete[] m_passkey;
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
PWScore::NewFile(const CMyString &passkey)
{
   ClearData();
   SetPassKey(passkey);
   m_changed = false;
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

  out->Close();
  delete out;

  m_changed = false;
  m_ReadFileVersion = version; // needed when saving a V17 as V20 1st time [871893]

  return SUCCESS;
}

int
PWScore::WritePlaintextFile(const CMyString &filename,
                            const bool &bwrite_header,
                            const std::bitset<16> &bsFields,
							const CString &subgroup,
							const int &iObject, const int &iFunction,
							TCHAR &delimiter, const ItemList *il)
{
#ifdef UNICODE
  wofstream ofs((const wchar_t *)LPCTSTR(filename));
#else
  ofstream ofs((const char *)LPCTSTR(filename));
#endif
  if (!ofs)
    return CANT_OPEN_FILE;
  if (bwrite_header) {
	if ( bsFields.count() == bsFields.size()) {
	  if (m_hdr.IsEmpty())
	    m_hdr.LoadString(IDSC_EXPORTHEADER);
	  ofs << m_hdr << endl;
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

		ofs << hdr.Left(hdr_len) << endl;
	}

  }

  CItemData temp;
  const ItemList &pwlist = (il == NULL) ? m_pwlist : *il;

  POSITION listPos = pwlist.GetHeadPosition();

  while (listPos != NULL) {
    temp = pwlist.GetAt(listPos);
    const CMyString line = temp.GetPlaintext(TCHAR('\t'), bsFields, subgroup, iObject, iFunction, delimiter);
    if (!line.IsEmpty() != 0)
    	ofs << line << endl;
    pwlist.GetNext(listPos);
  }
  ofs.close();

  return SUCCESS;
}

int
PWScore::WriteXMLFile(const CMyString &filename, const TCHAR delimiter,
                      const ItemList *il)
{
	ofstream of(filename);

	if (!of)
		return CANT_OPEN_FILE;

	CList<PWHistEntry, PWHistEntry&>* pPWHistList;
	CMyString tmp, pwh;
	CString cs_tmp;
	uuid_array_t uuid_array;

	TCHAR buffer[8];
	time_t time_now;
	int id = 1;

	pPWHistList = new CList<PWHistEntry, PWHistEntry&>;

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
	of << _T("Database=\"") << tmp << _T("\"") << endl;
	of << _T("ExportTimeStamp=\"") << now << _T("\"") << endl;
	cs_tmp.Format(_T("%d.%02d"), m_nCurrentMajorVersion, m_nCurrentMinorVersion);
	of << _T("FromDatabaseFormat=\"") << cs_tmp << _T("\"") << endl;
	if (!m_wholastsaved.IsEmpty())
		of << _T("WhoSaved=\"") << wls << _T("\"") << endl;
	if (!m_whatlastsaved.IsEmpty())
		of << _T("WhatSaved=\"") << m_whatlastsaved << _T("\"") << endl;
	of << _T("xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"") << endl;
	of << _T("xsi:noNamespaceSchemaLocation=\"pwsafe.xsd\">") << endl;
	of << endl;

	while (listPos != NULL) {
		CItemData temp = pwlist.GetAt(listPos);
#if _MSC_VER >= 1400
		_itoa_s( id, buffer, 8, 10 );
#else
		_itoa( id, buffer, 10 );
#endif
		of << _T("\t<entry id=\"") << buffer << _T("\">") << endl;
		// TODO: need to handle entity escaping of values.
		tmp =  temp.GetGroup();
		if (!tmp.IsEmpty())
			of << _T("\t\t<group><![CDATA[") << tmp << _T("]]></group>") << endl;

		tmp = temp.GetTitle();
		if (!tmp.IsEmpty())
			of <<_T("\t\t<title><![CDATA[") << tmp << _T("]]></title>") << endl;

		tmp = temp.GetUser();
		if (!tmp.IsEmpty())
			of << _T("\t\t<username><![CDATA[") << tmp << _T("]]></username>") << endl;

		tmp = temp.GetPassword();
		if (!tmp.IsEmpty())
			of << _T("\t\t<password><![CDATA[") << tmp << _T("]]></password>") << endl;

		tmp = temp.GetURL();
		if (!tmp.IsEmpty())
			of << _T("\t\t<url><![CDATA[") << tmp << _T("]]></url>") << endl;

		tmp = temp.GetAutoType();
		if (!tmp.IsEmpty())
			of << _T("\t\t<autotype><![CDATA[") << tmp << _T("]]></autotype>") << endl;

		tmp = temp.GetNotes(delimiter);
		if (!tmp.IsEmpty())
			of << _T("\t\t<notes><![CDATA[") << tmp << _T("]]></notes>") << endl;
		temp.GetUUID(uuid_array);
		TCHAR uuid_buffer[33];
#if _MSC_VER >= 1400
		_stprintf_s(uuid_buffer, 33,
#else
		_stprintf(uuid_buffer,
#endif
			_T("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"), 
			uuid_array[0], uuid_array[1], uuid_array[2], uuid_array[3],
			uuid_array[4], uuid_array[5], uuid_array[6], uuid_array[7],
			uuid_array[8], uuid_array[9], uuid_array[10], uuid_array[11],
			uuid_array[12], uuid_array[13], uuid_array[14], uuid_array[15]);
		uuid_buffer[32] = TCHAR('\0');
		of << _T("\t\t<uuid><![CDATA[") << uuid_buffer << _T("]]></uuid>") << endl;

		tmp = temp.GetCTimeXML();
		if (!tmp.IsEmpty()) {
			of << _T("\t\t<ctime>") << endl;
			of << _T("\t\t\t<date>") << tmp.Left(10) << _T("</date>") << endl;
			of << _T("\t\t\t<time>") << tmp.Right(8) << _T("</time>") << endl;
			of << _T("\t\t</ctime>") << endl;
		}

		tmp = temp.GetATimeXML();
		if (!tmp.IsEmpty()) {
			of << _T("\t\t<atime>") << endl;
			of << _T("\t\t\t<date>") << tmp.Left(10) << _T("</date>") << endl;
			of << _T("\t\t\t<time>") << tmp.Right(8) << _T("</time>") << endl;
			of << _T("\t\t</atime>") << endl;
		}

		tmp = temp.GetLTimeXML();
		if (!tmp.IsEmpty()) {
			of << _T("\t\t<ltime>") << endl;
			of << _T("\t\t\t<date>") << tmp.Left(10) << _T("</date>") << endl;
			of << _T("\t\t\t<time>") << tmp.Right(8) << _T("</time>") << endl;
			of << _T("\t\t</ltime>") << endl;
		}

		tmp = temp.GetPMTimeXML();
		if (!tmp.IsEmpty()) {
			of << _T("\t\t<pmtime>") << endl;
			of << _T("\t\t\t<date>") << tmp.Left(10) << _T("</date>") << endl;
			of << _T("\t\t\t<time>") << tmp.Right(8) << _T("</time>") << endl;
			of << _T("\t\t</pmtime>") << endl;
		}

		tmp = temp.GetRMTimeXML();
		if (!tmp.IsEmpty()) {
			of << _T("\t\t<rmtime>") << endl;
			of << _T("\t\t\t<date>") << tmp.Left(10) << _T("</date>") << endl;
			of << _T("\t\t\t<time>") << tmp.Right(8) << _T("</time>") << endl;
			of << _T("\t\t</rmtime>") << endl;
		}

		BOOL pwh_status;
		int pwh_max, pwh_num;
		temp.CreatePWHistoryList(pwh_status, pwh_max, pwh_num, pPWHistList, TMC_XML);
		if (pwh_status == TRUE || pwh_max > 0 || pwh_num > 0) {
			of << _T("\t\t<pwhistory>") << endl;
#if _MSC_VER >= 1400
			_stprintf_s(buffer, 3, _T("%1d"), pwh_status);
			of << _T("\t\t\t<status>") << buffer << _T("</status>") << endl;

			_stprintf_s(buffer, 3, "%2d", pwh_max);
			of << _T("\t\t\t<max>") << buffer << _T("</max>") << endl;

			_stprintf_s(buffer, 3, "%2d", pwh_num);
			of << _T("\t\t\t<num>") << buffer << _T("</num>") << endl;
#else
			_stprintf(buffer, "%1d", pwh_status);
			of << _T("\t\t\t<status>") << buffer << _T("</status>") << endl;

			_stprintf(buffer, "%2d", pwh_max);
			of << _T("\t\t\t<max>") << buffer << _T("</max>") << endl;

			_stprintf(buffer, "%2d", pwh_num);
			of << _T("\t\t\t<num>") << buffer << _T("</num>") << endl;
#endif
			if (pPWHistList->GetCount() > 0) {
				of << _T("\t\t\t<history_entries>") << endl;
				POSITION listpos = pPWHistList->GetHeadPosition();
				int num = 1;
				while (listpos != NULL) {
#if _MSC_VER >= 1400
					_itoa_s( num, buffer, 8, 10 );
#else
					_itoa( num, buffer, 10 );
#endif
					of << _T("\t\t\t\t<history_entry num=\"") << buffer << _T("\">") << endl;
					const PWHistEntry pwshe = pPWHistList->GetAt(listpos);
					of << _T("\t\t\t\t\t<changed>") << endl;
					of << _T("\t\t\t\t\t\t<date>") << pwshe.changedate.Left(10) << _T("</date>") << endl;
					of << _T("\t\t\t\t\t\t<time>") << pwshe.changedate.Right(8) << _T("</time>") << endl;
					of << _T("\t\t\t\t\t</changed>") << endl;
					of << _T("\t\t\t\t\t<oldpassword><![CDATA[") << pwshe.password << _T("]]></oldpassword>") << endl;
					of << _T("\t\t\t\t</history_entry>") << endl;

					pPWHistList->GetNext(listpos);
					num++;
				}
				of << _T("\t\t\t</history_entries>") << endl;
			}
			of << _T("\t\t</pwhistory>") << endl;
			pPWHistList->RemoveAll();
		}

		of << _T("\t</entry>") << endl;
		of << endl;

		pwlist.GetNext(listPos);
		id++;
	}
	of << _T("</passwordsafe>") << endl;
	of.close();
	delete pPWHistList;

	return SUCCESS;
}

int
PWScore::ImportXMLFile(const CString &ImportedPrefix, const CString &strXMLFileName,
				const CString &strXSDFileName, CString &strErrors,
				int &numValidated, int &numImported)
{
	PWSXML *iXML;
	bool status, validation;

	iXML = new PWSXML;
	strErrors = _T("");

	validation = true;
	status = iXML->XMLProcess(validation, ImportedPrefix, strXMLFileName, strXSDFileName);
	strErrors = iXML->m_strResultText;
	if (!status) {
		delete iXML;
		return XML_FAILED_VALIDATION;
	}

	numValidated = iXML->m_numEntriesValidated;

	iXML->SetCore(this);
	validation = false;
	status = iXML->XMLProcess(validation, ImportedPrefix, strXMLFileName, strXSDFileName);
	strErrors = iXML->m_strResultText;
	if (!status) {
		delete iXML;
		return XML_FAILED_IMPORT;
	}

	numImported = iXML->m_numEntriesImported;

	delete iXML;
	m_changed = true;
	return SUCCESS;
}

int
PWScore::ImportPlaintextFile(const CMyString &ImportedPrefix,
                             const CMyString &filename, CString &strErrors,
                             TCHAR fieldSeparator, TCHAR delimiter,
                             int &numImported, int &numSkipped, bool bimport_preV3)
{
#ifdef UNICODE
  wifstream ifs((const wchar_t *)LPCTSTR(filename));
#else
  ifstream ifs((const char *)LPCTSTR(filename));
#endif
  numImported = numSkipped = 0;
  strErrors = _T("");

  CItemData temp;
  CString buffer;

  if (m_hdr.IsEmpty())
    m_hdr.LoadString(IDSC_EXPORTHEADER);

  int numlines = 0;

  // Order of fields determined in CItemData::GetPlaintext()
  enum Fields {GROUPTITLE, USER, PASSWORD, URL, AUTOTYPE,
               CTIME, PMTIME, ATIME, LTIME, RMTIME,
               HISTORY, NOTES, NUMFIELDS};

  enum Fields_PreV3 {GROUPTITLE_V1V2, USER_V1V2, PASSWORD_V1V2,
                     NOTES_V1V2, NUMFIELDS_V1V2};
  if (!ifs)
    return CANT_OPEN_FILE;
  const int i_numfields = bimport_preV3 ? NUMFIELDS_V1V2 : NUMFIELDS;
  const int i_notes = bimport_preV3 ? NOTES_V1V2 : NOTES;

  for (;;) {
    // read a single line.
    string linebuf;
    if (!getline(ifs, linebuf, TCHAR('\n'))) break;
    numlines++;

    // remove MS-DOS linebreaks, if needed.
    if (!linebuf.empty() && *(linebuf.end() - 1) == TCHAR('\r')) {
      linebuf.resize(linebuf.size() - 1);
    }

	if (numlines == 1 && CString(linebuf.c_str()) == m_hdr) {
		strErrors.LoadString(IDSC_IMPHDRLINEIGNORED);
		continue;
	}

    // tokenize into separate elements
    vector<string> tokens;
    for (int startpos = 0; ; ) {
      int nextchar = linebuf.find_first_of(fieldSeparator, startpos);
      if (nextchar >= 0 && (int)tokens.size() < i_notes) {
        tokens.push_back(linebuf.substr(startpos, nextchar - startpos));
        startpos = nextchar + 1;
      } else {
        // Here for the Notes field. Notes may be double-quoted, and
        // if they are, they may span more than one line.
        string note(linebuf.substr(startpos));
        unsigned int first_quote = note.find_first_of('\"');
        unsigned int last_quote = note.find_last_of('\"');
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
            unsigned int fq = linebuf.find_first_of(TCHAR('\"'));
            unsigned int lq = linebuf.find_last_of(TCHAR('\"'));
            noteClosed = (fq == lq && fq != string::npos);
          } while (!noteClosed);
        } // multiline note processed
        tokens.push_back(note);
        break;
      }
    }
	if ((int)tokens.size() != i_numfields) {
		buffer.Format(IDSC_IMPINVALIDINPUT, numlines, fieldSeparator);
		strErrors += buffer;
		numSkipped++; // malformed entry
		continue; // try to process next records
    }

    // Start initializing the new record.
    temp.Clear();
    temp.CreateUUID();
    temp.SetUser(CMyString(tokens[USER].c_str()));
    temp.SetPassword(CMyString(tokens[PASSWORD].c_str()));

    // The group and title field are concatenated.
    // If the title field has periods, then it in doubleqoutes
    const string &grouptitle = tokens[GROUPTITLE];

    if (grouptitle[grouptitle.length()-1] == TCHAR('\"')) {
      size_t leftquote = grouptitle.find(TCHAR('\"'));
      if (leftquote != grouptitle.length()-1) {
        temp.SetGroup(grouptitle.substr(0, leftquote-1).c_str());
        temp.SetTitle(grouptitle.substr(leftquote+1,
                                        grouptitle.length()-leftquote-2).c_str(), delimiter);
      } else { // only a single " ?!
        // probably wrong, but a least we don't lose data
        temp.SetTitle(grouptitle.c_str(), delimiter);
      }
    } else { // title has no period
      size_t lastdot = grouptitle.find_last_of(TCHAR('.'));
      if (lastdot != string::npos) {
        CMyString newgroup(ImportedPrefix.IsEmpty() ?
                           _T("") : ImportedPrefix + _T("."));
        newgroup += grouptitle.substr(0, lastdot).c_str();
        temp.SetGroup(newgroup);
        temp.SetTitle(grouptitle.substr(lastdot + 1).c_str(), delimiter);
      } else {
        temp.SetGroup(ImportedPrefix);
        temp.SetTitle(grouptitle.c_str(), delimiter);
      }
    }

    // New 3.0 fields: URL, AutoType, CTime
    // XXX History NOT supported (yet)
	if (!bimport_preV3) {
      temp.SetURL(tokens[URL].c_str());
      temp.SetAutoType(tokens[AUTOTYPE].c_str());
      temp.SetCTime(tokens[CTIME].c_str());
      temp.SetPMTime(tokens[PMTIME].c_str());
      temp.SetATime(tokens[ATIME].c_str());
      temp.SetLTime(tokens[LTIME].c_str());
      temp.SetRMTime(tokens[RMTIME].c_str());
		CMyString newPWHistory;
		CString strPWHErrors;
		buffer.Format(IDSC_IMPINVALIDPWH, numlines);
		switch (PWSUtil::VerifyImportPWHistoryString(tokens[HISTORY].c_str(), newPWHistory, strPWHErrors)) {
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
		strErrors += buffer;
	}

    // The notes field begins and ends with a double-quote, with
    // no special escaping of any other internal characters.
    string quotedNotes = tokens[i_notes];
    if (!quotedNotes.empty() &&
        *quotedNotes.begin() == TCHAR('\"') &&
        *(quotedNotes.end() - 1) == TCHAR('\"')) {
      quotedNotes = quotedNotes.substr(1, quotedNotes.size() - 2);
      temp.SetNotes(CMyString(quotedNotes.c_str()), delimiter);
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

  // prepare handling of pre-2.0 DEFUSERCHR conversion
  if (m_ReadFileVersion == PWSfile::V17) {
    in->SetDefUsername(m_defusername);
    m_nCurrentMajorVersion = PWSfile::V17;
    m_nCurrentMinorVersion = 0;
  } else {
  	// for 2.0 & later, get pref string and tree display status
  	// both possibly empty
    PWSprefs::GetInstance()->Load(in->GetPrefString());
    m_nCurrentMajorVersion = in->GetCurrentMajorVersion();
    m_nCurrentMinorVersion = in->GetCurrentMinorVersion();
  }

  // Get pref string and tree display status & who saved when
  // all possibly empty!
  PWSprefs::GetInstance()->Load(in->GetPrefString());
  m_displaystatus = in->GetDisplayStatus();
  m_whenlastsaved = in->GetWhenLastSaved();
  m_wholastsaved = in->GetWhoLastSaved();
   m_whatlastsaved = in->GetWhatLastSaved();

   ClearData(); //Before overwriting old data, but after opening the file...

   SetPassKey(a_passkey); // so user won't be prompted for saves

   CItemData temp;

   status = in->ReadRecord(temp);

   while (status == PWSfile::SUCCESS) {
     m_pwlist.AddTail(temp);
     temp.Clear(); // Rather than creating a new one each time.
     status = in->ReadRecord(temp);
   }

   status = in->Close(); // in V3 this checks integrity
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

  m_passkey_len = new_passkey.GetLength();

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
      for (i = 0; i < BS; i++)
        if (x + i < m_passkey_len)
          retval += curblock[i];
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
  static const TCHAR *ImportedPrefix = { "ImportedKeePass" };
#ifdef UNICODE
  wifstream ifs((const wchar_t *)LPCTSTR(filename));
#else
  ifstream ifs((const char *)LPCTSTR(filename));
#endif

  if (!ifs) {
    return CANT_OPEN_FILE;
  }

  string linebuf;

  string group;
  string title;
  string user;
  string passwd;
  string notes;

  // read a single line.
  if (!getline(ifs, linebuf, TCHAR('\n')) || linebuf.empty()) {
    return INVALID_FORMAT;
  }

  // the first line of the keepass text file contains a few garbage characters
  linebuf = linebuf.erase(0, linebuf.find("["));

  int pos = -1;
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
      notes.append("\r\n\r\n");
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

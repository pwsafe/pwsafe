// file PWScore.cpp
//-----------------------------------------------------------------------------

#include "PWScore.h"
#include "BlowFish.h"
#include "PWSprefs.h"
#include "PWSrand.h"
#include "Util.h"
#include "PWSXML.h"

#include <fstream> // for WritePlaintextFile
#include <iostream>
#include <string>
#include <vector>
using namespace std;

#include <LMCONS.H> // for UNLEN definition
#include <io.h> // low level file routines for locking
#include <fcntl.h> // constants _O_* for above
#include <sys/stat.h> // constants _S_* for above
#include <bitset>

unsigned char PWScore::m_session_key[20];
unsigned char PWScore::m_session_salt[20];
unsigned char PWScore::m_session_initialized = false;
CString PWScore::m_hdr(_T("Group/Title\tUsername\tPassword\tURL\tAutoType\tCreated Time\tPassword Modified Time\tLast Access Time\tPassword Expiry Date\tRecord Modified Time\tHistory\tNotes"));

PWScore::PWScore() : m_currfile(_T("")), m_changed(false),
                     m_usedefuser(false), m_defusername(_T("")),
                     m_ReadFileVersion(PWSfile::UNKNOWN_VERSION),
                     m_passkey(NULL), m_passkey_len(0),
                     m_lockFileHandle(INVALID_HANDLE_VALUE)
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
  out->Close();
  delete out;

  m_changed = false;
  m_ReadFileVersion = version; // needed when saving a V17 as V20 1st time [871893]

  return SUCCESS;
}

int
PWScore::WritePlaintextFile(const CMyString &filename, const bool &bwrite_header, const std::bitset<16> &bsFields,
							const CString &subgroup, const int &iObject, const int &iFunction,
							TCHAR &delimiter)
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
	  ofs << m_hdr << endl;
	} else {
		CString hdr = _T("");
		if (bsFields.test(CItemData::GROUP))
			hdr += _T("Group/Title\t");
		if (bsFields.test(CItemData::USER))
			hdr += _T("Username\t");
		if (bsFields.test(CItemData::PASSWORD))
			hdr += _T("Password\t");
		if (bsFields.test(CItemData::URL))
			hdr += _T("URL\t");
		if (bsFields.test(CItemData::AUTOTYPE))
			hdr += _T("AutoType\t");
		if (bsFields.test(CItemData::CTIME))
			hdr += _T("Created Time\t");
		if (bsFields.test(CItemData::PMTIME))
			hdr += _T("Password Modified Time\t");
		if (bsFields.test(CItemData::ATIME))
			hdr += _T("Last Access Time\t");
		if (bsFields.test(CItemData::LTIME))
			hdr += _T("Password Expiry Date\t");
		if (bsFields.test(CItemData::RMTIME))
			hdr += _T("Record Modified Time\t");
		if (bsFields.test(CItemData::PWHIST))
			hdr += _T("History\t");
		if (bsFields.test(CItemData::NOTES))
			hdr += _T("Notes");

		int hdr_len = hdr.GetLength();
		if (hdr.Right(1) == _T("\t"))
			hdr_len--;

		ofs << hdr.Left(hdr_len) << endl;
	}
	
  }

  CItemData temp;
  POSITION listPos = m_sorted_pwlist.GetHeadPosition();

  while (listPos != NULL) {
    temp = m_sorted_pwlist.GetAt(listPos);
    const CMyString line = temp.GetPlaintext(TCHAR('\t'), bsFields, subgroup, iObject, iFunction, delimiter);
    if (!line.IsEmpty() != 0)
    	ofs << line << endl;
    m_sorted_pwlist.GetNext(listPos);
  }
  ofs.close();

  return SUCCESS;
}

int
PWScore::WriteXMLFile(const CMyString &filename, const TCHAR delimiter)
{
	ofstream of(filename);

	if (!of)
		return CANT_OPEN_FILE;

	CList<PWHistEntry, PWHistEntry&>* pPWHistList;
	CMyString tmp, pwh;

	char buffer[8];
	time_t time_now;
	int id = 1;
	
	pPWHistList = new CList<PWHistEntry, PWHistEntry&>;
	POSITION listPos = m_sorted_pwlist.GetHeadPosition();

	time(&time_now);
	const CMyString now = PWSUtil::ConvertToDateTimeString(time_now, XML);

	of << "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>" << endl;
	of << "<?xml-stylesheet type=\"text/xsl\" href=\"pwsafe.xsl\"?>" << endl;
	of << endl;
	of << "<passwordsafe" << endl;
	tmp = m_currfile;
	tmp.Replace(_T("&"), _T("&amp;"));
	of << "Database=\"" << tmp << "\"" << endl;
	of << "ExportTimeStamp=\"" << now << "\"" << endl;
	of << "delimiter=\"" << delimiter << "\"" << endl;
	of << "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"" << endl;
	of << "xsi:noNamespaceSchemaLocation=\"pwsafe.xsd\">" << endl;
	of << endl;

	while (listPos != NULL) {
		CItemData temp = m_sorted_pwlist.GetAt(listPos);
#if _MSC_VER >= 1400
		_itoa_s( id, buffer, 8, 10 );
#else
		_itoa( id, buffer, 10 );
#endif
		of << "\t<entry id=\"" << buffer << "\">" << endl;
		// TODO: need to handle entity escaping of values.
		tmp =  temp.GetGroup();
		if (!tmp.IsEmpty())
			of << "\t\t<group><![CDATA[" << tmp << "]]></group>" << endl;

		tmp = temp.GetTitle();
		if (!tmp.IsEmpty())
			of << "\t\t<title><![CDATA[" << tmp << "]]></title>" << endl;

		tmp = temp.GetUser();
		if (!tmp.IsEmpty())
			of << "\t\t<username><![CDATA[" << tmp << "]]></username>" << endl;

		tmp = temp.GetPassword();
		if (!tmp.IsEmpty())
			of << "\t\t<password><![CDATA[" << tmp << "]]></password>" << endl;

		tmp = temp.GetURL();
		if (!tmp.IsEmpty())
			of << "\t\t<url><![CDATA[" << tmp << "]]></url>" << endl;

		tmp = temp.GetAutoType();
		if (!tmp.IsEmpty())
			of << "\t\t<autotype><![CDATA[" << tmp << "]]></autotype>" << endl;

		tmp = temp.GetNotes(delimiter);
		if (!tmp.IsEmpty())
			of << "\t\t<notes><![CDATA[" << tmp << "]]></notes>" << endl;

		tmp = temp.GetCTimeXML();
		if (!tmp.IsEmpty()) {
			of << "\t\t<ctime>" << endl;
			of << "\t\t\t<date>" << tmp.Left(10) << "</date>" << endl;
			of << "\t\t\t<time>" << tmp.Right(8) << "</time>" << endl;
			of << "\t\t</ctime>" << endl;
		}

		tmp = temp.GetATimeXML();
		if (!tmp.IsEmpty()) {
			of << "\t\t<atime>" << endl;
			of << "\t\t\t<date>" << tmp.Left(10) << "</date>" << endl;
			of << "\t\t\t<time>" << tmp.Right(8) << "</time>" << endl;
			of << "\t\t</atime>" << endl;
		}

		tmp = temp.GetLTimeXML();
		if (!tmp.IsEmpty()) {
			of << "\t\t<ltime>" << endl;
			of << "\t\t\t<date>" << tmp.Left(10) << "</date>" << endl;
			of << "\t\t\t<time>" << tmp.Right(8) << "</time>" << endl;
			of << "\t\t</ltime>" << endl;
		}

		tmp = temp.GetPMTimeXML();
		if (!tmp.IsEmpty()) {
			of << "\t\t<pmtime>" << endl;
			of << "\t\t\t<date>" << tmp.Left(10) << "</date>" << endl;
			of << "\t\t\t<time>" << tmp.Right(8) << "</time>" << endl;
			of << "\t\t</pmtime>" << endl;
		}

		tmp = temp.GetRMTimeXML();
		if (!tmp.IsEmpty()) {
			of << "\t\t<rmtime>" << endl;
			of << "\t\t\t<date>" << tmp.Left(10) << "</date>" << endl;
			of << "\t\t\t<time>" << tmp.Right(8) << "</time>" << endl;
			of << "\t\t</rmtime>" << endl;
		}

		BOOL pwh_status;
		int pwh_max, pwh_num;
		temp.CreatePWHistoryList(pwh_status, pwh_max, pwh_num, pPWHistList, XML);
		if (pwh_status == TRUE || pwh_max > 0 || pwh_num > 0) {
			of << "\t\t<pwhistory>" << endl;
#if _MSC_VER >= 1400
			sprintf_s(buffer, 3, "%1d", pwh_status);
			of << "\t\t\t<status>" << buffer << "</status>" << endl;

			sprintf_s(buffer, 3, "%2d", pwh_max);
			of << "\t\t\t<max>" << buffer << "</max>" << endl;

			sprintf_s(buffer, 3, "%2d", pwh_num);
			of << "\t\t\t<num>" << buffer << "</num>" << endl;
#else
			sprintf(buffer, "%1d", pwh_status);
			of << "\t\t\t<status>" << buffer << "</status>" << endl;

			sprintf(buffer, "%2d", pwh_max);
			of << "\t\t\t<max>" << buffer << "</max>" << endl;

			sprintf(buffer, "%2d", pwh_num);
			of << "\t\t\t<num>" << buffer << "</num>" << endl;
#endif
			if (pPWHistList->GetCount() > 0) {
				of << "\t\t\t<history_entries>" << endl;
				POSITION listpos = pPWHistList->GetHeadPosition();
				int num = 1;
				while (listpos != NULL) {
#if _MSC_VER >= 1400
					_itoa_s( num, buffer, 8, 10 );
#else
					_itoa( num, buffer, 10 );
#endif
					of << "\t\t\t\t<history_entry num=\"" << buffer << "\">" << endl;
					const PWHistEntry pwshe = pPWHistList->GetAt(listpos);
					of << "\t\t\t\t\t<changed>" << endl;
					of << "\t\t\t\t\t\t<date>" << pwshe.changedate.Left(10) << "</date>" << endl;
					of << "\t\t\t\t\t\t<time>" << pwshe.changedate.Right(8) << "</time>" << endl;
					of << "\t\t\t\t\t</changed>" << endl;
					of << "\t\t\t\t\t<oldpassword><![CDATA[" << pwshe.password << "]]></oldpassword>" << endl;
					of << "\t\t\t\t</history_entry>" << endl;
					
					pPWHistList->GetNext(listpos);
					num++;
				}
				of << "\t\t\t</history_entries>" << endl;
			}
			of << "\t\t</pwhistory>" << endl;
			pPWHistList->RemoveAll();
		}

		of << "\t</entry>" << endl;
		of << endl;

		m_sorted_pwlist.GetNext(listPos);
		id++;
	}
	of << "</passwordsafe>" << endl;
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
    if (!getline(ifs, linebuf, '\n')) break;
    numlines++;

    // remove MS-DOS linebreaks, if needed.
    if (!linebuf.empty() && *(linebuf.end() - 1) == '\r') {
      linebuf.resize(linebuf.size() - 1);
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
            if (!getline(ifs, linebuf, '\n')) {
              buffer.Format(_T("\nFile ends on line %d before ending double quote of last entry's note field."),
               		numlines);
              strErrors += buffer;
              ifs.close(); // file ends before note closes
              return (numImported > 0) ? SUCCESS : INVALID_FORMAT;
            }
            numlines++;
            // remove MS-DOS linebreaks, if needed.
            if (!linebuf.empty() && *(linebuf.end() - 1) == '\r') {
              linebuf.resize(linebuf.size() - 1);
            }
            note += "\r\n";
            note += linebuf;
            unsigned int fq = linebuf.find_first_of('\"');
            unsigned int lq = linebuf.find_last_of('\"');
            noteClosed = (fq == lq && fq != string::npos);
          } while (!noteClosed);
        } // multiline note processed
        tokens.push_back(note);
        break;
      }
    }
	if ((int)tokens.size() != i_numfields) {
		if (numlines == 1 && CString(linebuf.c_str()) != m_hdr) {
			strErrors = _T("\nHeader line ignored.");
		} else {
			buffer.Format(_T("\nInvalid input on line %d.  Number of fields separated by '%c' is not as expected."),
						numlines, fieldSeparator);
			strErrors += buffer;
			numSkipped++; // malformed entry
		}
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
      size_t lastdot = grouptitle.find_last_of('.');
      if (lastdot != string::npos) {
        CMyString newgroup(ImportedPrefix.IsEmpty() ?
                           "" : ImportedPrefix + ".");
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
		buffer.Format(_T("\nError in Password History on line %d: "), numlines);
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
        *quotedNotes.begin() == '\"' &&
        *(quotedNotes.end() - 1) == '\"') {
      quotedNotes = quotedNotes.substr(1, quotedNotes.size() - 2);
      temp.SetNotes(CMyString(quotedNotes.c_str()), delimiter);
    }

    AddEntryToTail(temp);
    numImported++;
  } // file processing for (;;) loop
  ifs.close();

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

  if (status != PWSfile::SUCCESS) {
    delete in;
    return CANT_OPEN_FILE;
  }
  if (m_ReadFileVersion == PWSfile::UNKNOWN_VERSION) {
    delete in;
    return UNKNOWN_VERSION;
  }

  // prepare handling of pre-2.0 DEFUSERCHR conversion
  if (m_ReadFileVersion == PWSfile::V17)
    in->SetDefUsername(m_defusername);
  else {
  	// for 2.0 & later, get pref string and tree display status
  	// both possibly empty
    PWSprefs::GetInstance()->Load(in->GetPrefString());
    m_displaystatus = in->GetDisplayStatus();
  }

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


int PWScore::BackupCurFile()
{
  // renames CurFile to CurFile~
  CString newname(GetCurFile());
  newname += TCHAR('~');
  return PWSfile::RenameFile(GetCurFile(), newname);
}

void PWScore::ChangePassword(const CMyString &newPassword)
{
  SetPassKey(newPassword);
  m_changed = true;
}

// Finds stuff based on title & user fields only
POSITION
PWScore::Find(const CMyString &a_group,const CMyString &a_title, const CMyString &a_user)
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
    else
      m_pwlist.GetNext(listPos);
  }

  return listPos;
}

POSITION
PWScore::Find(const uuid_array_t &RUEuuid)
{
   POSITION listPos = m_pwlist.GetHeadPosition();
   uuid_array_t pw_uuidEntry;

   while (listPos != NULL)
   {
     const CItemData &item = m_pwlist.GetAt(listPos);
     item.GetUUID(pw_uuidEntry);
	 if (memcmp(pw_uuidEntry, RUEuuid, sizeof(uuid_array_t)) == 0)
		 break;
	 else 
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
  static const char *ImportedPrefix = { "ImportedKeePass" };
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
  if (!getline(ifs, linebuf, '\n') || linebuf.empty()) {
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
    if (*(linebuf.begin()) != '[' || *(linebuf.end() - 1) != ']') {
      return INVALID_FORMAT;
    }

    // set the title: line pattern: [<group>]
    title = linebuf.substr(linebuf.find("[") + 1, linebuf.rfind("]") - 1).c_str();

    // set the group: line pattern: Group: <user>
    if (!getline(ifs, linebuf, '\n') || (pos = linebuf.find("Group: ")) == -1) {
      return INVALID_FORMAT;
    }
    group = ImportedPrefix;
    if (!linebuf.empty()) {
      group.append(".");
      group.append(linebuf.substr(pos + 7));
    }

    // set the user: line pattern: UserName: <user>
    if (!getline(ifs, linebuf, '\n') || (pos = linebuf.find("UserName: ")) == -1) {
      return INVALID_FORMAT;
    }
    user = linebuf.substr(pos + 10);

    // set the url: line pattern: URL: <url>
    if (!getline(ifs, linebuf, '\n') || (pos = linebuf.find("URL: ")) == -1) {
      return INVALID_FORMAT;
    }
    if (!linebuf.substr(pos + 5).empty()) {
      notes.append(linebuf.substr(pos + 5));
      notes.append("\r\n\r\n");
    }

    // set the password: line pattern: Password: <passwd>
    if (!getline(ifs, linebuf, '\n') || (pos = linebuf.find("Password: ")) == -1) {
      return INVALID_FORMAT;
    }
    passwd = linebuf.substr(pos + 10);

    // set the first line of notes: line pattern: Notes: <notes>
    if (!getline(ifs, linebuf, '\n') || (pos = linebuf.find("Notes: ")) == -1) {
      return INVALID_FORMAT;
    }
    notes.append(linebuf.substr(pos + 7));

    // read in any remaining new notes and set up the next record
    for (;;) {
      // see if we hit the end of the file
      if (!getline(ifs, linebuf, '\n')) {
        break;
      }

      // see if we hit a new record
      if (linebuf.find("[") == 0 && linebuf.rfind("]") == linebuf.length() - 1) {
        break;
      }

      notes.append("\r\n");
      notes.append(linebuf);
    }

    // Create & append the new record.
    CItemData temp;
    temp.CreateUUID();
    temp.SetTitle(title.empty() ? group.c_str() : title.c_str());
    temp.SetGroup(group.c_str());
    temp.SetUser(user.empty() ? " " : user.c_str());
    temp.SetPassword(passwd.empty() ? " " : passwd.c_str());
    temp.SetNotes(notes.empty() ? "" : notes.c_str());

    AddEntryToTail(temp);
  }
  ifs.close();

  // TODO: maybe return an error if the full end of the file was not reached?

  return SUCCESS;
}

/*
 * The file lock/unlock functions were first implemented (in 2.08)
 * with Posix semantics (using open(_O_CREATE|_O_EXCL) to detect
 * an existing lock.
 * This fails to check liveness of the locker process, specifically,
 * if a user just turns of her PC, the lock file will remain.
 * So, I'm keeping the Posix code under idef POSIX_FILE_LOCK,
 * and re-implementing using the Win32 API, whose semantics
 * supposedly protect against this scenario.
 * Thanks to Frank (xformer) for discussion on the subject.
 */

static void GetLockFileName(const CMyString &filename,
			    CMyString &lock_filename)
{
  ASSERT(!filename.IsEmpty());
  // derive lock filename from filename
  lock_filename = CMyString(filename.Left(MAX_PATH - 4) + _T(".plk"));
}

bool PWScore::LockFile(const CMyString &filename, CMyString &locker)
{
  CMyString lock_filename;
  GetLockFileName(filename, lock_filename);
#ifdef POSIX_FILE_LOCK
  int fh = _open(lock_filename, (_O_CREAT | _O_EXCL | _O_WRONLY),
                 (_S_IREAD | _S_IWRITE));

  if (fh == -1) { // failed to open exclusively. Already locked, or ???
    switch (errno) {
    case EACCES:
      // Tried to open read-only file for writing, or file’s
      // sharing mode does not allow specified operations, or given path is directory
      locker = _T("Cannot create lock file - no permission in directory?");
      break;
    case EEXIST: // filename already exists
      {
        // read locker data ("user@machine") from file
        TCHAR lockerStr[UNLEN + MAX_COMPUTERNAME_LENGTH + sizeof(TCHAR)*2];
        int fh2 = _open(lock_filename, _O_RDONLY);
        if (fh2 == -1) {
          locker = _T("Unable to determine locker?");
        } else {
          int bytesRead = _read(fh2, lockerStr, sizeof(lockerStr)-1);
          _close(fh2);
          if (bytesRead > 0) {
            lockerStr[bytesRead] = TCHAR('\0');
            locker = lockerStr;
          } else { // read failed for some reason
            locker = _T("Unable to read locker?");
          } // read info from lock file
        } // open lock file for read
      } // EEXIST block
      break;
    case EINVAL: // Invalid oflag or pmode argument
      locker = _T("Internal error: Invalid oflag or pmode argument");
      break;
    case EMFILE: // No more file handles available (too many open files)
      locker = _T("System error: No morefile handles available");
      break;
    case ENOENT: //File or path not found
      locker = _T("File or path not found");
      break;
    default:
      locker = _T("Internal error: Unexpected errno");
      break;
    } // switch (errno)
    return false;
  } else { // valid filehandle, write our info
    TCHAR user[UNLEN+1];
    TCHAR sysname[MAX_COMPUTERNAME_LENGTH+1];
    DWORD len;
    len = sizeof(user);
    if (::GetUserName(user, &len)== FALSE) {
      user[0] = TCHAR('?'); user[1] = TCHAR('\0');
    }
    len = sizeof(sysname);
    if (::GetComputerName(sysname, &len) == FALSE) {
      sysname[0] = TCHAR('?'); sysname[1] = TCHAR('\0');
    }
    int numWrit;
    numWrit = _write(fh, user, _tcslen(user)*sizeof(TCHAR));
    numWrit += _write(fh, _T("@"), _tcslen("@")*sizeof(TCHAR));
    numWrit += _write(fh, sysname, _tcslen(sysname)*sizeof(TCHAR));
    ASSERT(numWrit > 0);
    _close(fh);
    return true;
  }
#else
  // Use Win32 API for locking - supposedly better at
  // detecting dead locking processes
  if (m_lockFileHandle != INVALID_HANDLE_VALUE) {
    // here if we've open another (or same) dbase previously,
    // need to unlock it. A bit inelegant...
    // If app was minimized and ClearData() called, we've a small
    // potential for a TOCTTOU issue here. Worse case, lock
    // will fail.
    UnlockFile(GetCurFile());
  }
  m_lockFileHandle = ::CreateFile(LPCTSTR(lock_filename),
                                  GENERIC_WRITE,
                                  FILE_SHARE_READ,
                                  NULL,
                                  CREATE_ALWAYS, // rely on share to fail if exists!
                                  FILE_ATTRIBUTE_NORMAL| FILE_FLAG_WRITE_THROUGH,
                                  NULL);
  if (m_lockFileHandle == INVALID_HANDLE_VALUE) {
    DWORD error = GetLastError();
    switch (error) {
    case ERROR_SHARING_VIOLATION: // already open by a live process
      {
        // read locker data ("user@machine") from file
        TCHAR lockerStr[UNLEN + MAX_COMPUTERNAME_LENGTH + sizeof(TCHAR)*2];
        // flags here counter (my) intuition, but see
        // http://msdn.microsoft.com/library/default.asp?url=/library/en-us/fileio/base/creating_and_opening_files.asp
        HANDLE h2 = ::CreateFile(LPCTSTR(lock_filename),
                                 GENERIC_READ, FILE_SHARE_WRITE,
                                 NULL, OPEN_EXISTING,
                                 FILE_ATTRIBUTE_NORMAL, NULL);
        if (h2 == INVALID_HANDLE_VALUE) {
          locker = _T("Unable to determine locker?");
        } else {
          DWORD bytesRead;
          (void)::ReadFile(h2, lockerStr, sizeof(lockerStr)-1,
                           &bytesRead, NULL);
          CloseHandle(h2);
          if (bytesRead > 0) {
            lockerStr[bytesRead] = TCHAR('\0');
            locker = lockerStr;
          } else { // read failed for some reason
            locker = _T("Unable to read locker?");
          } // read info from lock file
        } // open lock file for read
      } // ERROR_SHARING_VIOLATION block
      break;
    default:
      locker = _T("Cannot create lock file - no permission in directory?");
      break;
    } // switch (error)
    return false;
  } else { // valid filehandle, write our info
    TCHAR user[UNLEN+1];
    TCHAR sysname[MAX_COMPUTERNAME_LENGTH+1];
    DWORD len;
    len = sizeof(user);
    if (::GetUserName(user, &len)== FALSE) {
      user[0] = TCHAR('?'); user[1] = TCHAR('\0');
    }
    len = sizeof(sysname);
    if (::GetComputerName(sysname, &len) == FALSE) {
      sysname[0] = TCHAR('?'); sysname[1] = TCHAR('\0');
    }
    DWORD numWrit, sumWrit;
    BOOL write_status;
    write_status = ::WriteFile(m_lockFileHandle, user,
                               _tcslen(user)*sizeof(TCHAR),
                               &sumWrit, NULL);
    write_status = ::WriteFile(m_lockFileHandle,
                               _T("@"), _tcslen(_T("@"))*sizeof(TCHAR),
                               &numWrit, NULL);
    sumWrit += numWrit;
    write_status += ::WriteFile(m_lockFileHandle,
                                sysname, _tcslen(sysname)*sizeof(TCHAR),
                                &numWrit, NULL);
    sumWrit += numWrit;
    ASSERT(sumWrit > 0);
    return true;
  }
#endif // POSIX_FILE_LOCK
}

void PWScore::UnlockFile(const CMyString &filename)
{
#ifdef POSIX_FILE_LOCK
  CMyString lock_filename;
  GetLockFileName(filename, lock_filename);
  _unlink(lock_filename);
#else
  // Use Win32 API for locking - supposedly better at
  // detecting dead locking processes
  if (m_lockFileHandle != INVALID_HANDLE_VALUE) {
    CMyString lock_filename;
    GetLockFileName(filename, lock_filename);
    CloseHandle(m_lockFileHandle);
    m_lockFileHandle = INVALID_HANDLE_VALUE;
    DeleteFile(LPCTSTR(lock_filename));
  }
#endif // POSIX_FILE_LOCK
}

bool PWScore::IsLockedFile(const CMyString &filename) const
{
  CMyString lock_filename;
  GetLockFileName(filename, lock_filename);
#ifdef POSIX_FILE_LOCK
  return PWSfile::FileExists(lock_filename);
#else
  // under this scheme, we need to actually try to open the file to determine
  // if it's locked.
  HANDLE h = CreateFile(LPCTSTR(lock_filename),
			GENERIC_WRITE,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING, // don't create one!
			FILE_ATTRIBUTE_NORMAL| FILE_FLAG_WRITE_THROUGH,
			NULL);
  if (h == INVALID_HANDLE_VALUE) {
    DWORD error = GetLastError();
    if (error == ERROR_SHARING_VIOLATION)
      return true;
    else
      return false; // couldn't open it, probably doesn't exist.
  } else {
    CloseHandle(h); // here if exists but lockable.
    return false;
  }
#endif // POSIX_FILE_LOCK
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

void PWScore::SetDisplayStatus(char *p_char_displaystatus, const int length)
{
	m_displaystatus = CString(p_char_displaystatus, length);
}

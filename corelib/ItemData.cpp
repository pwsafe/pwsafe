/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
/// \file ItemData.cpp
//-----------------------------------------------------------------------------

#include "ItemData.h"
#include "BlowFish.h"
#include "TwoFish.h"
#include "PWSrand.h"

#include <time.h>
#include <sstream>

using namespace std;

// hide w_char/char differences where possible:
#ifdef UNICODE
typedef std::wostringstream ostringstreamT;
#else
typedef std::ostringstream ostringstreamT;
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

bool CItemData::IsSessionKeySet = false;
unsigned char CItemData::SessionKey[64];

void CItemData::SetSessionKey()
{
  // must be called once per session, no more, no less
  ASSERT(!IsSessionKeySet);
  PWSrand::GetInstance()->GetRandomData( SessionKey, sizeof( SessionKey ) );
  IsSessionKeySet = true;
}

//-----------------------------------------------------------------------------
// Constructors

CItemData::CItemData()
  : m_Name(NAME), m_Title(TITLE), m_User(USER), m_Password(PASSWORD),
    m_Notes(NOTES), m_UUID(UUID), m_Group(GROUP),
    m_URL(URL), m_AutoType(AUTOTYPE),
    m_tttCTime(CTIME), m_tttPMTime(PMTIME), m_tttATime(ATIME),
    m_tttLTime(LTIME), m_tttRMTime(RMTIME), m_PWHistory(PWHIST),
    m_display_info(NULL)
{
  PWSrand::GetInstance()->GetRandomData( m_salt, SaltLength );
}

CItemData::CItemData(const CItemData &that) :
  m_Name(that.m_Name), m_Title(that.m_Title), m_User(that.m_User),
  m_Password(that.m_Password), m_Notes(that.m_Notes), m_UUID(that.m_UUID),
  m_Group(that.m_Group), m_URL(that.m_URL), m_AutoType(that.m_AutoType),
  m_tttCTime(that.m_tttCTime), m_tttPMTime(that.m_tttPMTime), m_tttATime(that.m_tttATime),
  m_tttLTime(that.m_tttLTime), m_tttRMTime(that.m_tttRMTime), m_PWHistory(that.m_PWHistory),
  m_display_info(that.m_display_info)
{
  ::memcpy((char*)m_salt, (char*)that.m_salt, SaltLength);
  if (!that.m_URFL.empty())
    m_URFL = that.m_URFL;
  else
    m_URFL.clear();
}

CItemData::~CItemData()
{
  if (!m_URFL.empty()) {
    m_URFL.clear();
  }
}

//-----------------------------------------------------------------------------
// Accessors

void CItemData::GetField(const CItemField &field, CMyString &value) const
{
  BlowFish *bf = MakeBlowFish();
  field.Get(value, bf);
  delete bf;
}

void CItemData::GetField(const CItemField &field, unsigned char *value, unsigned int &length) const
{
  BlowFish *bf = MakeBlowFish();
  field.Get(value, length, bf);
  delete bf;
}


CMyString
CItemData::GetName() const
{
   CMyString ret;
   GetField(m_Name, ret);
   return ret;
}

CMyString
CItemData::GetTitle() const
{
   CMyString ret;
   GetField(m_Title, ret);
   return ret;
}

CMyString
CItemData::GetUser() const
{
   CMyString ret;
   GetField(m_User, ret);
   return ret;
}


CMyString
CItemData::GetPassword() const
{
   CMyString ret;
   GetField(m_Password, ret);
   return ret;
}

CMyString
CItemData::GetNotes(TCHAR delimiter) const
{
   CMyString ret;
   GetField(m_Notes, ret);
   if (delimiter != 0) {
     ret.Remove(TCHAR('\r'));
     ret.Replace(TCHAR('\n'), delimiter);
   }
   return ret;
}

CMyString
CItemData::GetGroup() const
{
   CMyString ret;
   GetField(m_Group, ret);
   return ret;
}

CMyString
CItemData::GetURL() const
{
   CMyString ret;
   GetField(m_URL, ret);
   return ret;
}

CMyString
CItemData::GetAutoType() const
{
   CMyString ret;
   GetField(m_AutoType, ret);
   return ret;
}

CMyString
CItemData::GetTime(int whichtime, int result_format) const
{
  time_t t;

  GetTime(whichtime, t);
   
  return PWSUtil::ConvertToDateTimeString(t, result_format);
}

void
CItemData::GetTime(int whichtime, time_t &t) const
{
  unsigned char in[TwoFish::BLOCKSIZE]; // required by GetField
  unsigned int tlen = sizeof(in); // ditto

  switch (whichtime) {
  case ATIME:
    GetField(m_tttATime, (unsigned char *)in, tlen);
    break;
  case CTIME:
    GetField(m_tttCTime, (unsigned char *)in, tlen);
    break;
  case LTIME:
    GetField(m_tttLTime, (unsigned char *)in, tlen);
    break;
  case PMTIME:
    GetField(m_tttPMTime, (unsigned char *)in, tlen);
    break;
  case RMTIME:
    GetField(m_tttRMTime, (unsigned char *)in, tlen);
    break;
  default:
    ASSERT(0);
  }

  if (tlen != 0) {
    ASSERT(tlen == sizeof(t));
    memcpy(&t, in, sizeof(t));
  } else {
    t = 0;
  }
}

CMyString CItemData::GetXMLTime(int indent, const TCHAR *name, time_t t) const
{
  int i;
  const CString tmp = PWSUtil::ConvertToDateTimeString(t, TMC_XML);
  ostringstreamT oss;

  for (i = 0; i < indent; i++) oss << _T("\t");
  oss << _T("<") << name << _T(">") << endl;
  for (i = 0; i <= indent; i++) oss << _T("\t");
  oss << _T("<date>") << LPCTSTR(tmp.Left(10)) << _T("</date>") << endl;
  for (i = 0; i <= indent; i++) oss << _T("\t");
  oss << _T("<time>") << LPCTSTR(tmp.Right(8)) << _T("</time>") << endl;
  for (i = 0; i < indent; i++) oss << _T("\t");
  oss << _T("</") << name << _T(">") << endl;
  CMyString retval(oss.str().c_str());
  return retval;
}


void CItemData::GetUUID(uuid_array_t &uuid_array) const
{
  unsigned int length = sizeof(uuid_array);
  GetField(m_UUID, (unsigned char *)uuid_array, length);
}

void CItemData::GetUnknownField(unsigned char &type, unsigned int &length,
                                unsigned char * &pdata,
                                const CItemField &item) const
{
  ASSERT(pdata == NULL && length == 0);

  const unsigned int BLOCKSIZE = 8;

  type = item.GetType();
  unsigned int flength = item.GetLength();
  length = flength;
  flength += BLOCKSIZE; // ensure that we've enough for at least one block
  pdata = new unsigned char[flength];
  GetField(item, pdata, flength);
}


void CItemData::GetUnknownField(unsigned char &type, unsigned int &length,
                                unsigned char * &pdata,
                                const unsigned int &num) const
{
  const CItemField &unkrfe = m_URFL.at(num);
  GetUnknownField(type, length, pdata, unkrfe);
}

void CItemData::GetUnknownField(unsigned char &type, unsigned int &length,
                                unsigned char * &pdata,
                                const UnknownFieldsConstIter &iter) const
{
  const CItemField &unkrfe = *iter;
  GetUnknownField(type, length, pdata, unkrfe);
}

void
CItemData::SetUnknownField(const unsigned char type,
                           const unsigned int length,
                           const unsigned char * ufield)
{
  CItemField unkrfe(type);
  SetField(unkrfe, ufield, length);
  m_URFL.push_back(unkrfe);
}

/*
 * Password History (PWH):
 * Password history is represented in the entry record as a textual field
 * with the following semantics:
 *
 * Password History Header: 
 * %01x - status for saving PWH for this entry (0 = no; 1 = yes) 
 * %02x - maximum number of entries in this entry 
 * %02x - number of entries currently saved 
 *
 * Each Password History Entry: 
 * %08x - time of this old password was set (time_t) 
 * %04x - length of old password (in TCHAR)
 * %s   - old password 
 *
 * No history being kept for a record can be represented either by the lack
 * of the PWH field (preferred), or by a header of _T("00000"):
 * status = 0, max = 00, num = 00 
 *
 * Note that 0aabb where bb <= aa is possible if password history was enabled in the past
 * but has been disabled and the history hasn't been cleared.
 *
 */

CMyString
CItemData::GetPWHistory() const
{
  CMyString ret;
  GetField(m_PWHistory, ret);
	if (ret == _T("0") || ret == _T("00000"))
		ret = _T("");
  return ret;
}

CMyString CItemData::GetPlaintext(const TCHAR &separator,
                                  const FieldBits &bsFields,
                                  const TCHAR &delimiter) const
{
  CMyString ret(_T(""));

  CMyString grouptitle;
  const CMyString title(GetTitle());
  const CMyString group(GetGroup());
  const CMyString user(GetUser());
  const CMyString url(GetURL());
  const CMyString notes(GetNotes(delimiter));

  // a '.' in title gets Import confused re: Groups
  grouptitle = title;
  if (grouptitle.Find(TCHAR('.')) != -1) {
    if (delimiter != 0) {
      grouptitle.Replace(TCHAR('.'), delimiter);
    } else {
      grouptitle = TCHAR('\"') + title + TCHAR('\"');
    }
  }

  if (!group.IsEmpty())
    grouptitle = group + TCHAR('.') + grouptitle;

	CMyString history(_T(""));
	if (bsFields.test(CItemData::PWHIST)) {
	    // History exported as "00000" if empty, to make parsing easier
   		BOOL pwh_status;
	   	size_t pwh_max, pwh_num;
   		PWHistList PWHistList;

   		CreatePWHistoryList(pwh_status, pwh_max, pwh_num,
                            &PWHistList, TMC_EXPORT_IMPORT);

		//  Build export string
		TCHAR buffer[8];
#if _MSC_VER >= 1400
		_stprintf_s(buffer, 8, _T("%1x%02x%02x"), pwh_status, pwh_max, pwh_num);
#else
		_stprintf(buffer, _T("%1x%02x%02x"), pwh_status, pwh_max, pwh_num);
#endif
		history = CMyString(buffer);
        PWHistList::iterator iter;
        for (iter = PWHistList.begin(); iter != PWHistList.end(); iter++) {
            const PWHistEntry pwshe = *iter;
            history += _T(' ');
            history += pwshe.changedate;
#if _MSC_VER >= 1400
            _stprintf_s(buffer, 8, _T(" %04x "), pwshe.password.GetLength());
#else
            _stprintf(buffer, _T("%04x "), pwshe.password.GetLength());
#endif
            history += CMyString(buffer);
            history += pwshe.password;
        }
	}

	// Notes field must be last, for ease of parsing import
	if (bsFields.count() == bsFields.size()) {
		// Everything - note can't actually set all bits via dialog!
		ret = grouptitle + separator + user + separator +
			GetPassword() + separator + url +
			separator + GetAutoType() + separator +
			GetCTimeExp() + separator +
			GetPMTimeExp() + separator +
			GetATimeExp() + separator +
			GetLTimeExp() + separator +
			GetRMTimeExp() + separator +
			history + separator +
			_T("\"") + notes + _T("\"");
	} else {
		// Not everything
		if (bsFields.test(CItemData::GROUP))
			ret += grouptitle + separator;
		if (bsFields.test(CItemData::USER))
			ret += user + separator;
		if (bsFields.test(CItemData::PASSWORD))
			ret += GetPassword() + separator;
		if (bsFields.test(CItemData::URL))
			ret += url + separator;
		if (bsFields.test(CItemData::AUTOTYPE))
			ret += GetAutoType() + separator;
		if (bsFields.test(CItemData::CTIME))
			ret += GetCTimeExp() + separator;
		if (bsFields.test(CItemData::PMTIME))
			ret += GetPMTimeExp() + separator;
		if (bsFields.test(CItemData::ATIME))
			ret += GetATimeExp() + separator;
		if (bsFields.test(CItemData::LTIME))
			ret += GetLTimeExp() + separator;
		if (bsFields.test(CItemData::RMTIME))
			ret += GetRMTimeExp() + separator;
		if (bsFields.test(CItemData::PWHIST))
			ret += history + separator;
		if (bsFields.test(CItemData::NOTES))
			ret += _T("\"") + notes + _T("\"");
		// remove trailing separator
		if ((CString)ret.Right(1) == separator) {
			int rl = ret.GetLength();
			ret.Left(rl - 1);
		}
	}

    return ret;
}

CMyString CItemData::GetXML(unsigned id, const FieldBits &bsExport) const
{
  ostringstreamT oss;
  // TODO: need to handle entity escaping of values.
  oss << _T("\t<entry id=\"") << id << _T("\">") << endl;

  CMyString tmp;

  tmp = GetGroup();
  if (bsExport.test(CItemData::GROUP) && !tmp.IsEmpty())
    oss << _T("\t\t<group><![CDATA[") << LPCTSTR(tmp)
        << _T("]]></group>") << endl;

  // Title mandatory (see pwsafe.xsd)
  tmp = GetTitle();
  oss <<_T("\t\t<title><![CDATA[") << LPCTSTR(tmp)
      << _T("]]></title>") << endl;

  tmp = GetUser();
  if (bsExport.test(CItemData::USER) && !tmp.IsEmpty())
    oss << _T("\t\t<username><![CDATA[") << LPCTSTR(tmp)
        << _T("]]></username>") << endl;

  tmp = GetPassword();
  // Password mandatory (see pwsafe.xsd)
  oss << _T("\t\t<password><![CDATA[") << LPCTSTR(tmp)
      << _T("]]></password>") << endl;

  tmp = GetURL();
  if (bsExport.test(CItemData::URL) && !tmp.IsEmpty())
    oss << _T("\t\t<url><![CDATA[") << LPCTSTR(tmp)
        << _T("]]></url>") << endl;

  tmp = GetAutoType();
  if (bsExport.test(CItemData::AUTOTYPE) && !tmp.IsEmpty())
    oss << _T("\t\t<autotype><![CDATA[") << LPCTSTR(tmp)
        << _T("]]></autotype>") << endl;

  tmp = GetNotes();
  if (bsExport.test(CItemData::NOTES) && !tmp.IsEmpty())
    oss << _T("\t\t<notes><![CDATA[") << LPCTSTR(tmp)
        << _T("]]></notes>") << endl;

  uuid_array_t uuid_array;
  GetUUID(uuid_array);
  TCHAR uuid_buffer[37];
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
  oss << _T("\t\t<uuid><![CDATA[") << uuid_buffer << _T("]]></uuid>") << endl;

  time_t t;
  GetCTime(t);
  if (bsExport.test(CItemData::CTIME) && (long)t != 0)
    oss << GetXMLTime(2, _T("ctime"), t);

  GetATime(t);
  if (bsExport.test(CItemData::ATIME) && (long)t != 0)
    oss << GetXMLTime(2, _T("atime"), t);

  GetLTime(t);
  if (bsExport.test(CItemData::LTIME) && (long)t != 0)
    oss << GetXMLTime(2, _T("ltime"), t);

  GetPMTime(t);
  if (bsExport.test(CItemData::PMTIME) && (long)t != 0)
    oss << GetXMLTime(2, _T("pmtime"), t);

  GetRMTime(t);
  if (bsExport.test(CItemData::RMTIME) && (long)t != 0)
    oss << GetXMLTime(2, _T("rmtime"), t);

  if (bsExport.test(CItemData::PWHIST)) {
    BOOL pwh_status;
    size_t pwh_max, pwh_num;
    PWHistList PWHistList;
    CreatePWHistoryList(pwh_status, pwh_max, pwh_num,
                        &PWHistList, TMC_XML);
    if (pwh_status == TRUE || pwh_max > 0 || pwh_num > 0) {
      TCHAR buffer[8];
      oss << _T("\t\t<pwhistory>") << endl;
#if _MSC_VER >= 1400
      _stprintf_s(buffer, 3, _T("%1d"), pwh_status);
      oss << _T("\t\t\t<status>") << buffer << _T("</status>") << endl;

      _stprintf_s(buffer, 3, _T("%2d"), pwh_max);
      oss << _T("\t\t\t<max>") << buffer << _T("</max>") << endl;

      _stprintf_s(buffer, 3, _T("%2d"), pwh_num);
      oss << _T("\t\t\t<num>") << buffer << _T("</num>") << endl;
#else
      _stprintf(buffer, _T("%1d"), pwh_status);
      oss << _T("\t\t\t<status>") << buffer << _T("</status>") << endl;

      _stprintf(buffer, _T("%2d"), pwh_max);
      oss << _T("\t\t\t<max>") << buffer << _T("</max>") << endl;

      _stprintf(buffer, _T("%2d"), pwh_num);
      oss << _T("\t\t\t<num>") << buffer << _T("</num>") << endl;
#endif
      if (!PWHistList.empty()) {
        oss << _T("\t\t\t<history_entries>") << endl;
        int num = 1;
        PWHistList::iterator hiter;
        for (hiter = PWHistList.begin(); hiter != PWHistList.end();
             hiter++) {
          oss << _T("\t\t\t\t<history_entry num=\"") << num << _T("\">") << endl;
          const PWHistEntry pwshe = *hiter;
          oss << _T("\t\t\t\t\t<changed>") << endl;
          oss << _T("\t\t\t\t\t\t<date>")
              << LPCTSTR(pwshe.changedate.Left(10))
              << _T("</date>") << endl;
          oss << _T("\t\t\t\t\t\t<time>")
              << LPCTSTR(pwshe.changedate.Right(8))
              << _T("</time>") << endl;
          oss << _T("\t\t\t\t\t</changed>") << endl;
          oss << _T("\t\t\t\t\t<oldpassword><![CDATA[")
              << LPCTSTR(pwshe.password)
              << _T("]]></oldpassword>") << endl;
          oss << _T("\t\t\t\t</history_entry>") << endl;

          num++;
        } // for
        oss << _T("\t\t\t</history_entries>") << endl;
      } // if !empty
      oss << _T("\t\t</pwhistory>") << endl;
    }
  }

  if (NumberUnknownFields() > 0) {
    oss << _T("\t\t<unknownrecordfields>") << endl;
    for (unsigned int i = 0; i != NumberUnknownFields(); i++) {
      unsigned int length = 0;
      unsigned char type;
      unsigned char *pdata(NULL);
      GetUnknownField(type, length, pdata, i);
      if (length == 0)
        continue;
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
      oss << _T("\t\t\t<field ftype=\"") << int(type) << _T("\">") <<  LPCTSTR(tmp) << _T("</field>") << endl;
      trashMemory(pdata, length);
      delete[] pdata;
    } // iteration over unknown fields
    oss << _T("\t\t</unknownrecordfields>") << endl;  
  } // if there are unknown fields

  oss << _T("\t</entry>") << endl << endl;
  CMyString retval(oss.str().c_str());
  return retval;
}

void CItemData::SplitName(const CMyString &name,
                            CMyString &title, CMyString &username)
{
    int pos = name.FindByte(SPLTCHR);
    if (pos==-1) {//Not a split name
      int pos2 = name.FindByte(DEFUSERCHR);
      if (pos2 == -1)  {//Make certain that you remove the DEFUSERCHR
        title = name;
      } else {
        title = CMyString(name.Left(pos2));
      }
    } else {
      /*
       * There should never ever be both a SPLITCHR and a DEFUSERCHR in
       * the same string
       */
      CMyString temp;
      temp = CMyString(name.Left(pos));
      temp.TrimRight();
      title = temp;
      temp = CMyString(name.Right(name.GetLength() - (pos+1))); // Zero-index string
      temp.TrimLeft();
      username = temp;
    }
}

//-----------------------------------------------------------------------------
// Setters

void CItemData::SetField(CItemField &field, const CMyString &value)
{
    BlowFish *bf = MakeBlowFish();
    field.Set(value, bf);
    delete bf;
}

void CItemData::SetField(CItemField &field, const unsigned char *value, unsigned int length)
{
    BlowFish *bf = MakeBlowFish();
    field.Set(value, length, bf);
    delete bf;
}

void CItemData::CreateUUID()
{
    CUUIDGen uuid;
    uuid_array_t uuid_array;
    uuid.GetUUID(uuid_array);
    SetUUID(uuid_array);
}

void
CItemData::SetName(const CMyString &name, const CMyString &defaultUsername)
{
    // the m_name is from pre-2.0 versions, and may contain the title and user
    // separated by SPLTCHR. Also, DEFUSERCHR signified that the default username is to be used.
    // Here we fill the title and user fields so that
    // the application can ignore this difference after an ItemData record
    // has been created
    CMyString title, user;
    int pos = name.FindByte(DEFUSERCHR);
    if (pos != -1) {
      title = CMyString(name.Left(pos));
      user = defaultUsername;
    } else
      SplitName(name, title, user);
    // In order to avoid unecessary BlowFish construction/deletion,
    // we forego SetField here...
    BlowFish *bf = MakeBlowFish();
    m_Name.Set(name, bf);
    m_Title.Set(title, bf);
    m_User.Set(user, bf);
    delete bf;
}

void
CItemData::SetTitle(const CMyString &title, TCHAR delimiter)
{
	if (delimiter == 0)
      SetField(m_Title, title);
	else {
      CMyString new_title(_T(""));
      CMyString newCString, tmpCString;
      int pos = 0;

      newCString = title;
      do {
        pos = newCString.Find(delimiter);
        if ( pos != -1 ) {
          new_title += CMyString(newCString.Left(pos)) + _T(".");

          tmpCString = CMyString(newCString.Mid(pos + 1));
          newCString = tmpCString;
        }
      } while ( pos != -1 );

      if (!newCString.IsEmpty())
        new_title += newCString;

      SetField(m_Title, new_title);
	}
}

void
CItemData::SetUser(const CMyString &user)
{
    SetField(m_User, user);
}

void
CItemData::SetPassword(const CMyString &password)
{
    SetField(m_Password, password);
}

void
CItemData::SetNotes(const CMyString &notes, TCHAR delimiter)
{
    if (delimiter == 0)
      SetField(m_Notes, notes);
    else {
      const CMyString CRLF = _T("\r\n");
      CMyString multiline_notes(_T(""));

      CMyString newCString;
      CMyString tmpCString;

      int pos = 0;

      newCString = notes;
      do {
        pos = newCString.Find(delimiter);
        if ( pos != -1 ) {
          multiline_notes += CMyString(newCString.Left(pos)) + CRLF;

          tmpCString = CMyString(newCString.Mid(pos + 1));
          newCString = tmpCString;
        }
      } while ( pos != -1 );

      if (!newCString.IsEmpty())
        multiline_notes += newCString;

      SetField(m_Notes, multiline_notes);
    }
}

void
CItemData::SetGroup(const CMyString &title)
{
    SetField(m_Group, title);
}

void
CItemData::SetUUID(const uuid_array_t &UUID)
{
    SetField(m_UUID, (const unsigned char *)UUID, sizeof(UUID));
}

void
CItemData::SetURL(const CMyString &URL)
{
    SetField(m_URL, URL);
}

void
CItemData::SetAutoType(const CMyString &autotype)
{
    SetField(m_AutoType, autotype);
}

void
CItemData::SetTime(int whichtime)
{
    time_t t;
    time(&t);
    SetTime(whichtime, t);
}

void
CItemData::SetTime(int whichtime, time_t t)
{
    switch (whichtime) {
    case ATIME:
      SetField(m_tttATime, (const unsigned char *)&t, sizeof(t));
      break;
    case CTIME:
      SetField(m_tttCTime, (const unsigned char *)&t, sizeof(t));
      break;
    case LTIME:
      SetField(m_tttLTime, (const unsigned char *)&t, sizeof(t));
      break;
    case PMTIME:
      SetField(m_tttPMTime, (const unsigned char *)&t, sizeof(t));
      break;
    case RMTIME:
      SetField(m_tttRMTime, (const unsigned char *)&t, sizeof(t));
      break;
    default:
      ASSERT(0);
    }
}

void
CItemData::SetTime(int whichtime, const CString &time_str)
{
    if (time_str.GetLength() == 0) {
      SetTime(whichtime, (time_t)0);
      return;
    }
  
    time_t t;

    if (!PWSUtil::VerifyImportDateTimeString(time_str, t))
      if (!PWSUtil::VerifyXMLDateTimeString(time_str, t))
        if (!PWSUtil::VerifyASCDateTimeString(time_str, t))
      return;

    if (t == (time_t)-1)	// error despite all our verification!
      return;

    SetTime(whichtime, t);
}

void
CItemData::SetPWHistory(const CMyString &PWHistory)
{
	CMyString pwh = PWHistory;
	if (pwh == _T("0") || pwh == _T("00000"))
		pwh = _T("");
	SetField(m_PWHistory, pwh);
}

int
CItemData::CreatePWHistoryList(BOOL &status,
                               size_t &pwh_max, size_t &pwh_num,
                               PWHistList* pPWHistList,
							   const int time_format) const
{
  PWHistEntry pwh_ent;
  CMyString tmp, pwh;
  int ipwlen, m, n, i_error;
  BOOL s;
  long t;

  status = FALSE;
  pwh_max = 0;
  pwh_num = 0;
  i_error = 0;

  pwh = this->GetPWHistory();
  int len = pwh.GetLength();

  if (len < 5)
  	return (len != 0 ? 1 : 0);

  TCHAR *lpszPWHistory = pwh.GetBuffer(len + sizeof(TCHAR));

#if _MSC_VER >= 1400
  int iread = _stscanf_s(lpszPWHistory, _T("%01d%02x%02x"), &s, &m, &n);
#else
  int iread = _stscanf(lpszPWHistory, _T("%01d%02x%02x"), &s, &m, &n);
#endif
  if (iread != 3)
    return 1;

  lpszPWHistory += 5;
  for (int i = 0; i < n; i++) {
#if _MSC_VER >= 1400
    iread = _stscanf_s(lpszPWHistory, _T("%8x"), &t);
#else
    iread = _stscanf(lpszPWHistory, _T("%8x"), &t);
#endif
    if (iread != 1) {
    	i_error = 1;
    	break;
    }
    pwh_ent.changetttdate = (time_t) t;
    pwh_ent.changedate =
      PWSUtil::ConvertToDateTimeString((time_t) t, time_format);
    if (pwh_ent.changedate.IsEmpty()) {
		  //                       1234567890123456789
      pwh_ent.changedate = _T("1970-01-01 00:00:00");
    }
    lpszPWHistory += 8;
#if _MSC_VER >= 1400
    iread = _stscanf_s(lpszPWHistory, _T("%4x"), &ipwlen);
#else
    iread = _stscanf(lpszPWHistory, _T("%4x"), &ipwlen);
#endif
    if (iread != 1) {
    	i_error = 1;
    	break;
    }
    lpszPWHistory += 4;
    pwh_ent.password = CMyString(lpszPWHistory, ipwlen);
    lpszPWHistory += ipwlen;
    pPWHistList->push_back(pwh_ent);
  }

  status = s;
  pwh_max = m;
  pwh_num = n;
  pwh.ReleaseBuffer();
  return i_error;
}

BlowFish *
CItemData::MakeBlowFish() const
{
  ASSERT(IsSessionKeySet);
  return BlowFish::MakeBlowFish(SessionKey, sizeof(SessionKey),
                                m_salt, SaltLength);
}

CItemData&
CItemData::operator=(const CItemData &that)
{
  //Check for self-assignment
  if (this != &that) {
    m_UUID = that.m_UUID;
    m_Name = that.m_Name;
    m_Title = that.m_Title;
    m_User = that.m_User;
    m_Password = that.m_Password;
    m_Notes = that.m_Notes;
    m_Group = that.m_Group;
    m_URL = that.m_URL;
    m_AutoType = that.m_AutoType;
    m_tttCTime = that.m_tttCTime;
    m_tttPMTime = that.m_tttPMTime;
    m_tttATime = that.m_tttATime;
    m_tttLTime = that.m_tttLTime;
    m_tttRMTime = that.m_tttRMTime;
    m_PWHistory = that.m_PWHistory;
    m_display_info = that.m_display_info;
    if (!that.m_URFL.empty())
      m_URFL = that.m_URFL;
    else
      m_URFL.clear();

    memcpy((char*)m_salt, (char*)that.m_salt, SaltLength);
  }
  
  return *this;
}

void
CItemData::Clear()
{
  m_Title.Empty();
  m_User.Empty();
  m_Password.Empty();
  m_Notes.Empty();
  m_Group.Empty();
  m_URL.Empty();
  m_AutoType.Empty();
  m_tttCTime.Empty();
  m_tttPMTime.Empty();
  m_tttATime.Empty();
  m_tttLTime.Empty();
  m_tttRMTime.Empty();
  m_PWHistory.Empty();
  m_URFL.clear();
}

int
CItemData::ValidateUUID(const unsigned short &nMajor, const unsigned short &nMinor,
						uuid_array_t &uuid_array)
{
  // currently only ensure that item has a uuid, creating one if missing.

  /* NOTE the unusual position of the default statement.

     This is by design as it assumes that if we don't know the version, the
     database probably has all the problems and should be fixed.

     To date, we know that databases of format 0x0200 and 0x0300 have a UUID
     problem if records were duplicated.  Databases of format 0x0100 did not
     have the duplicate function and it has been fixed in databases in format
     0x0301.

     As more problems are detected and need to be fixed, this code will expand
     and may have to change.
  */
  int iResult(0);
  switch ((nMajor << 8) + nMinor) {
  	default:
  	case 0x0200:  // V2 format
  	case 0x0300:  // V3 format prior to PWS V3.03
  		CreateUUID();
  		GetUUID(uuid_array);
  		iResult = 1;
  	case 0x0100:  // V1 format
  	case 0x0301:  // V3 format PWS V3.03 and later
  		break;
  }
  return iResult;
}

int
CItemData::ValidatePWHistory()
{
  if (m_PWHistory.IsEmpty())
    return 0;

  CMyString pwh = GetPWHistory();
  if (pwh.GetLength() < 5) {
    SetPWHistory(_T(""));
    return 1;
  }

  BOOL pwh_status;
  size_t pwh_max, pwh_num;
  PWHistList PWHistList;
  int iResult = CreatePWHistoryList(pwh_status, pwh_max,
                                    pwh_num, &PWHistList, TMC_EXPORT_IMPORT);
  if (iResult == 0) {
    return 0;
  }

  size_t listnum = PWHistList.size();
  if (listnum > pwh_num)
    pwh_num = listnum;

  if (pwh_max == 0 && pwh_num == 0) {
    SetPWHistory(_T(""));
    return 1;
  }

  if (listnum > pwh_max)
    pwh_max = listnum;

  pwh_num = listnum;

  if (pwh_max < pwh_num)
  	pwh_max = pwh_num;

  // Rebuild PWHistory from the data we have
  CMyString history;
  CString cs_buffer(_T(""));
  TCHAR buffer[8];
#if _MSC_VER >= 1400
  _stprintf_s(buffer, 8, _T("%1x%02x%02x"), pwh_status, pwh_max, pwh_num);
#else
  _stprintf(buffer, _T("%1x%02x%02x"), pwh_status, pwh_max, pwh_num);
#endif
  history = CMyString(buffer);

  PWHistList::iterator iter;
  for (iter = PWHistList.begin(); iter != PWHistList.end(); iter++) {
      PWHistEntry pwh_ent = *iter;
      cs_buffer.Format(_T("%08x%04x%s"), pwh_ent.changetttdate, 
                       pwh_ent.password.GetLength(), pwh_ent.password);
      history += (LPCTSTR)cs_buffer;
      cs_buffer.Empty();
  }
  SetPWHistory(history);

  return 1;
}

bool
CItemData::Matches(const CString &subgroup_name, int iObject,
                   int iFunction) const
{
  ASSERT(iFunction != 0); // must be positive or negative!

  CMyString csObject;
  switch(iObject) {
  case SGO_GROUP:
    csObject = GetGroup();
    break;
  case SGO_TITLE:
    csObject = GetTitle();
    break;
  case SGO_USER:
    csObject = GetUser();
    break;
  case SGO_GROUPTITLE:
    csObject = GetGroup() + TCHAR('.') + GetTitle();
    break;
  case SGO_URL:
    csObject = GetURL();
    break;
  case SGO_NOTES:
    csObject = GetNotes();
    break;
  default:
    ASSERT(0);
  }

  const int sb_len = subgroup_name.GetLength();
  const int ob_len = csObject.GetLength();

  // Negative = Case   Sensitive
  // Positive = Case INsensitive
  switch (iFunction) {
  case -SGF_EQUALS:
  case SGF_EQUALS:
    return ((ob_len == sb_len) &&
            (((iFunction < 0) &&
              csObject.Compare((LPCTSTR)subgroup_name) == 0) ||
             ((iFunction > 0) &&
              (csObject.CompareNoCase((LPCTSTR)subgroup_name) != 0))));
  case -SGF_NOTEQUAL:
  case SGF_NOTEQUAL:
    return (((iFunction < 0) &&
             csObject.Compare((LPCTSTR)subgroup_name) != 0) ||
            ((iFunction > 0) &&
             (csObject.CompareNoCase((LPCTSTR)subgroup_name) != 0)));
  case -SGF_BEGINS:
  case SGF_BEGINS:
    if (ob_len >= sb_len) {
      csObject = csObject.Left(sb_len);
      return (((iFunction < 0) &&
               subgroup_name.Compare((LPCTSTR)csObject) == 0) ||
              ((iFunction > 0) &&
               (subgroup_name.CompareNoCase((LPCTSTR)csObject) == 0)));
    } else {
      return false;
    }
  case -SGF_NOTBEGIN:
  case SGF_NOTBEGIN:
    if (ob_len >= sb_len) {
      csObject = csObject.Left(sb_len);
      return (((iFunction < 0) &&
               subgroup_name.Compare((LPCTSTR)csObject) != 0) ||
              ((iFunction > 0) &&
               (subgroup_name.CompareNoCase((LPCTSTR)csObject) != 0)));
    } else {
      return false;
    }
  case -SGF_ENDS:
  case SGF_ENDS:
    if (ob_len > sb_len) {
      csObject = csObject.Right(sb_len);
      return (((iFunction < 0) &&
               subgroup_name.Compare((LPCTSTR)csObject) == 0) ||
              ((iFunction > 0) &&
               (subgroup_name.CompareNoCase((LPCTSTR)csObject) == 0)));
    } else {
      return false;
    }
  case -SGF_NOTEND:
  case SGF_NOTEND:
    if (ob_len > sb_len) {
      csObject = csObject.Right(sb_len);
      return (((iFunction < 0) &&
               subgroup_name.Compare((LPCTSTR)csObject) != 0) ||
              ((iFunction > 0) &&
               (subgroup_name.CompareNoCase((LPCTSTR)csObject) != 0)));
    } else
      return true;
  case -SGF_CONTAINS:
    return (csObject.Find((LPCTSTR)subgroup_name) != -1);
  case SGF_CONTAINS:
    {
      csObject.MakeLower();
      CString subgroupLC(subgroup_name);
      subgroupLC.MakeLower();
      return (csObject.Find((LPCTSTR)subgroupLC) != -1);
    }
  case -SGF_NOTCONTAIN:
    return (csObject.Find((LPCTSTR)subgroup_name)== -1);
  case SGF_NOTCONTAIN:
    {
      csObject.MakeLower();
      CString subgroupLC(subgroup_name);
      subgroupLC.MakeLower();
      return (csObject.Find((LPCTSTR)subgroupLC) == -1);
    }
  default:
    ASSERT(0);
  }

  return true; // should never get here!
}

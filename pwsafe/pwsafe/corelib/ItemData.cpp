/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file ItemData.cpp
//-----------------------------------------------------------------------------

#include "ItemData.h"
#include "os/typedefs.h"
#include "os/pws_tchar.h"
#include "os/mem.h"
#include "BlowFish.h"
#include "TwoFish.h"
#include "PWSrand.h"
#include "UTF8Conv.h"
#include "PWSprefs.h"
#include "VerifyFormat.h"
#include "PWHistory.h"
#include "Util.h"
#include "StringXStream.h"

#include <time.h>
#include <sstream>
#include <iomanip>

using namespace std;

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
  pws_os::mlock(SessionKey, sizeof(SessionKey));
  PWSrand::GetInstance()->GetRandomData( SessionKey, sizeof( SessionKey ) );
  IsSessionKeySet = true;
}

//-----------------------------------------------------------------------------
// Constructors

CItemData::CItemData()
  : m_Name(NAME), m_Title(TITLE), m_User(USER), m_Password(PASSWORD),
  m_Notes(NOTES), m_UUID(UUID), m_Group(GROUP),
  m_URL(URL), m_AutoType(AUTOTYPE),
  m_tttATime(ATIME), m_tttCTime(CTIME), m_tttXTime(XTIME),
  m_tttPMTime(PMTIME), m_tttRMTime(RMTIME), m_PWHistory(PWHIST),
  m_PWPolicy(POLICY), m_XTimeInterval(XTIME_INT),
  m_entrytype(ET_NORMAL), m_display_info(NULL)
{
  PWSrand::GetInstance()->GetRandomData( m_salt, SaltLength );
}

CItemData::CItemData(const CItemData &that) :
  m_Name(that.m_Name), m_Title(that.m_Title), m_User(that.m_User),
  m_Password(that.m_Password), m_Notes(that.m_Notes), m_UUID(that.m_UUID),
  m_Group(that.m_Group), m_URL(that.m_URL), m_AutoType(that.m_AutoType),
  m_tttATime(that.m_tttATime), m_tttCTime(that.m_tttCTime),
  m_tttXTime(that.m_tttXTime), m_tttPMTime(that.m_tttPMTime),
  m_tttRMTime(that.m_tttRMTime), m_PWHistory(that.m_PWHistory),
  m_PWPolicy(that.m_PWPolicy), m_XTimeInterval(that.m_XTimeInterval),
  m_entrytype(that.m_entrytype), m_display_info(that.m_display_info)
{
  memcpy((char*)m_salt, (char*)that.m_salt, SaltLength);
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

StringX CItemData::GetField(const CItemField &field) const
{
  StringX retval;
  BlowFish *bf = MakeBlowFish();
  field.Get(retval, bf);
  delete bf;
  return retval;
}

void CItemData::GetField(const CItemField &field, unsigned char *value, unsigned int &length) const
{
  BlowFish *bf = MakeBlowFish();
  field.Get(value, length, bf);
  delete bf;
}

StringX CItemData::GetName() const
{
  return GetField(m_Name);
}

StringX CItemData::GetTitle() const
{
  return GetField(m_Title);
}

StringX CItemData::GetUser() const
{
  return GetField(m_User);
}

StringX CItemData::GetPassword() const
{
  return GetField(m_Password);
}

static void CleanNotes(StringX &s, TCHAR delimiter)
{
  if (delimiter != 0) {
    StringX r2;
    for (StringX::iterator iter = s.begin(); iter != s.end(); iter++)
      switch (*iter) {
      case TCHAR('\r'): continue;
      case TCHAR('\n'): r2 += delimiter; continue;
      default: r2 += *iter;
      }
    s = r2;
  }
}

StringX CItemData::GetNotes(TCHAR delimiter) const
{
  StringX ret = GetField(m_Notes);
  CleanNotes(ret, delimiter);
  return ret;
}

StringX CItemData::GetGroup() const
{
  return GetField(m_Group);
}

StringX CItemData::GetURL() const
{
  return GetField(m_URL);
}

StringX CItemData::GetAutoType() const
{
  return GetField(m_AutoType);
}

StringX CItemData::GetTime(int whichtime, int result_format) const
{
  time_t t;

  GetTime(whichtime, t);
  return PWSUtil::ConvertToDateTimeString(t, result_format);
}

void CItemData::GetTime(int whichtime, time_t &t) const
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
    case XTIME:
      GetField(m_tttXTime, (unsigned char *)in, tlen);
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
    int t32;
    ASSERT(tlen == sizeof(t32));
    memcpy(&t32, in, sizeof(t32));
    t = t32;
  } else {
    t = 0;
  }
}

void CItemData::GetUUID(uuid_array_t &uuid_array) const
{
  unsigned int length = sizeof(uuid_array);
  GetField(m_UUID, (unsigned char *)uuid_array, length);
}

static void String2PWPolicy(const stringT &cs_pwp, PWPolicy &pwp)
{
  // should really be a c'tor of PWPolicy - later...

  // We need flags(4), length(3), lower_len(3), upper_len(3)
  //   digit_len(3), symbol_len(3) = 4 + 5 * 3 = 19 
  // Note: order of fields set by PWSprefs enum that can have minimum lengths.
  // Later releases must support these as a minimum.  Any fields added
  // by these releases will be lost if the user changes these field.
  ASSERT(cs_pwp.length() == 19);
  istringstreamT is_flags(stringT(cs_pwp, 0, 4));
  istringstreamT is_length(stringT(cs_pwp, 4, 3));
  istringstreamT is_digitminlength(stringT(cs_pwp, 7, 3));
  istringstreamT is_lowreminlength(stringT(cs_pwp, 10, 3));
  istringstreamT is_symbolminlength(stringT(cs_pwp, 13, 3));
  istringstreamT is_upperminlength(stringT(cs_pwp, 16, 3));
  unsigned int f; // dain bramaged istringstream requires this runaround
  is_flags >> hex >> f;
  pwp.flags = static_cast<WORD>(f);
  is_length >> hex >> pwp.length;
  is_digitminlength >> hex >> pwp.digitminlength;
  is_lowreminlength >> hex >> pwp.lowerminlength;
  is_symbolminlength >> hex >> pwp.symbolminlength;
  is_upperminlength >> hex >> pwp.upperminlength;
}

void CItemData::GetPWPolicy(PWPolicy &pwp) const
{
  StringX cs_pwp(GetField(m_PWPolicy));

  int len = cs_pwp.length();
  pwp.flags = 0;
  if (len == 19)
    String2PWPolicy(cs_pwp.c_str(), pwp);
}

StringX CItemData::GetPWPolicy() const
{
  return GetField(m_PWPolicy);
}

void CItemData::GetXTimeInt(int &xint) const
{
  unsigned char in[TwoFish::BLOCKSIZE]; // required by GetField
  unsigned int tlen = sizeof(in); // ditto

  GetField(m_XTimeInterval, (unsigned char *)in, tlen);

  if (tlen != 0) {
    ASSERT(tlen == sizeof(int));
    memcpy(&xint, in, sizeof(int));
  } else {
    xint = 0;
  }
}

StringX CItemData::GetXTimeInt() const
{
  int xint;
  GetXTimeInt(xint);
  if (xint == 0)
    return _T("");

  oStringXStream os;
  os << xint << ends;
  return os.str();
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

void CItemData::SetUnknownField(const unsigned char type,
                                const unsigned int length,
                                const unsigned char * ufield)
{
  CItemField unkrfe(type);
  SetField(unkrfe, ufield, length);
  m_URFL.push_back(unkrfe);
}

StringX CItemData::GetPWHistory() const
{
  StringX ret = GetField(m_PWHistory);
  if (ret == _T("0") || ret == _T("00000"))
    ret = _T("");
  return ret;
}

StringX CItemData::GetPlaintext(const TCHAR &separator,
                                  const FieldBits &bsFields,
                                  const TCHAR &delimiter,
                                  const CItemData *cibase) const
{
  StringX ret(_T(""));

  StringX grouptitle;
  const StringX title(GetTitle());
  const StringX group(GetGroup());
  const StringX user(GetUser());
  const StringX url(GetURL());
  const StringX notes(GetNotes(delimiter));

  // a '.' in title gets Import confused re: Groups
  grouptitle = title;
  if (grouptitle.find(TCHAR('.')) != StringX::npos) {
    if (delimiter != 0) {
      StringX s;
      for (StringX::iterator iter = grouptitle.begin();
           iter != grouptitle.end(); iter++)
        s += (*iter == TCHAR('.')) ? delimiter : *iter;
      grouptitle = s;
    } else {
      grouptitle = TCHAR('\"') + title + TCHAR('\"');
    }
  }

  if (!group.empty())
    grouptitle = group + TCHAR('.') + grouptitle;

  StringX history(_T(""));
  if (bsFields.test(CItemData::PWHIST)) {
    // History exported as "00000" if empty, to make parsing easier
    BOOL pwh_status;
    size_t pwh_max, num_err;
    PWHistList PWHistList;

    pwh_status = CreatePWHistoryList(GetPWHistory(), pwh_max, num_err,
                                     PWHistList, TMC_EXPORT_IMPORT);

    //  Build export string
    history = MakePWHistoryHeader(pwh_status, pwh_max, PWHistList.size());
    PWHistList::iterator iter;
    for (iter = PWHistList.begin(); iter != PWHistList.end(); iter++) {
      const PWHistEntry &pwshe = *iter;
      history += _T(' ');
      history += pwshe.changedate;
      ostringstreamT os1;
      os1 << hex << charT(' ') << setfill(charT('0')) << setw(4)
          << pwshe.password.length() << charT(' ') << ends;
      history += os1.str().c_str();
      history += pwshe.password;
    }
  }

  StringX csPassword;
  if (m_entrytype == ET_ALIAS) {
    ASSERT(cibase != NULL);
    csPassword = _T("[[") + 
                 cibase->GetGroup() + _T(":") + 
                 cibase->GetTitle() + _T(":") + 
                 cibase->GetUser() + _T("]]") ;
  } else if (m_entrytype == ET_SHORTCUT) {
    ASSERT(cibase != NULL);
    csPassword = _T("[~") + 
                 cibase->GetGroup() + _T(":") + 
                 cibase->GetTitle() + _T(":") + 
                 cibase->GetUser() + _T("~]") ;
  } else
    csPassword = GetPassword();

  // Notes field must be last, for ease of parsing import
  if (bsFields.count() == bsFields.size()) {
    // Everything - note can't actually set all bits via dialog!
    ret = grouptitle + separator + 
          user + separator +
          csPassword + separator + 
          url + separator + 
          GetAutoType() + separator +
          GetCTimeExp() + separator +
          GetPMTimeExp() + separator +
          GetATimeExp() + separator +
          GetXTimeExp() + separator +
          GetXTimeInt() + separator +
          GetRMTimeExp() + separator +
          GetPWPolicy() + separator +
          history + separator +
          _T("\"") + notes + _T("\"");
  } else {
    // Not everything
    if (bsFields.test(CItemData::GROUP) && bsFields.test(CItemData::TITLE))
      ret += grouptitle + separator;
    else if (bsFields.test(CItemData::GROUP))
      ret += group + separator;
    else if (bsFields.test(CItemData::TITLE))
      ret += title + separator;
    if (bsFields.test(CItemData::USER))
      ret += user + separator;
    if (bsFields.test(CItemData::PASSWORD))
      ret += csPassword + separator;
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
    if (bsFields.test(CItemData::XTIME))
      ret += GetXTimeExp() + separator;
    if (bsFields.test(CItemData::XTIME_INT))
      ret += GetXTimeInt() + separator;
    if (bsFields.test(CItemData::RMTIME))
      ret += GetRMTimeExp() + separator;
    if (bsFields.test(CItemData::POLICY))
      ret += GetPWPolicy() + separator;
    if (bsFields.test(CItemData::PWHIST))
      ret += history + separator;
    if (bsFields.test(CItemData::NOTES))
      ret += _T("\"") + notes + _T("\"");
    // remove trailing separator
    if (ret[ret.length()-1] == separator) {
      int rl = ret.length();
      ret = ret.substr(0, rl - 1);
    }
  }

  return ret;
}

string CItemData::GetXML(unsigned id, const FieldBits &bsExport,
                         TCHAR delimiter, const CItemData *cibase,
                         bool bforce_normal_entry) const
{
  ostringstream oss; // ALWAYS a string of chars, never wchar_t!
  oss << "\t<entry id=\"" << id << "\"";
  if (bforce_normal_entry)
    oss << " normal=\"" << "true" << "\"";

  oss << ">" << endl;

  StringX tmp;
  CUTF8Conv utf8conv;

  tmp = GetGroup();
  if (bsExport.test(CItemData::GROUP) && !tmp.empty())
    PWSUtil::WriteXMLField(oss, "group", tmp, utf8conv);

  // Title mandatory (see pwsafe.xsd)
  PWSUtil::WriteXMLField(oss, "title", GetTitle(), utf8conv);

  tmp = GetUser();
  if (bsExport.test(CItemData::USER) && !tmp.empty())
    PWSUtil::WriteXMLField(oss, "username", tmp, utf8conv);

  // Password mandatory (see pwsafe.xsd)
  if (m_entrytype == ET_ALIAS) {
    ASSERT(cibase != NULL);
    tmp = _T("[[") + 
          cibase->GetGroup() + _T(":") + 
          cibase->GetTitle() + _T(":") + 
          cibase->GetUser() + _T("]]") ;
  } else
  if (m_entrytype == ET_SHORTCUT) {
    ASSERT(cibase != NULL);
    tmp = _T("[~") + 
          cibase->GetGroup() + _T(":") + 
          cibase->GetTitle() + _T(":") + 
          cibase->GetUser() + _T("~]") ;
  } else
    tmp = GetPassword();
  PWSUtil::WriteXMLField(oss, "password", tmp, utf8conv);

  tmp = GetURL();
  if (bsExport.test(CItemData::URL) && !tmp.empty())
    PWSUtil::WriteXMLField(oss, "url", tmp, utf8conv);

  tmp = GetAutoType();
  if (bsExport.test(CItemData::AUTOTYPE) && !tmp.empty())
    PWSUtil::WriteXMLField(oss, "autotype", tmp, utf8conv);

  tmp = GetNotes();
  if (bsExport.test(CItemData::NOTES) && !tmp.empty()) {
    CleanNotes(tmp, delimiter);
    PWSUtil::WriteXMLField(oss, "notes", tmp, utf8conv);
  }

  uuid_array_t uuid_array;
  GetUUID(uuid_array);
  const CUUIDGen uuid(uuid_array);
  oss << "\t\t<uuid><![CDATA[" << uuid << "]]></uuid>" << endl;

  time_t t;
  int xint;

  GetCTime(t);
  if (bsExport.test(CItemData::CTIME) && (long)t != 0L)
    oss << PWSUtil::GetXMLTime(2, "ctime", t, utf8conv);

  GetATime(t);
  if (bsExport.test(CItemData::ATIME) && (long)t != 0L)
    oss << PWSUtil::GetXMLTime(2, "atime", t, utf8conv);

  GetXTime(t);
  if (bsExport.test(CItemData::XTIME) && (long)t != 0L)
    oss << PWSUtil::GetXMLTime(2, "xtime", t, utf8conv);

  GetXTimeInt(xint);
  if (bsExport.test(CItemData::XTIME_INT) && xint > 0 && xint <= 3650)
    oss << "\t\t<xtime_interval>" << xint << "</xtime_interval>" << endl;

  GetPMTime(t);
  if (bsExport.test(CItemData::PMTIME) && (long)t != 0L)
    oss << PWSUtil::GetXMLTime(2, "pmtime", t, utf8conv);

  GetRMTime(t);
  if (bsExport.test(CItemData::RMTIME) && (long)t != 0L)
    oss << PWSUtil::GetXMLTime(2, "rmtime", t, utf8conv);

  PWPolicy pwp;
  GetPWPolicy(pwp);
  if (bsExport.test(CItemData::POLICY) && pwp.flags != 0) {
    oss << "\t\t<PasswordPolicy>" << endl;
    oss << dec;
    oss << "\t\t\t<PWLength>" << pwp.length << "</PWLength>" << endl;
    if (pwp.flags & PWSprefs::PWPolicyUseLowercase)
      oss << "\t\t\t<PWUseLowercase>1</PWUseLowercase>" << endl;
    if (pwp.flags & PWSprefs::PWPolicyUseUppercase)
      oss << "\t\t\t<PWUseUppercase>1</PWUseUppercase>" << endl;
    if (pwp.flags & PWSprefs::PWPolicyUseDigits)
      oss << "\t\t\t<PWUseDigits>1</PWUseDigits>" << endl;
    if (pwp.flags & PWSprefs::PWPolicyUseSymbols)
      oss << "\t\t\t<PWUseSymbols>1</PWUseSymbols>" << endl;
    if (pwp.flags & PWSprefs::PWPolicyUseHexDigits)
      oss << "\t\t\t<PWUseHexDigits>1</PWUseHexDigits>" << endl;
    if (pwp.flags & PWSprefs::PWPolicyUseEasyVision)
      oss << "\t\t\t<PWUseEasyVision>1</PWUseEasyVision>" << endl;
    if (pwp.flags & PWSprefs::PWPolicyMakePronounceable)
      oss << "\t\t\t<PWMakePronounceable>1</PWMakePronounceable>" << endl;

    if (pwp.lowerminlength > 0) {
      oss << "\t\t\t<PWLowercaseMinLength>" << pwp.lowerminlength << "</PWLowercaseMinLength>" << endl;
    }
    if (pwp.upperminlength > 0) {
      oss << "\t\t\t<PWUppercaseMinLength>" << pwp.upperminlength << "</PWUppercaseMinLength>" << endl;
    }
    if (pwp.digitminlength > 0) {
      oss << "\t\t\t<PWDigitMinLength>" << pwp.digitminlength << "</PWDigitMinLength>" << endl;
    }
    if (pwp.symbolminlength > 0) {
      oss << "\t\t\t<PWSymbolMinLength>" << pwp.symbolminlength << "</PWSymbolMinLength>" << endl;
    }
    oss << "\t\t</PasswordPolicy>" << endl;
  }

  if (bsExport.test(CItemData::PWHIST)) {
    size_t pwh_max, num_err;
    PWHistList PWHistList;
    bool pwh_status = CreatePWHistoryList(GetPWHistory(), pwh_max, num_err,
                                          PWHistList, TMC_XML);
    oss << dec;
    if (pwh_status || pwh_max > 0 || !PWHistList.empty()) {
      oss << "\t\t<pwhistory>" << endl;
      oss << "\t\t\t<status>" << pwh_status << "</status>" << endl;
      oss << "\t\t\t<max>" << pwh_max << "</max>" << endl;
      oss << "\t\t\t<num>" << PWHistList.size() << "</num>" << endl;
      if (!PWHistList.empty()) {
        oss << "\t\t\t<history_entries>" << endl;
        int num = 1;
        PWHistList::iterator hiter;
        for (hiter = PWHistList.begin(); hiter != PWHistList.end();
             hiter++) {
          const unsigned char * utf8 = NULL;
          int utf8Len = 0;

          oss << "\t\t\t\t<history_entry num=\"" << num << "\">" << endl;
          const PWHistEntry &pwshe = *hiter;
          oss << "\t\t\t\t\t<changed>" << endl;
          oss << "\t\t\t\t\t\t<date>";
          if (utf8conv.ToUTF8(pwshe.changedate.substr(0, 10), utf8, utf8Len))
            oss.write(reinterpret_cast<const char *>(utf8), utf8Len);
          oss << "</date>" << endl;
          oss << "\t\t\t\t\t\t<time>";
          if (utf8conv.ToUTF8(pwshe.changedate.substr(pwshe.changedate.length()-8),
                              utf8, utf8Len))
            oss.write(reinterpret_cast<const char *>(utf8), utf8Len);
          oss << "</time>" << endl;
          oss << "\t\t\t\t\t</changed>" << endl;
          PWSUtil::WriteXMLField(oss, "oldpassword", pwshe.password,
                        utf8conv, "\t\t\t\t\t");
          oss << "\t\t\t\t</history_entry>" << endl;

          num++;
        } // for
        oss << "\t\t\t</history_entries>" << endl;
      } // if !empty
      oss << "\t\t</pwhistory>" << endl;
    }
  }

  if (NumberUnknownFields() > 0) {
    oss << "\t\t<unknownrecordfields>" << endl;
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
      tmp = PWSUtil::Base64Encode(pdata, length).c_str();
#else
      tmp.clear();
      unsigned char * pdata2(pdata);
      unsigned char c;
      for (int j = 0; j < (int)length; j++) {
        c = *pdata2++;
        Format(cs_tmp, _T("%02x"), c);
        tmp += cs_tmp;
      }
#endif
      oss << "\t\t\t<field ftype=\"" << int(type) << "\">"
          << tmp.c_str() << "</field>" << endl;
      trashMemory(pdata, length);
      delete[] pdata;
    } // iteration over unknown fields
    oss << "\t\t</unknownrecordfields>" << endl;  
  } // if there are unknown fields

  oss << "\t</entry>" << endl << endl;
  return oss.str();
}

void CItemData::SplitName(const StringX &name,
                          StringX &title, StringX &username)
{
  StringX::size_type pos = name.find(SPLTCHR);
  if (pos == StringX::npos) {//Not a split name
    StringX::size_type pos2 = name.find(DEFUSERCHR);
    if (pos2 == StringX::npos)  {//Make certain that you remove the DEFUSERCHR
      title = name;
    } else {
      title = (name.substr(0, pos2));
    }
  } else {
    /*
    * There should never ever be both a SPLITCHR and a DEFUSERCHR in
    * the same string
    */
    StringX temp;
    temp = name.substr(0, pos);
    TrimRight(temp);
    title = temp;
    temp = name.substr(pos+1); // Zero-index string
    TrimLeft(temp);
    username = temp;
  }
}

//-----------------------------------------------------------------------------
// Setters

void CItemData::SetField(CItemField &field, const StringX &value)
{
  BlowFish *bf = MakeBlowFish();
  field.Set(value, bf);
  delete bf;
}

void CItemData::SetField(CItemField &field,
                         const unsigned char *value, unsigned int length)
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

void CItemData::SetName(const StringX &name, const StringX &defaultUsername)
{
  // the m_name is from pre-2.0 versions, and may contain the title and user
  // separated by SPLTCHR. Also, DEFUSERCHR signified that the default username is to be used.
  // Here we fill the title and user fields so that
  // the application can ignore this difference after an ItemData record
  // has been created
  StringX title, user;
  StringX::size_type pos = name.find(DEFUSERCHR);
  if (pos != StringX::npos) {
    title = name.substr(0, pos);
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

void CItemData::SetTitle(const StringX &title, TCHAR delimiter)
{
  if (delimiter == 0)
    SetField(m_Title, title);
  else {
    StringX new_title(_T(""));
    StringX newstringT, tmpstringT;
    StringX::size_type pos = 0;

    newstringT = title;
    do {
      pos = newstringT.find(delimiter);
      if ( pos != StringX::npos ) {
        new_title += newstringT.substr(0, pos) + _T(".");

        tmpstringT = newstringT.substr(pos + 1);
        newstringT = tmpstringT;
      }
    } while ( pos != StringX::npos );

    if (!newstringT.empty())
      new_title += newstringT;

    SetField(m_Title, new_title);
  }
}

void CItemData::SetUser(const StringX &user)
{
  SetField(m_User, user);
}

void CItemData::SetPassword(const StringX &password)
{
  SetField(m_Password, password);
}

void CItemData::SetNotes(const StringX &notes, TCHAR delimiter)
{
  if (delimiter == 0)
    SetField(m_Notes, notes);
  else {
    const StringX CRLF = _T("\r\n");
    StringX multiline_notes(_T(""));

    StringX newstringT;
    StringX tmpstringT;

    StringX::size_type pos = 0;

    newstringT = notes;
    do {
      pos = newstringT.find(delimiter);
      if ( pos != StringX::npos ) {
        multiline_notes += newstringT.substr(0, pos) + CRLF;

        tmpstringT = newstringT.substr(pos + 1);
        newstringT = tmpstringT;
      }
    } while ( pos != StringX::npos );

    if (!newstringT.empty())
      multiline_notes += newstringT;

    SetField(m_Notes, multiline_notes);
  }
}

void CItemData::SetGroup(const StringX &title)
{
  SetField(m_Group, title);
}

void CItemData::SetUUID(const uuid_array_t &UUID)
{
  SetField(m_UUID, (const unsigned char *)UUID, sizeof(UUID));
}

void CItemData::SetURL(const StringX &URL)
{
  SetField(m_URL, URL);
}

void CItemData::SetAutoType(const StringX &autotype)
{
  SetField(m_AutoType, autotype);
}

void CItemData::SetTime(int whichtime)
{
  time_t t;
  time(&t);
  SetTime(whichtime, t);
}

void CItemData::SetTime(int whichtime, time_t t)
{
  int t32 = (int)t;
  switch (whichtime) {
    case ATIME:
      SetField(m_tttATime, (const unsigned char *)&t32, sizeof(t32));
      break;
    case CTIME:
      SetField(m_tttCTime, (const unsigned char *)&t32, sizeof(t32));
      break;
    case XTIME:
      SetField(m_tttXTime, (const unsigned char *)&t32, sizeof(t32));
      break;
    case PMTIME:
      SetField(m_tttPMTime, (const unsigned char *)&t32, sizeof(t32));
      break;
    case RMTIME:
      SetField(m_tttRMTime, (const unsigned char *)&t32, sizeof(t32));
      break;
    default:
      ASSERT(0);
  }
}

bool CItemData::SetTime(int whichtime, const stringT &time_str)
{
  time_t t(0);

  if (time_str.empty()) {
    SetTime(whichtime, t);
    return true;
  } else
    if (time_str == _T("now")) {
      time(&t);
      SetTime(whichtime, t);
      return true;
    } else
      if ((VerifyImportDateTimeString(time_str, t) ||
           VerifyXMLDateTimeString(time_str, t) ||
           VerifyASCDateTimeString(time_str, t)) &&
          (t != (time_t)-1)  // checkerror despite all our verification!
          ) {
        SetTime(whichtime, t);
        return true;
      }
  return false;
}

void CItemData::SetXTimeInt(int &xint)
{
   SetField(m_XTimeInterval, (const unsigned char *)&xint, sizeof(int));
}

bool CItemData::SetXTimeInt(const stringT &xint_str)
{
  int xint(0);

  if (xint_str.empty()) {
    SetXTimeInt(xint);
    return true;
  }

  if (xint_str.find_first_not_of(_T("0123456789")) == stringT::npos) {
    istringstreamT is(xint_str);
    is >> xint;
    if (xint >= 0 && xint <= 3650) {
      SetXTimeInt(xint);
      return true;
    }
  }
  return false;
}

void CItemData::SetPWHistory(const StringX &PWHistory)
{
  StringX pwh = PWHistory;
  if (pwh == _T("0") || pwh == _T("00000"))
    pwh = _T("");
  SetField(m_PWHistory, pwh);
}

void CItemData::SetPWPolicy(const PWPolicy &pwp)
{
  // Must be some flags; however hex incompatible with other flags
  bool bhex_flag = (pwp.flags & PWSprefs::PWPolicyUseHexDigits) != 0;
  bool bother_flags = (pwp.flags & (~PWSprefs::PWPolicyUseHexDigits)) != 0;

  StringX cs_pwp;
  if (pwp.flags == 0 || (bhex_flag && bother_flags)) {
    cs_pwp = _T("");
  } else {
    ostringstreamT os;
    unsigned int f; // dain bramaged istringstream requires this runaround
    f = static_cast<unsigned int>(pwp.flags);
    os.fill(charT('0'));
    os << hex << setw(4) << f
       << setw(3) << pwp.length
       << setw(3) << pwp.digitminlength
       << setw(3) << pwp.lowerminlength
       << setw(3) << pwp.symbolminlength
       << setw(3) << pwp.upperminlength << ends;
    cs_pwp = os.str().c_str();
  }
  SetField(m_PWPolicy, cs_pwp);
}

bool CItemData::SetPWPolicy(const stringT &cs_pwp)
{
  // Basic sanity checks
  if (cs_pwp.empty()) {
    SetField(m_PWPolicy, cs_pwp.c_str());
    return true;
  }
  if (cs_pwp.length() < 19)
    return false;

  // Parse policy string, more sanity checks
  // See String2PWPolicy for valid format
  PWPolicy pwp;
  String2PWPolicy(stringT(cs_pwp), pwp);
  StringX cs_pwpolicy(cs_pwp.c_str());

  // Must be some flags; however hex incompatible with other flags
  bool bhex_flag = (pwp.flags & PWSprefs::PWPolicyUseHexDigits) != 0;
  bool bother_flags = (pwp.flags & (~PWSprefs::PWPolicyUseHexDigits)) != 0;
  const int total_sublength = pwp.digitminlength + pwp.lowerminlength +
    pwp.symbolminlength + pwp.upperminlength;

  if (pwp.flags == 0 || (bhex_flag && bother_flags) ||
      pwp.length > 1024 || total_sublength > pwp.length ||
      pwp.digitminlength > 1024 || pwp.lowerminlength > 1024 ||
      pwp.symbolminlength > 1024 || pwp.upperminlength > 1024) {
    cs_pwpolicy.clear();
  }

  SetField(m_PWPolicy, cs_pwpolicy);
  return true;
}

BlowFish *CItemData::MakeBlowFish() const
{
  ASSERT(IsSessionKeySet);
  return BlowFish::MakeBlowFish(SessionKey, sizeof(SessionKey),
    m_salt, SaltLength);
}

CItemData& CItemData::operator=(const CItemData &that)
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
    m_tttXTime = that.m_tttXTime;
    m_tttRMTime = that.m_tttRMTime;
    m_PWHistory = that.m_PWHistory;
    m_PWPolicy = that.m_PWPolicy;
    m_XTimeInterval = that.m_XTimeInterval;
    m_display_info = that.m_display_info;
    if (!that.m_URFL.empty())
      m_URFL = that.m_URFL;
    else
      m_URFL.clear();
    m_entrytype = that.m_entrytype;      
    memcpy((char*)m_salt, (char*)that.m_salt, SaltLength);
  }

  return *this;
}

void CItemData::Clear()
{
  m_Group.Empty();
  m_Title.Empty();
  m_User.Empty();
  m_Password.Empty();
  m_Notes.Empty();
  m_URL.Empty();
  m_AutoType.Empty();
  m_tttCTime.Empty();
  m_tttPMTime.Empty();
  m_tttATime.Empty();
  m_tttXTime.Empty();
  m_tttRMTime.Empty();
  m_PWHistory.Empty();
  m_PWPolicy.Empty();
  m_XTimeInterval.Empty();
  m_URFL.clear();
  m_entrytype = ET_NORMAL;
}

int CItemData::ValidateUUID(const unsigned short &nMajor, const unsigned short &nMinor,
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

bool CItemData::ValidatePWHistory()
{
  if (m_PWHistory.IsEmpty())
    return true;

  const StringX pwh = GetPWHistory();
  if (pwh.length() < 5) { // not empty, but too short.
    SetPWHistory(_T(""));
    return false;
  }

  size_t pwh_max, num_err;
  PWHistList PWHistList;
  bool pwh_status = CreatePWHistoryList(pwh,  pwh_max, num_err,
                                        PWHistList, TMC_EXPORT_IMPORT);
  if (num_err == 0)
    return true;

  size_t listnum = PWHistList.size();

  if (pwh_max == 0 && listnum == 0) {
    SetPWHistory(_T(""));
    return false;
  }

  if (listnum > pwh_max)
    pwh_max = listnum;

  if (pwh_max < listnum)
    pwh_max = listnum;

  // Rebuild PWHistory from the data we have
  StringX history = MakePWHistoryHeader(pwh_status, pwh_max, listnum);

  PWHistList::iterator iter;
  for (iter = PWHistList.begin(); iter != PWHistList.end(); iter++) {
    const PWHistEntry &pwshe = *iter;
    history += pwshe.changedate;
    oStringXStream os1;
    os1 << hex << setfill(charT('0')) << setw(4)
        << pwshe.password.length();
    history += os1.str();
    history += pwshe.password;
  }
  SetPWHistory(history);

  return false;
}

bool CItemData::Matches(const stringT &string1, int iObject,
                        int iFunction) const
{
  ASSERT(iFunction != 0); // must be positive or negative!

  StringX csObject;
  switch(iObject) {
    case GROUP:
      csObject = GetGroup();
      break;
    case TITLE:
      csObject = GetTitle();
      break;
    case USER:
      csObject = GetUser();
      break;
    case GROUPTITLE:
      csObject = GetGroup() + TCHAR('.') + GetTitle();
      break;
    case URL:
      csObject = GetURL();
      break;
    case NOTES:
      csObject = GetNotes();
      break;
    case PASSWORD:
      csObject = GetPassword();
      break;
    case AUTOTYPE:
      csObject = GetAutoType();
      break;
    default:
      ASSERT(0);
  }

  const bool bValue = !csObject.empty();
  if (iFunction == PWSMatch::MR_PRESENT || iFunction == PWSMatch::MR_NOTPRESENT) {
    return PWSMatch::Match(bValue, iFunction);
  }

  if (!bValue) // String empty - always return false for other comparisons
    return false;
  else
    return PWSMatch::Match(string1.c_str(), csObject, iFunction);
}

bool CItemData::Matches(int num1, int num2, int iObject,
                        int iFunction) const
{
  //   Check integer values are selected
  int iValue;

  switch (iObject) {
    case XTIME_INT:
      GetXTimeInt(iValue);
      break;
    default:
      ASSERT(0);
      return false;
  }

  const bool bValue = (iValue != 0);
  if (iFunction == PWSMatch::MR_PRESENT || iFunction == PWSMatch::MR_NOTPRESENT)
    return PWSMatch::Match(bValue, iFunction);

  if (!bValue) // integer empty - always return false for other comparisons
    return false;
  else
    return PWSMatch::Match(num1, num2, iValue, iFunction);
}

bool CItemData::Matches(time_t time1, time_t time2, int iObject,
                        int iFunction) const
{
  //   Check time values are selected
  time_t tValue;

  switch (iObject) {
    case CTIME:
    case PMTIME:
    case ATIME:
    case XTIME:
    case RMTIME:
      GetTime(iObject, tValue);
      break;
    default:
      ASSERT(0);
      return false;
  }

  const bool bValue = (tValue != (time_t)0);
  if (iFunction == PWSMatch::MR_PRESENT || iFunction == PWSMatch::MR_NOTPRESENT) {
    return PWSMatch::Match(bValue, iFunction);
  }

  if (!bValue)  // date empty - always return false for other comparisons
    return false;
  else {
    time_t testtime;
    if (tValue != (time_t)0) {
      CTime ct(tValue);
      CTime ct2;
      ct2 = CTime(ct.GetYear(), ct.GetMonth(), ct.GetDay(), 0, 0, 0);
      testtime = (time_t)ct2.GetTime();
    } else
      testtime = (time_t)0;
    return PWSMatch::Match(time1, time2, testtime, iFunction);
  }
}
  
bool CItemData::Matches(EntryType etype1, int iFunction) const
{
  switch (iFunction) {
    case PWSMatch::MR_IS:
      return GetEntryType() == etype1;
    case PWSMatch::MR_ISNOT:
      return GetEntryType() != etype1;
    default:
      ASSERT(0);
  }
  return false;
}

bool CItemData::IsExpired()
{
  time_t now, XTime;
  time(&now);

  GetXTime(XTime);
  if ((XTime != (time_t)0) && (XTime < now))
    return true;
  else
    return false;
}

bool CItemData::WillExpire(const int numdays)
{
  time_t now, exptime, XTime;
  time(&now);

  GetXTime(XTime);
  // Check if there is an expiry date?
  if (XTime == (time_t)0)
    return false;

  // Ignore if already expired
  if (XTime <= now)
    return false;

  struct tm st;
#if _MSC_VER >= 1400
  errno_t err;
  err = localtime_s(&st, &now);  // secure version
  ASSERT(err == 0);
#else
  st = *localtime(&now);
#endif
  st.tm_mday += numdays;
  exptime = mktime(&st);
  if (exptime == (time_t)-1)
    exptime = now;

  // Will it expire in numdays?
  if (XTime < exptime)
    return true;
  else
    return false;
}

static bool pull_string(StringX &str, unsigned char *data, int len)
{
  CUTF8Conv utf8conv;
  vector<unsigned char> v(data, (data + len));
  v.push_back(0); // null terminate for FromUTF8.
  bool utf8status = utf8conv.FromUTF8((unsigned char *)&v[0],
    len, str);
  if (!utf8status) {
    TRACE(_T("CItemData::DeserializePlainText(): FromUTF8 failed!\n"));
  }
  trashMemory(&v[0], len);
  return utf8status;
}

static bool pull_time(time_t &t, unsigned char *data, size_t len)
{
  if (len == sizeof(__time32_t)) {
    t = *reinterpret_cast<__time32_t *>(data);
  } else if (len == sizeof(__time64_t)) {
    // convert from 64 bit time to currently supported 32 bit
    struct tm ts;
    __time64_t *t64 = reinterpret_cast<__time64_t *>(data);
    if (_gmtime64_s(&ts, t64) != 0) {
      ASSERT(0); return false;
    }
    t = _mkgmtime32(&ts);
    if (t == time_t(-1)) { // time is past 2038!
      t = 0; return false;
    }
  } else {
    ASSERT(0); return false;
  }
  return true;
}

static bool pull_int(int &i, unsigned char *data, size_t len)
{
  if (len == sizeof(int)) {
    i = *reinterpret_cast<int *>(data);
  } else {
    ASSERT(0);
    return false;
  }
  return true;
}

bool CItemData::DeserializePlainText(const std::vector<char> &v)
{
  vector<char>::const_iterator iter = v.begin();
  int emergencyExit = 255;

  while (iter != v.end()) {
    int type = (*iter++ & 0xff); // required since enum is an int
    if (size_t(v.end() - iter) < sizeof(size_t)) {
      ASSERT(0); // type must ALWAYS be followed by length
      return false;
    }

    if (type == END)
      return true; // happy end

    unsigned int len = *((unsigned int *)&(*iter));
    ASSERT(len < v.size()); // sanity check
    iter += sizeof(unsigned int);

    if (--emergencyExit == 0) {
      ASSERT(0);
      return false;
    }
    if (!SetField(type, (unsigned char *)&(*iter), len))
      return false;
    iter += len;
  }
  return false; // END tag not found!
}

bool CItemData::SetField(int type, unsigned char *data, int len)
{
  StringX str;
  time_t t;
  int xint;
  switch (type) {
    case NAME:
      ASSERT(0); // not serialized, or in v3 format
      return false;
    case UUID:
    {
      uuid_array_t uuid_array;
      ASSERT(len == sizeof(uuid_array));
      for (unsigned i = 0; i < sizeof(uuid_array); i++)
        uuid_array[i] = data[i];
      SetUUID(uuid_array);
      break;
    }
    case GROUP:
      if (!pull_string(str, data, len)) return false;
      SetGroup(str);
      break;
    case TITLE:
      if (!pull_string(str, data, len)) return false;
      SetTitle(str);
      break;
    case USER:
      if (!pull_string(str, data, len)) return false;
      SetUser(str);
      break;
    case NOTES:
      if (!pull_string(str, data, len)) return false;
      SetNotes(str);
      break;
    case PASSWORD:
      if (!pull_string(str, data, len)) return false;
      SetPassword(str);
      break;
    case CTIME:
      if (!pull_time(t, data, len)) return false;
      SetCTime(t);
      break;
    case  PMTIME:
      if (!pull_time(t, data, len)) return false;
      SetPMTime(t);
      break;
    case ATIME:
      if (!pull_time(t, data, len)) return false;
      SetATime(t);
      break;
    case XTIME:
      if (!pull_time(t, data, len)) return false;
      SetXTime(t);
      break;
    case XTIME_INT:
      if (!pull_int(xint, data, len)) return false;
      SetXTimeInt(xint);
      break;
    case POLICY:
      if (!pull_string(str, data, len)) return false;
      SetPWPolicy(str.c_str());
      break;
    case RMTIME:
      if (!pull_time(t, data, len)) return false;
      SetRMTime(t);
      break;
    case URL:
      if (!pull_string(str, data, len)) return false;
      SetURL(str);
      break;
    case AUTOTYPE:
      if (!pull_string(str, data, len)) return false;
      SetAutoType(str);
      break;
    case PWHIST:
      if (!pull_string(str, data, len)) return false;
      SetPWHistory(str);
      break;
    case END:
      break;
    default:
      // unknowns!
      SetUnknownField(char(type), len, data);
      break;
  }
  return true;
}

static void push_length(vector<char> &v, unsigned int s)
{
  v.insert(v.end(),
    (char *)&s, (char *)&s + sizeof(s));
}

static void push_string(vector<char> &v, char type,
                        const StringX &str)
{
  if (!str.empty()) {
    CUTF8Conv utf8conv;
    bool status;
    const unsigned char *utf8;
    int utf8Len;
    status = utf8conv.ToUTF8(str, utf8, utf8Len);
    if (status) {
      v.push_back(type);
      push_length(v, utf8Len);
      v.insert(v.end(), (char *)utf8, (char *)utf8 + utf8Len);
    } else
      TRACE(_T("ItemData::SerializePlainText:ToUTF8(%s) failed\n"),
            str.c_str());
  }
}

static void push_time(vector<char> &v, char type, time_t t)
{
  if (t != 0) {
    __time32_t t32 = (__time32_t)t;
    v.push_back(type);
    push_length(v, sizeof(t32));
    v.insert(v.end(),
      (char *)&t32, (char *)&t32 + sizeof(t32));
  }
}

static void push_int(vector<char> &v, char type, int i)
{
  if (i != 0) {
    v.push_back(type);
    push_length(v, sizeof(int));
    v.insert(v.end(),
      (char *)&i, (char *)&i + sizeof(int));
  }
}

void CItemData::SerializePlainText(vector<char> &v, CItemData *cibase)  const
{
  StringX tmp;
  uuid_array_t uuid_array;
  time_t t = 0;
  int xi = 0;

  v.clear();
  GetUUID(uuid_array);
  v.push_back(UUID);
  push_length(v, sizeof(uuid_array));
  v.insert(v.end(), uuid_array, (uuid_array + sizeof(uuid_array)));
  push_string(v, GROUP, GetGroup());
  push_string(v, TITLE, GetTitle());
  push_string(v, USER, GetUser());

  if (m_entrytype == ET_ALIAS) {
    // I am an alias entry
    ASSERT(cibase != NULL);
    tmp = _T("[[") + cibase->GetGroup() + _T(":") + cibase->GetTitle() + _T(":") + cibase->GetUser() + _T("]]");
  } else
  if (m_entrytype == ET_SHORTCUT) {
    // I am a shortcut entry
    ASSERT(cibase != NULL);
    tmp = _T("[~") + cibase->GetGroup() + _T(":") + cibase->GetTitle() + _T(":") + cibase->GetUser() + _T("~]");
  } else
    tmp = GetPassword();

  push_string(v, PASSWORD, tmp);
  push_string(v, NOTES, GetNotes());
  push_string(v, URL, GetURL());
  push_string(v, AUTOTYPE, GetAutoType());

  GetCTime(t);   push_time(v, CTIME, t);
  GetPMTime(t);  push_time(v, PMTIME, t);
  GetATime(t);   push_time(v, ATIME, t);
  GetXTime(t);   push_time(v, XTIME, t);
  GetRMTime(t);  push_time(v, RMTIME, t);

  GetXTimeInt(xi); push_int(v, XTIME_INT, xi);

  push_string(v, POLICY, GetPWPolicy());
  push_string(v, PWHIST, GetPWHistory());

  UnknownFieldsConstIter vi_IterURFE;
  for (vi_IterURFE = GetURFIterBegin();
       vi_IterURFE != GetURFIterEnd();
       vi_IterURFE++) {
    unsigned char type;
    unsigned int length = 0;
    unsigned char *pdata = NULL;
    GetUnknownField(type, length, pdata, vi_IterURFE);
    if (length != 0) {
      v.push_back((char)type);
      push_length(v, length);
      v.insert(v.end(), (char *)pdata, (char *)pdata + length);
      trashMemory(pdata, length);
    }
    delete[] pdata;
  }

  int end = END; // just to keep the compiler happy...
  v.push_back(static_cast<const char>(end));
  push_length(v, 0);
}

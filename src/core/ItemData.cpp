/*
* Copyright (c) 2003-2015 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file ItemData.cpp
//-----------------------------------------------------------------------------

#include "ItemData.h"
#include "BlowFish.h"
#include "TwoFish.h"
#include "PWSrand.h"
#include "UTF8Conv.h"
#include "PWSprefs.h"
#include "VerifyFormat.h"
#include "PWHistory.h"
#include "Util.h"
#include "StringXStream.h"
#include "core.h"
#include "PWSfile.h"
#include "PWStime.h"

#include "os/typedefs.h"
#include "os/pws_tchar.h"
#include "os/mem.h"
#include "os/env.h"
#include "os/utf8conv.h"

#include <time.h>
#include <sstream>
#include <iomanip>

using namespace std;
using pws_os::CUUID;

bool CItemData::IsSessionKeySet = false;
unsigned char CItemData::SessionKey[64];

// some fwd declarations:
static bool pull_string(StringX &str, const unsigned char *data, size_t len);
static bool pull_time(time_t &t, const unsigned char *data, size_t len);
static bool pull_int32(int32 &i, const unsigned char *data, size_t len);
static bool pull_int16(int16 &i16, const unsigned char *data, size_t len);
static bool pull_char(unsigned char &uc, const unsigned char *data, size_t len);


void CItemData::SetSessionKey()
{
  // must be called once per session, no more, no less
  ASSERT(!IsSessionKeySet);
  pws_os::mlock(SessionKey, sizeof(SessionKey));
  PWSrand::GetInstance()->GetRandomData(SessionKey, sizeof(SessionKey));
  IsSessionKeySet = true;
}

//-----------------------------------------------------------------------------
// Constructors

CItemData::CItemData()
  : m_entrytype(ET_NORMAL), m_entrystatus(ES_CLEAN),
    m_display_info(NULL)
{
  PWSrand::GetInstance()->GetRandomData( m_salt, SaltLength );
}

CItemData::CItemData(const CItemData &that) :
  m_fields(that.m_fields),
  m_entrytype(that.m_entrytype), m_entrystatus(that.m_entrystatus),
  m_display_info(that.m_display_info == NULL ?
                      NULL : that.m_display_info->clone())
{
  memcpy(m_salt, that.m_salt, SaltLength);
  m_URFL = that.m_URFL;
}

CItemData::~CItemData()
{
  delete m_display_info;
}

CItemData& CItemData::operator=(const CItemData &that)
{
  // Check for self-assignment
  if (this != &that) {
    m_fields = that.m_fields;

    delete m_display_info;
    m_display_info = that.m_display_info == NULL ?
      NULL : that.m_display_info->clone();

    m_URFL = that.m_URFL;

    m_entrytype = that.m_entrytype;
    m_entrystatus = that.m_entrystatus;
    memcpy(m_salt, that.m_salt, SaltLength);
  }

  return *this;
}

void CItemData::Clear()
{
  m_fields.clear();
  m_URFL.clear();
  m_entrytype = ET_NORMAL;
  m_entrystatus = ES_CLEAN;
}

void CItemData::ParseSpecialPasswords()
{
  // For V3 records, the Base UUID and dependent type (shortcut or alias)
  // is encoded in the password field. 
  // If the password isn't in the encoded format, this is a no-op
  // If it is, then this 'normalizes' the entry record to be the same
  // as a V4 one.

  const StringX csMyPassword = GetPassword();
  if (csMyPassword.length() == 36) { // look for "[[uuid]]" or "[~uuid~]"
    StringX cs_possibleUUID = csMyPassword.substr(2, 32); // try to extract uuid
    ToLower(cs_possibleUUID);
    if (((csMyPassword.substr(0,2) == _T("[[") &&
          csMyPassword.substr(csMyPassword.length() - 2) == _T("]]")) ||
         (csMyPassword.substr(0, 2) == _T("[~") &&
          csMyPassword.substr(csMyPassword.length() - 2) == _T("~]"))) &&
        cs_possibleUUID.find_first_not_of(_T("0123456789abcdef")) == StringX::npos) {
      CUUID buuid(cs_possibleUUID.c_str());
      SetUUID(buuid, BASEUUID);
      uuid_array_t uuid;
      GetUUID(uuid);
      FieldType ft = UUID;
      if (csMyPassword.substr(0, 2) == _T("[[")) {
        ft = ALIASUUID;
      } else if (csMyPassword.substr(0, 2) == _T("[~")) {
        ft = SHORTCUTUUID;
      } else {
        ASSERT(0);
      }
      ClearField(UUID);
      SetUUID(uuid, ft);
    }
  }
}

bool CItemData::HasUUID() const
{
  return (((m_entrytype == ET_NORMAL ||
            m_entrytype == ET_ALIASBASE ||
            m_entrytype == ET_SHORTCUTBASE) && IsFieldSet(UUID)) ||
          (m_entrytype == ET_ALIAS && IsFieldSet(ALIASUUID)) ||
          (m_entrytype == ET_SHORTCUT && IsFieldSet(SHORTCUTUUID)));
}

void CItemData::SetSpecialPasswords()
{
  // Meant to be used for writing a record
  // in V3 format
  ASSERT(HasUUID());

  CUUID base_uuid(CUUID::NullUUID());
  if (m_fields.find(ALIASUUID) != m_fields.end())
    base_uuid = GetUUID(ALIASUUID);
  else if (m_fields.find(SHORTCUTUUID) != m_fields.end())
    base_uuid = GetUUID(SHORTCUTUUID);

  if (base_uuid != CUUID::NullUUID()) {
    StringX uuid_str;

    if (IsAlias()) {
      uuid_str = _T("[[");
      uuid_str += base_uuid;
      uuid_str += _T("]]");
    } else if (IsShortcut()) {
      uuid_str = _T("[~");
      uuid_str += base_uuid;
      uuid_str += _T("~]");
    } else {
      ASSERT(0);
    }
    SetPassword(uuid_str);
  }
}

int CItemData::Read(PWSfile *in)
{
  int status = PWSfile::SUCCESS;

  signed long numread = 0;
  unsigned char type;

  int emergencyExit = 255; // to avoid endless loop.
  signed long fieldLen; // <= 0 means end of file reached

  do {
    unsigned char *utf8 = NULL;
    size_t utf8Len = 0;
    fieldLen = static_cast<signed long>(in->ReadField(type, utf8,
                                                      utf8Len));

    if (fieldLen > 0) {
      numread += fieldLen;
      if (!SetField(type, utf8, utf8Len)) {
        status = PWSfile::FAILURE;
        break;
      }
    } // if (fieldLen > 0)

    if (utf8 != NULL) {
      trashMemory(utf8, utf8Len * sizeof(utf8[0]));
      delete[] utf8; utf8 = NULL; utf8Len = 0;
    }
  } while (type != END && fieldLen > 0 && --emergencyExit > 0);

  if (numread > 0) {
    // Determine entry type:
    // ET_NORMAL (which may later change to ET_ALIASBASE or ET_SHORTCUTBASE)
    // ET_ALIAS or ET_SHORTCUT
    // For V4, this is simple, as we have different UUID types
    // For V3, we need to parse the password
    ParseSpecialPasswords();
    if (m_fields.find(UUID) != m_fields.end())
      m_entrytype = ET_NORMAL; // may change later to ET_*BASE
    else if (m_fields.find(ALIASUUID) != m_fields.end())
      m_entrytype = ET_ALIAS;
    else if (m_fields.find(SHORTCUTUUID) != m_fields.end())
      m_entrytype = ET_SHORTCUT;
    else 
      ASSERT(0);
    return status;
  } else
    return PWSfile::END_OF_FILE;
}

size_t CItemData::WriteIfSet(FieldType ft, PWSfile *out, bool isUTF8) const
{
  FieldConstIter fiter = m_fields.find(ft);
  size_t retval = 0;
  if (fiter != m_fields.end()) {
    const CItemField &field = fiter->second;
    ASSERT(!field.IsEmpty());
    size_t flength = field.GetLength() + BlowFish::BLOCKSIZE;
    unsigned char *pdata = new unsigned char[flength];
    GetField(field, pdata, flength);
    if (isUTF8) {
      wchar_t *wpdata = reinterpret_cast<wchar_t *>(pdata);
      size_t srclen = field.GetLength()/sizeof(TCHAR);
      wpdata[srclen] = 0;
      size_t dstlen = pws_os::wcstombs(NULL, 0, wpdata, srclen);
      ASSERT(dstlen > 0);
      char *dst = new char[dstlen+1];
      dstlen = pws_os::wcstombs(dst, dstlen, wpdata, srclen);
      ASSERT(dstlen != size_t(-1));
      //[BR1150, BR1167]: Discard the terminating NULLs in text fields
      if (dstlen && !dst[dstlen-1])
        dstlen--;
      retval = out->WriteField(static_cast<unsigned char>(ft), reinterpret_cast<unsigned char *>(dst), dstlen);
      trashMemory(dst, dstlen);
      delete[] dst;
    } else {
      retval = out->WriteField(static_cast<unsigned char>(ft), pdata, field.GetLength());
    }
    trashMemory(pdata, flength);
    delete[] pdata;
  }
  return retval;
}

int CItemData::Write(PWSfile *out) const
{
  int status = PWSfile::SUCCESS;
  uuid_array_t item_uuid;
  int i;

  const FieldType TextFields[] = {GROUP, TITLE, USER, PASSWORD,
                                  NOTES, URL, AUTOTYPE, POLICY,
                                  PWHIST, RUNCMD, EMAIL,
                                  SYMBOLS, POLICYNAME,
                                  END};
  const FieldType TimeFields[] = {ATIME, CTIME, XTIME, PMTIME, RMTIME,
                                  END};

  ASSERT(HasUUID());
  GetUUID(item_uuid);
  out->WriteField(UUID, item_uuid, sizeof(uuid_array_t));

  for (i = 0; TextFields[i] != END; i++)
    WriteIfSet(TextFields[i], out, true);

  for (i = 0; TimeFields[i] != END; i++) {
    time_t t = 0;
    GetTime(TimeFields[i], t);
    if (t != 0) {
      if (out->timeFieldLen() == 4) {
        unsigned char buf[4];
        putInt32(buf, static_cast<int32>(t));
        out->WriteField(static_cast<unsigned char>(TimeFields[i]), buf, out->timeFieldLen());
      } else if (out->timeFieldLen() == PWStime::TIME_LEN) {
        PWStime pwt(t);
        out->WriteField(static_cast<unsigned char>(TimeFields[i]), pwt, pwt.GetLength());
      } else ASSERT(0);
    } // t != 0
  }

  int32 i32 = 0;
  unsigned char buf32[sizeof(i32)];
  GetXTimeInt(i32);
  if (i32 > 0 && i32 <= 3650) {
    putInt(buf32, i32);
    out->WriteField(XTIME_INT, buf32, sizeof(int32));
  }

  i32 = 0;
  GetKBShortcut(i32);
  if (i32 != 0) {
    putInt(buf32, i32);
    out->WriteField(KBSHORTCUT, buf32, sizeof(int32));
  }

  int16 i16 = 0;
  unsigned char buf16[sizeof(i16)];
  GetDCA(i16);
  if (i16 >= PWSprefs::minDCA && i16 <= PWSprefs::maxDCA) {
    putInt(buf16, i16);
    out->WriteField(DCA, buf16, sizeof(int16));
  }
  i16 = 0;
  GetShiftDCA(i16);
  if (i16 >= PWSprefs::minDCA && i16 <= PWSprefs::maxDCA) {
    putInt(buf16, i16);
    out->WriteField(SHIFTDCA, buf16, sizeof(int16));
  }
  WriteIfSet(PROTECTED, out, false);

  WriteUnknowns(out);
  // Assume that if previous write failed, last one will too for same reason
  status = out->WriteField(END, _T(""));

  return status;
}

int CItemData::WriteUnknowns(PWSfile *out) const
{
  for (UnknownFieldsConstIter uiter = m_URFL.begin();
       uiter != m_URFL.end();
       uiter++) {
    unsigned char type;
    size_t length = 0;
    unsigned char *pdata = NULL;
    GetUnknownField(type, length, pdata, *uiter);
    out->WriteField(type, pdata, length);
    trashMemory(pdata, length);
    delete[] pdata;
  }
  return PWSfile::SUCCESS;
}



//-----------------------------------------------------------------------------
// Accessors

StringX CItemData::GetField(const FieldType ft) const
{
  FieldConstIter fiter = m_fields.find(ft);
  return fiter == m_fields.end() ? _T("") : GetField(fiter->second);
}

StringX CItemData::GetField(const CItemField &field) const
{
  StringX retval;
  BlowFish *bf = MakeBlowFish(field.IsEmpty());
  field.Get(retval, bf);
  delete bf;
  return retval;
}

void CItemData::GetField(const CItemField &field, unsigned char *value, size_t &length) const
{
  BlowFish *bf = MakeBlowFish(field.IsEmpty());
  field.Get(value, length, bf);
  delete bf;
}

StringX CItemData::GetFieldValue(FieldType ft) const
{
  if (IsTextField(static_cast<unsigned char>(ft)) && ft != GROUPTITLE &&
      ft != NOTES && ft != PWHIST) {
    return GetField(ft);
  } else {
    StringX str(_T(""));
    switch (ft) {
    case GROUPTITLE: /* 00 */
      str = GetGroup() + TCHAR('.') + GetTitle();
      break;
    case UUID:       /* 01 */
      {
        uuid_array_t uuid_array = {0};
        GetUUID(uuid_array);
        str = CUUID(uuid_array, true);
        break;
      }
    case NOTES:      /* 05 */
      return GetNotes();
    case CTIME:      /* 07 */
      return GetCTimeL();
    case PMTIME:     /* 08 */
      return GetPMTimeL();
    case ATIME:      /* 09 */
      return GetATimeL();
    case XTIME:      /* 0a */
      {
        int32 xint(0);
        str = GetXTimeL();
        GetXTimeInt(xint);
        if (xint != 0)
          str += _T(" *");
        return str;
      }
    case RESERVED:   /* 0b */
      break;
    case RMTIME:     /* 0c */
      return GetRMTimeL();
    case PWHIST:     /* 0f */
      return GetPWHistory();
    case XTIME_INT:  /* 11 */
      return GetXTimeInt();
    case DCA:        /* 13 */
      return GetDCA();
    case PROTECTED:  /* 15 */
      {
        unsigned char uc;
        StringX sxProtected = _T("");
        GetProtected(uc);
        if (uc != 0)
          LoadAString(sxProtected, IDSC_YES);
        return sxProtected;
      }
    case SHIFTDCA:   /* 17 */
      return GetShiftDCA();
    case KBSHORTCUT: /* 19 */
      return GetKBShortcut();
    default:
      ASSERT(0);
    }
    return str;
  }
}

size_t CItemData::GetSize() const
{
  size_t length(0);

  for (FieldConstIter fiter = m_fields.begin(); fiter != m_fields.end(); fiter++)
    length += fiter->second.GetLength();


  for (UnknownFieldsConstIter ufiter = m_URFL.begin();
       ufiter != m_URFL.end(); ufiter++)
    length += ufiter->GetLength();

  return length;
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
  StringX ret = GetField(NOTES);
  CleanNotes(ret, delimiter);
  return ret;
}

StringX CItemData::GetTime(int whichtime, PWSUtil::TMC result_format) const
{
  time_t t;

  GetTime(whichtime, t);
  return PWSUtil::ConvertToDateTimeString(t, result_format);
}

void CItemData::GetTime(int whichtime, time_t &t) const
{
  FieldConstIter fiter = m_fields.find(FieldType(whichtime));
  if (fiter != m_fields.end()) {
    unsigned char in[TwoFish::BLOCKSIZE]; // required by GetField
    size_t tlen = sizeof(in); // ditto

    GetField(fiter->second, in, tlen);
    if (tlen != 0) {
    // time field's store in native time_t size, regardless of
    // the representation on file
      ASSERT(tlen == sizeof(t));
      if (!pull_time(t, in, tlen))
        ASSERT(0);
    } else {
      t = 0;
    }
  } else // fiter == m_fields.end()
    t = 0;
}

void CItemData::GetUUID(uuid_array_t &uuid_array, FieldType ft) const
{
  size_t length = sizeof(uuid_array_t);
  FieldConstIter fiter = m_fields.end();
  if (ft != END) { // END means "infer correct UUID from entry type"
    // anything != END is used as-is, no questions asked
    fiter = m_fields.find(ft);
  } else switch (m_entrytype) {
    case ET_NORMAL:
    case ET_ALIASBASE:
    case ET_SHORTCUTBASE:
      fiter = m_fields.find(UUID);
      break;
    case ET_ALIAS:
      fiter = m_fields.find(ALIASUUID);
      break;
    case ET_SHORTCUT:
      fiter = m_fields.find(SHORTCUTUUID);
      break;
    default:
      ASSERT(0);
    }
  if (fiter == m_fields.end()) {
    pws_os::Trace(_T("CItemData::GetUUID(uuid_array_t) - no UUID found!"));
    memset(uuid_array, 0, length);
  } else
    GetField(fiter->second, static_cast<unsigned char *>(uuid_array), length);
}

const CUUID CItemData::GetUUID(FieldType ft) const
{
  // Ideally we'd like to return a uuid_array_t, but C++ doesn't
  // allow array return values.
  // If we returned the uuid_array_t pointer, we'd have a scope problem,
  // as the pointer's owner would be deleted too soon.
  // Frustrating, but that's life...

  uuid_array_t ua;
  GetUUID(ua, ft);
  return CUUID(ua);
}

void CItemData::GetPWPolicy(PWPolicy &pwp) const
{
  PWPolicy mypol(GetField(POLICY));
  pwp = mypol;
}

void CItemData::GetXTimeInt(int32 &xint) const
{
  FieldConstIter fiter = m_fields.find(XTIME_INT);
  if (fiter == m_fields.end())
    xint = 0;
  else {
    unsigned char in[TwoFish::BLOCKSIZE]; // required by GetField
    size_t tlen = sizeof(in); // ditto

    GetField(fiter->second, in, tlen);
    if (tlen != 0) {
      ASSERT(tlen == sizeof(int32));
      memcpy(&xint, in, sizeof(int32));
    } else {
      xint = 0;
    }
  }
}

StringX CItemData::GetXTimeInt() const
{
  int32 xint;
  GetXTimeInt(xint);
  if (xint == 0)
    return _T("");

  oStringXStream os;
  os << xint;
  return os.str();
}

void CItemData::GetProtected(unsigned char &ucprotected) const
{
  FieldConstIter fiter = m_fields.find(PROTECTED);
  if (fiter == m_fields.end())
    ucprotected = 0;
  else {
    unsigned char in[TwoFish::BLOCKSIZE]; // required by GetField
    size_t tlen = sizeof(in); // ditto
    GetField(fiter->second, in, tlen);
    if (tlen != 0) {
      ASSERT(tlen == sizeof(char));
      ucprotected = in[0];
    } else {
      ucprotected = 0;
    }
  }
}

StringX CItemData::GetProtected() const
{
  unsigned char ucprotected;
  GetProtected(ucprotected);

  return ucprotected != 0 ? StringX(_T("1")) : StringX(_T(""));
}

bool CItemData::IsProtected() const
{
  unsigned char ucprotected;
  GetProtected(ucprotected);
  return ucprotected != 0;
}

void CItemData::GetDCA(int16 &iDCA, const bool bShift) const
{
  FieldConstIter fiter = m_fields.find(bShift ? SHIFTDCA : DCA);
  if (fiter != m_fields.end()) {
    unsigned char in[TwoFish::BLOCKSIZE]; // required by GetField
    size_t tlen = sizeof(in); // ditto
    GetField(fiter->second, in, tlen);

    if (tlen != 0) {
      ASSERT(tlen == sizeof(int16));
      memcpy(&iDCA, in, sizeof(int16));
    } else {
      iDCA = -1;
    }
  } else // fiter == m_fields.end()
    iDCA = -1;
}

StringX CItemData::GetDCA(const bool bShift) const
{
  int16 dca;
  GetDCA(dca, bShift);
  oStringXStream os;
  os << dca;
  return os.str();
}

void CItemData::GetKBShortcut(int32 &iKBShortcut) const
{
  FieldConstIter fiter = m_fields.find(KBSHORTCUT);
  if (fiter != m_fields.end()) {
    unsigned char in[TwoFish::BLOCKSIZE]; // required by GetField
    size_t tlen = sizeof(in); // ditto
    GetField(fiter->second, in, tlen);

    if (tlen != 0) {
      ASSERT(tlen == sizeof(int32));
      memcpy(&iKBShortcut, in, sizeof(int32));
    } else {
      iKBShortcut = 0;
    }
  } else // fiter == m_fields.end()
    iKBShortcut = 0;
}

StringX CItemData::GetKBShortcut() const
{
  int32 iKBShortcut;
  GetKBShortcut(iKBShortcut);

  if (iKBShortcut != 0) {
    StringX kbs(_T(""));

    WORD wVirtualKeyCode = iKBShortcut & 0xff;
    WORD wPWSModifiers = iKBShortcut >> 16;

    if (iKBShortcut != 0) {
      if (wPWSModifiers & PWS_HOTKEYF_ALT)
        kbs += _T("A");
      if (wPWSModifiers & PWS_HOTKEYF_CONTROL)
        kbs += _T("C");
      if (wPWSModifiers & PWS_HOTKEYF_SHIFT)
        kbs += _T("S");
      if (wPWSModifiers & PWS_HOTKEYF_EXT)
        kbs += _T("E");
      if (wPWSModifiers & PWS_HOTKEYF_META)
        kbs += _T("M");
      if (wPWSModifiers & PWS_HOTKEYF_WIN)
        kbs += _T("W");
      if (wPWSModifiers & PWS_HOTKEYF_CMD)
        kbs += _T("D");

      kbs += _T(":");
      ostringstreamT os1;
      os1 << hex << setfill(charT('0')) << setw(4) << wVirtualKeyCode;
      kbs += os1.str().c_str();
      return kbs;
    }
  }
  return _T("");
}

void CItemData::GetUnknownField(unsigned char &type, size_t &length,
                                unsigned char * &pdata,
                                const CItemField &item) const
{
  ASSERT(pdata == NULL && length == 0);

  type = item.GetType();
  size_t flength = item.GetLength();
  length = flength;
  flength += BlowFish::BLOCKSIZE; // ensure that we've enough for at least one block
  pdata = new unsigned char[flength];
  GetField(item, pdata, flength);
}

StringX CItemData::GetPWHistory() const
{
  StringX ret = GetField(PWHIST);
  if (ret == _T("0") || ret == _T("00000"))
    ret = _T("");
  return ret;
}

StringX CItemData::GetPlaintext(const TCHAR &separator,
                                const FieldBits &bsFields,
                                const TCHAR &delimiter,
                                const CItemData *pcibase) const
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
    PWHistList pwhistlist;

    pwh_status = CreatePWHistoryList(GetPWHistory(), pwh_max, num_err,
                                     pwhistlist, PWSUtil::TMC_EXPORT_IMPORT);

    //  Build export string
    history = MakePWHistoryHeader(pwh_status, pwh_max, pwhistlist.size());
    PWHistList::iterator iter;
    for (iter = pwhistlist.begin(); iter != pwhistlist.end(); iter++) {
      const PWHistEntry &pwshe = *iter;
      history += _T(' ');
      history += pwshe.changedate;
      ostringstreamT os1;
      os1 << hex << charT(' ') << setfill(charT('0')) << setw(4)
          << pwshe.password.length() << charT(' ');
      history += os1.str().c_str();
      history += pwshe.password;
    }
  }

  StringX csPassword;
  if (m_entrytype == ET_ALIAS) {
    ASSERT(pcibase != NULL);
    csPassword = _T("[[") +
                 pcibase->GetGroup() + _T(":") +
                 pcibase->GetTitle() + _T(":") +
                 pcibase->GetUser() + _T("]]") ;
  } else if (m_entrytype == ET_SHORTCUT) {
    ASSERT(pcibase != NULL);
    csPassword = _T("[~") +
                 pcibase->GetGroup() + _T(":") +
                 pcibase->GetTitle() + _T(":") +
                 pcibase->GetUser() + _T("~]") ;
  } else
    csPassword = GetPassword();

  // Notes field must be last, for ease of parsing import
  if (bsFields.count() == bsFields.size()) {
    // Everything - note can't actually set all bits via dialog!
    // Must be in same order as full header
    unsigned char uc;
    GetProtected(uc);
    StringX sxProtected = uc != 0 ? _T("Y") : _T("N");
    ret = (grouptitle + separator +
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
           GetPolicyName() + separator +
           history + separator +
           GetRunCommand() + separator +
           GetDCA() + separator +
           GetShiftDCA() + separator +
           GetEmail() + separator +
           sxProtected + separator +
           GetSymbols() + separator +
           GetKBShortcut() + separator +
           _T("\"") + notes + _T("\""));
  } else {
    // Not everything
    // Must be in same order as custom header
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

    StringX sxPolicyName = GetPolicyName();
    if (sxPolicyName.empty()) {
      // print policy only if policy name is not available
      if (bsFields.test(CItemData::POLICY))
        ret += GetPWPolicy() + separator;
      if (bsFields.test(CItemData::POLICYNAME))
        ret += separator;
    } else {
      // if policy name is available, ignore the policy
      if (bsFields.test(CItemData::POLICY))
        ret += separator;
      if (bsFields.test(CItemData::POLICYNAME))
        ret += sxPolicyName + separator;
    }

    if (bsFields.test(CItemData::PWHIST))
      ret += history + separator;
    if (bsFields.test(CItemData::RUNCMD))
      ret += GetRunCommand() + separator;
    if (bsFields.test(CItemData::DCA))
      ret += GetDCA() + separator;
    if (bsFields.test(CItemData::SHIFTDCA))
      ret += GetShiftDCA() + separator;
    if (bsFields.test(CItemData::EMAIL))
      ret += GetEmail() + separator;
    if (bsFields.test(CItemData::PROTECTED)) {
      unsigned char uc;
      GetProtected(uc);
      StringX sxProtected = uc != 0 ? _T("Y") : _T("N");
      ret += sxProtected + separator;
    }
    if (bsFields.test(CItemData::SYMBOLS))
      ret += GetSymbols() + separator;

    if (bsFields.test(CItemData::KBSHORTCUT)) {
      ret += GetKBShortcut() + separator;
    }

    if (bsFields.test(CItemData::NOTES))
      ret += _T("\"") + notes + _T("\"");
    // remove trailing separator
    if (ret[ret.length()-1] == separator) {
      size_t rl = ret.length();
      ret = ret.substr(0, rl - 1);
    }
  }

  return ret;
}

string CItemData::GetXML(unsigned id, const FieldBits &bsExport,
                         TCHAR delimiter, const CItemData *pcibase,
                         bool bforce_normal_entry,
                         bool &bXMLErrorsFound) const
{
  bXMLErrorsFound = false;
  ostringstream oss; // ALWAYS a string of chars, never wchar_t!
  oss << "\t<entry id=\"" << dec << id << "\"";
  if (bforce_normal_entry)
    oss << " normal=\"" << "true" << "\"";

  oss << ">" << endl;

  StringX tmp;
  CUTF8Conv utf8conv;
  unsigned char uc;
  bool brc;

  tmp = GetGroup();
  if (bsExport.test(CItemData::GROUP) && !tmp.empty()) {
    brc = PWSUtil::WriteXMLField(oss, "group", tmp, utf8conv);
    if (!brc) bXMLErrorsFound = true;
  }

  // Title mandatory (see pwsafe.xsd)
  brc = PWSUtil::WriteXMLField(oss, "title", GetTitle(), utf8conv);
  if (!brc) bXMLErrorsFound = true;

  tmp = GetUser();
  if (bsExport.test(CItemData::USER) && !tmp.empty()) {
    brc = PWSUtil::WriteXMLField(oss, "username", tmp, utf8conv);
    if (!brc) bXMLErrorsFound = true;
  }

  // Password mandatory (see pwsafe.xsd)
  if (m_entrytype == ET_ALIAS) {
    ASSERT(pcibase != NULL);
    tmp = _T("[[") +
          pcibase->GetGroup() + _T(":") +
          pcibase->GetTitle() + _T(":") +
          pcibase->GetUser() + _T("]]") ;
  } else
  if (m_entrytype == ET_SHORTCUT) {
    ASSERT(pcibase != NULL);
    tmp = _T("[~") +
          pcibase->GetGroup() + _T(":") +
          pcibase->GetTitle() + _T(":") +
          pcibase->GetUser() + _T("~]") ;
  } else
    tmp = GetPassword();

  brc = PWSUtil::WriteXMLField(oss, "password", tmp, utf8conv);
  if (!brc) bXMLErrorsFound = true;

  tmp = GetURL();
  if (bsExport.test(CItemData::URL) && !tmp.empty()) {
    brc = PWSUtil::WriteXMLField(oss, "url", tmp, utf8conv);
    if (!brc) bXMLErrorsFound = true;
  }

  tmp = GetAutoType();
  if (bsExport.test(CItemData::AUTOTYPE) && !tmp.empty()) {
    brc = PWSUtil::WriteXMLField(oss, "autotype", tmp, utf8conv);
    if (!brc) bXMLErrorsFound = true;
  }

  tmp = GetNotes();
  if (bsExport.test(CItemData::NOTES) && !tmp.empty()) {
    CleanNotes(tmp, delimiter);
    brc = PWSUtil::WriteXMLField(oss, "notes", tmp, utf8conv);
    if (!brc) bXMLErrorsFound = true;
  }

  oss << "\t\t<uuid><![CDATA[" << GetUUID() << "]]></uuid>" << endl;

  time_t t;
  int32 i32;
  int16 i16;

  GetCTime(t);
  if (bsExport.test(CItemData::CTIME) && t)
    oss << PWSUtil::GetXMLTime(2, "ctimex", t, utf8conv);

  GetATime(t);
  if (bsExport.test(CItemData::ATIME) && t)
    oss << PWSUtil::GetXMLTime(2, "atimex", t, utf8conv);

  GetXTime(t);
  if (bsExport.test(CItemData::XTIME) && t)
    oss << PWSUtil::GetXMLTime(2, "xtimex", t, utf8conv);

  GetXTimeInt(i32);
  if (bsExport.test(CItemData::XTIME_INT) && i32 > 0 && i32 <= 3650)
    oss << "\t\t<xtime_interval>" << dec << i32 << "</xtime_interval>" << endl;

  GetPMTime(t);
  if (bsExport.test(CItemData::PMTIME) && t)
    oss << PWSUtil::GetXMLTime(2, "pmtimex", t, utf8conv);

  GetRMTime(t);
  if (bsExport.test(CItemData::RMTIME) && t)
    oss << PWSUtil::GetXMLTime(2, "rmtimex", t, utf8conv);

  StringX sxPolicyName = GetPolicyName();
  if (sxPolicyName.empty()) {
    PWPolicy pwp;
    GetPWPolicy(pwp);
    if (bsExport.test(CItemData::POLICY) && pwp.flags != 0) {
      oss << "\t\t<PasswordPolicy>" << endl;
      oss << dec;
      oss << "\t\t\t<PWLength>" << pwp.length << "</PWLength>" << endl;
      if (pwp.flags & PWPolicy::UseLowercase)
        oss << "\t\t\t<PWUseLowercase>1</PWUseLowercase>" << endl;
      if (pwp.flags & PWPolicy::UseUppercase)
        oss << "\t\t\t<PWUseUppercase>1</PWUseUppercase>" << endl;
      if (pwp.flags & PWPolicy::UseDigits)
        oss << "\t\t\t<PWUseDigits>1</PWUseDigits>" << endl;
      if (pwp.flags & PWPolicy::UseSymbols)
        oss << "\t\t\t<PWUseSymbols>1</PWUseSymbols>" << endl;
      if (pwp.flags & PWPolicy::UseHexDigits)
        oss << "\t\t\t<PWUseHexDigits>1</PWUseHexDigits>" << endl;
      if (pwp.flags & PWPolicy::UseEasyVision)
        oss << "\t\t\t<PWUseEasyVision>1</PWUseEasyVision>" << endl;
      if (pwp.flags & PWPolicy::MakePronounceable)
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
  } else {
    if (bsExport.test(CItemData::POLICY) || bsExport.test(CItemData::POLICYNAME)) {
      brc = PWSUtil::WriteXMLField(oss, "PasswordPolicyName", sxPolicyName,
                        utf8conv, "\t\t");
      if (!brc) bXMLErrorsFound = true;
    }
  }

  if (bsExport.test(CItemData::PWHIST)) {
    size_t pwh_max, num_err;
    PWHistList pwhistlist;
    bool pwh_status = CreatePWHistoryList(GetPWHistory(), pwh_max, num_err,
                                          pwhistlist, PWSUtil::TMC_XML);
    oss << dec;
    if (pwh_status || pwh_max > 0 || !pwhistlist.empty()) {
      oss << "\t\t<pwhistory>" << endl;
      oss << "\t\t\t<status>" << pwh_status << "</status>" << endl;
      oss << "\t\t\t<max>" << pwh_max << "</max>" << endl;
      oss << "\t\t\t<num>" << pwhistlist.size() << "</num>" << endl;
      if (!pwhistlist.empty()) {
        oss << "\t\t\t<history_entries>" << endl;
        int num = 1;
        PWHistList::iterator hiter;
        for (hiter = pwhistlist.begin(); hiter != pwhistlist.end();
             hiter++) {
          const unsigned char *utf8 = NULL;
          size_t utf8Len = 0;

          oss << "\t\t\t\t<history_entry num=\"" << num << "\">" << endl;
          const PWHistEntry &pwshe = *hiter;
          oss << "\t\t\t\t\t<changedx>";
          if (utf8conv.ToUTF8(pwshe.changedate.substr(0, 10), utf8, utf8Len))
            oss.write(reinterpret_cast<const char *>(utf8), utf8Len);
          else
            oss << "1970-01-01";

          oss << "T";
          if (utf8conv.ToUTF8(pwshe.changedate.substr(pwshe.changedate.length() - 8),
                              utf8, utf8Len))
            oss.write(reinterpret_cast<const char *>(utf8), utf8Len);
          else
            oss << "00:00";

          oss << "</changedx>" << endl;
          brc = PWSUtil::WriteXMLField(oss, "oldpassword", pwshe.password,
                        utf8conv, "\t\t\t\t\t");
          if (!brc) bXMLErrorsFound = true;

          oss << "\t\t\t\t</history_entry>" << endl;

          num++;
        } // for
        oss << "\t\t\t</history_entries>" << endl;
      } // if !empty
      oss << "\t\t</pwhistory>" << endl;
    }
  }

  tmp = GetRunCommand();
  if (bsExport.test(CItemData::RUNCMD) && !tmp.empty()) {
    brc = PWSUtil::WriteXMLField(oss, "runcommand", tmp, utf8conv);
    if (!brc) bXMLErrorsFound = true;
  }

  GetDCA(i16);
  if (bsExport.test(CItemData::DCA) &&
      i16 >= PWSprefs::minDCA && i16 <= PWSprefs::maxDCA)
    oss << "\t\t<dca>" << i16 << "</dca>" << endl;

  GetShiftDCA(i16);
  if (bsExport.test(CItemData::SHIFTDCA) &&
      i16 >= PWSprefs::minDCA && i16 <= PWSprefs::maxDCA)
    oss << "\t\t<shiftdca>" << i16 << "</shiftdca>" << endl;


  tmp = GetEmail();
  if (bsExport.test(CItemData::EMAIL) && !tmp.empty()) {
    brc = PWSUtil::WriteXMLField(oss, "email", tmp, utf8conv);
    if (!brc) bXMLErrorsFound = true;
  }

  GetProtected(uc);
  if (bsExport.test(CItemData::PROTECTED) && uc != 0)
    oss << "\t\t<protected>1</protected>" << endl;

  tmp = GetSymbols();
  if (bsExport.test(CItemData::SYMBOLS) && !tmp.empty()) {
    brc = PWSUtil::WriteXMLField(oss, "symbols", tmp, utf8conv);
    if (!brc) bXMLErrorsFound = true;
  }

  tmp = GetKBShortcut();
  if (bsExport.test(CItemData::KBSHORTCUT) && !tmp.empty()) {
    brc = PWSUtil::WriteXMLField(oss, "kbshortcut", tmp, utf8conv);
    if (!brc) bXMLErrorsFound = true;
  }

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

void CItemData::SetField(FieldType ft, const StringX &value)
{
  ASSERT(ft != END);
  if (!value.empty()) {
    BlowFish *bf = MakeBlowFish(false);
    m_fields[ft].Set(value, bf, static_cast<unsigned char>(ft));
    delete bf;
  } else
    m_fields.erase(static_cast<FieldType>(ft));
}

void CItemData::SetField(FieldType ft, const unsigned char *value, size_t length)
{
  ASSERT(ft != END);
  if (length != 0) {
    BlowFish *bf = MakeBlowFish(false);
    m_fields[ft].Set(value, length, bf, static_cast<unsigned char>(ft));
    delete bf;
  } else
    m_fields.erase(static_cast<FieldType>(ft));
}

void CItemData::CreateUUID()
{
  CUUID uuid;
  SetUUID(*uuid.GetARep());
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
  CItemField nameField(NAME);
  BlowFish *bf = MakeBlowFish();
  nameField.Set(name, bf);
  m_fields[NAME] = nameField;
  CItemField titleField(TITLE);
  titleField.Set(title, bf);
  m_fields[TITLE] = titleField;
  CItemField userField(USER);
  userField.Set(user, bf);
  m_fields[USER] = userField;
  delete bf;
}

void CItemData::SetTitle(const StringX &title, TCHAR delimiter)
{
  if (delimiter == 0)
    SetField(TITLE, title);
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

    SetField(TITLE, new_title);
  }
}

void CItemData::SetUser(const StringX &user)
{
  SetField(USER, user);
}

void CItemData::UpdatePassword(const StringX &password)
{
  // use when password changed - manages history, modification times
  UpdatePasswordHistory();
  SetPassword(password);

  time_t t;
  time(&t);
  SetPMTime(t);

  int32 xint;
  GetXTimeInt(xint);
  if (xint != 0) {
    // convert days to seconds for time_t
    t += (xint * 86400);
    SetXTime(t);
  } else {
    SetXTime(time_t(0));
  }
}

void CItemData::UpdatePasswordHistory()
{
  PWHistList pwhistlist;
  size_t pwh_max;
  bool saving;
  const StringX pwh_str = GetPWHistory();
  if (pwh_str.empty()) {
    // If GetPWHistory() is empty, use preference values!
    const PWSprefs *prefs = PWSprefs::GetInstance();
    saving = prefs->GetPref(PWSprefs::SavePasswordHistory);
    pwh_max = prefs->GetPref(PWSprefs::NumPWHistoryDefault);
  } else {
    size_t num_err;
    saving = CreatePWHistoryList(pwh_str, pwh_max, num_err,
                                 pwhistlist, PWSUtil::TMC_EXPORT_IMPORT);
  }
  if (!saving)
    return;

  size_t num = pwhistlist.size();

  time_t t;
  GetPMTime(t); // get mod time of last password

  if (!t) // if never set - try creation date
    GetCTime(t);

  PWHistEntry pwh_ent;
  pwh_ent.password = GetPassword();
  pwh_ent.changetttdate = t;
  pwh_ent.changedate =
    PWSUtil::ConvertToDateTimeString(t, PWSUtil::TMC_EXPORT_IMPORT);

  if (pwh_ent.changedate.empty()) {
    StringX unk;
    LoadAString(unk, IDSC_UNKNOWN);
    pwh_ent.changedate = unk;
  }

  // Now add the latest
  pwhistlist.push_back(pwh_ent);

  // Increment count
  num++;

  // Too many? remove the excess
  if (num > pwh_max) {
    PWHistList hl(pwhistlist.begin() + (num - pwh_max),
                  pwhistlist.end());
    ASSERT(hl.size() == pwh_max);
    pwhistlist = hl;
    num = pwh_max;
  }

  // Now create string version!
  StringX new_PWHistory, buffer;

  Format(new_PWHistory, L"1%02x%02x", pwh_max, num);

  PWHistList::iterator iter;
  for (iter = pwhistlist.begin(); iter != pwhistlist.end(); iter++) {
    Format(buffer, L"%08x%04x%ls",
           static_cast<long>(iter->changetttdate), iter->password.length(),
           iter->password.c_str());
    new_PWHistory += buffer;
    buffer = _T("");
  }
  SetPWHistory(new_PWHistory);
}


void CItemData::SetPassword(const StringX &password)
{
  SetField(PASSWORD, password);
}

void CItemData::SetNotes(const StringX &notes, TCHAR delimiter)
{
  if (delimiter == 0)
    SetField(NOTES, notes);
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

    SetField(NOTES, multiline_notes);
  }
}

void CItemData::SetGroup(const StringX &group)
{
  SetField(GROUP, group);
}

void CItemData::SetUUID(const uuid_array_t &uuid)
{
  SetField(UUID, static_cast<const unsigned char *>(uuid), sizeof(uuid));
}

void CItemData::SetUUID(const CUUID &uuid, FieldType ft)
{
  SetField(ft, static_cast<const unsigned char *>(*uuid.GetARep()), sizeof(uuid_array_t));
}

void CItemData::SetURL(const StringX &url)
{
  SetField(URL, url);
}

void CItemData::SetAutoType(const StringX &autotype)
{
  SetField(AUTOTYPE, autotype);
}

void CItemData::SetTime(int whichtime)
{
  time_t t;
  time(&t);
  SetTime(whichtime, t);
}

void CItemData::SetTime(int whichtime, time_t t)
{
  unsigned char buf[sizeof(time_t)];
  putInt(buf, t);
  SetField(static_cast<FieldType>(whichtime), buf, sizeof(time_t));
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
          (t != time_t(-1))  // checkerror despite all our verification!
          ) {
        SetTime(whichtime, t);
        return true;
      }
  return false;
}

void CItemData::SetXTimeInt(int32 xint)
{
  unsigned char buf[sizeof(int32)];
  putInt(buf, xint);
  SetField(XTIME_INT, buf, sizeof(int32));
}

bool CItemData::SetXTimeInt(const stringT &xint_str)
{
  int32 xint(0);

  if (xint_str.empty()) {
    SetXTimeInt(xint);
    return true;
  }

  if (xint_str.find_first_not_of(_T("0123456789")) == stringT::npos) {
    istringstreamT is(xint_str);
    is >> xint;
    if (is.fail())
      return false;
    if (xint >= 0 && xint <= 3650) {
      SetXTimeInt(xint);
      return true;
    }
  }
  return false;
}

void CItemData::SetUnknownField(unsigned char type,
                                size_t length,
                                const unsigned char *ufield)
{
  /**
     TODO - check that this unknown field from the XML Import file is now
     known and it should be added as that instead!
  **/

  CItemField unkrfe(type);
  BlowFish *bf = MakeBlowFish(false);
  unkrfe.Set(ufield, length, bf);
  delete bf;
  m_URFL.push_back(unkrfe);
}

void CItemData::SetPWHistory(const StringX &PWHistory)
{
  StringX pwh = PWHistory;
  if (pwh == _T("0") || pwh == _T("00000"))
    pwh = _T("");
  SetField(PWHIST, pwh);
}

void CItemData::SetPWPolicy(const PWPolicy &pwp)
{
  const StringX cs_pwp(pwp);

  SetField(POLICY, cs_pwp);
  if (!pwp.symbols.empty())
    SetSymbols(pwp.symbols);
}

bool CItemData::SetPWPolicy(const stringT &cs_pwp)
{
  // Basic sanity checks
  if (cs_pwp.empty()) {
    SetField(POLICY, cs_pwp.c_str());
    return true;
  }

  const StringX cs_pwpolicy(cs_pwp.c_str());
  PWPolicy pwp(cs_pwpolicy);
  PWPolicy emptyPol;
  // a non-empty string creates an empty policy iff it's ill-formed
  if (pwp == emptyPol)
    return false;

  SetField(POLICY, cs_pwpolicy);
  return true;
}

void CItemData::SetRunCommand(const StringX &cs_RunCommand)
{
  SetField(RUNCMD, cs_RunCommand);
}

void CItemData::SetEmail(const StringX &sx_email)
{
  SetField(EMAIL, sx_email);
}

void CItemData::SetSymbols(const StringX &sx_symbols)
{
  SetField(SYMBOLS, sx_symbols);
}

void CItemData::SetPolicyName(const StringX &sx_PolicyName)
{
  SetField(POLICYNAME, sx_PolicyName);
}

void CItemData::SetDCA(int16 iDCA, const bool bShift)
{
  unsigned char buf[sizeof(int16)];
  putInt(buf, iDCA);
  SetField(bShift ? SHIFTDCA : DCA, buf, sizeof(int16));
}

bool CItemData::SetDCA(const stringT &cs_DCA, const bool bShift)
{
  int16 iDCA(-1);

  if (cs_DCA.empty()) {
    SetDCA(iDCA, bShift);
    return true;
  }

  if (cs_DCA.find_first_not_of(_T("0123456789")) == stringT::npos) {
    istringstreamT is(cs_DCA);
    is >> iDCA;
    if (is.fail())
      return false;
    if (iDCA == -1 || (iDCA >= PWSprefs::minDCA && iDCA <= PWSprefs::maxDCA)) {
      SetDCA(iDCA, bShift);
      return true;
    }
  }
  return false;
}

void CItemData::SetProtected(bool bOnOff)
{
  if (bOnOff) {
    const unsigned char ucProtected = 1;
    SetField(PROTECTED, &ucProtected, sizeof(char));
  } else { // remove field
    m_fields.erase(PROTECTED);
  }
}

void CItemData::SetKBShortcut(int32 iKBShortcut)
{
  unsigned char buf[sizeof(int32)];
  putInt(buf, iKBShortcut);
  SetField(KBSHORTCUT, buf, sizeof(int32));
}

void CItemData::SetKBShortcut(const StringX &sx_KBShortcut)
{
  int32 iKBShortcut(0);
  WORD wVirtualKeyCode(0);
  WORD wPWSModifiers(0);
  size_t len = sx_KBShortcut.length();
  if (!sx_KBShortcut.empty()) {
    for (size_t i = 0; i < len; i++) {
      if (sx_KBShortcut.substr(i, 1) == _T(":")) {
        // 4 hex digits should follow the colon
        ASSERT(i + 5 == len);
        istringstreamT iss(sx_KBShortcut.substr(i + 1, 4).c_str());
        iss >> hex >> wVirtualKeyCode;
        break;
      }
      if (sx_KBShortcut.substr(i, 1) == _T("A"))
        wPWSModifiers |= PWS_HOTKEYF_ALT;
      if (sx_KBShortcut.substr(i, 1) == _T("C"))
        wPWSModifiers |= PWS_HOTKEYF_CONTROL;
      if (sx_KBShortcut.substr(i, 1) == _T("S"))
        wPWSModifiers |= PWS_HOTKEYF_SHIFT;
      if (sx_KBShortcut.substr(i, 1) == _T("E"))
        wPWSModifiers |= PWS_HOTKEYF_EXT;
      if (sx_KBShortcut.substr(i, 1) == _T("M"))
        wPWSModifiers |= PWS_HOTKEYF_META;
      if (sx_KBShortcut.substr(i, 1) == _T("W"))
        wPWSModifiers |= PWS_HOTKEYF_WIN;
      if (sx_KBShortcut.substr(i, 1) == _T("D"))
        wPWSModifiers |= PWS_HOTKEYF_CMD;
    }
  }

  if (wPWSModifiers != 0 && wVirtualKeyCode != 0) {
    iKBShortcut = (wPWSModifiers << 16) + wVirtualKeyCode;
  }

  SetKBShortcut(iKBShortcut);
}

void CItemData::SetFieldValue(FieldType ft, const StringX &value)
{
  switch (ft) {
    case GROUP:      /* 02 */
    case TITLE:      /* 03 */
    case USER:       /* 04 */
    case NOTES:      /* 05 */
    case PASSWORD:   /* 06 */
    case URL:        /* 0d */
    case AUTOTYPE:   /* 0e */
    case PWHIST:     /* 0f */
    case EMAIL:      /* 14 */
    case RUNCMD:     /* 12 */
    case SYMBOLS:    /* 16 */
    case POLICYNAME: /* 18 */
      SetField(ft, value);
      break;
    case CTIME:      /* 07 */
    case PMTIME:     /* 08 */
    case ATIME:      /* 09 */
    case XTIME:      /* 0a */
    case RMTIME:     /* 0c */
      SetTime(ft, value.c_str());
      break;
    case POLICY:     /* 10 */
      SetPWPolicy(value.c_str());
      break;
    case XTIME_INT:  /* 11 */
      SetXTimeInt(value.c_str());
      break;
    case DCA:        /* 13 */
      SetDCA(value.c_str());
      break;
    case PROTECTED:  /* 15 */
      SetProtected(value.compare(_T("1")) == 0 || value.compare(_T("Yes")) == 0);
      break;
    case SHIFTDCA:   /* 17 */
      SetShiftDCA(value.c_str());
      break;
    case KBSHORTCUT: /* 19 */
      SetKBShortcut(value);
      break;
    case GROUPTITLE: /* 00 */
    case UUID:       /* 01 */
    case RESERVED:   /* 0b */
    default:
      ASSERT(0);     /* Not supported */
  }
}

BlowFish *CItemData::MakeBlowFish(bool noData) const
{
  ASSERT(IsSessionKeySet);
  // Creating a BlowFish object's relatively expensive. No reason
  // to bother if we don't have any data to process.
  if (noData)
    return NULL;
  else
    return BlowFish::MakeBlowFish(SessionKey, sizeof(SessionKey),
                                  m_salt, SaltLength);
}

bool CItemData::ValidatePWHistory()
{
  // Return true if valid
  if (!IsPasswordHistorySet())
    return true; // empty is a kind of valid

  const StringX pwh = GetPWHistory();
  if (pwh.length() < 5) { // not empty, but too short.
    SetPWHistory(_T(""));
    return false;
  }

  size_t pwh_max, num_err;
  PWHistList pwhistlist;
  bool pwh_status = CreatePWHistoryList(pwh, pwh_max, num_err,
                                        pwhistlist, PWSUtil::TMC_EXPORT_IMPORT);
  if (num_err == 0)
    return true;

  size_t listnum = pwhistlist.size();

  if (pwh_max == 0 && listnum == 0) {
    SetPWHistory(_T(""));
    return false;
  }

  if (listnum > pwh_max)
    pwh_max = listnum;

  // Rebuild PWHistory from the data we have
  StringX sxBuffer;
  StringX sxNewHistory = MakePWHistoryHeader(pwh_status, pwh_max, listnum);

  PWHistList::const_iterator citer;
  for (citer = pwhistlist.begin(); citer != pwhistlist.end(); citer++) {
    Format(sxBuffer, L"%08x%04x%ls",
             static_cast<long>(citer->changetttdate), citer->password.length(),
             citer->password.c_str());
      sxNewHistory += sxBuffer;
      sxBuffer = _T("");
  }

  if (pwh != sxNewHistory)
    SetPWHistory(sxNewHistory);

  return false;
}

bool CItemData::Matches(const stringT &stValue, int iObject,
                        int iFunction) const
{
  ASSERT(iFunction != 0); // must be positive or negative!

  StringX sx_Object;
  FieldType ft = static_cast<FieldType>(iObject);
  switch(ft) {
    case GROUP:
    case TITLE:
    case USER:
    case URL:
    case NOTES:
    case PASSWORD:
    case RUNCMD:
    case EMAIL:
    case SYMBOLS:
    case POLICYNAME:
    case AUTOTYPE:
      sx_Object = GetField(ft);
      break;
    case GROUPTITLE:
      sx_Object = GetGroup() + TCHAR('.') + GetTitle();
      break;
    default:
      ASSERT(0);
  }

  const bool bValue = !sx_Object.empty();
  if (iFunction == PWSMatch::MR_PRESENT || iFunction == PWSMatch::MR_NOTPRESENT) {
    return PWSMatch::Match(bValue, iFunction);
  }

  return PWSMatch::Match(stValue.c_str(), sx_Object, iFunction);
}

bool CItemData::Matches(int num1, int num2, int iObject,
                        int iFunction) const
{
  //  Check integer values are selected
  int iValue;

  switch (iObject) {
    case XTIME_INT:
      GetXTimeInt(iValue);
      break;
    case ENTRYSIZE:
      GetSize(reinterpret_cast<size_t &>(iValue));
      break;
    case PASSWORDLEN:
      iValue = GetPasswordLength();
      break;
    case KBSHORTCUT:
      GetKBShortcut(iValue);
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

bool CItemData::Matches(int16 dca, int iFunction, const bool bShift) const
{
  int16 iDCA;
  GetDCA(iDCA, bShift);
  if (iDCA < 0)
    iDCA = static_cast<int16>(PWSprefs::GetInstance()->GetPref(bShift ?
               PWSprefs::ShiftDoubleClickAction : PWSprefs::DoubleClickAction));

  switch (iFunction) {
    case PWSMatch::MR_IS:
      return iDCA == dca;
    case PWSMatch::MR_ISNOT:
      return iDCA != dca;
    default:
      ASSERT(0);
  }
  return false;
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

  const bool bValue = (tValue != time_t(0));
  if (iFunction == PWSMatch::MR_PRESENT || iFunction == PWSMatch::MR_NOTPRESENT) {
    return PWSMatch::Match(bValue, iFunction);
  }

  if (!bValue)  // date empty - always return false for other comparisons
    return false;
  else {
    time_t testtime = time_t(0);
    if (tValue) {
      struct tm st;
      errno_t err;
      err = localtime_s(&st, &tValue);
      ASSERT(err == 0);
      if (!err) {
        st.tm_hour = 0;
        st.tm_min = 0;
        st.tm_sec = 0;
        testtime = mktime(&st);
      }
    }
    return PWSMatch::Match(time1, time2, testtime, iFunction);
  }
}

bool CItemData::Matches(EntryType etype, int iFunction) const
{
  switch (iFunction) {
    case PWSMatch::MR_IS:
      return GetEntryType() == etype;
    case PWSMatch::MR_ISNOT:
      return GetEntryType() != etype;
    default:
      ASSERT(0);
  }
  return false;
}

bool CItemData::Matches(EntryStatus estatus, int iFunction) const
{
  switch (iFunction) {
    case PWSMatch::MR_IS:
      return GetStatus() == estatus;
    case PWSMatch::MR_ISNOT:
      return GetStatus() != estatus;
    default:
      ASSERT(0);
  }
  return false;
}

bool CItemData::IsExpired() const
{
  time_t now, XTime;
  time(&now);

  GetXTime(XTime);
  return (XTime && (XTime < now));
}

bool CItemData::WillExpire(const int numdays) const
{
  time_t now, exptime=time_t(-1), XTime;
  time(&now);

  GetXTime(XTime);
  // Check if there is an expiry date?
  if (!XTime)
    return false;

  // Ignore if already expired
  if (XTime <= now)
    return false;

  struct tm st;
  errno_t err;
  err = localtime_s(&st, &now);  // secure version
  ASSERT(err == 0);
  if (!err){
    st.tm_mday += numdays;
    exptime = mktime(&st);
  }
  if (exptime == time_t(-1))
    exptime = now;

  // Will it expire in numdays?
  return (XTime < exptime);
}

static bool pull_string(StringX &str, const unsigned char *data, size_t len)
{
  /**
   * cp_acp is used to force reading data as non-utf8 encoded
   * This is for databases that were incorrectly written, e.g., 3.05.02
   * PWS_CP_ACP is either set externally or via the --CP_ACP argv
   *
   * We use a static variable purely for efficiency, as this won't change
   * over the course of the program.
   */

  static int cp_acp = -1;
  if (cp_acp == -1) {
    cp_acp = pws_os::getenv("PWS_CP_ACP", false).empty() ? 0 : 1;
  }
  CUTF8Conv utf8conv(cp_acp != 0);
  vector<unsigned char> v(data, (data + len));
  v.push_back(0); // null terminate for FromUTF8.
  bool utf8status = utf8conv.FromUTF8(&v[0], len, str);
  if (!utf8status) {
    pws_os::Trace(_T("ItemData.cpp: pull_string(): FromUTF8 failed!\n"));
  }
  trashMemory(&v[0], len);
  return utf8status;
}

static bool pull_time(time_t &t, const unsigned char *data, size_t len)
{
  // len can be either 4, 5 or 8...
  // len == 5 is new for V4
  ASSERT(len == 4 || len == 5 || len == 8);
  if (!(len == 4 || len == 5 || len == 8))
    return false;
  // sizeof(time_t) is either 4 or 8
  if (len == sizeof(time_t)) { // 4 == 4 or 8 == 8
    t = getInt<time_t>(data);
  } else if (len < sizeof(time_t)) { // 4 < 8 or 5 < 8
    unsigned char buf[sizeof(time_t)] = {0};
    memcpy(buf, data, len);
    t = getInt<time_t>(buf);
  } else { // convert from 40 or 64 bit time to 32 bit
    // XXX Change to use localtime, not GMT
    unsigned char buf[sizeof(__time64_t)] = {0};
    memcpy(buf, data, len); // not needed if len == 8, but no harm
    struct tm ts;
    const __time64_t t64 = getInt<__time64_t>(buf);
    if (_gmtime64_s(&ts, &t64) != 0) {
      ASSERT(0); return false;
    }
    t = _mkgmtime32(&ts);
    if (t == time_t(-1)) { // time is past 2038!
      t = 0; return false;
    }
  }
  return true;
}

static bool pull_int32(int32 &i, const unsigned char *data, size_t len)
{
  if (len == sizeof(int32)) {
    i = getInt32(data);
  } else {
    ASSERT(0);
    return false;
  }
  return true;
}

static bool pull_int16(int16 &i16, const unsigned char *data, size_t len)
{
  if (len == sizeof(int16)) {
    i16 = getInt16(data);
  } else {
    ASSERT(0);
    return false;
  }
  return true;
}

static bool pull_char(unsigned char &uc, const unsigned char *data, size_t len)
{
  if (len == sizeof(char)) {
    uc = *data;
  } else {
    ASSERT(0);
    return false;
  }
  return true;
}

bool CItemData::DeSerializePlainText(const std::vector<char> &v)
{
  vector<char>::const_iterator iter = v.begin();
  int emergencyExit = 255;

  while (iter != v.end()) {
    int type = (*iter++ & 0xff); // required since enum is an int
    if (static_cast<uint32>(distance(v.end(), iter)) < sizeof(uint32)) {
      ASSERT(0); // type must ALWAYS be followed by length
      return false;
    }

    if (type == END)
      return true; // happy end

    uint32 len = *(reinterpret_cast<const uint32 *>(&(*iter)));
    ASSERT(len < v.size()); // sanity check
    iter += sizeof(uint32);

    if (--emergencyExit == 0) {
      ASSERT(0);
      return false;
    }
    if (!SetField(type, reinterpret_cast<const unsigned char *>(&(*iter)), len))
      return false;
    iter += len;
  }
  return false; // END tag not found!
}

bool CItemData::SetField(int type, const unsigned char *data, size_t len)
{
  StringX str;
  time_t t;
  int32 i32;
  int16 i16;
  unsigned char uc;

  FieldType ft = static_cast<FieldType>(type);
  switch (ft) {
    case NAME:
      ASSERT(0); // not serialized, or in v3 format
      return false;
    case UUID:
    {
      uuid_array_t uuid_array;
      ASSERT(len == sizeof(uuid_array_t));
      for (size_t i = 0; i < sizeof(uuid_array_t); i++)
        uuid_array[i] = data[i];
      SetUUID(uuid_array);
      break;
    }
    case GROUP:
    case TITLE:
    case USER:
    case NOTES:
    case PASSWORD:
    case POLICY:
    case URL:
    case AUTOTYPE:
    case PWHIST:
    case RUNCMD:
    case EMAIL:
    case SYMBOLS:
    case POLICYNAME:
      if (!pull_string(str, data, len)) return false;
      SetField(ft, str);
      break;
    case CTIME:
    case PMTIME:
    case ATIME:
    case XTIME:
    case RMTIME:
      if (!pull_time(t, data, len)) return false;
      SetTime(ft, t);
      break;
    case XTIME_INT:
      if (!pull_int32(i32, data, len)) return false;
      SetXTimeInt(i32);
      break;
    case DCA:
      if (!pull_int16(i16, data, len)) return false;
      SetDCA(i16);
      break;
    case SHIFTDCA:
      if (!pull_int16(i16, data, len)) return false;
      SetShiftDCA(i16);
      break;
    case PROTECTED:
      if (!pull_char(uc, data, len)) return false;
      SetProtected(uc != 0);
      break;
    case KBSHORTCUT:
      if (!pull_int32(i32, data, sizeof(int32))) return false;
      SetKBShortcut(i32);
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

static void push_length(vector<char> &v, uint32 s)
{
  v.insert(v.end(),
    reinterpret_cast<char *>(&s), reinterpret_cast<char *>(&s) + sizeof(uint32));
}

static void push_string(vector<char> &v, char type,
                        const StringX &str)
{
  if (!str.empty()) {
    CUTF8Conv utf8conv;
    bool status;
    const unsigned char *utf8;
    size_t utf8Len;
    status = utf8conv.ToUTF8(str, utf8, utf8Len);
    if (status) {
      v.push_back(type);
      push_length(v, static_cast<uint32>(utf8Len));
      v.insert(v.end(), reinterpret_cast<const char *>(utf8),
               reinterpret_cast<const char *>(utf8) + utf8Len);
    } else
      pws_os::Trace(_T("ItemData.cpp: push_string(%ls): ToUTF8 failed!\n"), str.c_str());
  }
}

static void push_time(vector<char> &v, char type, time_t t)
{
  if (t != 0) {
    v.push_back(type);
    push_length(v, sizeof(t));
    v.insert(v.end(),
             reinterpret_cast<char *>(&t), reinterpret_cast<char *>(&t) + sizeof(t));
  }
}

static void push_int32(vector<char> &v, char type, int32 i)
{
  if (i != 0) {
    v.push_back(type);
    push_length(v, sizeof(int32));
    v.insert(v.end(),
             reinterpret_cast<char *>(&i), reinterpret_cast<char *>(&i) + sizeof(int32));
  }
}

static void push_int16(vector<char> &v, char type, int16 i)
{
  if (i != 0) {
    v.push_back(type);
    push_length(v, sizeof(int16));
    v.insert(v.end(),
      reinterpret_cast<char *>(&i), reinterpret_cast<char *>(&i) + sizeof(int16));
  }
}

static void push_uchar(vector<char> &v, char type, unsigned char uc)
{
  if (uc != 0) {
    v.push_back(type);
    push_length(v, sizeof(char));
    v.insert(v.end(), reinterpret_cast<char *>(&uc), reinterpret_cast<char *>(&uc) + sizeof(char));
  }
}

void CItemData::SerializePlainText(vector<char> &v,
                                   const CItemData *pcibase)  const
{
  StringX tmp;
  uuid_array_t uuid_array;
  time_t t = 0;
  int32 i32 = 0;
  int16 i16 = 0;
  unsigned char uc = 0;

  v.clear();
  GetUUID(uuid_array);
  v.push_back(UUID);
  push_length(v, sizeof(uuid_array_t));
  v.insert(v.end(), uuid_array, (uuid_array + sizeof(uuid_array_t)));
  push_string(v, GROUP, GetGroup());
  push_string(v, TITLE, GetTitle());
  push_string(v, USER, GetUser());

  if (m_entrytype == ET_ALIAS) {
    // I am an alias entry
    ASSERT(pcibase != NULL);
    tmp = _T("[[") + pcibase->GetGroup() + _T(":") + pcibase->GetTitle() + _T(":") + pcibase->GetUser() + _T("]]");
  } else
  if (m_entrytype == ET_SHORTCUT) {
    // I am a shortcut entry
    ASSERT(pcibase != NULL);
    tmp = _T("[~") + pcibase->GetGroup() + _T(":") + pcibase->GetTitle() + _T(":") + pcibase->GetUser() + _T("~]");
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

  GetXTimeInt(i32); push_int32(v, XTIME_INT, i32);

  push_string(v, POLICY, GetPWPolicy());
  push_string(v, PWHIST, GetPWHistory());

  push_string(v, RUNCMD, GetRunCommand());
  GetDCA(i16);   push_int16(v, DCA, i16);
  GetShiftDCA(i16);   push_int16(v, SHIFTDCA, i16);
  push_string(v, EMAIL, GetEmail());
  GetProtected(uc); push_uchar(v, PROTECTED, uc);
  push_string(v, SYMBOLS, GetSymbols());
  push_string(v, POLICYNAME, GetPolicyName());
  GetKBShortcut(i32); push_int32(v, KBSHORTCUT, i32);

  UnknownFieldsConstIter vi_IterURFE;
  for (vi_IterURFE = m_URFL.begin();
       vi_IterURFE != m_URFL.end();
       vi_IterURFE++) {
    unsigned char type;
    size_t length = 0;
    unsigned char *pdata = NULL;
    GetUnknownField(type, length, pdata, *vi_IterURFE);
    if (length != 0) {
      v.push_back(static_cast<char>(type));
      push_length(v, static_cast<uint32>(length));
      v.insert(v.end(), reinterpret_cast<char *>(pdata), reinterpret_cast<char *>(pdata) + length);
      trashMemory(pdata, length);
    }
    delete[] pdata;
  }

  int end = END; // just to keep the compiler happy...
  v.push_back(static_cast<const char>(end));
  push_length(v, 0);
}

  // Convenience: Get the name associated with FieldType
stringT CItemData::FieldName(FieldType ft)
{
  stringT retval(_T(""));
  switch (ft) {
  case GROUPTITLE:   LoadAString(retval, IDSC_FLDNMGROUPTITLE); break;
  case UUID:         LoadAString(retval, IDSC_FLDNMUUID); break;
  case GROUP:        LoadAString(retval, IDSC_FLDNMGROUP); break;
  case TITLE:        LoadAString(retval, IDSC_FLDNMTITLE); break;
  case USER:         LoadAString(retval, IDSC_FLDNMUSERNAME); break;
  case NOTES:        LoadAString(retval, IDSC_FLDNMNOTES); break;
  case PASSWORD:     LoadAString(retval, IDSC_FLDNMPASSWORD); break;
  case CTIME:        LoadAString(retval, IDSC_FLDNMCTIME); break;
  case PMTIME:       LoadAString(retval, IDSC_FLDNMPMTIME); break;
  case ATIME:        LoadAString(retval, IDSC_FLDNMATIME); break;
  case XTIME:        LoadAString(retval, IDSC_FLDNMXTIME); break;
  case RMTIME:       LoadAString(retval, IDSC_FLDNMRMTIME); break;
  case URL:          LoadAString(retval, IDSC_FLDNMURL); break;
  case AUTOTYPE:     LoadAString(retval, IDSC_FLDNMAUTOTYPE); break;
  case PWHIST:       LoadAString(retval, IDSC_FLDNMPWHISTORY); break;
  case POLICY:       LoadAString(retval, IDSC_FLDNMPWPOLICY); break;
  case XTIME_INT:    LoadAString(retval, IDSC_FLDNMXTIMEINT); break;
  case RUNCMD:       LoadAString(retval, IDSC_FLDNMRUNCOMMAND); break;
  case DCA:          LoadAString(retval, IDSC_FLDNMDCA); break;
  case SHIFTDCA:     LoadAString(retval, IDSC_FLDNMSHIFTDCA); break;
  case EMAIL:        LoadAString(retval, IDSC_FLDNMEMAIL); break;
  case PROTECTED:    LoadAString(retval, IDSC_FLDNMPROTECTED); break;
  case SYMBOLS:      LoadAString(retval, IDSC_FLDNMSYMBOLS); break;
  case POLICYNAME:   LoadAString(retval, IDSC_FLDNMPWPOLICYNAME); break;
  case KBSHORTCUT:   LoadAString(retval, IDSC_FLDNMKBSHORTCUT); break;
  default:
    ASSERT(0);
  };
  return retval;
}
  // Convenience: Get the untranslated (English) name of a FieldType
stringT CItemData::EngFieldName(FieldType ft)
{
  switch (ft) {
  case GROUPTITLE: return _T("Group/Title");
  case UUID:       return _T("UUID");
  case GROUP:      return _T("Group");
  case TITLE:      return _T("Title");
  case USER:       return _T("Username");
  case NOTES:      return _T("Notes");
  case PASSWORD:   return _T("Password");
  case CTIME:      return _T("Created Time");
  case PMTIME:     return _T("Password Modified Time");
  case ATIME:      return _T("Last Access Time");
  case XTIME:      return _T("Password Expiry Date");
  case RMTIME:     return _T("Record Modified Time");
  case URL:        return _T("URL");
  case AUTOTYPE:   return _T("AutoType");
  case PWHIST:     return _T("History");
  case POLICY:     return _T("Password Policy");
  case XTIME_INT:  return _T("Password Expiry Interval");
  case RUNCMD:     return _T("Run Command");
  case DCA:        return _T("DCA");
  case SHIFTDCA:   return _T("Shift-DCA");
  case EMAIL:      return _T("e-mail");
  case PROTECTED:  return _T("Protected");
  case SYMBOLS:    return _T("Symbols");
  case POLICYNAME: return _T("Password Policy Name");
  case KBSHORTCUT: return _T("Keyboard Shortcut");
  case ATTREF:     return _T("Attachment Reference");
  case BASEUUID:   return _T("Base UUID");
  case ALIASUUID:  return _T("Alias UUID");
  case SHORTCUTUUID:return _T("Shortcut UUID");
  default:
    ASSERT(0);
    return _T("");
  };
}


/*
* Copyright (c) 2003-2015 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file ItemAtt.cpp
//-----------------------------------------------------------------------------

#include "ItemAtt.h"
#include "BlowFish.h"
#include "TwoFish.h"
#include "PWSrand.h"
#include "PWSfile.h"
#include "PWScore.h"

#include "os/typedefs.h"
#include "os/pws_tchar.h"
#include "os/file.h"

using namespace std;
using pws_os::CUUID;

//-----------------------------------------------------------------------------
// Constructors

CItemAtt::CItemAtt()
  : m_entrystatus(ES_CLEAN), m_offset(-1L)
{
}

CItemAtt::CItemAtt(const CItemAtt &that) :
  CItem(that), m_entrystatus(that.m_entrystatus)
{
}

CItemAtt::~CItemAtt()
{
}

CItemAtt& CItemAtt::operator=(const CItemAtt &that)
{
  if (this != &that) { // Check for self-assignment
    CItem::operator=(that);
    m_entrystatus = that.m_entrystatus;
  }
  return *this;
}

bool CItemAtt::operator==(const CItemAtt &that) const
{
  return (m_entrystatus == that.m_entrystatus &&
          CItem::operator==(that));
}

void CItemAtt::SetTitle(const StringX &title)
{
  CItem::SetField(TITLE, title);
}

void CItemAtt::CreateUUID()
{
  CUUID uuid;
  SetUUID(uuid);
}


void CItemAtt::SetUUID(const CUUID &uuid)
{
  CItem::SetField(ATTUUID, static_cast<const unsigned char *>(*uuid.GetARep()), sizeof(uuid_array_t));
}

void CItemAtt::GetUUID(uuid_array_t &uuid_array) const
{
  size_t length = sizeof(uuid_array_t);
  FieldConstIter fiter = m_fields.find(ATTUUID);
  if (fiter != m_fields.end()) {
    CItem::GetField(fiter->second,
                    static_cast<unsigned char *>(uuid_array), length);
  } else {
    ASSERT(0);
    pws_os::Trace(_T("CItemAtt::GetUUID(uuid_array_t) - no UUID found!"));
    memset(uuid_array, 0, length);
  }
}

const CUUID CItemAtt::GetUUID() const
{
  uuid_array_t ua;
  GetUUID(ua);
  return CUUID(ua);
}

void CItemAtt::SetCTime(time_t t)
{
  unsigned char buf[sizeof(time_t)];
  putInt(buf, t);
  SetField(CTIME, buf, sizeof(time_t));
}

void CItemAtt::SetEK(const key256T &key)
{
  SetField(ATTEK, key, sizeof(key));
}

void CItemAtt::SetAK(const key256T &key)
{
  SetField(ATTAK, key, sizeof(key));
}

void CItemAtt::SetHMAC(const contentHMACT &hm)
{
  SetField(CONTENTHMAC, hm, sizeof(hm));
}

void CItemAtt::SetContent(const unsigned char *content, size_t clen)
{
  SetField(CONTENT, content, clen);
}

time_t CItemAtt::GetCTime(time_t &t) const
{
  GetTime(CTIME, t);
  return t;
}

void CItemAtt::GetKey(FieldType ft, key256T &key) const
{
  auto fiter = m_fields.find(ft);
  ASSERT(fiter != m_fields.end());
  size_t len = sizeof(key);
  GetField(fiter->second, key, len);
}

void CItemAtt::GetHMAC(contentHMACT &hm) const
{
  // should templatize GetKey...
  auto fiter = m_fields.find(CONTENTHMAC);
  ASSERT(fiter != m_fields.end());
  size_t len = sizeof(hm);
  GetField(fiter->second, hm, len);
}

size_t CItemAtt::GetContentLength() const
{
  auto fiter = m_fields.find(CONTENT);

  if (fiter != m_fields.end())
    return fiter->second.GetLength();
  else
    return 0;
}

size_t CItemAtt::GetContentSize() const
{
  auto fiter = m_fields.find(CONTENT);

  if (fiter != m_fields.end())
    return fiter->second.GetSize();
  else
    return 0;
}

bool CItemAtt::GetContent(unsigned char *content, size_t csize) const
{
  ASSERT(content != NULL);

  if (!HasContent() || csize < GetContentSize())
    return false;

  GetField(m_fields.find(CONTENT)->second, content, csize);
  return true;
}


int CItemAtt::Import(const stringT &fname)
{
  int status = PWScore::SUCCESS;

  ASSERT(!fname.empty());
  if (!pws_os::FileExists(fname))
    return PWScore::CANT_OPEN_FILE;
  std::FILE *fhandle = pws_os::FOpen(fname, L"rb");
  if (!fhandle)
    return PWScore::CANT_OPEN_FILE;
  size_t flen = static_cast<size_t>(pws_os::fileLength(fhandle));
  unsigned char *data = new unsigned char[flen];
  if (data == NULL)
    return PWScore::FAILURE;
  size_t nread = fread(data, flen, 1, fhandle);
  if (nread != 1) {
    fclose(fhandle);
    status = PWScore::READ_FAIL;
    goto done;
  }
  if (fclose(fhandle) != 0) {
    status = PWScore::READ_FAIL;
    goto done;
  }
  SetField(CONTENT, data, flen);
  SetField(FILENAME, fname.c_str());
 done:
  trashMemory(data, flen);
  delete[] data;
  return status;
}

int CItemAtt::Export(const stringT &fname) const
{
  int status = PWScore::SUCCESS;

  ASSERT(!fname.empty());
  ASSERT(IsFieldSet(CONTENT));
  // fail safely @runtime:
  if (!IsFieldSet(CONTENT))
    return PWScore::FAILURE;

  const CItemField &field = m_fields.find(CONTENT)->second;
  std::FILE *fhandle = pws_os::FOpen(fname, L"wb");
  if (!fhandle)
    return PWScore::CANT_OPEN_FILE;
  size_t flen = field.GetLength() + 8; // Add 8 for block size
  unsigned char *value = new unsigned char[flen];
  if (value == NULL) {
    fclose(fhandle);
    return PWScore::FAILURE;
  }
  CItem::GetField(field, value, flen); // flen adjusted to real value
  size_t nwritten = fwrite(value, flen, 1, fhandle);
  if (nwritten != 1) {
    status = PWScore::WRITE_FAIL;
    goto done;
  }
  if (fclose(fhandle) != 0) {
    status = PWScore::WRITE_FAIL;
    goto done;
  }
 done:
  trashMemory(value, flen);
  delete[] value;
  return status;
}


int CItemAtt::Read(PWSfile *in)
{
  int status = PWSfile::SUCCESS;
#if 0
  signed long numread = 0;
  unsigned char type;

  int emergencyExit = 255; // to avoid endless loop.
  signed long fieldLen; // <= 0 means end of file reached

  Clear();
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
#endif
  return status;
}

#if 0
size_t CItemAtt::WriteIfSet(FieldType ft, PWSfile *out, bool isUTF8) const
{
  FieldConstIter fiter = m_fields.find(ft);
  size_t retval = 0;
  if (fiter != m_fields.end()) {
    const CItemField &field = fiter->second;
    ASSERT(!field.IsEmpty());
    size_t flength = field.GetLength() + BlowFish::BLOCKSIZE;
    unsigned char *pdata = new unsigned char[flength];
    CItem::GetField(field, pdata, flength);
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

int CItemAtt::WriteCommon(PWSfile *out) const
{
  int i;

  const FieldType TextFields[] = {GROUP, TITLE, USER, PASSWORD,
                                  NOTES, URL, AUTOTYPE, POLICY,
                                  PWHIST, RUNCMD, EMAIL,
                                  SYMBOLS, POLICYNAME,
                                  END};
  const FieldType TimeFields[] = {ATIME, CTIME, XTIME, PMTIME, RMTIME,
                                  END};

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
  // Assume that if previous write failed, last one will too.
  if (out->WriteField(END, _T("")) > 0) {
    return PWSfile::SUCCESS;
  } else {
    return PWSfile::FAILURE;
  }
}
#endif

int CItemAtt::Write(PWSfile *out) const
{
  int status = PWSfile::SUCCESS;
#if 0
  // Map different UUID types (V4 concept) to original V3 UUID
  uuid_array_t item_uuid;
  FieldType ft = END;

  ASSERT(HasUUID());
  if (!IsDependent())
    ft = UUID;
  else if (IsAlias())
    ft = ALIASUUID;
  else if (IsShortcut())
    ft = SHORTCUTUUID;
  else ASSERT(0);
  GetUUID(item_uuid, ft);

  out->WriteField(UUID, item_uuid, sizeof(uuid_array_t));

  // We need to cast away constness to change Password field
  // for dependent entries
  // We restore the password afterwards (not that it should matter
  // for a dependent), so logically we're still const.

  CItemAtt *self = const_cast<CItemAtt *>(this);
  const StringX saved_password = GetPassword();
  self->SetSpecialPasswords(); // encode baseuuid in password if IsDependent

  status = WriteCommon(out);

  self->SetPassword(saved_password);
  return status;
}

int CItemAtt::Write(PWSfileV4 *out) const
{
  int status = PWSfile::SUCCESS;
  uuid_array_t item_uuid;

  ASSERT(HasUUID());

  FieldType ft = END;

  ASSERT(HasUUID());
  if (!IsDependent())
    ft = UUID;
  else if (IsAlias())
    ft = ALIASUUID;
  else if (IsShortcut())
    ft = SHORTCUTUUID;
  else ASSERT(0);
  GetUUID(item_uuid, ft);

  out->WriteField(static_cast<unsigned char>(ft), item_uuid,
                  sizeof(uuid_array_t));
  if (IsDependent()) {
    uuid_array_t base_uuid;
    ASSERT(IsFieldSet(BASEUUID));
    GetUUID(base_uuid, BASEUUID);
    out->WriteField(BASEUUID, base_uuid, sizeof(uuid_array_t));
  }

  status = WriteCommon(out);
#endif
  return status;
}

#if 0
int CItemAtt::WriteUnknowns(PWSfile *out) const
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

StringX CItemAtt::GetField(const FieldType ft) const
{
  FieldConstIter fiter = m_fields.find(ft);
  return fiter == m_fields.end() ? _T("") : GetField(fiter->second);
}

StringX CItemAtt::GetField(const CItemField &field) const
{
  StringX retval;
  BlowFish *bf = MakeBlowFish(field.IsEmpty());
  field.Get(retval, bf);
  delete bf;
  return retval;
}

StringX CItemAtt::GetFieldValue(FieldType ft) const
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

StringX CItemAtt::GetTime(int whichtime, PWSUtil::TMC result_format) const
{
  time_t t;

  GetTime(whichtime, t);
  return PWSUtil::ConvertToDateTimeString(t, result_format);
}

void CItemAtt::GetTime(int whichtime, time_t &t) const
{
  FieldConstIter fiter = m_fields.find(FieldType(whichtime));
  if (fiter != m_fields.end()) {
    unsigned char in[TwoFish::BLOCKSIZE]; // required by GetField
    size_t tlen = sizeof(in); // ditto

    CItem::GetField(fiter->second, in, tlen);
    if (tlen != 0) {
    // time field's store in native time_t size, regardless of
    // the representation on file
      ASSERT(tlen == sizeof(t));
      if (!PWSUtil::pull_time(t, in, tlen))
        ASSERT(0);
    } else {
      t = 0;
    }
  } else // fiter == m_fields.end()
    t = 0;
}

//-----------------------------------------------------------------------------
// Setters

void CItemAtt::SetTime(int whichtime)
{
  time_t t;
  time(&t);
  SetTime(whichtime, t);
}

void CItemAtt::SetTime(int whichtime, time_t t)
{
  unsigned char buf[sizeof(time_t)];
  putInt(buf, t);
  SetField(static_cast<FieldType>(whichtime), buf, sizeof(time_t));
}

bool CItemAtt::SetTime(int whichtime, const stringT &time_str)
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


void CItemAtt::SetFieldValue(FieldType ft, const StringX &value)
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


bool CItemAtt::SetField(unsigned char type, const unsigned char *data, size_t len)
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
    case BASEUUID:
    case ALIASUUID:
    case SHORTCUTUUID:
      {
        uuid_array_t uuid_array;
        ASSERT(len == sizeof(uuid_array_t));
        for (size_t i = 0; i < sizeof(uuid_array_t); i++)
          uuid_array[i] = data[i];
        SetUUID(uuid_array, ft);
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
      if (!PWSUtil::pull_time(t, data, len)) return false;
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


  // Convenience: Get the name associated with FieldType
stringT CItemAtt::FieldName(FieldType ft)
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
stringT CItemAtt::EngFieldName(FieldType ft)
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
#endif

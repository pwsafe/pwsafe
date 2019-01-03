/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
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
#include "PWSfileV4.h"
#include "PWScore.h"

#include "os/typedefs.h"
#include "os/pws_tchar.h"
#include "os/file.h"
#include "os/media.h"
#include "os/utf8conv.h"
#include "os/dir.h"

#include <sys/types.h>
#include <sys/stat.h>

using namespace std;
using pws_os::CUUID;

//-----------------------------------------------------------------------------
// Constructors

CItemAtt::CItemAtt()
  : m_entrystatus(ES_CLEAN), m_offset(-1L), m_refcount(0)
{
}

CItemAtt::CItemAtt(const CItemAtt &that) :
  CItem(that), m_entrystatus(that.m_entrystatus),
  m_offset(that.m_offset), m_refcount(that.m_refcount)
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
    m_offset = that.m_offset;
    m_refcount = that.m_refcount;
  }
  return *this;
}

bool CItemAtt::operator==(const CItemAtt &that) const
{
  return (m_entrystatus == that.m_entrystatus &&
          m_offset == that.m_offset &&
          m_refcount == that.m_refcount &&
          CItem::operator==(that));
}

void CItemAtt::SetTitle(const StringX &title)
{
  CItem::SetField(ATTTITLE, title);
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
  auto fiter = m_fields.find(ATTUUID);
  if (fiter != m_fields.end()) {
    CItem::GetField(fiter->second,
                    static_cast<unsigned char *>(uuid_array), length);
  } else {
    ASSERT(0);
    pws_os::Trace(_T("CItemAtt::GetUUID(uuid_array_t) - no UUID found!\n"));
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
  SetField(ATTCTIME, buf, sizeof(time_t));
}

void CItemAtt::SetContent(const unsigned char *content, size_t clen)
{
  SetField(CONTENT, content, clen);
}

time_t CItemAtt::GetCTime(time_t &t) const
{
  CItem::GetTime(ATTCTIME, t);
  return t;
}

StringX CItemAtt::GetTime(int whichtime, PWSUtil::TMC result_format) const
{
  time_t t;

  CItem::GetTime(whichtime, t);
  return PWSUtil::ConvertToDateTimeString(t, result_format);
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
  ASSERT(content != nullptr);

  if (!HasContent() || csize < GetContentSize())
    return false;

  GetField(m_fields.find(CONTENT)->second, content, csize);
  return true;
}

int CItemAtt::Import(const stringT &fname)
{
  stringT spath, sdrive, sdir, sfname, sextn;
  time_t atime(0), ctime(0), mtime(0);
  int status = PWScore::SUCCESS;

  ASSERT(!fname.empty());
  if (!pws_os::FileExists(fname))
    return PWScore::CANT_OPEN_FILE;

  std::FILE *fhandle = pws_os::FOpen(fname, L"rb");
  if (!fhandle)
    return PWScore::CANT_OPEN_FILE;

  auto flen = static_cast<size_t>(pws_os::fileLength(fhandle));
  auto *data = new unsigned char[flen];
  if (data == nullptr)
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

  // derive the file's path and name
  pws_os::splitpath(fname, sdrive, sdir, sfname, sextn);
  spath = pws_os::makepath(sdrive, sdir, _T(""), _T(""));

  CItem::SetField(FILENAME, (sfname + sextn).c_str());
  CItem::SetField(FILEPATH, spath.c_str());
  CItem::SetField(MEDIATYPE, pws_os::GetMediaType(fname.c_str()).c_str());

  if (pws_os::GetFileTimes(fname, ctime, mtime, atime)) {
    unsigned char buf[sizeof(time_t)];
    putInt(buf, ctime);
    CItem::SetField(FILECTIME, buf, sizeof(buf));
    putInt(buf, mtime);
    CItem::SetField(FILEMTIME, buf, sizeof(buf));
    putInt(buf, atime);
    CItem::SetField(FILEATIME, buf, sizeof(buf));
  } else {
    ASSERT(0);
    goto done;
  }

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
  auto *value = new unsigned char[flen];
  if (value == nullptr) {
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

bool CItemAtt::SetField(unsigned char type, const unsigned char *data,
                        size_t len)
{
  auto ft = static_cast<FieldType>(type);
  switch (ft) {
  case ATTUUID:
    {
      uuid_array_t uuid_array;
      ASSERT(len == sizeof(uuid_array_t));
      for (size_t i = 0; i < sizeof(uuid_array_t); i++)
        uuid_array[i] = data[i];
      SetUUID(uuid_array);
      break;
    }
  case ATTTITLE:
  case MEDIATYPE:
  case FILENAME:
  case FILEPATH:
    if (!SetTextField(ft, data, len)) return false;
    break;
  case ATTCTIME:
  case FILECTIME:
  case FILEMTIME:
  case FILEATIME:
    if (!SetTimeField(ft, data, len)) return false;
    break;
  case CONTENT:
    CItem::SetField(type, data, len);
    break;
  case ATTIV:
  case ATTEK:
  case ATTAK:
  case CONTENTHMAC:
    // These fields have no business in the record, created and used
    // solely for file i/o.
    ASSERT(0);
    return false;
  case END:
    break;
  default:
    // unknowns!
    // SetUnknownField(char(type), len, data); -- good idea for ItemAtt too
    break;
  }
  return true;
}

int CItemAtt::Read(PWSfile *in)
{
  int status = PWSfile::FAILURE; // generic failure
  signed long numread = 0;
  unsigned char type;

  int emergencyExit = 255; // to avoid endless loop.
  signed long fieldLen; // <= 0 means end of file reached

  bool gotIV(false), gotEK(false), gotAK(false); // pre-reqs for content
  bool gotContent(false), gotHMAC(false); // post-requisites
  unsigned char IV[TwoFish::BLOCKSIZE] = {0};
  unsigned char EK[PWSfileV4::KLEN] = {0};
  unsigned char AK[PWSfileV4::KLEN] = {0};

  unsigned char *content = nullptr;
  size_t content_len = 0;
  unsigned char expected_digest[SHA256::HASHLEN] = {0};

  unsigned char *utf8 = nullptr;
  size_t utf8Len = 0;

  Clear();

  do {
    fieldLen = static_cast<signed long>(in->ReadField(type, utf8,
                                                      utf8Len));

    if (fieldLen > 0) {
      numread += fieldLen;
      switch (type) {
      case ATTIV: {
        ASSERT(utf8Len == sizeof(IV));
        ASSERT(!gotIV);
        if (utf8Len != sizeof(IV) || gotIV)
          goto exit;
        gotIV = true;
        memcpy(IV, utf8, sizeof(IV));
        emergencyExit--;
        break;
      }
      case ATTEK: {
        ASSERT(utf8Len == sizeof(EK));
        ASSERT(!gotEK);
        if (utf8Len != sizeof(EK) || gotEK)
          goto exit;
        gotEK = true;
        memcpy(EK, utf8, sizeof(EK));
        emergencyExit--;
        break;
      }
      case ATTAK: {
        ASSERT(utf8Len == sizeof(AK));
        ASSERT(!gotAK);
        if (utf8Len != sizeof(AK) || gotAK)
          goto exit;
        gotAK = true;
        memcpy(AK, utf8, sizeof(AK));
        emergencyExit--;
        break;
      }
      case CONTENT: {
        // Yes, we're supposed to be able to handle this
        // even if IV and EK haven't been read yet.
        // One step at a time, though...
        ASSERT(gotIV && gotEK);
        ASSERT(!gotContent);

        ASSERT(utf8Len == sizeof(uint32));
        if (!gotIV || !gotEK || gotContent || utf8Len != sizeof(uint32))
          goto exit;
        content_len = getInt32(utf8);

        TwoFish fish(EK, sizeof(EK));
        trashMemory(EK, sizeof(EK));
        const unsigned int BS = fish.GetBlockSize();

        auto *in4 = dynamic_cast<PWSfileV4 *>(in);
        ASSERT(in4 != nullptr);
        size_t nread = in4->ReadContent(&fish, IV, content, content_len);
        // nread should be content_len rounded up to nearest BS:
        ASSERT(nread == (content_len/BS + 1)*BS);
        if (nread != (content_len/BS + 1)*BS) {
          status = PWSfile::READ_FAIL;
          goto exit;
        }
        gotContent = true;
        break;
      }
      case CONTENTHMAC: {
        ASSERT(!gotHMAC);
        ASSERT(utf8Len == SHA256::HASHLEN);
        if (gotHMAC || utf8Len != SHA256::HASHLEN)
          goto exit;
        gotHMAC = true;
        memcpy(expected_digest, utf8, SHA256::HASHLEN);
        break;
      }
      default: // "normal" fields
        if (!SetField(type, utf8, utf8Len))
          goto exit;
      } // switch {type)
    } // if (fieldLen > 0)

    if (utf8 != nullptr) {
      trashMemory(utf8, utf8Len * sizeof(utf8[0]));
      delete[] utf8; utf8 = nullptr; utf8Len = 0;
    }
  } while (type != END && fieldLen > 0 && --emergencyExit > 0);

  // Post-field read processing:
  // - Ensure we have all we need
  // - Check HMAC
  // - Set Content field
  // - Clean-up

  if (gotContent && gotAK && gotHMAC) {
    unsigned char calculated_digest[SHA256::HASHLEN] = {0};
    HMAC<SHA256, SHA256::HASHLEN, SHA256::BLOCKSIZE> hmac;

    hmac.Init(AK, sizeof(AK));
    trashMemory(AK, sizeof(AK));
    
    // calculate HMAC
    hmac.Update(content, (unsigned long)content_len);
    hmac.Final(calculated_digest);

    if (memcmp(expected_digest, calculated_digest,
               sizeof(calculated_digest)) == 0) {
      SetField(CONTENT, content, content_len);
      status = PWSfile::SUCCESS;
    } else {
      status = PWSfile::BAD_DIGEST;
    }
  } else {
    status = PWSfile::READ_FAIL;
  }

 exit:
  trashMemory(content, content_len);
  delete[] content;
  delete[] utf8; // if here via goto exit

  if (numread > 0) {
    m_offset = in->GetOffset();
    return status;
  } else
    return PWSfile::READ_FAIL;
}

size_t CItemAtt::WriteIfSet(FieldType ft, PWSfile *out, bool isUTF8) const
{
  auto fiter = m_fields.find(ft);
  size_t retval = 0;
  if (fiter != m_fields.end()) {
    const CItemField &field = fiter->second;
    ASSERT(!field.IsEmpty());
    size_t flength = field.GetLength() + BlowFish::BLOCKSIZE;
    auto *pdata = new unsigned char[flength];
    CItem::GetField(field, pdata, flength);
    if (isUTF8) {
      wchar_t *wpdata = reinterpret_cast<wchar_t *>(pdata);
      size_t srclen = field.GetLength()/sizeof(TCHAR);
      wpdata[srclen] = 0;
      size_t dstlen = pws_os::wcstombs(nullptr, 0, wpdata, srclen);
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

int CItemAtt::Write(PWSfile *out) const
{
  int status = PWSfile::SUCCESS;
  uuid_array_t att_uuid;

  ASSERT(HasUUID());
  GetUUID(att_uuid);

  out->WriteField(static_cast<unsigned char>(ATTUUID), att_uuid,
                  sizeof(uuid_array_t));

  WriteIfSet(ATTTITLE, out, true);
  WriteIfSet(ATTCTIME, out, false);
  WriteIfSet(MEDIATYPE, out, true);
  WriteIfSet(FILENAME, out, true);
  WriteIfSet(FILEPATH, out, true);
  WriteIfSet(FILECTIME, out, false);
  WriteIfSet(FILEMTIME, out, false);
  WriteIfSet(FILEATIME, out, false);

  auto fiter = m_fields.find(CONTENT);
  // XXX TBD - fail if no content, as this is a mandatory field
  if (fiter != m_fields.end()) {
    auto *out4 = dynamic_cast<PWSfileV4 *>(out);
    ASSERT(out4 != nullptr);

    size_t clength = fiter->second.GetLength() + BlowFish::BLOCKSIZE;
    auto *content = new unsigned char[clength];
    CItem::GetField(fiter->second, content, clength);
    out4->WriteContentFields(content, clength);
    trashMemory(content, clength);
    delete[] content;
  }

  if (out->WriteField(END, _T("")) > 0) {
    status = PWSfile::SUCCESS;
  } else {
    status = PWSfile::FAILURE;
  }
  return status;
}

bool CItemAtt::Matches(const stringT &stValue, int iObject,
  int iFunction) const
{
  ASSERT(iFunction != 0); // must be positive or negative!

  StringX sx_Object;
  auto ft = static_cast<FieldType>(iObject);
  switch (ft) {
    case AT_TITLE:
    case AT_FILENAME:
    case AT_FILEPATH:
    case AT_MEDIATYPE:
      sx_Object = GetField(ft);
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

bool CItemAtt::Matches(time_t time1, time_t time2, int iObject,
  int iFunction) const
{
  //   Check time values are selected
  time_t tValue;

  switch (iObject) {
    case AT_CTIME:
    case AT_FILECTIME:
    case AT_FILEMTIME:
    case AT_FILEATIME:
      CItem::GetTime(iObject, tValue);
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

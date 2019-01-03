/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file Item.cpp
//-----------------------------------------------------------------------------

#include "Item.h"
#include "BlowFish.h"
#include "TwoFish.h"
#include "PWSrand.h"
#include "UTF8Conv.h"
#include "Util.h"
#include "os/env.h"

#include <vector>

CItem::CItem()
{
  PWSrand::GetInstance()->GetRandomData( m_key, sizeof(m_key) );
}

CItem::CItem(const CItem &that) :
  m_fields(that.m_fields),
  m_URFL(that.m_URFL)
{
  memcpy(m_key, that.m_key, sizeof(m_key));
}

CItem::~CItem()
{
  delete m_blowfish;
  // Following protects against possible use-after-delete
  // bug, since new BF will be created, rather than
  // using one with trashed values
  m_blowfish = nullptr;
}

CItem& CItem::operator=(const CItem &that)
{
  if (this != &that) { // Check for self-assignment
    m_fields = that.m_fields;
    m_URFL = that.m_URFL;

    memcpy(m_key, that.m_key, sizeof(m_key));
    delete m_blowfish;
    m_blowfish = nullptr;
  }
  return *this;
}

bool CItem::CompareFields(const CItemField &fthis,
                          const CItem &that, const CItemField &fthat) const
{
  if (fthis.GetLength() != fthat.GetLength() ||
      fthis.GetType() != fthat.GetType())
    return false;
  size_t flength = fthis.GetLength() + BlowFish::BLOCKSIZE;
  auto *dthis = new unsigned char[flength];
  auto *dthat = new unsigned char[flength];
  GetField(fthis, dthis, flength);
  flength = fthis.GetLength() + BlowFish::BLOCKSIZE; // GetField updates length, reset
  that.GetField(fthat, dthat, flength);
  bool retval = (memcmp(dthis, dthat, flength) == 0);
  trashMemory(dthis, flength); trashMemory(dthis, flength);
  delete[] dthis; delete[] dthat;
  return retval;
}

bool CItem::operator==(const CItem &that) const
{
  if (m_fields.size() == that.m_fields.size() &&
      m_URFL.size() == that.m_URFL.size()) {
    /**
     * It would be nice to be able to compare the m_fields
     * and m_URFL directly, but the fields would be
     * encrypted with different keys, making byte-wise
     * field comparisons infeasible.
     */
    FieldConstIter ithis, ithat;
    for (ithis = m_fields.begin(), ithat = that.m_fields.begin();
         ithis != m_fields.end();
         ithis++, ithat++) {
      if (ithis->first != ithat->first)
        return false;
      const CItemField &fthis = ithis->second;
      const CItemField &fthat = ithat->second;
      if (!CompareFields(fthis, that, fthat))
        return false;
    } // for m_fields
  } else
    return false;

  // If we made it so far, now compare the unknown record fields
  // (We already know their sizes are equal)
  if (!m_URFL.empty()) {
    auto ithis = m_URFL.begin();
    auto ithat = that.m_URFL.begin();
    for (;
         ithis != m_URFL.end();
         ithis++, ithat++) {
      if (!CompareFields(*ithis, that, *ithat))
        return false;
    } // for m_URFL
  }
  return true;
}

size_t CItem::GetSize() const
{
  size_t length(0);

  for (FieldConstIter fiter = m_fields.begin(); fiter != m_fields.end(); fiter++)
    length += fiter->second.GetLength();

  for (auto ufiter = m_URFL.begin();
       ufiter != m_URFL.end(); ufiter++)
    length += ufiter->GetLength();

  return length;
}

BlowFish *CItem::MakeBlowFish() const
{
  // Creating a BlowFish object's relatively expensive, so we use
  // the singleton design pattern for the life of the CItem object
  if(!m_blowfish) {
    m_blowfish = BlowFish::MakeBlowFish(m_key, sizeof(m_key));
  }
  return m_blowfish;
}

void CItem::SetUnknownField(unsigned char type,
                            size_t length,
                            const unsigned char *ufield)
{
  /**
     TODO - check that this unknown field from the XML Import file is now
     known and it should be added as that instead!
  **/

  CItemField unkrfe(type);
  unkrfe.Set(ufield, length, MakeBlowFish());
  m_URFL.push_back(unkrfe);
}

void CItem::GetUnknownField(unsigned char &type, size_t &length,
                            unsigned char * &pdata,
                            const CItemField &item) const
{
  ASSERT(pdata == nullptr && length == 0);

  type = item.GetType();
  size_t flength = item.GetLength();
  length = flength;
  flength += BlowFish::BLOCKSIZE; // ensure that we've enough for at least one block
  pdata = new unsigned char[flength];
  GetField(item, pdata, flength);
}

void CItem::Clear()
{
  m_fields.clear();
  m_URFL.clear();
}

void CItem::SetField(int ft, const unsigned char *value, size_t length)
{
  if (length != 0) {
    m_fields[ft].Set(value, length,
                     MakeBlowFish(),
                     static_cast<unsigned char>(ft));
  } else
    m_fields.erase(ft);
}

void CItem::SetField(int ft, const StringX &value)
{
  if (!value.empty()) {
    m_fields[ft].Set(value,
                     MakeBlowFish(),
                     static_cast<unsigned char>(ft));
  } else
    m_fields.erase(ft);
}

static bool pull_string(StringX &str,
                        const unsigned char *data, size_t len)
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
  std::vector<unsigned char> v(data, (data + len));
  v.push_back(0); // null terminate for FromUTF8.
  bool utf8status = utf8conv.FromUTF8(&v[0], len, str);
  if (!utf8status) {
    pws_os::Trace(_T("Item.cpp: pull_string(): FromUTF8 failed!\n"));
  }
  trashMemory(&v[0], len);
  return utf8status;
}

bool CItem::SetTextField(int ft, const unsigned char *value,
                         size_t length)
{
  StringX str;
  if (pull_string(str, value, length)) {
    SetField(ft, str);
    return true;
  } else
    return false;
}

void CItem::SetTime(const int whichtime, time_t t)
{
  unsigned char buf[sizeof(time_t)];
  putInt(buf, t);
  SetField(whichtime, buf, sizeof(time_t));
}

bool CItem::SetTimeField(int ft, const unsigned char *value,
                         size_t length)
{
  time_t t;
  if (PWSUtil::pull_time(t, value, length)) {
    SetTime(ft, t);
    return true;
  } else
    return false;
}

void CItem::GetField(const CItemField &field,
                     unsigned char *value, size_t &length) const
{
  field.Get(value, length, MakeBlowFish());
}

StringX CItem::GetField(const int ft) const
{
  auto fiter = m_fields.find(ft);
  return fiter == m_fields.end() ? _T("") : GetField(fiter->second);
}

StringX CItem::GetField(const CItemField &field) const
{
  StringX retval;
  field.Get(retval, MakeBlowFish());
  return retval;
}

void CItem::GetTime(int whichtime, time_t &t) const
{
  auto fiter = m_fields.find(whichtime);
  if (fiter != m_fields.end()) {
    unsigned char in[TwoFish::BLOCKSIZE] = {0}; // required by GetField
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

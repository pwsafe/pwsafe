/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file ItemField.cpp
//-----------------------------------------------------------------------------

#include <math.h>

#include "ItemField.h"
#include "Util.h"
#include "Fish.h"
#include "PWSrand.h"
#include "os/funcwrap.h"

//Returns the number of bytes of 8 byte blocks needed to store 'size' bytes
size_t CItemField::GetBlockSize(size_t size) const
{
  return static_cast<size_t>(ceil(static_cast<double>(size) / 8.0)) * 8;
}

CItemField::CItemField(const CItemField &that)
  : m_Type(that.m_Type), m_Length(that.m_Length)
{
  if (m_Length > 0) {
    size_t bs = GetBlockSize(m_Length);
    m_Data = new unsigned char[bs];
    memcpy(m_Data, that.m_Data, bs);
  } else {
    m_Data = nullptr;
  }
}

CItemField &CItemField::operator=(const CItemField &that)
{
  if (this != &that) {
    m_Type = that.m_Type;
    m_Length = that.m_Length;
    delete[] m_Data;
    if (m_Length > 0) {
      size_t bs = GetBlockSize(m_Length);
      m_Data = new unsigned char[bs];
      memcpy(m_Data, that.m_Data, bs);
    } else {
      m_Data = nullptr;
    }
  }
  return *this;
}

void CItemField::Empty()
{
  if (m_Data != nullptr) {
    delete[] m_Data;
    m_Data = nullptr;
    m_Length = 0;
  }
}

void CItemField::Set(const unsigned char* value, size_t length,
                     const Fish *bf, unsigned char type)
{
  size_t BlockLength;

  m_Length = length;
  BlockLength = GetBlockSize(m_Length);

  delete[] m_Data;

  if (m_Length == 0) {
    m_Data = nullptr;
  } else {
    m_Data = new unsigned char[BlockLength];
    if (m_Data == nullptr) { // out of memory - try to fail gracefully
      m_Length = 0; // at least keep structure consistent
      return;
    }

    auto *tempmem = new unsigned char[BlockLength];
    // invariant: BlockLength >= plainlength
    memcpy_s(tempmem, BlockLength, value, m_Length);

    //Fill the unused characters in with random stuff
    PWSrand::GetInstance()->GetRandomData(tempmem + m_Length, static_cast<unsigned long>(BlockLength - m_Length));

    //Do the actual encryption
    for (size_t x = 0; x < BlockLength; x += 8)
      bf->Encrypt(tempmem + x, m_Data + x);

    trashMemory(tempmem, BlockLength);
    delete[] tempmem;
  }
  if (type != 0xff)
    m_Type = type;
}

void CItemField::Set(const StringX &value, const Fish *bf, unsigned char type)
{
  const LPCTSTR plainstr = value.c_str();

  Set(reinterpret_cast<const unsigned char *>(plainstr),
      value.length() * sizeof(*plainstr), bf, type);
}

void CItemField::Get(unsigned char *value, size_t &length, const Fish *bf) const
{
  // Sanity check: length is 0 iff data ptr is nullptr
  ASSERT((m_Length == 0 && m_Data == nullptr) ||
         (m_Length > 0 && m_Data != nullptr));
  /*
  * length is an in/out parameter:
  * In: size of value array - must be at least BlockLength
  * Out: size of data stored: m_Length (No trailing zero!)
  * if In < BlockLength, assertion is triggered (no way to handle gracefully)
  */
  if (m_Length == 0) {
    value[0] = TCHAR('\0');
    length = 0;
  } else { // we have data to decrypt
    size_t BlockLength = GetBlockSize(m_Length);
    ASSERT(length >= BlockLength);
    auto *tempmem = new unsigned char[BlockLength];

    size_t x;
    for (x = 0; x < BlockLength; x += 8)
      bf->Decrypt(m_Data + x, tempmem + x);

    for (x = 0; x < BlockLength; x++)
      value[x] = (x < m_Length) ? tempmem[x] : 0;

    length = m_Length;
    delete [] tempmem;
  }
}

void CItemField::Get(StringX &value, const Fish *bf) const
{
  // Sanity check: length is 0 iff data ptr is nullptr
  ASSERT((m_Length == 0 && m_Data == nullptr) ||
         (m_Length > 0 && m_Data != nullptr && m_Length % sizeof(TCHAR) == 0));

  if (m_Length == 0) {
    value = _T("");
  } else { // we have data to decrypt
    size_t BlockLength = GetBlockSize(m_Length);
    auto *tempmem = new unsigned char[BlockLength];
    TCHAR *pt = reinterpret_cast<TCHAR *>(tempmem);
    size_t x;

    // decrypt block by block
    for (x = 0; x < BlockLength; x += 8)
      bf->Decrypt(m_Data + x, tempmem + x);

    // copy to value TCHAR by TCHAR
    for (x = 0; x < m_Length/sizeof(TCHAR); x++)
      value += pt[x];

    trashMemory(tempmem, BlockLength);
    delete [] tempmem;
  }
}

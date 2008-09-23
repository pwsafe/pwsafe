/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
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
#include "BlowFish.h"
#include "PWSrand.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//Returns the number of bytes of 8 byte blocks needed to store 'size' bytes
int CItemField::GetBlockSize(int size) const
{
  return (int)ceil((double)size/8.0) * 8;
}

CItemField::CItemField(const CItemField &that)
  : m_Type(that.m_Type), m_Length(that.m_Length)
{
  if (m_Length > 0) {
    int bs = GetBlockSize(m_Length);
    m_Data = new unsigned char[bs];
    ::memcpy(m_Data, that.m_Data, bs);
  } else {
    m_Data = NULL;
  }
}

CItemField &CItemField::operator=(const CItemField &that)
{
  if (this != &that) {
    m_Type = that.m_Type;
    m_Length = that.m_Length;
    if (m_Data != NULL)
      delete[] m_Data;
    if (m_Length > 0) {
      int bs = GetBlockSize(m_Length);
      m_Data = new unsigned char[bs];
      ::memcpy(m_Data, that.m_Data, bs);
    } else {
      m_Data = NULL;
    }
  }
  return *this;
}

void CItemField::Empty()
{
  if (m_Data != NULL) {
    delete[] m_Data;
    m_Data = NULL;
    m_Length = 0;
  }
}

void CItemField::Set(const unsigned char* value, unsigned int length, BlowFish *bf)
{
  const LPCSTR plainstr = (const LPCSTR)value;
  int BlockLength;

  m_Length = length;
  BlockLength = GetBlockSize(m_Length);

  if (m_Data != NULL)
    delete[] m_Data;

  if (m_Length == 0) {
    m_Data = NULL;
  } else {
    m_Data = new unsigned char[BlockLength];
    if (m_Data == NULL) { // out of memory - try to fail gracefully
      m_Length = 0; // at least keep structure consistent
      return;
    }

    unsigned char *tempmem = new unsigned char[BlockLength];
    // invariant: BlockLength >= plainlength
    ::memcpy((char*)tempmem, (char*)plainstr, m_Length);

    //Fill the unused characters in with random stuff
    PWSrand::GetInstance()->GetRandomData(tempmem+m_Length, BlockLength-m_Length );

    //Do the actual encryption
    for (int x=0; x<BlockLength; x+=8)
      bf->Encrypt(tempmem+x, m_Data+x);

    delete[] tempmem;
  }
}

void CItemField::Set(const StringX &value, BlowFish *bf)
{
  const LPCTSTR plainstr = value.c_str();

  Set((const unsigned char *)plainstr,
      value.length() * sizeof(*plainstr), bf);
}

void CItemField::Get(unsigned char *value, unsigned int &length, BlowFish *bf) const
{
  // Sanity check: length is 0 iff data ptr is NULL
  ASSERT((m_Length == 0 && m_Data == NULL) ||
         (m_Length > 0 && m_Data != NULL));
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
    int BlockLength = GetBlockSize(m_Length);
    ASSERT(length >= (unsigned int)BlockLength);
    unsigned char *tempmem = new unsigned char[BlockLength];

    int x;
    for (x = 0; x < BlockLength; x += 8)
      bf->Decrypt(m_Data + x, tempmem + x);

    for (x = 0; x < BlockLength; x++)
      value[x] = (x < int(m_Length)) ? tempmem[x] : 0;

    length = m_Length;
    delete [] tempmem;
  }
}

void CItemField::Get(StringX &value, BlowFish *bf) const
{
  // Sanity check: length is 0 iff data ptr is NULL
  ASSERT((m_Length == 0 && m_Data == NULL) ||
         (m_Length > 0 && m_Data != NULL));

  if (m_Length == 0) {
    value = _T("");
  } else { // we have data to decrypt
    size_t BlockLength = GetBlockSize(m_Length);
    value.resize(BlockLength/sizeof(TCHAR) + 1);
    unsigned char *tempmem = new unsigned char[BlockLength];
    TCHAR *pt = reinterpret_cast<TCHAR *>(tempmem);
    size_t x;
    // decrypt block by block
    for (x = 0; x < BlockLength; x += 8)
      bf->Decrypt(m_Data + x, tempmem + x);

    // copy to value TCHAR by TCHAR
    for (x = 0; x < BlockLength/sizeof(TCHAR); x++)
      value[x] = (x < m_Length/sizeof(TCHAR)) ? pt[x] : 0;

    value += TCHAR('\0');

    trashMemory(tempmem, BlockLength);
    delete [] tempmem;
  }
}

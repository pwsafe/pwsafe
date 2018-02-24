/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// ItemField.h
//-----------------------------------------------------------------------------

#ifndef __ITEMFIELD_H
#define __ITEMFIELD_H

#include "StringX.h"

//-----------------------------------------------------------------------------

/*
* CItemField contains the data for a given CItemData field in encrypted
* form.
* Set() encrypts, Get() decrypts
*/

class Fish;

class CItemField
{
public:
  explicit CItemField(unsigned char type = 0xff): m_Type(type), m_Length(0), m_Data(nullptr)
  {}
  CItemField(const CItemField &that); // copy ctor
  ~CItemField() {if (m_Length > 0) delete[] m_Data;}

  CItemField &operator=(const CItemField &that);

  void Set(const StringX &value, const Fish *bf, unsigned char type = 0xff);
  void Set(const unsigned char* value, size_t length, const Fish *bf, unsigned char type = 0xff);

  void Get(StringX &value, const Fish *bf) const;
  void Get(unsigned char *value, size_t &length, const Fish *bf) const;
  unsigned char GetType() const {return m_Type;}
  size_t GetLength() const {return m_Length;}
  size_t GetSize() const {return GetBlockSize(m_Length);}
  bool IsEmpty() const {return m_Length == 0;}
  void Empty();

private:
  //Number of 8 byte blocks needed for size
  size_t GetBlockSize(size_t size) const;

  unsigned char m_Type; // almost const
  size_t m_Length;
  unsigned char *m_Data;
};

#endif /* __ITEMFIELD_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:

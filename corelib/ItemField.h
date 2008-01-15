/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// ItemField.h
//-----------------------------------------------------------------------------

#if !defined ItemField_h
#define ItemField_h

#include "MyString.h"

//-----------------------------------------------------------------------------

/*
* CItemField contains the data for a given CItemData field in encrypted
* form.
* Set() encrypts, Get() decrypts
*/

class BlowFish;

class CItemField {
public:
  CItemField(unsigned char type): m_Type(type), m_Length(0), m_Data(NULL)
  {}
  CItemField(const CItemField &that); // copy ctor
  ~CItemField() {if (m_Length > 0) delete[] m_Data;}

  CItemField &operator=(const CItemField &that);

  void Set(const CMyString &value, BlowFish *bf);
  void Set(const unsigned char* value, unsigned int length, BlowFish *bf);

  void Get(CMyString &value, BlowFish *bf) const;
  void Get(unsigned char *value, unsigned int &length, BlowFish *bf) const;
  unsigned char GetType() const {return m_Type;}
  unsigned int GetLength() const {return m_Length;}
  bool IsEmpty() const {return m_Length == 0;}
  void Empty();

private:
  //Number of 8 byte blocks needed for size
  int GetBlockSize(int size) const;

  unsigned char m_Type; // const except for assignment operator
  unsigned int m_Length;
  unsigned char *m_Data;
};


#endif
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:

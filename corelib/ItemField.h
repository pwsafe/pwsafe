// ItemField.h
//-----------------------------------------------------------------------------

#if !defined ItemField_h
#define ItemField_h

#include "MyString.h"

//-----------------------------------------------------------------------------

/*
 * CItemField contains the data for a given CitemData field in encrypted
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

private:
  //Actual encryption/decryption
  void EncryptData(const CMyString &plain, BlowFish *bf);
  void DecryptData(CMyString &plain, BlowFish *bf) const;

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

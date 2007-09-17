#include "UnknownField.h"

#include "Util.h"

UnknownFieldEntry::UnknownFieldEntry(unsigned char t, size_t s,
                                                 unsigned char *d)
{
  uc_Type = t;
  st_length =s;
  if (d != NULL) {
    ASSERT(s != 0);
    uc_pUField = new unsigned char[s];
    memcpy(uc_pUField, d, s);
  } else
    uc_pUField = NULL;
}


UnknownFieldEntry::~UnknownFieldEntry()
{
  if (st_length > 0 && uc_pUField != NULL) {
    trashMemory((void *)uc_pUField, st_length);
    delete[] uc_pUField;
    st_length = 0;
    uc_pUField = NULL;
  }
}

UnknownFieldEntry::UnknownFieldEntry(const UnknownFieldEntry &that)
  : uc_Type(that.uc_Type), st_length(that.st_length)
{
  if (that.uc_pUField != NULL) {
    ASSERT(that.st_length != 0);
    uc_pUField = new unsigned char[st_length];
    memcpy(uc_pUField, that.uc_pUField, st_length);
  } else
    uc_pUField = NULL;
}

UnknownFieldEntry &UnknownFieldEntry::operator=(const UnknownFieldEntry &that)
{
  if (this != &that) {
    uc_Type = that.uc_Type;
    st_length = that.st_length;
    if (uc_pUField != NULL) {
      ASSERT(st_length != 0);
      trashMemory(uc_pUField, st_length);
    }
    uc_pUField = new unsigned char[st_length];
    memcpy(uc_pUField, that.uc_pUField, st_length);
  }
  return *this;
}

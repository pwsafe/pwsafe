// UUIDGen.h
// Silly class for generating UUIDs
// Each instance has its own unique value, 
// which can be accessed as an array of bytes or as a human-readable
// ASCII string.
//

#ifndef __UUIDGEN_H
#define __UUIDGEN_H

typedef unsigned char uuid_array_t[16];
typedef unsigned char uuid_str_t[37]; //"204012e6-600f-4e01-a5eb-515267cb0d50"

#include "PwsPlatform.h"

class CUUIDGen {
 public:
  CUUIDGen(); // UUID generated at creation time
  CUUIDGen(const uuid_array_t &); // for storing an existing UUID
  ~CUUIDGen();
  void GetUUID(uuid_array_t &) const;
  void GetUUIDStr(uuid_str_t &) const;
 private:
  UUID uuid;
};

#endif /* __UUIDGEN_H */

/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
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
#include <memory> // for memcmp

class CUUIDGen {
 public:
  CUUIDGen(); // UUID generated at creation time
  CUUIDGen(const uuid_array_t &uuid_array); // for storing an existing UUID
  ~CUUIDGen();
  void GetUUID(uuid_array_t &uuid_array) const;
  // Following is for map<> compare function
  struct ltuuid {
    bool operator()(const CUUIDGen &u1, const CUUIDGen &u2) const
    {
      return std::memcmp(&u1.uuid,
                         &u2.uuid, sizeof(u1.uuid)) < 0;
    }
  };
 private:
  UUID uuid;
};

#endif /* __UUIDGEN_H */

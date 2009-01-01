/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// UUIDGen.h
// Silly class for generating UUIDs
// Each instance has its own unique value, 
// which can be accessed as an array of bytes or as a human-readable
// ASCII string.
//

#ifndef __UUIDGEN_H
#define __UUIDGEN_H

#ifdef _WIN32
typedef unsigned char uuid_array_t[16];
#else
#include <uuid/uuid.h> // aptitude install uuid-dev
typedef uuid_t uuid_array_t;
typedef uuid_t UUID;
#endif

#include <memory> // for memcmp
#include <iostream>
#include "PwsPlatform.h"
#include "StringX.h"

#include <vector>

class CUUIDGen
{
public:
  CUUIDGen(); // UUID generated at creation time
  CUUIDGen(const uuid_array_t &uuid_array, bool canonic = false); // for storing an existing UUID
  CUUIDGen(const StringX &s); // s is a hex string as returned by GetHexStr()
  ~CUUIDGen();
  void GetUUID(uuid_array_t &uuid_array) const;
  StringX GetHexStr() const ; // e.g., "204012e6600f4e01a5eb515267cb0d50"
  // Following is for map<> compare function
  struct ltuuid {
    bool operator()(const CUUIDGen &u1, const CUUIDGen &u2) const
    {
      return std::memcmp(&u1.uuid,
        &u2.uuid, sizeof(u1.uuid)) < 0;
    }
  };

  friend std::ostream &operator<<(std::ostream &os, const CUUIDGen &uuid);
  friend std::wostream &operator<<(std::wostream &os, const CUUIDGen &uuid);
private:
  UUID uuid;
  mutable bool m_canonic;
};

std::ostream &operator<<(std::ostream &os, const CUUIDGen &uuid);
std::wostream &operator<<(std::wostream &os, const CUUIDGen &uuid);

typedef std::vector<CUUIDGen> UUIDList;
typedef UUIDList::iterator UUIDListIter;

#endif /* __UUIDGEN_H */

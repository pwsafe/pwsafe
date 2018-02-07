/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// UUID.h
// Wrapper class for UUIDs, generating and converting them to/from
// various representations.
// Each instance has its own unique value, 
// which can be accessed as an array of bytes or as a human-readable
// ASCII string.
//

#ifndef __UUID_H
#define __UUID_H

#ifdef _WIN32
typedef unsigned char uuid_array_t[16];
#else
#include <uuid/uuid.h> // aptitude install uuid-dev
typedef uuid_t uuid_array_t;
typedef uuid_t UUID;
#endif

#include <memory> // for memcmp
#include <iostream>
#include "typedefs.h"
#include "../core/StringX.h"

#include <vector>

namespace pws_os {
class CUUID
{
public:
  CUUID(); // UUID generated at creation time
  CUUID(const CUUID &uuid);
  CUUID(const uuid_array_t &ua, bool canonic = false); // for storing an existing UUID
  CUUID(const StringX &s); // s is a hex string as returned by cast to StringX
  static const CUUID &NullUUID(); // singleton all-zero
  ~CUUID();

  // Following get Array Representation of the uuid:
  void GetARep(uuid_array_t &ua) const;
  const uuid_array_t *GetARep() const; // internally allocated, deleted in d'tor
  
  CUUID &operator=(const CUUID &that);
  operator StringX() const; // GetHexStr, e.g., "204012e6600f4e01a5eb515267cb0d50"
  bool operator==(const CUUID &that) const;
  bool operator!=(const CUUID &that) const { return !(*this == that); }
  bool operator<(const CUUID &that) const;

  friend std::ostream &operator<<(std::ostream &os, const pws_os::CUUID &uuid);
  friend std::wostream &operator<<(std::wostream &os, const pws_os::CUUID &uuid);

private:
  UUID m_uuid;
  mutable uuid_array_t *m_ua; // for GetUUID();
  mutable bool m_canonic;
};

std::ostream &operator<<(std::ostream &os, const CUUID &uuid);
std::wostream &operator<<(std::wostream &os, const CUUID &uuid);
} // end of pws_os namespace

typedef std::vector<pws_os::CUUID> UUIDVector;
typedef UUIDVector::iterator UUIDVectorIter;

#endif /* __UUID_H */

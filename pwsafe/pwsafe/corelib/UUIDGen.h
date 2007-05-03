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
// which can be accessed as an array of bytes. 
//

#ifndef __UUIDGEN_H
#define __UUIDGEN_H

/// binary representation of a UUID; should not be changed to wide characters
typedef unsigned char uuid_array_t[16];

#include "PwsPlatform.h"

class CUUIDGen {
 public:
  CUUIDGen(); // UUID generated at creation time
  CUUIDGen(const uuid_array_t &); // for storing an existing UUID
  ~CUUIDGen();
  void GetUUID(uuid_array_t &) const;
 private:
  GUID guid;
};

#endif /* __UUIDGEN_H */

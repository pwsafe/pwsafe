/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */

// UnknownField.h

#pragma once

#include "Util.h"

// Unknown Field structure
struct UnknownFieldEntry {
  unsigned char uc_Type;
  size_t st_length;
  unsigned char * uc_pUField;

  UnknownFieldEntry() :uc_Type(0), st_length(0), uc_pUField(NULL) {}
  ~UnknownFieldEntry()
    {
      if (st_length > 0 && uc_pUField != NULL) {
        trashMemory((void *)uc_pUField, st_length);
        free(uc_pUField);
        st_length = 0;
        uc_pUField = NULL;
      }
    }

  // copy c'tor and assignment operator, standard idioms
  UnknownFieldEntry(const UnknownFieldEntry &that)
    : uc_Type(that.uc_Type), st_length(that.st_length)
  {
    if (that.st_length > 0 && that.uc_pUField != NULL) {
      uc_pUField = (unsigned char *)malloc(st_length + sizeof(TCHAR));
      memset(uc_pUField, 0x00, st_length + sizeof(TCHAR));
      memcpy(uc_pUField, that.uc_pUField, st_length);
    } else
      uc_pUField = NULL;
  }

  UnknownFieldEntry &operator=(const UnknownFieldEntry &that)
    { if (this != &that) {
        uc_Type = that.uc_Type;
        st_length = that.st_length;
        uc_pUField = that.uc_pUField;
      }
      return *this;
    }
};

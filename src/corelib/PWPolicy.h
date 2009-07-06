/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// PWPolicy.h
// Struct encapsulating the Password generation policy: length,
// how many digits, upper/lowercase characters, etc.
//-----------------------------------------------------------------------------

#ifndef __PWPOLICY_H
#define __PWPOLICY_H

#include "StringX.h"

struct PWPolicy {
  unsigned short flags;
  int length;
  int digitminlength;
  int lowerminlength;
  int symbolminlength;
  int upperminlength;

  // For Password Policy flag definitions - see PWSprefs.h
  PWPolicy() : flags(0), length(0), 
    digitminlength(0), lowerminlength(0),
    symbolminlength(0), upperminlength(0) {}

  // copy c'tor and assignment operator, standard idioms
  PWPolicy(const PWPolicy &that)
    : flags(that.flags), length(that.length),
    digitminlength(that.digitminlength),
    lowerminlength(that.lowerminlength),
    symbolminlength(that.symbolminlength),
    upperminlength(that.upperminlength) {}

  PWPolicy &operator=(const PWPolicy &that)
  {
    if (this != &that) {
      flags = that.flags;
      length = that.length;
      digitminlength = that.digitminlength;
      lowerminlength = that.lowerminlength;
      symbolminlength = that.symbolminlength;
      upperminlength = that.upperminlength;
    }
    return *this;
  }

  bool operator==(const PWPolicy &that) const;

  bool operator!=(const PWPolicy &that) const
  { return !(*this == that);}

  void Empty()
  { 
    flags = 0; length = 0;
    digitminlength = lowerminlength = 0;
    symbolminlength = upperminlength = 0;
  }
  // Following calls CPasswordCharPool::MakePassword()
  // with arguments matching 'this' policy, or,
  // preference-defined policy if this->flags == 0
  StringX MakeRandomPassword() const;
};

#endif
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:

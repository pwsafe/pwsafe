/*
* Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// PWPolicy.h
//-----------------------------------------------------------------------------

#ifndef __PWPOLICY_H
#define __PWPOLICY_H

#include "StringX.h"

// Password Policy related stuff
enum {DEFAULT_POLICY = 0, NAMED_POLICY, SPECIFIC_POLICY};
enum {DEFAULT_SYMBOLS = 0, OWN_SYMBOLS = 1}; // TBD - try to eliminate, as this should be implicit

/**
 * PWPolicy is a struct encapsulating the Password generation policy:
 * The policy consists of the following attributes:
 * - The length of the password to be generated
 * - Which type of characters to use from the following: lowercase, uppercase,
 *   digits, symbols
 * - Whether or not to restrict the generated password to hexadecimal (candidate for removal?)
 * - Whether or not to use only characters that are easily distinguishable
 *   (i.e., no '1', 'l', 'I', etc.)
 * - Whether or not to make a password that's pronounceable (and hence easier to memorize)
 *
 */
struct PWPolicy {
  enum {
    UseLowercase        = 0x8000, // Can have a minimum length field
    UseUppercase        = 0x4000, // Can have a minimum length field
    UseDigits           = 0x2000, // Can have a minimum length field
    UseSymbols          = 0x1000, // Can have a minimum length field
    UseHexDigits        = 0x0800,
    UseEasyVision       = 0x0400,
    MakePronounceable   = 0x0200,
    Unused              = 0x01ff};

  unsigned short flags; // bitwise-or of the above
  int length;
  int digitminlength;
  int lowerminlength;
  int symbolminlength;
  int upperminlength;
  StringX symbols; // policy-specific set of 'symbol' characters
  size_t usecount; // how many entries use this policy?

  PWPolicy() : flags(0), length(0), 
               digitminlength(0), lowerminlength(0),
               symbolminlength(0), upperminlength(0),
               symbols(_T("")), usecount(0) {}

  // copy c'tor and assignment operator, standard idioms
  PWPolicy(const PWPolicy &that)
    : flags(that.flags), length(that.length),
      digitminlength(that.digitminlength),
      lowerminlength(that.lowerminlength),
      symbolminlength(that.symbolminlength),
      upperminlength(that.upperminlength),
      symbols(that.symbols), usecount(that.usecount) {}

  PWPolicy &operator=(const PWPolicy &that)
  {
    if (this != &that) {
      flags  = that.flags;
      length = that.length;
      digitminlength  = that.digitminlength;
      lowerminlength  = that.lowerminlength;
      symbolminlength = that.symbolminlength;
      upperminlength  = that.upperminlength;
      symbols = that.symbols;
      // don't care about usecount!
    }
    return *this;
  }

  bool operator==(const PWPolicy &that) const;

  bool operator!=(const PWPolicy &that) const
  {return !(*this == that);}

  void Empty()
  { 
    flags = 0; length = 0;
    digitminlength  = lowerminlength = 0;
    symbolminlength = upperminlength = 0;
    symbols = _T(""); usecount = 0;
  }

  // Following calls CPasswordCharPool::MakePassword()
  // with arguments matching 'this' policy, or,
  // preference-defined policy if this->flags == 0
  StringX MakeRandomPassword() const;

  void SetToDefaults(); // from Prefs
  void UpdateDefaults(bool bUseCopy = false) const; // to prefs
  typedef void (*RowPutter)(int row, const stringT &name, const stringT &value, void *table);
  void Policy2Table(RowPutter rp, void *table);
private:
  void Normalize(); // make policy internally consistent
};

//-----------------------------------------------------------------
// Structure for maintaining history of policy changes for Undo/Redo

// Change flags
enum  CPP_FLAGS {CPP_INVALID = 0, CPP_ADD = 1, CPP_DELETE = 2, CPP_MODIFIED = 4};

struct st_PSWDPolicyChange {
  StringX name;
  PWPolicy st_pp_save;
  PWPolicy st_pp_new;
  CPP_FLAGS flags;

  st_PSWDPolicyChange()
  : name(_T("")), flags(CPP_INVALID)
  {
    st_pp_save.Empty();
    st_pp_new.Empty();
  }

  st_PSWDPolicyChange(const StringX &in_name, CPP_FLAGS in_flags,
          const PWPolicy &in_st_pp_original,
          const PWPolicy &in_st_pp_new)
  : name(in_name), st_pp_save(in_st_pp_original),
  st_pp_new(in_st_pp_new), flags(in_flags)
  {}

  st_PSWDPolicyChange(const st_PSWDPolicyChange &that)
    : name(that.name), st_pp_save(that.st_pp_save),
    st_pp_new(that.st_pp_new), flags(that.flags)
  {}

  st_PSWDPolicyChange &operator=(const st_PSWDPolicyChange &that)
  {
    if (this != &that) {
      name = that.name;
      flags = that.flags;
      st_pp_save = that.st_pp_save;
      st_pp_new = that.st_pp_new;
    }
    return *this;
  }

  bool operator==(const st_PSWDPolicyChange &that) const
  {
    if (this != &that) {
      if (name != that.name ||
          st_pp_save != that.st_pp_save ||
          st_pp_new != that.st_pp_new)
        return false;
    }
    return true;
  }

  bool operator!=(const st_PSWDPolicyChange &that) const
  {return !(*this == that);}

  void Empty()
  { 
    name.clear();
    flags = CPP_INVALID;
    st_pp_save.Empty();
    st_pp_new.Empty();
  }
};

#endif /* __PWPOLICY_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:

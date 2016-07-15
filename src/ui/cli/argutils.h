//
//  argutils.h
//  pwsafe-xcode6
//
//  Created by Saurav Ghosh on 19/06/16.
//  Copyright (c) 2016 Open Source Software. All rights reserved.
//

#ifndef __pwsafe_xcode6__argutils__
#define __pwsafe_xcode6__argutils__

#include "../../core/PWScore.h"
#include "../../core/Match.h"

#include "./strutils.h"

struct Restriction {
  CItemData::FieldType field;
  PWSMatch::MatchRule rule;
  std::wstring value;
  bool caseSensitive;
  Restriction(CItemData::FieldType f, PWSMatch::MatchRule r,
              const std::wstring &v, bool caseSensitive = false):
              field{f}, rule{r}, value{v}, caseSensitive{false}
  {}
  Restriction(): field{CItem::LAST_DATA}, rule{PWSMatch::MR_INVALID}, caseSensitive{false}
  {}
  bool valid() const { return field != CItem::LAST_DATA     &&
                              rule != PWSMatch::MR_INVALID  &&
                              !value.empty();
  }
};

struct UserArgs {
  UserArgs()  { fields.set(); }
  StringX safe;
  enum OpType {Unset, Import, Export, CreateNew, Search, Add,
               Diff, Sync, Merge} Operation{Unset};
  enum {Print, Delete, Update} SearchAction{Print};
  enum {Unknown, XML, Text} Format{Unknown};

  bool dry_run{false};

  // The arg taken by the main operation
  std::wstring opArg;
  
  // used for search, diff, etc.
  CItemData::FieldBits fields;
  Restriction subset;
  bool ignoreCase{false};
  bool confirmed{false};
  std::wstring opArg2;

  enum class DiffFmt { Unified, Context, SideBySide };
  DiffFmt dfmt{DiffFmt::Unified};
  unsigned int colwidth{60}; // for side-by-side diff

  void SetFields(const std::wstring &f);
  void SetSubset(const std::wstring &s);
  void SetMainOp(OpType op, const char *arg = nullptr) {
    if (Operation != Unset)
      throw std::invalid_argument("Only one main operation can be specified");
    Operation = op;
    if (arg)
      opArg = Utf82wstring(arg);
  }
};

CItemData::FieldType String2FieldType(const std::wstring& str);
PWSMatch::MatchRule Str2MatchRule( const std::wstring &s);



#endif /* defined(__pwsafe_xcode6__argutils__) */

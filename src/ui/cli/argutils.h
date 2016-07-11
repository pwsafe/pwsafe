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
  UserArgs():   Operation(Unset), SearchAction{Print}, Format(Unknown), ignoreCase{false}, confirmed{false},
                dfmt{DiffFmt::Unified}
  {
    fields.set();
  }
  StringX safe, fname;
  enum OpType {Unset, Import, Export, CreateNew, Search, Add, Diff} Operation;
  enum {Print, Delete, Update} SearchAction;
  enum {Unknown, XML, Text} Format;
  
  bool dry_run{false};

  // The arg taken by the main operation
  std::wstring opArg;
  
  // used for search, diff, etc.
  CItemData::FieldBits fields;
  Restriction subset;
  bool ignoreCase;
  bool confirmed;
  std::wstring opArg2;

  enum class DiffFmt { Unified, Context, SideBySide };
  DiffFmt dfmt;

  void SetFields(const std::wstring &f);
  void SetSubset(const std::wstring &s);
};

CItemData::FieldType String2FieldType(const std::wstring& str);
PWSMatch::MatchRule Str2MatchRule( const std::wstring &s);



#endif /* defined(__pwsafe_xcode6__argutils__) */

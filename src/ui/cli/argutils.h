//
//  argutils.h
//  pwsafe-xcode6
//
//  Created by Saurav Ghosh on 19/06/16.
//  Copyright (c) 2016 Open Source Software. All rights reserved.
//

#ifndef __pwsafe_xcode6__argutils__
#define __pwsafe_xcode6__argutils__

#include <regex>

#include "../../core/PWScore.h"
#include "../../core/Match.h"

struct UserArgs {
  UserArgs() : Operation(Unset), SearchAction{Print}, Format(Unknown), ignoreCase{false}, confirmed{false} {}
  StringX safe, fname;
  enum {Unset, Import, Export, CreateNew, Search, Add} Operation;
  enum {Print, Delete, Update} SearchAction;
  enum {Unknown, XML, Text} Format;
  
  // The arg taken by the main operation
  std::wstring opArg;
  
  // used for search
  std::wstring searchedFields;
  std::wstring searchedSubset;
  bool ignoreCase;
  bool confirmed;
  std::wstring opArg2;
};

CItemData::FieldType String2FieldType(const std::wstring& str);
PWSMatch::MatchRule Str2MatchRule( const std::wstring &s);

template <class CallbackType>
void Split(const std::wstring &str, const std::wstring &sep, CallbackType cb)
{
  // we have to create a temp variable like this, or else it crashes in Xcode 6
  std::wregex r(sep);
  std::wsregex_token_iterator pos(str.cbegin(), str.cend(), r, -1);
  std::wsregex_token_iterator end;
  for_each( pos, end, cb );
}



#endif /* defined(__pwsafe_xcode6__argutils__) */

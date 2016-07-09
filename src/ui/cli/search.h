//
//  search.h
//  pwsafe-xcode6
//
//  Created by Saurav Ghosh on 19/06/16.
//  Copyright (c) 2016 Open Source Software. All rights reserved.
//

#ifndef __pwsafe_xcode6__search__
#define __pwsafe_xcode6__search__

#include <string>
#include "../../os/UUID.h"
#include "../../core/PWScore.h"
#include "./argutils.h"


struct SearchAction
{
  SearchAction() {}
  virtual void operator()(const pws_os::CUUID &uuid, const CItemData &data) = 0;
  virtual int Execute() = 0;
  
  SearchAction& operator=(const SearchAction&) = delete;
  SearchAction& operator=(const SearchAction&&) = delete;
  SearchAction( const SearchAction& ) = delete;
  SearchAction( const SearchAction&& ) = delete;
  
};

void SearchForEntries(PWScore &core, const std::wstring &searchText, bool ignoreCase,
                      const Restriction &restrictToEntries, const CItemData::FieldBits &fieldsToSearch,
                      SearchAction &cb);

#endif /* defined(__pwsafe_xcode6__search__) */

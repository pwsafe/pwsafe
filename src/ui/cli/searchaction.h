//
//  searchaction.h
//  pwsafe-xcode6
//
//  Created by Saurav Ghosh on 19/06/16.
//  Copyright (c) 2016 Open Source Software. All rights reserved.
//

#ifndef __pwsafe_xcode6__searchaction__
#define __pwsafe_xcode6__searchaction__

#include <vector>

#include "../../core/PWScore.h"

class UserArgs;

struct SearchAction
{
  bool confirmed{false};
  std::vector<const CItemData *> itemids;

  SearchAction(bool conf): confirmed{conf} {}
  virtual void OnMatch(const pws_os::CUUID &uuid, const CItemData &data) final;
  virtual int Execute() = 0;

  SearchAction& operator=(const SearchAction&) = delete;
  SearchAction& operator=(const SearchAction&&) = delete;
  SearchAction( const SearchAction& ) = delete;
  SearchAction( const SearchAction&& ) = delete;

};

SearchAction* CreateSearchAction(PWScore *core, const UserArgs &ua);

#endif /* defined(__pwsafe_xcode6__searchaction__) */

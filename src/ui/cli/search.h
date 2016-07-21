//
//  search.h
//  pwsafe-xcode6
//
//  Created by Saurav Ghosh on 19/06/16.
//  Copyright (c) 2016 Open Source Software. All rights reserved.
//

#ifndef __pwsafe_xcode6__search__
#define __pwsafe_xcode6__search__

class PWScore;
class UserArgs;

int Search(PWScore &core, const UserArgs &ua);
int SaveAfterSearch(PWScore &core, const UserArgs &ua);

#endif /* defined(__pwsafe_xcode6__search__) */

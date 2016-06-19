//
//  searchaction.h
//  pwsafe-xcode6
//
//  Created by Saurav Ghosh on 19/06/16.
//  Copyright (c) 2016 Open Source Software. All rights reserved.
//

#ifndef __pwsafe_xcode6__searchaction__
#define __pwsafe_xcode6__searchaction__

class PWScore;
class SearchAction;

SearchAction* CreateSearchAction(int action, PWScore *core, const std::wstring &actionArgs, bool confirmed);

#endif /* defined(__pwsafe_xcode6__searchaction__) */

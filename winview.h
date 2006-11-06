/*
 * Copyright (c) 2003-2006 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */

#ifndef WINVIEW_H
#define WINVIEW_H

#include "view.h"

//  Contains methods to display user interface, called by model

class WinView : public View
{		
public:
	static View * Instance()
	{
		if ( _instance == 0 )
			_instance = new WinView();
		return _instance;
	}
	virtual ~WinView() {};
	
	virtual int     PromptUserForCombination();
	virtual int     ShowList() { return 0; };

protected:
	WinView() {};

private:
	static View * _instance;
	
};


#endif //  WINVIEW_H

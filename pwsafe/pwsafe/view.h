/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */

#ifndef VIEW_H
#define VIEW_H

//  Contains methods to display user interface, called by model

class View
{
public:
	static View * Instance() {};
	View() {};
	virtual ~View() {};

	virtual int PromptUserForCombination() = 0;
	virtual int ShowList()  = 0;
protected:
	static View * _instance;
};


#endif //  VIEW_H

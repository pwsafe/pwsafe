/*
 * Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */


#ifndef MODEL_H
#define MODEL_H

#include "ThisMfcApp.h"
#include "view.h"


//  Contains methods called by view (GUI)

class Model
{
public:
	static Model * Instance()
	{
		if ( _instance == 0 )
			_instance = new Model();
		return _instance;
	}
	~Model();

	//  Kick off application
	void Main();

	const char * GetDatabaseName() { return 0; };
	int      CheckSafeCombination(const char *combo);

protected:
	//  Don't allow anyone to create this object
	Model() {};
private:
	static Model * _instance;
	View	     * _view;

};


#endif //  MODEL_H

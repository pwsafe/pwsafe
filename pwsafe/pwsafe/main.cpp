/*
 * Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

#include "model.h"
#include "view.h"

#ifdef WIN32
/*
  The one and only ThisMfcApp object.  In the MFC world, creating
  this global object is what starts the application.
*/
#include "ThisMfcApp.h"

ThisMfcApp app;

#else

int main( int argc, char** argv)
{
	Model * model = Model::Instance();

	model->Main();

	return 0;
}

#endif

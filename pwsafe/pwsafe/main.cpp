
#include "model.h"
#include "view.h"
#include "corelib/global.h"

//  Global object for use by everyone
Global global;

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

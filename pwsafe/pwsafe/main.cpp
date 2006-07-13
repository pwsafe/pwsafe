
#include "model.h"
#include "view.h"

#ifdef WIN32
/*
  The one and only ThisMfcApp object.  In the MFC world, creating
  this global object is what starts the application.
*/
#include "ThisMfcApp.h"
#include "resource.h"

ThisMfcApp app;

UINT statustext[] =
{
	IDS_STATCOMPANY,
	IDS_STAT_NUM_IN_DB
};

#else

int main( int argc, char** argv)
{
	Model * model = Model::Instance();

	model->Main();

	return 0;
}

#endif


#include "model.h"
#ifdef WIN32
#include "winview.h"
#endif

Model * Model::_instance = 0;

void Model::Main()
{
#ifdef WIN32
	//  Create windows-specific view here.
	_view = WinView::Instance();
#endif

	_view->PromptUserForCombination();

}

int CheckSafeCombination(const char *combo)
{
	//  Fake code for now
	if ( *combo )
		return 1;

	return 0;
};


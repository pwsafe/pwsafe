

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

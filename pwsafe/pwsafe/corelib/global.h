
#ifndef GLOBAL_H
#define GLOBAL_H

//  Temporary global object, used to separate out pwsafe logic from
//  MFC logic

#include "MyString.h"
#include "Util.h"

class Global
{
public:
	Global() {};
	~Global() {};

	CMyString m_passkey; // the main one, in memory?!? yikes {jpr}

};

extern Global global; // XXX should this be a Singleton?

#endif  //  GLOBAL_H

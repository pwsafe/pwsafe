
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

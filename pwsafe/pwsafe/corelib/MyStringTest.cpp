#include "MyStringTest.h"
#include <iostream>
using namespace std;

int main()
{
   CMyStringTest t;
   t.setStream(&cout); // must precede run()
   t.run();
   t.report();
	return 0;
}


// MyStringTest.h: Use the test class
#include "test.h"
#include "MyString.h"

class CMyStringTest : public Test
{
    CMyString str;
    CMyString str2;
	 CString comparator;

public:
    CMyStringTest()
    {
		 comparator = "test string";
    }

    void run()
    {
       // The tests to run:
       testConstructors();

       // add more tests...

       // last test
       testTrash();

    }

    // first test
    void testConstructors()
    {
       // start with an empty string
		 _test(str.GetLength() == 0);
       _test(str.GetBuffer(0) == "");
       str.ReleaseBuffer();

       // set the string from a CString
       str = CMyString(comparator);

		 _test(str.GetLength() == comparator.GetLength());
       _test(str.GetBuffer(comparator.GetLength()) == comparator);
       str.ReleaseBuffer();
    }

    // add more tests...


    // last test
    void testTrash()
    {
       str.Trash();
       _test(str.GetBuffer(comparator.GetLength()) != comparator);
    }
};



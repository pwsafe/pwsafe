// suite.cpp

#include "suite.h"
#include <iostream>
#include <stdexcept>
#include <cassert>
using namespace std;

class TestSuiteError : public logic_error
{
public:
    TestSuiteError(const string& s = "")
        : logic_error(s)
    {}
};

void Suite::addTest(Test* t) throw(TestSuiteError)
{
    // Make sure test has a stream:
    if (t == 0)
        throw TestSuiteError("Null test in Suite::addTest");
    else if (m_osptr != 0 && t->getStream() == 0)
        t->setStream(m_osptr);

    m_tests.push_back(t);
    t->reset();
}

void Suite::addSuite(const Suite& s) throw(TestSuiteError)
{
    for (size_t i = 0; i < s.m_tests.size(); ++i)
        addTest(s.m_tests[i]);
}

void Suite::free()
{
    // This is not a destructor because tests
    // don't have to be on the heap.
    for (size_t i = 0; i < m_tests.size(); ++i)
    {
        delete m_tests[i];
        m_tests[i] = 0;
    }
}

void Suite::run()
{
    reset();
    for (size_t i = 0; i < m_tests.size(); ++i)
    {
        assert(m_tests[i]);
        m_tests[i]->run();
    }
}


long Suite::report() const
{
    if (m_osptr)
    {
        long totFail = 0;
        *m_osptr << "Suite \"" << m_name << "\"\n=======";
        size_t i;
        for (i = 0; i < m_name.size(); ++i)
            *m_osptr << '=';
        *m_osptr << "=\n";

        for (i = 0; i < m_tests.size(); ++i)
        {
            assert(m_tests[i]);
            totFail += m_tests[i]->report();
        }

        *m_osptr << "=======";
        for (i = 0; i < m_name.size(); ++i)
            *m_osptr << '=';
        *m_osptr << "=\n";
        return totFail;
    }
    else
        return getNumFailed();
}

long Suite::getNumPassed() const
{
    long totPass = 0;
    for (size_t i = 0; i < m_tests.size(); ++i)
    {
        assert(m_tests[i]);
        totPass += m_tests[i]->getNumPassed();
    }
    return totPass;
}

long Suite::getNumFailed() const
{
    long totFail = 0;
    for (size_t i = 0; i < m_tests.size(); ++i)
    {
        assert(m_tests[i]);
        totFail += m_tests[i]->getNumFailed();
    }
    return totFail;
}

void Suite::reset()
{
    for (size_t i = 0; i < m_tests.size(); ++i)
    {
        assert(m_tests[i]);
        m_tests[i]->reset();
    }
}

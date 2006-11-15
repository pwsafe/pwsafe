/*
 * Copyright (c) 2003-2006 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
// test.h

#ifndef TEST_H
#define TEST_H

#include <string>
#include <iosfwd>

using std::string;
using std::ostream;

// The following have underscores because they are macros
// (and it's impolite to usurp other users' functions!).
// For consistency, _succeed() also has an underscore.
#define _test(cond) do_test(cond, #cond, __FILE__, __LINE__)
#define _fail(str) do_fail(str, __FILE__, __LINE__)

class Test
{
public:
    Test(ostream* osptr = 0);
    virtual ~Test(){}
    virtual void run() = 0;

    long getNumPassed() const;
    long getNumFailed() const;
    const ostream* getStream() const;
    void setStream(ostream* osptr);
    
    void _succeed();
    long report() const;
    virtual void reset();

protected:
    void do_test(bool cond, const string& lbl,
                 const char* fname, long lineno);
    void do_fail(const string& lbl,
                 const char* fname, long lineno);

private:
    ostream* m_osptr;
    long m_nPass;
    long m_nFail;

    // Disallowed:
    Test(const Test&);
    Test& operator=(const Test&);
};

inline
Test::Test(ostream* osptr)
{
    m_osptr = osptr;
    m_nPass = m_nFail = 0;
}

inline
long Test::getNumPassed() const
{
    return m_nPass;
}

inline
long Test::getNumFailed() const
{
    return m_nFail;
}

inline
const ostream* Test::getStream() const
{
    return m_osptr;
}

inline
void Test::setStream(ostream* osptr)
{
    m_osptr = osptr;
}

inline
void Test::_succeed()
{
    ++m_nPass;
}

inline
void Test::reset()
{
    m_nPass = m_nFail = 0;
}
#endif


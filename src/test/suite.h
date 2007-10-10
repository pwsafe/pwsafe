/*
 * Copyright (c) 2003-2006 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */
// suite.h

#ifndef SUITE_H
#define SUITE_H

#include "test.h"   // includes <string>, <iosfwd>
#include <vector>
using std::string;
using std::ostream;
using std::vector;

class TestSuiteError;

class Suite
{
public:
    Suite(const string& name, ostream* osptr = 0);

    string getName() const;
    long getNumPassed() const;
    long getNumFailed() const;
    const ostream* getStream() const;
    void setStream(ostream* osptr);
    
    void addTest(Test* t) throw (TestSuiteError);
    void addSuite(const Suite&) throw(TestSuiteError);
    void run();     // Calls Test::run() repeatedly
    long report() const;
    void free();    // deletes tests

private:
    string m_name;
    ostream* m_osptr;
    vector<Test*> m_tests;
    void reset();

    // Disallowed ops:
    Suite(const Suite&);
    Suite& operator=(const Suite&);
};

inline
Suite::Suite(const string& name, ostream* osptr)
     : m_name(name)
{
    m_osptr = osptr;
}

inline
string Suite::getName() const
{
    return m_name;
}

inline
const ostream* Suite::getStream() const
{
    return m_osptr;
}

inline
void Suite::setStream(ostream* osptr)
{
    m_osptr = osptr;
}

#endif

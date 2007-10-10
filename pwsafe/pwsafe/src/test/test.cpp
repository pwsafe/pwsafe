/*
 * Copyright (c) 2003-2006 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */
// test.cpp

#include "test.h"
#include <iostream>
#include <typeinfo>     // Visual Studio requires /GR""

#ifdef _MSC_VER
// Allow return-less mains:
#pragma warning(disable: 4541)
#endif

using namespace std;

void Test::do_test(bool cond, const std::string& lbl,
                   const char* fname, long lineno)
{
    if (!cond)
        do_fail(lbl, fname, lineno);
    else
        _succeed();
}

void Test::do_fail(const std::string& lbl,
                   const char* fname, long lineno)
{
    ++m_nFail;
    if (m_osptr)
    {
        *m_osptr << typeid(*this).name()
                             << " failure: (" << lbl << ") , "
                                 << fname
                 << " (line " << lineno << ")\n";
    }
}

long Test::report() const
{
    if (m_osptr)
        {
            *m_osptr << "Test \"" 
                         << typeid(*this).name() << "\":\n"
                     << "\tPassed: " << m_nPass
                     << "\tFailed: " << m_nFail
                     << endl;
        }
    return m_nFail;
}

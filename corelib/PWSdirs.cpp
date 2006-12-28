/*
 * Copyright (c) 2003-2006 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
#include <stdlib.h> // for getenv()
#include "PWSdirs.h"
/**
 * Provide directories used by application
 * The functions here return values that cause the application
 * to default to current behaviour, UNLESS the U3 (www.u3.com) env. vars
 * are defined, in which case values corresponding to the U3 layout are used,
 * as follows (corrsponding to the layout in the .u3p package):
 *
 * GetSafeDir()   Default database directory:
 *                U3_DEVICE_DOCUMENT_PATH\My Safes\
 * GetConfigDir() Location of configuration file:
 *                U3_APP_DATA_PATH
 * GetXMLDir()    Location of XML .xsd and .xsl files:
 *                U3_APP_DATA_PATH\xml\
 * GetHelpDir()   Location of help file(s):
 *                U3_DEVICE_EXEC_PATH
 * GetExeDir()    Location of executable:
 *                U3_HOST_EXEC_PATH
 */
//-----------------------------------------------------------------------------

CString PWSdirs::GetEnv(const char *env)
{
    ASSERT(env != NULL);
    CString retval;
#if _MSC_VER < 1400
    retval = getenv(env);
#else
    char* value;
    size_t requiredSize;

    getenv_s(&requiredSize, NULL, 0, env);
    if (requiredSize > 0) {
        value = new char[requiredSize];
        ASSERT(value);
        if (value != NULL) {
            getenv_s( &requiredSize, value, requiredSize, env);
            retval = value;
            delete[] value;
            // make sure path has trailing '\'
            // yeah, this breaks non-dir getenvs - sosueme
            if (retval[retval.GetLength()-1] != TCHAR('\\'))
                retval += _T("\\");
        }
    }
#endif
    return retval;
}

CString PWSdirs::GetMFNDir()
{
    // returns the directory part of ::GetModuleFileName()
    TCHAR acPath[MAX_PATH + 1];

    if ( GetModuleFileName( NULL, acPath, MAX_PATH + 1 ) != 0) {
        // guaranteed file name of at least one character after path '\'
        *(_tcsrchr(acPath, _T('\\')) + 1) = _T('\0');
    } else {
        acPath[0] = TCHAR('\\'); acPath[1] = TCHAR('\0');
    }
    return CString(acPath);
    
}

CString PWSdirs::GetSafeDir()
{
    // returns empty string unless U3 environment detected
    CString retval(GetEnv("U3_DEVICE_DOCUMENT_PATH"));
    if (!retval.IsEmpty())
        retval += "My Safes\\";
    return retval;
}

CString PWSdirs::GetConfigDir()
{
    // returns directory of executable unless U3 environment detected
    CString retval(GetEnv("U3_APP_DATA_PATH"));
    if (retval.IsEmpty()) {
        retval = GetMFNDir();
    }
    return retval;
}

CString PWSdirs::GetXMLDir()
{
    CString retval(GetEnv("U3_APP_DATA_PATH"));
    if (!retval.IsEmpty())
        retval += "\\xml\\";
    else {
        retval = GetMFNDir();
    }
        return retval;
}

CString PWSdirs::GetHelpDir()
{
    CString retval(GetEnv("U3_DEVICE_EXEC_PATH"));
    if (retval.IsEmpty()) {
        retval = GetMFNDir();
    }
    return retval;
}

CString PWSdirs::GetExeDir()
{
    CString retval(GetEnv("U3_HOST_EXEC_PATH"));
    if (retval.IsEmpty()) {
        retval = GetMFNDir();
    }
    return retval;
}

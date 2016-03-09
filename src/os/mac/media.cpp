/**
* \file Mac-specific implementation of media.h
*/

#include "../media.h"
#include "../file.h"
#include "../utf8conv.h"

#include <cstdlib>
#include <cstdio>

using namespace std;

stringT pws_os::GetMediaType(const stringT &sfilename)
{
    return _T("unknown");
}

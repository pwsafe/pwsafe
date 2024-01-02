/*
* Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
* Linux-specific implementation of media.h
*/

#include "../media.h"
#include "../file.h"
#include "../utf8conv.h"

#include <magic.h>

using namespace std;

stringT pws_os::GetMediaType(const stringT &sfilename) {
    /**
     * Using libmagic instead of external 'file' command
     */

    if (!pws_os::FileExists(sfilename))
        return _T("unknown");

    const char *smimeType;
    magic_t magic_cookie;
    stringT wcmimeType;

    //MAGIC_MIME_TYPE -> return mime type of the file
    magic_cookie = magic_open(MAGIC_MIME_TYPE);

    if (magic_cookie == nullptr) {
        pws_os::Trace(L"GetMediaType - Error during libmagic initialization");
        return _T("unknown");
    }

    //Load default magic db
    if (magic_load(magic_cookie, nullptr) != 0) {
        pws_os::Trace(L"GetMediaType - Cannot load libmagic database - %s", magic_error(magic_cookie));
        magic_close(magic_cookie);
        return _T("unknown");
    }

    smimeType = magic_file(magic_cookie, pws_os::tomb(sfilename).c_str());

    wcmimeType = pws_os::towc(smimeType);
    //close at end since result is lost after that
    magic_close(magic_cookie);

    return wcmimeType;
}

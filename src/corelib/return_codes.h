/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/*
  Return codes used by all routines within corelib
*/

#ifndef __RETURN_CODES_H
#define __RETURN_CODES_H

namespace PWSRC {
  enum {
    SUCCESS = 0,                              // 0x00 = 0
    FAILURE,                                  // 0x01 = 1

    CANT_OPEN_FILE = 0x10,                    // 0x10 = 16
    NOT_PWS3_FILE,                            // 0x11 = 17
    UNSUPPORTED_VERSION,                      // 0x12 = 18
    WRONG_VERSION,                            // 0x13 = 19
    UNKNOWN_VERSION,                          // 0x14 = 20
    INVALID_FORMAT,                           // 0x15 = 21
    END_OF_FILE,                              // 0x16 = 22
    ALREADY_OPEN,                             // 0x17 = 23

    WRONG_PASSWORD = 0x20,                    // 0x20 = 32
    BAD_DIGEST,                               // 0x21 = 33

    USER_CANCEL = 0x30,                       // 0x30 = 48
    USER_EXIT,                                // 0x31 = 49

    XML_FAILED_VALIDATION = 0x40,             // 0x40 = 64
    XML_FAILED_IMPORT,                        // 0x41 = 65
    LIMIT_REACHED,                            // 0x42 = 66
    NO_ENTRIES_EXPORTED,                      // 0x43 = 67
    DB_HAS_DUPLICATES,                        // 0x44 = 68
    OK_WITH_ERRORS,                           // 0x45 = 69

    XML_LOAD_FAILED = 0x50,                   // 0x50 = 80
    XML_NODE_NOT_FOUND,                       // 0x51 = 81
    XML_PUT_TEXT_FAILED,                      // 0x52 = 82
    XML_SAVE_FAILED,                          // 0x53 = 83
    
    HEADERS_INVALID = 0x80,                   // 0x80 = 128
    BAD_ATTACHMENT,                           // 0x81 = 129
    END_OF_DATA,                              // 0x82 = 130

    UNIMPLEMENTED = 0xF0,                     // 0xF0 = 240
  };
};

#endif /* __RETURN_CODES_H */

#ifndef _VERSION_H_
#define _VERSION_H_

// Using Tortoise SVN's program SubWCRev.exe to copy this to "version.h"
// for use by the Resource Compiler and replace "WCREV" by the Revision number
// SubWCRev.exe path\to\working\copy version.in version.h

// Format: Major, Minor, Build, Revision
//   Build  = 0 for all Formally Released versions
//   Build != 0 for all Intermediate versions
// Full information shown in AboutBox; only Major & Minor are displayed in initial dialog

#define FILEVER         3, 4, 1, 1058
#define PRODUCTVER      3, 4, 1, 1058
#define STRFILEVER     "3, 4, 1, 1058"
#define STRPRODUCTVER  "3, 4, 1, 1058"

#endif // _VERSION_H_

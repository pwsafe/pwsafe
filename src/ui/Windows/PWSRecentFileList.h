/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/*
* This class works around a problem with CRecentFileList's implementation of Add,
* when the file being added doesn't exist (e.g., on a USB stick that has been
* removed). See the .cpp file for more details.
*
* Also, overrides ReadList & WriteList to work with our preference mechanism
* (that is, not necessarily via the registry)
*/

#pragma once

class CPWSRecentFileList : public CRecentFileList
{
public:
  CPWSRecentFileList(UINT nStart, LPCWSTR lpszSection,
    LPCWSTR lpszEntryFormat, int nSize,
    int nMaxDispLen = AFX_ABBREV_FILENAME_LEN)
    : CRecentFileList(nStart, lpszSection, lpszEntryFormat,
    nSize, nMaxDispLen){}

  virtual void ReadList();    // reads from registry or config file
  virtual void WriteList();   // writes to registry or config file
  bool IsMRUEmpty() const {return GetNumUsed() == 0;}

  // Despite what the documentation implies, GetSize returns maximum size not number used
  int GetMaxSize() const { return GetSize(); }
  int GetNumUsed() const;
};

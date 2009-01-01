/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#include "stdafx.h"
#include <afxadv.h>

#include "PWSRecentFileList.h"
#include "corelib/PWSprefs.h"
#include "resource2.h" // for ID_FILE_MRU_*

/*
  PasswordSafe uses a config file instead of registry when possible.
  This means that ReadList/WriteList need to use our config mechanism,
  rather than m'soft's code, which uses the registry.

  Also, it turns out that m'soft is a bit uptight when using Add() for
  files that don't exist, such as might happen when restoring the list
  from config, and one of the list members was on removable media. Therefore,
  we take care in ReadList not to call Add. (Previous versions of this class
  overrode Add(), which was a bit of an overkill...)
*/

void CPWSRecentFileList::ReadList()
{
  PWSprefs *pref = PWSprefs::GetInstance();
  // reads from registry or config file
  if (pref->IsUsingRegistry()) {
    CRecentFileList::ReadList();
  } else {
    const int nMRUItems = pref->GetPref(PWSprefs::MaxMRUItems);
    ASSERT(nMRUItems == m_nSize);
    stringT *arrNames = new stringT[nMRUItems];
    pref->GetMRUList(arrNames);
    for (int i = 0; i < nMRUItems; i++)
      m_arrNames[i] = arrNames[i].c_str();
    delete[] arrNames;
  }
}

void CPWSRecentFileList::WriteList()
{
  PWSprefs *pref = PWSprefs::GetInstance();
  // writes to registry or config file
  if (pref->IsUsingRegistry()) {
    CRecentFileList::WriteList();
  } else {
    const int num_MRU = GetSize();
    const int max_MRU = ID_FILE_MRU_ENTRYMAX - ID_FILE_MRU_ENTRY1;
    stringT *csMRUFiles = new stringT[num_MRU];

    for (int i = 0; i < num_MRU; i++) {
      csMRUFiles[i] = (*this)[i];
      Trim(csMRUFiles[i]);
    }

    pref->SetMRUList(csMRUFiles, num_MRU, max_MRU);
    delete[] csMRUFiles;
  }
}

bool CPWSRecentFileList::IsMRUEmpty()
{
  CString csMRUFileName;

  return GetDisplayName(csMRUFileName, 0, _T(""), 0, TRUE) == FALSE;
}

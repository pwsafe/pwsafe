/*
* Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#include "stdafx.h"
#include <afxadv.h>

#include "PWSRecentFileList.h"
#include "winutils.h"
#include "core/PWSprefs.h"
#include "os/file.h"
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
    int nMRUItems = pref->GetPref(PWSprefs::MaxMRUItems);
    ASSERT(nMRUItems == m_nSize);
    std::vector<stringT> arrNames;
    arrNames.reserve(nMRUItems);
    pref->GetMRUList(arrNames);
    if (nMRUItems > arrNames.size()) // can happen, e.g., if cfg file is missing
      nMRUItems = static_cast<int>(arrNames.size());
    for (int i = 0; i < nMRUItems; i++) {
      std::wstring path = arrNames[i].c_str();
      pws_os::AddDrive(path);
      m_arrNames[i] = path.c_str();
    }
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
    std::vector<stringT> sMRUFiles;
    sMRUFiles.reserve(num_MRU);

    for (int i = 0; i < num_MRU; i++) {
      sMRUFiles.push_back((*this)[i].GetString());
      if (!sMRUFiles[i].empty()) {
        Trim(sMRUFiles[i]);
        WinUtil::RelativizePath(sMRUFiles[i]);
      }
    }

    pref->SetMRUList(sMRUFiles, max_MRU);
  }
}

int CPWSRecentFileList::GetNumUsed() const
{
  int n = 0;
  const int num_MRU = GetSize();
  // workaround silly MFC lack of const operator[]:
  CPWSRecentFileList *self = const_cast<CPWSRecentFileList *>(this);

  for (int i = 0; i < num_MRU; i++) {
  if (!(*self)[i].IsEmpty())
      n++;
  }

  return n;
}

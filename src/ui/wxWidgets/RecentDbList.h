/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file RecentDbList.h
 * 
 */

#ifndef _RECENTDBLIST_H_
#define _RECENTDBLIST_H_

// For wxFileHistory
#include <wx/docview.h>

#include "wxUtilities.h"

class RecentDbList : public wxFileHistory
{
public:
    RecentDbList() : wxFileHistory(PWSprefs::GetInstance()->GetPref(PWSprefs::MaxMRUItems))
    {} 

    void RemoveFile(const wxString& file) {
      for (size_t idx = 0, max = GetCount(); idx < max; ++idx) {
        if (GetHistoryFile(idx) == file) {
          RemoveFileFromHistory(idx);
          return;
        }
      }
    }

    void GetAll(wxArrayString& sa) const {
      for (size_t idx = 0, max = GetCount(); idx < max; ++idx)
        sa.Add(GetHistoryFile(idx));
    }

    void Save() const {
      std::vector<stringT> mruList;
      for (size_t idx = 0, max = GetCount(); idx < max; ++idx) {
        mruList.push_back(stringT(GetHistoryFile(idx)));
      }
      PWSprefs::GetInstance()->SetMRUList(mruList, PWSprefs::GetInstance()->GetPref(PWSprefs::MaxMRUItems));
    }

    void Load() {
      PWSprefs* prefs = PWSprefs::GetInstance();
      const auto nExpected = prefs->GetPref(PWSprefs::MaxMRUItems);
      std::vector<stringT> mruList(nExpected);
      const auto nFound = prefs->GetMRUList(mruList);
      wxASSERT(nExpected >= nFound);
      for (unsigned int idx = 0; idx < nFound; ++idx) {
        if (!mruList[idx].empty())
          AddFileToHistory(towxstring(mruList[idx]));
      }
    }

    void Clear() {
      while(GetCount() > 0)
        RemoveFileFromHistory(0);
    }
};

#endif // _RECENTDBLIST_H_

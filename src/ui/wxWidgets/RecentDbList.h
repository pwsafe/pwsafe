/*
 * Copyright (c) 2003-2025 Rony Shapiro <ronys@pwsafe.org>.
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

    // Calling wxFileHistory::AddFileToHistory() crashes if maxFiles is initialized to 0.
    void AddFileToHistory(const wxString& file) {
      if (GetMaxFiles() > 0)
        wxFileHistory::AddFileToHistory(file);
    }

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
      std::vector<stringT> mruList;
      const auto nFound = prefs->GetMRUList(mruList);
      wxASSERT(nExpected >= nFound);

      // wxFileHistory::AddFileToHistory() appears to add to the begining of the list,
      // so we iterate backward to keep the order the same.
      for (auto iter = mruList.rbegin(); iter != mruList.rend(); iter++) {
        wxFileHistory::AddFileToHistory(towxstring(*iter));
      }
    }

    void Clear() {
      while(GetCount() > 0)
        RemoveFileFromHistory(0);
    }
};

#endif // _RECENTDBLIST_H_

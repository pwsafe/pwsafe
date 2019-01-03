/*
 * Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file RecentDBList.h
 * 
 */

#ifndef __RECENTDBLIST_H__
#define __RECENTDBLIST_H__

// For wxFileHistory
#include <wx/docview.h>
#include "./wxutils.h"

class CRecentDBList : public wxFileHistory
{
public:
    CRecentDBList() : wxFileHistory(PWSprefs::GetInstance()->GetPref(PWSprefs::MaxMRUItems))
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
      PWSprefs::GetInstance()->SetMRUList(&mruList[0], static_cast<int>(mruList.size()), 
                  PWSprefs::GetInstance()->GetPref(PWSprefs::MaxMRUItems));
    }
    
    void Load() {
      PWSprefs* prefs = PWSprefs::GetInstance();
      const auto nExpected = prefs->GetPref(PWSprefs::MaxMRUItems);
      std::vector<stringT> mruList(nExpected);
      const int nFound = prefs->GetMRUList(&mruList[0]);
      wxASSERT(nExpected >= nFound);
      for (int idx = 0; idx < nFound; ++idx) {
        if (!mruList[idx].empty())
          AddFileToHistory(towxstring(mruList[idx]));
      }
    }
    
    void Clear() {
      while(GetCount() > 0)
        RemoveFileFromHistory(0);
    }
};

#endif

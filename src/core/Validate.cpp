/*
* Copyright (c) 2003-2026 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// Validate.cpp

#include "PWScore.h"
#include "core.h"
#include "Report.h"
#include "ItemData.h"
#include "UTF8Conv.h"
#include "PWSLog.h"
#include "Validate.h"

#include "os/debug.h"

#include <vector>
#include <algorithm>

// For Validate only
struct st_GroupTitleUser2 {
  StringX group;
  StringX title;
  StringX user;
  StringX newtitle;

  st_GroupTitleUser2() {}

  st_GroupTitleUser2(const StringX &g, const StringX &t, const StringX &u,
    const StringX &n)
  : group(g), title(t), user(u), newtitle(n) {}

  st_GroupTitleUser2(const st_GroupTitleUser2 &other)
    : group(other.group), title(other.title), user(other.user), newtitle(other.newtitle) {}

  st_GroupTitleUser2 &operator=(const st_GroupTitleUser2 &that) {
    if (this != &that) {
      group = that.group; title = that.title; user = that.user;
      newtitle = that.newtitle;
    }
    return *this;
  }
};

// For Validate only
struct st_AttTitle_Filename {
  StringX title;
  StringX filename;

  st_AttTitle_Filename() {}

  st_AttTitle_Filename(const StringX &t, const StringX &fn)
    : title(t), filename(fn) {}

  st_AttTitle_Filename(const st_AttTitle_Filename &other)
    : title(other.title), filename(other.filename) {}

  st_AttTitle_Filename &operator=(const st_AttTitle_Filename &that) {
    if (this != &that) {
      title = that.title; filename = that.filename;
    }
    return *this;
  }
};

static bool GTUCompareV2(const st_GroupTitleUser2 &gtu1, const st_GroupTitleUser2 &gtu2)
{
  if (gtu1.group != gtu2.group)
    return gtu1.group.compare(gtu2.group) < 0;
  else if (gtu1.title != gtu2.title)
    return gtu1.title.compare(gtu2.title) < 0;
  else if (gtu1.user != gtu2.user)
    return gtu1.user.compare(gtu2.user) < 0;
  else
    return gtu1.newtitle.compare(gtu2.newtitle) < 0;
}

bool PWScore::Validate(const size_t iMAXCHARS, CReport *pRpt, st_ValidateResults &st_vr)
{
  /*
     1. Check PWH is valid
     2. Check that the 2 mandatory fields are present (Title & Password)
     3. Check group/title/user must be unique.
     4. Check that no text field has more than iMAXCHARS, that can displayed
        in the GUI's text control.
     5. Validate Empty Groups
     6. For attachments (V4):
     6.1 Check that each ATTREF in a data entry has a corresponding ItemAtt
     6.2 Check that each ItemAtt has a corresponding "owner" ItemData

     Note:
     m_pwlist is implemented as a map keyed on UUIDs, each entry is
     guaranteed to have a unique uuid. The uniqueness invariant
     should be enforced elsewhere.
     (ReadFile during Open and Import have already ensured UUIDs are unique
     and valid)
  */

  PWS_LOGIT_ARGS("iMAXCHARS=%d; pRpt=%p", iMAXCHARS, pRpt);

  int n = -1;
  size_t uimaxsize(0);

  stringT cs_Error;
  pws_os::Trace(_T("Start validation\n"));

  st_GroupTitleUser st_gtu;
  GTUSet setGTU;
  GTUSetPair pr_gtu;
  std::vector<st_GroupTitleUser> vGTU_UUID, vGTU_EmptyPassword, vGTU_PWH, vGTU_TEXT,
                                 vGTU_ALIASES, vGTU_SHORTCUTS;
  std::vector<st_GroupTitleUser2> vGTU_NONUNIQUE, vGTU_EmptyTitle;
  std::vector<st_GroupTitleUser> vGTU_MissingAtt;
  std::vector<st_AttTitle_Filename> vOrphanAtt;
  std::set<pws_os::CUUID> sAtts;

  ItemListIter iter;

  for (iter = m_pwlist.begin(); iter != m_pwlist.end(); iter++) {
    CItemData &ci = iter->second;
    CItemData fixedItem(ci);
    bool bFixed(false);

    n++;

    // Fix GTU uniqueness - can't do this in a CItemData member function as it causes
    // circular includes:
    //  "ItemData.h" would need to include "coredefs.h", which needs to include "ItemData.h"!
    StringX sxgroup(ci.GetGroup()), sxtitle(ci.GetTitle()), sxuser(ci.GetUser());
    st_gtu.group = sxgroup;
    st_gtu.title = sxtitle;
    st_gtu.user = sxuser;

    if (sxtitle.empty()) {
      // This field is mandatory!
      // Change it and insert into a std::set which guarantees uniqueness
      int i = 0;
      StringX sxnewtitle(sxtitle);
      do {
        i++;
        Format(sxnewtitle, IDSC_MISSINGTITLE, i);
        st_gtu.title = sxnewtitle;
        pr_gtu =  setGTU.insert(st_gtu);
      } while (!pr_gtu.second);

      fixedItem.SetTitle(sxnewtitle);

      bFixed = true;
      vGTU_EmptyTitle.push_back(st_GroupTitleUser2(sxgroup, sxtitle, sxuser, sxnewtitle));
      st_vr.num_empty_titles++;
      sxtitle = sxnewtitle;
    } else {
      // Title was not empty
      // Insert into a std::set which guarantees uniqueness
      pr_gtu = setGTU.insert(st_gtu);
      if (!pr_gtu.second) {
        // Already have this group/title/user entry
        int i = 0;
        StringX s_copy, sxnewtitle(sxtitle);
        do {
          i++;
          Format(s_copy, IDSC_DUPLICATENUMBER, i);
          sxnewtitle = sxtitle + s_copy;
          st_gtu.title = sxnewtitle;
          pr_gtu =  setGTU.insert(st_gtu);
        } while (!pr_gtu.second);

        fixedItem.SetTitle(sxnewtitle);

        bFixed = true;
        vGTU_NONUNIQUE.push_back(st_GroupTitleUser2(sxgroup, sxtitle, sxuser, sxnewtitle));
        st_vr.num_duplicate_GTU_fixed++;
        sxtitle = sxnewtitle;
      }
    }

    // Test if Password is present as it is mandatory! was fixed
    if (ci.GetPassword().empty()) {
      StringX sxMissingPassword;
      LoadAString(sxMissingPassword, IDSC_MISSINGPASSWORD);
      fixedItem.SetPassword(sxMissingPassword);

      bFixed = true;
      vGTU_EmptyPassword.push_back(st_GroupTitleUser(sxgroup, sxtitle, sxuser));
      st_vr.num_empty_passwords++;
    }

    // Test if Password History was fixed
    if (!fixedItem.ValidatePWHistory()) {
      bFixed = true;
      vGTU_PWH.push_back(st_GroupTitleUser(sxgroup, sxtitle, sxuser));
      st_vr.num_PWH_fixed++;
    }

    // Note excessively sized text fields
    if (iMAXCHARS > 0) {
      bool bEntryHasBigField(false);
      for (auto uc = static_cast<unsigned char>(CItem::GROUP);
           uc < static_cast<unsigned char>(CItem::LAST_DATA); uc++) {
        if (CItemData::IsTextField(uc)) {
          StringX sxvalue = ci.GetFieldValue(static_cast<CItemData::FieldType>(uc));
          if (sxvalue.length() > iMAXCHARS && (!m_bIsReadOnly && !ci.IsProtected())) {
            bEntryHasBigField = true;
            //  We don't truncate the field, but if we did, then the the code would be:
            //  fixedItem.SetFieldValue((CItemData::FieldType)uc, sxvalue.substr(0, iMAXCHARS))
            break;
          }
        }
      }

      if (bEntryHasBigField) {
        uimaxsize = std::max(uimaxsize, ci.GetSize());
        vGTU_TEXT.push_back(st_GroupTitleUser(sxgroup, sxtitle, sxuser));
        st_vr.num_excessivetxt_found++;
      }
    }

    // Attachment Reference check (6.1)
    if (ci.HasAttRef()) {
      sAtts.insert(ci.GetAttUUID());
      if (!HasAtt(ci.GetAttUUID())) {
        vGTU_MissingAtt.push_back(st_GroupTitleUser(ci.GetGroup(),
                                                    ci.GetTitle(),
                                                    ci.GetUser()));
        st_vr.num_missing_att++;
        // Fix the problem:
        fixedItem.ClearAttUUID();
        bFixed = true;
      }
    }

    // Empty group can't have entries!
    // This removes the empty group if it is an exact match to this entry's group
    std::vector<StringX>::iterator itEG;
    itEG = std::find(m_vEmptyGroups.begin(), m_vEmptyGroups.end(), sxgroup);
    if (itEG != m_vEmptyGroups.end()) {
      m_vEmptyGroups.erase(itEG);
    }

    // This remove the empty group if it contains this entry in one of its subgroups
    // Need to use reverse iterator so that can erase elements and still
    // iterate the vector but erase only takes a normal iterator!
    auto ritEG = m_vEmptyGroups.rbegin();
    while (ritEG != m_vEmptyGroups.rend()) {
      StringX sxEGDot = *ritEG + L".";
      ritEG++;
      if (sxgroup.length() > sxEGDot.length() &&
          _tcsncmp(sxEGDot.c_str(), sxgroup.c_str(), sxEGDot.length()) == 0) {
        ritEG = std::vector<StringX>::reverse_iterator(m_vEmptyGroups.erase(ritEG.base()));
      }
    }

    if (bFixed) {
      // Mark as modified
      fixedItem.SetStatus(CItemData::ES_MODIFIED);
      // We assume that this is run during file read. If not, then we
      // need to run using the Command mechanism for Undo/Redo.
      m_pwlist[fixedItem.GetUUID()] = fixedItem;
    }
  } // iteration over m_pwlist

  // Validate Empty Groups don't have empty sub-groups
  if (!m_vEmptyGroups.empty()) {
    std::sort(m_vEmptyGroups.begin(), m_vEmptyGroups.end());
    std::vector<size_t> viDelete;
    for (size_t ieg = 0; ieg < m_vEmptyGroups.size() - 1; ieg++) {
      StringX sxEG = m_vEmptyGroups[ieg] + L".";
      if (sxEG == m_vEmptyGroups[ieg + 1].substr(0, sxEG.length())) {
        // Can't be empty as has empty sub-group. Save to delete later
        viDelete.push_back(ieg);
      }
    }

    if (!viDelete.empty()) {
      // Remove non-empty groups
      std::vector<size_t>::reverse_iterator rit;
      for (rit = viDelete.rbegin(); rit != viDelete.rend(); rit++) {
        m_vEmptyGroups.erase(m_vEmptyGroups.begin() + *rit);
      }
    }
  }

  // Check for orphan attachments (6.2)
  std::vector<pws_os::CUUID> orphans;
  for (auto att_iter = m_attlist.begin(); att_iter != m_attlist.end(); att_iter++) {
    if (sAtts.find(att_iter->first) == sAtts.end()) {
      st_AttTitle_Filename stATFN;
      stATFN.title = att_iter->second.GetTitle();
      stATFN.filename = att_iter->second.GetFileName();
      vOrphanAtt.push_back(stATFN);
      st_vr.num_orphan_att++;
      // we will remove the orphan since the user may not see the report, so the alternative
      // is to leave sensitive data floating around on the user's disk.
      orphans.push_back(att_iter->first);
    }
  }

  // actually remove orphans, since we can't do it while traversing m_attlist
  for (auto &orphan : orphans)
    m_attlist.erase(orphan);

  if (st_vr.TotalIssues() != 0 && pRpt != nullptr) {
    // Only report problems if a. There are some and b. We have a report file
    if ((st_vr.num_invalid_UUIDs == 0 && st_vr.num_duplicate_UUIDs == 0)) {
      // As both zero, we didn't put error header in report - so do it now
      pRpt->WriteLine();
      LoadAString(cs_Error, IDSC_VALIDATE_ERRORS);
      pRpt->WriteLine(cs_Error);
    }

    if (!vGTU_EmptyTitle.empty()) {
      std::sort(vGTU_EmptyTitle.begin(), vGTU_EmptyTitle.end(), GTUCompareV2);
      pRpt->WriteLine();
      LoadAString(cs_Error, IDSC_VALIDATE_EMPTYTITLE);
      pRpt->WriteLine(cs_Error);
      for (size_t iv = 0; iv < vGTU_EmptyTitle.size(); iv++) {
        st_GroupTitleUser2 &gtu2 = vGTU_EmptyTitle[iv];
        stringT cs_newtitle;
        Format(cs_newtitle, IDSC_VALIDATE_ENTRY2, gtu2.newtitle.c_str());
        Format(cs_Error, IDSC_VALIDATE_ENTRY,
               gtu2.group.c_str(), gtu2.title.c_str(), gtu2.user.c_str(), cs_newtitle.c_str());
        pRpt->WriteLine(cs_Error);
      }
    }

    if (!vGTU_EmptyPassword.empty()) {
      StringX sxMissingPassword;
      LoadAString(sxMissingPassword, IDSC_MISSINGPASSWORD);
      std::sort(vGTU_EmptyPassword.begin(), vGTU_EmptyPassword.end(), GTUCompareV1);
      pRpt->WriteLine();
      Format(cs_Error, IDSC_VALIDATE_EMPTYPSWD, sxMissingPassword.c_str());
      pRpt->WriteLine(cs_Error);
      for (size_t iv = 0; iv < vGTU_EmptyPassword.size(); iv++) {
        st_GroupTitleUser &gtu = vGTU_EmptyPassword[iv];
        Format(cs_Error, IDSC_VALIDATE_ENTRY,
               gtu.group.c_str(), gtu.title.c_str(), gtu.user.c_str(), _T(""));
        pRpt->WriteLine(cs_Error);
      }
    }

    if (!vGTU_NONUNIQUE.empty()) {
      std::sort(vGTU_NONUNIQUE.begin(), vGTU_NONUNIQUE.end(), GTUCompareV2);
      pRpt->WriteLine();
      LoadAString(cs_Error, IDSC_VALIDATE_DUPLICATES);
      pRpt->WriteLine(cs_Error);
      for (size_t iv = 0; iv < vGTU_NONUNIQUE.size(); iv++) {
        st_GroupTitleUser2 &gtu2 = vGTU_NONUNIQUE[iv];
        stringT cs_newtitle;
        Format(cs_newtitle, IDSC_VALIDATE_ENTRY2, gtu2.newtitle.c_str());
        Format(cs_Error, IDSC_VALIDATE_ENTRY,
               gtu2.group.c_str(), gtu2.title.c_str(), gtu2.user.c_str(), cs_newtitle.c_str());
        pRpt->WriteLine(cs_Error);
      }
    }

    if (!vGTU_UUID.empty()) {
      std::sort(vGTU_UUID.begin(), vGTU_UUID.end(), GTUCompareV1);
      pRpt->WriteLine();
      LoadAString(cs_Error, IDSC_VALIDATE_BADUUID);
      pRpt->WriteLine(cs_Error);
      for (size_t iv = 0; iv < vGTU_UUID.size(); iv++) {
        st_GroupTitleUser &gtu = vGTU_UUID[iv];
        Format(cs_Error, IDSC_VALIDATE_ENTRY,
               gtu.group.c_str(), gtu.title.c_str(), gtu.user.c_str(), _T(""));
        pRpt->WriteLine(cs_Error);
      }
    }

    if (!vGTU_PWH.empty()) {
      std::sort(vGTU_PWH.begin(), vGTU_PWH.end(), GTUCompareV1);
      pRpt->WriteLine();
      LoadAString(cs_Error, IDSC_VALIDATE_PWH);
      pRpt->WriteLine(cs_Error);
      for (size_t iv = 0; iv < vGTU_PWH.size(); iv++) {
        st_GroupTitleUser &gtu = vGTU_PWH[iv];
        Format(cs_Error, IDSC_VALIDATE_ENTRY,
               gtu.group.c_str(), gtu.title.c_str(), gtu.user.c_str(), _T(""));
        pRpt->WriteLine(cs_Error);
      }
    }

    if ((!vGTU_ALIASES.empty() || !vGTU_SHORTCUTS.empty() || !vGTU_TEXT.empty()) &&
        pRpt != nullptr) {
      // We have warnings
      pRpt->WriteLine();
      LoadAString(cs_Error, IDSC_VALIDATE_WARNINGS);
      pRpt->WriteLine(cs_Error);
    }

    if (!vGTU_ALIASES.empty()) {
      std::sort(vGTU_ALIASES.begin(), vGTU_ALIASES.end(), GTUCompareV1);
      pRpt->WriteLine();
      stringT sxAlias;
      LoadAString(sxAlias, IDSC_FALIAS);
      Format(cs_Error, IDSC_VALIDATE_DEPS, sxAlias.c_str());
      pRpt->WriteLine(cs_Error);
      for (size_t iv = 0; iv < vGTU_ALIASES.size(); iv++) {
        st_GroupTitleUser &gtu = vGTU_ALIASES[iv];
        Format(cs_Error, IDSC_VALIDATE_ENTRY,
               gtu.group.c_str(), gtu.title.c_str(), gtu.user.c_str(), _T(""));
        pRpt->WriteLine(cs_Error);
      }
    }

    if (!vGTU_SHORTCUTS.empty()) {
      std::sort(vGTU_SHORTCUTS.begin(), vGTU_SHORTCUTS.end(), GTUCompareV1);
      pRpt->WriteLine();
      stringT sxShortcut;
      LoadAString(sxShortcut, IDSC_FSHORTCUT);
      Format(cs_Error, IDSC_VALIDATE_DEPS,  sxShortcut.c_str());
      pRpt->WriteLine(cs_Error);
      for (size_t iv = 0; iv < vGTU_SHORTCUTS.size(); iv++) {
        st_GroupTitleUser &gtu = vGTU_SHORTCUTS[iv];
        Format(cs_Error, IDSC_VALIDATE_ENTRY,
               gtu.group.c_str(), gtu.title.c_str(), gtu.user.c_str(), _T(""));
        pRpt->WriteLine(cs_Error);
      }
    }

    if (!vGTU_TEXT.empty()) {
      std::sort(vGTU_TEXT.begin(), vGTU_TEXT.end(), GTUCompareV1);
      pRpt->WriteLine();
      int units(0);
      uimaxsize >>= 10;    // make bytes -> KB
      if (uimaxsize > 999) {
        uimaxsize >>= 10;  // make KB -> MB
        units++;
      }
      Format(cs_Error, IDSC_VALIDATE_TEXT, iMAXCHARS, uimaxsize,
             units == 0 ? L"KB" : L"MB");
      pRpt->WriteLine(cs_Error);

      for (size_t iv = 0; iv < vGTU_TEXT.size(); iv++) {
        st_GroupTitleUser &gtu = vGTU_TEXT[iv];
        Format(cs_Error, IDSC_VALIDATE_ENTRY,
               gtu.group.c_str(), gtu.title.c_str(), gtu.user.c_str(), _T(""));
        pRpt->WriteLine(cs_Error);
      }
    }

    // Attachment-related issues
    if (st_vr.num_missing_att > 0) {
      pRpt->WriteLine();
      LoadAString(cs_Error, IDSC_VALIDATE_MISSING_ATT);
      pRpt->WriteLine(cs_Error);
      for_each(vGTU_MissingAtt.begin(), vGTU_MissingAtt.end(), [&](const st_GroupTitleUser &gtu) {
          Format(cs_Error, IDSC_VALIDATE_ENTRY,
                 gtu.group.c_str(), gtu.title.c_str(), gtu.user.c_str(), _T(""));
          pRpt->WriteLine(cs_Error);
      } );
    }

    if (st_vr.num_orphan_att > 0) {
      pRpt->WriteLine();
      LoadAString(cs_Error, IDSC_VALIDATE_ORPHAN_ATT);
      pRpt->WriteLine(cs_Error);
      stringT cs_NotSet;
      LoadAString(cs_NotSet, IDCS_VALIDATE_NOTSET);
      for_each(vOrphanAtt.begin(), vOrphanAtt.end(), [&](const st_AttTitle_Filename &stATFN) {
        stringT sTitle = stATFN.title.empty() ? cs_NotSet : stATFN.title.c_str();
        Format(cs_Error, IDSC_VALIDATE_ATTACHMENT, sTitle.c_str(), stATFN.filename.c_str());
        pRpt->WriteLine(cs_Error);
      } );
    }
  } // End of issues report handling

  pws_os::Trace(_T("End validation. %d entries processed\n"), n + 1);

  m_bUniqueGTUValidated = true;
  if (st_vr.TotalIssues() > 0) {
    m_DBCurrentState = DIRTY;
    return true;
  } else {
    return false;
  }
} 

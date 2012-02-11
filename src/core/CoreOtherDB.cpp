/*
* Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// file CoreOtherDB.cpp
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------
// 'Other' Database function - PWScore member functions
//     Compare, Merge & Synchronize
//-----------------------------------------------------------------
#include "PWScore.h"
#include "core.h"
#include "Util.h"
#include "Report.h"
#include "StringXStream.h"
#include "DBCompareData.h"

#include "os/typedefs.h"

#include <string>
#include <vector>
#include <algorithm>
#include <set>

using namespace std;

using pws_os::CUUID;

// hide w_char/char differences where possible:
#ifdef UNICODE
typedef std::wifstream ifstreamT;
typedef std::wofstream ofstreamT;
#else
typedef std::ifstream ifstreamT;
typedef std::ofstream ofstreamT;
#endif
typedef std::vector<stringT>::iterator viter;

const StringX sx_StartChevron(_T("\xab"));
const StringX sx_MiddleChevron(_T("\xbb \xab"));
const StringX sx_EndChevron(_T("\xbb"));

inline bool FieldsNotEqual(StringX a, StringX b, const bool bTreatWhiteSpaceasEmpty)
{
  if (bTreatWhiteSpaceasEmpty) { // m_treatwhitespaceasempty
    EmptyIfOnlyWhiteSpace(a);
    EmptyIfOnlyWhiteSpace(b);
  }
  return a != b;
}

void PWScore::Compare(PWScore *pothercore,
                      const CItemData::FieldBits &bsFields, const bool &subgroup_bset,
                      const bool &bTreatWhiteSpaceasEmpty,  const stringT &subgroup_name,
                      const int &subgroup_object, const int &subgroup_function,
                      CompareData &list_OnlyInCurrent, CompareData &list_OnlyInComp,
                      CompareData &list_Conflicts, CompareData &list_Identical)
{
  /*
  Purpose:
    Compare entries from comparison database (compCore) with current database (m_core)

  Algorithm:
    Foreach entry in current database {
      Find in comparison database - subject to subgroup checking
      if found {
        Compare
        if match
          OK
       else
          There are conflicts; note them & increment numConflicts
      } else {
        save & increment numOnlyInCurrent
      }
    }

    Foreach entry in comparison database {
      Find in current database - subject to subgroup checking
      if not found
        save & increment numOnlyInComp
    }
  */

  CItemData::FieldBits bsConflicts(0);
  st_CompareData st_data;
  int numOnlyInCurrent(0), numOnlyInComp(0), numConflicts(0), numIdentical(0);

  ItemListIter currentPos;
  for (currentPos = GetEntryIter();
       currentPos != GetEntryEndIter();
       currentPos++) {
    st_data.Empty();
    CItemData currentItem = GetEntry(currentPos);

    if (!subgroup_bset ||
        currentItem.Matches(std::wstring(subgroup_name), subgroup_object,
                            subgroup_function)) {
      st_data.group = currentItem.GetGroup();
      st_data.title = currentItem.GetTitle();
      st_data.user = currentItem.GetUser();

      StringX sx_original = sx_StartChevron +
                             st_data.group + sx_MiddleChevron +
                             st_data.title + sx_MiddleChevron +
                             st_data.user  + sx_EndChevron;

      // Update the Wizard page
      UpdateWizard(sx_original.c_str());

      ItemListIter foundPos = pothercore->Find(st_data.group,
                                               st_data.title, st_data.user);
      if (foundPos != pothercore->GetEntryEndIter()) {
        // found a match, see if all other fields also match
        // Difference flags:
        /*
         First byte (values in square brackets taken from ItemData.h)
         1... ....  NAME      [0x00] - n/a - depreciated
         .1.. ....  UUID      [0x01] - n/a - unique
         ..1. ....  GROUP     [0x02] - not checked - must be identical
         ...1 ....  TITLE     [0x03] - not checked - must be identical
         .... 1...  USER      [0x04] - not checked - must be identical
         .... .1..  NOTES     [0x05]
         .... ..1.  PASSWORD  [0x06]
         .... ...1  CTIME     [0x07] - not checked by default

         Second byte
         1... ....  PMTIME    [0x08] - not checked by default
         .1.. ....  ATIME     [0x09] - not checked by default
         ..1. ....  XTIME     [0x0a] - not checked by default
         ...1 ....  RESERVED  [0x0b] - not used
         .... 1...  RMTIME    [0x0c] - not checked by default
         .... .1..  URL       [0x0d]
         .... ..1.  AUTOTYPE  [0x0e]
         .... ...1  PWHIST    [0x0f]

         Third byte
         1... ....  POLICY    [0x10] - not checked by default
         .1.. ....  XTIME_INT [0x11] - not checked by default
         ..1. ....  RUNCMD    [0x12]
         ...1 ....  DCA       [0x13]
         .... 1...  EMAIL     [0x14]
         .... .1..  PROTECTED [0x15]
         .... ..1.  SYMBOLS   [0x16]
         .... ...1  SHIFTDCA   [0x17]

         Fourth byte
         1... ....  POLICYNAME [0x18] - not checked by default


         */
        bsConflicts.reset();
        StringX sxCurrentPassword, sxComparisonPassword;

        CItemData compItem = pothercore->GetEntry(foundPos);

        if (currentItem.GetEntryType() == CItemData::ET_ALIAS ||
            currentItem.GetEntryType() == CItemData::ET_SHORTCUT) {
          CItemData *pci_base = GetBaseEntry(&currentItem);
          sxCurrentPassword == pci_base->GetPassword();
        } else
          sxCurrentPassword == currentItem.GetPassword();

        if (compItem.GetEntryType() == CItemData::ET_ALIAS ||
            compItem.GetEntryType() == CItemData::ET_SHORTCUT) {
          CItemData *pci_base = pothercore->GetBaseEntry(&compItem);
          sxComparisonPassword == pci_base->GetPassword();
        } else
          sxComparisonPassword == compItem.GetPassword();

        if (bsFields.test(CItemData::PASSWORD) &&
            sxCurrentPassword != sxComparisonPassword)
          bsConflicts.flip(CItemData::PASSWORD);

        if (bsFields.test(CItemData::NOTES) &&
            FieldsNotEqual(currentItem.GetNotes(), compItem.GetNotes(), bTreatWhiteSpaceasEmpty))
          bsConflicts.flip(CItemData::NOTES);
        if (bsFields.test(CItemData::CTIME) &&
            currentItem.GetCTime() != compItem.GetCTime())
          bsConflicts.flip(CItemData::CTIME);
        if (bsFields.test(CItemData::PMTIME) &&
            currentItem.GetPMTime() != compItem.GetPMTime())
          bsConflicts.flip(CItemData::PMTIME);
        if (bsFields.test(CItemData::ATIME) &&
            currentItem.GetATime() != compItem.GetATime())
          bsConflicts.flip(CItemData::ATIME);
        if (bsFields.test(CItemData::XTIME) &&
            currentItem.GetXTime() != compItem.GetXTime())
          bsConflicts.flip(CItemData::XTIME);
        if (bsFields.test(CItemData::RMTIME) &&
            currentItem.GetRMTime() != compItem.GetRMTime())
          bsConflicts.flip(CItemData::RMTIME);
        if (bsFields.test(CItemData::XTIME_INT)) {
          int current_xint, comp_xint;
          currentItem.GetXTimeInt(current_xint);
          compItem.GetXTimeInt(comp_xint);
          if (current_xint != comp_xint)
            bsConflicts.flip(CItemData::XTIME_INT);
        }
        if (bsFields.test(CItemData::URL) &&
            FieldsNotEqual(currentItem.GetURL(), compItem.GetURL(),
                           bTreatWhiteSpaceasEmpty))
          bsConflicts.flip(CItemData::URL);
        if (bsFields.test(CItemData::AUTOTYPE) &&
            FieldsNotEqual(currentItem.GetAutoType(), compItem.GetAutoType(),
                           bTreatWhiteSpaceasEmpty))
          bsConflicts.flip(CItemData::AUTOTYPE);
        if (bsFields.test(CItemData::PWHIST) &&
            currentItem.GetPWHistory() != compItem.GetPWHistory())
          bsConflicts.flip(CItemData::PWHIST);
        if (bsFields.test(CItemData::POLICY) &&
            currentItem.GetPWPolicy() != compItem.GetPWPolicy())
          bsConflicts.flip(CItemData::POLICY);
        if (bsFields.test(CItemData::POLICYNAME) &&
            currentItem.GetPolicyName() != compItem.GetPolicyName())
          bsConflicts.flip(CItemData::POLICYNAME);
        if (bsFields.test(CItemData::RUNCMD) &&
            currentItem.GetRunCommand() != compItem.GetRunCommand())
          bsConflicts.flip(CItemData::RUNCMD);
        if (bsFields.test(CItemData::DCA) &&
            currentItem.GetDCA() != compItem.GetDCA())
          bsConflicts.flip(CItemData::DCA);
        if (bsFields.test(CItemData::SHIFTDCA) &&
            currentItem.GetShiftDCA() != compItem.GetShiftDCA())
          bsConflicts.flip(CItemData::SHIFTDCA);
        if (bsFields.test(CItemData::EMAIL) &&
            currentItem.GetEmail() != compItem.GetEmail())
          bsConflicts.flip(CItemData::EMAIL);
        if (bsFields.test(CItemData::PROTECTED) &&
            currentItem.GetProtected() != compItem.GetProtected())
          bsConflicts.flip(CItemData::PROTECTED);
        if (bsFields.test(CItemData::SYMBOLS) &&
            currentItem.GetSymbols() != compItem.GetSymbols())
          bsConflicts.flip(CItemData::SYMBOLS);

        st_data.uuid0 = currentPos->first;
        st_data.uuid1 = foundPos->first;
        st_data.bsDiffs = bsConflicts;
        st_data.indatabase = BOTH;
        st_data.unknflds0 = currentItem.NumberUnknownFields() > 0;
        st_data.unknflds1 = compItem.NumberUnknownFields() > 0;
        st_data.bIsProtected0 = currentItem.IsProtected();

        if (bsConflicts.any()) {
          numConflicts++;
          st_data.id = numConflicts;
          list_Conflicts.push_back(st_data);
        } else {
          numIdentical++;
          st_data.id = numIdentical;
          list_Identical.push_back(st_data);
        }
      } else {
        // didn't find any match...
        numOnlyInCurrent++;
        st_data.uuid0 = currentPos->first;
        st_data.uuid1 = CUUID::NullUUID();
        st_data.bsDiffs.reset();
        st_data.indatabase = CURRENT;
        st_data.unknflds0 = currentItem.NumberUnknownFields() > 0;
        st_data.unknflds1 = false;
        st_data.id = numOnlyInCurrent;
        list_OnlyInCurrent.push_back(st_data);
      }
    }
  } // iteration over our entries

  ItemListIter compPos;
  for (compPos = pothercore->GetEntryIter();
       compPos != pothercore->GetEntryEndIter();
       compPos++) {
    st_data.Empty();
    CItemData compItem = pothercore->GetEntry(compPos);

    if (!subgroup_bset ||
        compItem.Matches(std::wstring(subgroup_name), subgroup_object,
                         subgroup_function)) {
      st_data.group = compItem.GetGroup();
      st_data.title = compItem.GetTitle();
      st_data.user = compItem.GetUser();

      StringX sx_compare = sx_StartChevron +
                             st_data.group + sx_MiddleChevron +
                             st_data.title + sx_MiddleChevron +
                             st_data.user  + sx_EndChevron;

      // Update the Wizard page
      UpdateWizard(sx_compare.c_str());

      if (Find(st_data.group, st_data.title, st_data.user) ==
          GetEntryEndIter()) {
        // Didn't find any match...
        numOnlyInComp++;
        st_data.uuid0 = CUUID::NullUUID();
        st_data.uuid1 = compPos->first;
        st_data.bsDiffs.reset();
        st_data.indatabase = COMPARE;
        st_data.unknflds0 = false;
        st_data.unknflds1 = compItem.NumberUnknownFields() > 0;
        st_data.id = numOnlyInComp;
        list_OnlyInComp.push_back(st_data);
      }
    }
  } // iteration over other core's element
}

// Return whether first '«g» «t» «u»' is greater than the second '«g» «t» «u»'
// used in std::sort below.
// Need this as '»' is not in the correct lexical order for blank fields in entry
bool MergeSyncGTUCompare(const StringX &elem1, const StringX &elem2)
{
  StringX g1, t1, u1, g2, t2, u2, tmp1, tmp2;

  StringX::size_type i1 = elem1.find(L'\xbb');
  g1 = (i1 == StringX::npos) ? elem1 : elem1.substr(0, i1 - 1);
  StringX::size_type i2 = elem2.find(L'\xbb');
  g2 = (i2 == StringX::npos) ? elem2 : elem2.substr(0, i2 - 1);
  if (g1 != g2)
    return g1.compare(g2) < 0;

  tmp1 = elem1.substr(g1.length() + 3);
  tmp2 = elem2.substr(g2.length() + 3);
  i1 = tmp1.find(L'\xbb');
  t1 = (i1 == StringX::npos) ? tmp1 : tmp1.substr(0, i1 - 1);
  i2 = tmp2.find(L'\xbb');
  t2 = (i2 == StringX::npos) ? tmp2 : tmp2.substr(0, i2 - 1);
  if (t1 != t2)
    return t1.compare(t2) < 0;

  tmp1 = tmp1.substr(t1.length() + 3);
  tmp2 = tmp2.substr(t2.length() + 3);
  i1 = tmp1.find(L'\xbb');
  u1 = (i1 == StringX::npos) ? tmp1 : tmp1.substr(0, i1 - 1);
  i2 = tmp2.find(L'\xbb');
  u2 = (i2 == StringX::npos) ? tmp2 : tmp2.substr(0, i2 - 1);
  return u1.compare(u2) < 0;
}

// Merge flags indicating differing fields if group, title and user are identical
#define MRG_PASSWORD   0x8000
#define MRG_NOTES      0x4000
#define MRG_URL        0x2000
#define MRG_AUTOTYPE   0x1000
#define MRG_HISTORY    0x0800
#define MRG_POLICY     0x0400
#define MRG_XTIME      0x0200
#define MRG_XTIME_INT  0x0100
#define MRG_EXECUTE    0x0080
#define MRG_DCA        0x0040
#define MRG_EMAIL      0x0020
#define MRG_SYMBOLS    0x0010
#define MRG_SHIFTDCA   0x0008
#define MRG_POLICYNAME 0x0004
#define MRG_UNUSED     0x0003

stringT PWScore::Merge(PWScore *pothercore,
                       const bool &subgroup_bset,
                       const stringT &subgroup_name,
                       const int &subgroup_object, const int &subgroup_function,
                       CReport *pRpt)
{
  std::vector<StringX> vs_added;
  std::vector<StringX> vs_AliasesAdded;
  std::vector<StringX> vs_ShortcutsAdded;
  std::vector<StringX> vs_PoliciesAdded;
  std::map<StringX, StringX> mapRenamedPolicies;

  const StringX sxMerge_DateTime = PWSUtil::GetTimeStamp(true).c_str();
  /*
    Purpose:
      Merge entries from otherCore to m_core

    Algorithm:
      Foreach entry in otherCore
        Find in m_core based on group/title/username
          if match found {
            if all other fields match {
              no merge
            } else {
              add to m_core with new title suffixed with -merged-YYYYMMDD-HHMMSS
            }
          } else {
            add to m_core directly
          }
   */

  int numAdded = 0;
  int numConflicts = 0;
  int numAliasesAdded = 0;
  int numShortcutsAdded = 0;
  uuid_array_t base_uuid, new_base_uuid;
  bool bTitleRenamed(false);
  StringX sx_merged;
  LoadAString(sx_merged, IDSC_MERGED);

  MultiCommands *pmulticmds = MultiCommands::Create(this);
  Command *pcmd1 = UpdateGUICommand::Create(this, UpdateGUICommand::WN_UNDO,
                                            UpdateGUICommand::GUI_UNDO_MERGESYNC);
  pmulticmds->Add(pcmd1);

  ItemListConstIter otherPos;
  for (otherPos = pothercore->GetEntryIter();
       otherPos != pothercore->GetEntryEndIter();
       otherPos++) {
    CItemData otherItem = pothercore->GetEntry(otherPos);
    CItemData::EntryType et = otherItem.GetEntryType();

    // Handle Aliases and Shortcuts when processing their base entries
    if (otherItem.IsDependent())
      continue;

    if (subgroup_bset &&
        !otherItem.Matches(subgroup_name, subgroup_object, subgroup_function))
      continue;

    const StringX sx_otherGroup = otherItem.GetGroup();
    const StringX sx_otherTitle = otherItem.GetTitle();
    const StringX sx_otherUser = otherItem.GetUser();

    stringT str_timestring(_T(""));
    ItemListConstIter foundPos = Find(sx_otherGroup, sx_otherTitle, sx_otherUser);

    otherItem.GetUUID(base_uuid);
    memcpy(new_base_uuid, base_uuid, sizeof(new_base_uuid));
    bTitleRenamed = false;

    if (foundPos != GetEntryEndIter()) {
      // Found a match, see if other fields also match
      CItemData curItem = GetEntry(foundPos);

      // Can't merge into a protected entry.  If we were going to - add instead
      unsigned char ucprotected;
      curItem.GetProtected(ucprotected);

      stringT str_diffs(_T("")), str_temp;
      int diff_flags = 0;
      int cxtint, oxtint;
      time_t cxt, oxt;
      if (otherItem.GetPassword() != curItem.GetPassword()) {
        diff_flags |= MRG_PASSWORD;
        LoadAString(str_temp, IDSC_FLDNMPASSWORD);
        str_diffs += str_temp + _T(", ");
      }
      if (otherItem.GetNotes() != curItem.GetNotes()) {
        diff_flags |= MRG_NOTES;
        LoadAString(str_temp, IDSC_FLDNMNOTES);
        str_diffs += str_temp + _T(", ");
      }
      if (otherItem.GetURL() != curItem.GetURL()) {
        diff_flags |= MRG_URL;
        LoadAString(str_temp, IDSC_FLDNMURL);
        str_diffs += str_temp + _T(", ");
      }
      if (otherItem.GetAutoType() != curItem.GetAutoType()) {
        diff_flags |= MRG_AUTOTYPE;
        LoadAString(str_temp, IDSC_FLDNMAUTOTYPE);
        str_diffs += str_temp + _T(", ");
      }
      if (otherItem.GetPWHistory() != curItem.GetPWHistory()) {
        diff_flags |= MRG_HISTORY;
        LoadAString(str_temp, IDSC_FLDNMPWHISTORY);
        str_diffs += str_temp + _T(", ");
      }
      if (otherItem.GetPWPolicy() != curItem.GetPWPolicy()) {
        diff_flags |= MRG_POLICY;
        LoadAString(str_temp, IDSC_FLDNMPWPOLICY);
        str_diffs += str_temp + _T(", ");
      }
      otherItem.GetXTime(oxt);
      curItem.GetXTime(cxt);
      if (oxt != cxt) {
        diff_flags |= MRG_XTIME;
        LoadAString(str_temp, IDSC_FLDNMXTIME);
        str_diffs += str_temp + _T(", ");
      }
      otherItem.GetXTimeInt(oxtint);
      curItem.GetXTimeInt(cxtint);
      if (oxtint != cxtint) {
        diff_flags |= MRG_XTIME_INT;
        LoadAString(str_temp, IDSC_FLDNMXTIMEINT);
        str_diffs += str_temp + _T(", ");
      }
      if (otherItem.GetRunCommand() != curItem.GetRunCommand()) {
        diff_flags |= MRG_EXECUTE;
        LoadAString(str_temp, IDSC_FLDNMRUNCOMMAND);
        str_diffs += str_temp + _T(", ");
      }
      // Must use integer values not compare strings
      short other_hDCA, cur_hDCA;
      otherItem.GetDCA(other_hDCA);
      curItem.GetDCA(cur_hDCA);
      if (other_hDCA != cur_hDCA) {
        diff_flags |= MRG_DCA;
        LoadAString(str_temp, IDSC_FLDNMDCA);
        str_diffs += str_temp + _T(", ");
      }
      if (otherItem.GetEmail() != curItem.GetEmail()) {
        diff_flags |= MRG_EMAIL;
        LoadAString(str_temp, IDSC_FLDNMEMAIL);
        str_diffs += str_temp + _T(", ");
      }
      if (otherItem.GetSymbols() != curItem.GetSymbols()) {
        diff_flags |= MRG_SYMBOLS;
        LoadAString(str_temp, IDSC_FLDNMSYMBOLS);
        str_diffs += str_temp + _T(", ");
      }
      otherItem.GetShiftDCA(other_hDCA);
      curItem.GetShiftDCA(cur_hDCA);
      if (other_hDCA != cur_hDCA) {
        diff_flags |= MRG_SHIFTDCA;
        LoadAString(str_temp, IDSC_FLDNMSHIFTDCA);
        str_diffs += str_temp + _T(", ");
      }
      if (otherItem.GetPolicyName() != curItem.GetPolicyName()) {
        diff_flags |= MRG_POLICYNAME;
        LoadAString(str_temp, IDSC_FLDNMPWPOLICYNAME);
        str_diffs += str_temp + _T(", ");
      }
      if (diff_flags != 0) {
        // have a match on group/title/user, but not on other fields
        // add an entry suffixed with -merged-YYYYMMDD-HHMMSS
        StringX sx_newTitle;
        time_t time_now;
        time(&time_now);
        str_timestring = PWSUtil::ConvertToDateTimeString(time_now, TMC_XML).c_str();
        Replace(str_timestring, _T('T'), _T('-'));
        Format(sx_newTitle, _T("%s%s%s"), sx_otherTitle.c_str(), sx_merged.c_str(),
                            str_timestring.c_str());

        // note it as an issue for the user
        stringT strWarnMsg;
        Format(strWarnMsg, IDSC_MERGECONFLICTS,
                       sx_otherGroup.c_str(), sx_otherTitle.c_str(), sx_otherUser.c_str(),
                       sx_otherGroup.c_str(), sx_newTitle.c_str(), sx_otherUser.c_str(),
                       str_diffs.c_str());

        // log it
        if (pRpt != NULL)
          pRpt->WriteLine(strWarnMsg.c_str());

        // Check no conflict of unique uuid
        if (Find(base_uuid) != GetEntryEndIter()) {
          otherItem.CreateUUID();
          otherItem.GetUUID(new_base_uuid);
        }

        // do it
        bTitleRenamed = true;
        StringX osxPolicyName = otherItem.GetPolicyName();
        // If named policy in use....
        if (!osxPolicyName.empty()) {
          st_PSWDPolicy st_pp, o_st_ppp;
          bool bInOtherCore = pothercore->GetPolicyFromName(osxPolicyName, o_st_ppp);
          bool bInThisCore = GetPolicyFromName(osxPolicyName, st_pp);
          if (bInThisCore) {
            // In current database - need to make sure the same settings
            if (bInOtherCore) {
              // In this database and other database
              if (st_pp != o_st_ppp) {
                // But have different setting. Make unique
                MakePolicyUnique(mapRenamedPolicies, osxPolicyName,
                                 sxMerge_DateTime, IDSC_MERGEPOLICY);
                // Don't add it twice
                if (std::find(vs_PoliciesAdded.begin(), vs_PoliciesAdded.end(), osxPolicyName) ==
                    vs_PoliciesAdded.end()) {
                  Command *pcmd = DBPolicyNamesCommand::Create(this, osxPolicyName, o_st_ppp);
                  pmulticmds->Add(pcmd);
                  vs_PoliciesAdded.push_back(osxPolicyName);
                } 
              } else {
                 // In both but have same settings - no need to do anything
              }
            } else {
              // Not there!!!!  Remove any reference to it
              osxPolicyName = _T("");
            }
          } else {
            // Not in current database
            if (bInOtherCore) {
              // Was in other database, so we can add it, if not already added
              if (std::find(vs_PoliciesAdded.begin(), vs_PoliciesAdded.end(), osxPolicyName) ==
                   vs_PoliciesAdded.end()) {
                Command *pcmd = DBPolicyNamesCommand::Create(this, osxPolicyName, o_st_ppp);
                pmulticmds->Add(pcmd);
                vs_PoliciesAdded.push_back(osxPolicyName);
              }
            } else {
              // Not here or there!!!!  Remove any reference to it
              osxPolicyName = _T("");
            }
          }
          otherItem.SetPolicyName(osxPolicyName);
        }
        otherItem.SetTitle(sx_newTitle);
        otherItem.SetStatus(CItemData::ES_ADDED);
        Command *pcmd = AddEntryCommand::Create(this, otherItem);
        pcmd->SetNoGUINotify();
        pmulticmds->Add(pcmd);

        StringX sx_merged1 = sx_StartChevron +
                              sx_otherGroup + sx_MiddleChevron +
                              sx_otherTitle + sx_MiddleChevron +
                              sx_otherUser  + sx_EndChevron;
        // Update the Wizard page
        UpdateWizard(sx_merged1.c_str());

        numConflicts++;
      }
    } else {
      // Didn't find any match...add it directly
      // Check no conflict of unique uuid
      if (Find(base_uuid) != GetEntryEndIter()) {
        otherItem.CreateUUID();
        otherItem.GetUUID(new_base_uuid);
      }

      StringX osxPolicyName = otherItem.GetPolicyName();
      // If named policy in use and not already added....
        if (!osxPolicyName.empty() &&
            std::find(vs_PoliciesAdded.begin(), vs_PoliciesAdded.end(), osxPolicyName) ==
                 vs_PoliciesAdded.end()) {
        st_PSWDPolicy st_pp;
        // If this policy does not exist in current database - get it and add it
        if (!GetPolicyFromName(osxPolicyName, st_pp)) {
          // But only if it exists there
          if (pothercore->GetPolicyFromName(osxPolicyName, st_pp)) {
            Command *pcmd = DBPolicyNamesCommand::Create(this, osxPolicyName, st_pp);
            pmulticmds->Add(pcmd);
            vs_PoliciesAdded.push_back(osxPolicyName);
          } else {
            // Not there or here - remove any reference to it
            otherItem.SetPolicyName(_T(""));
          }
        }
      }
      otherItem.SetStatus(CItemData::ES_ADDED);
      Command *pcmd = AddEntryCommand::Create(this, otherItem);
      pcmd->SetNoGUINotify();
      pmulticmds->Add(pcmd);

      StringX sx_added = sx_StartChevron +
                           sx_otherGroup + sx_MiddleChevron +
                           sx_otherTitle + sx_MiddleChevron +
                           sx_otherUser  + sx_EndChevron;
      vs_added.push_back(sx_added);

      // Update the Wizard page
      UpdateWizard(sx_added.c_str());
      numAdded++;
    }

    if (et == CItemData::ET_ALIASBASE)
      numAliasesAdded += MergeDependents(pothercore, pmulticmds,
                      base_uuid, new_base_uuid,
                      bTitleRenamed, str_timestring, CItemData::ET_ALIAS,
                      vs_AliasesAdded);

    if (et == CItemData::ET_SHORTCUTBASE)
      numShortcutsAdded += MergeDependents(pothercore, pmulticmds,
                      base_uuid, new_base_uuid,
                      bTitleRenamed, str_timestring, CItemData::ET_SHORTCUT,
                      vs_ShortcutsAdded);
  } // iteration over other core's entries

  stringT str_results;
  if (numAdded > 0 && pRpt != NULL) {
    std::sort(vs_added.begin(), vs_added.end(), MergeSyncGTUCompare);
    stringT str_singular_plural_type, str_singular_plural_verb;
    LoadAString(str_singular_plural_type, numAdded == 1 ? IDSC_ENTRY : IDSC_ENTRIES);
    LoadAString(str_singular_plural_verb, numAdded == 1 ? IDSC_WAS : IDSC_WERE);
    Format(str_results, IDSC_MERGEADDED, str_singular_plural_type.c_str(),
                    str_singular_plural_verb.c_str());
    pRpt->WriteLine(str_results.c_str());
    for (size_t i = 0; i < vs_added.size(); i++) {
      Format(str_results, _T("\t%s"), vs_added[i].c_str());
      pRpt->WriteLine(str_results.c_str());
    }
  }

  if (numAliasesAdded > 0 && pRpt != NULL) {
    std::sort(vs_AliasesAdded.begin(), vs_AliasesAdded.end(), MergeSyncGTUCompare);
    stringT str_singular_plural_type, str_singular_plural_verb;
    LoadAString(str_singular_plural_type, numAliasesAdded == 1 ? IDSC_ENTRY : IDSC_ENTRIES);
    LoadAString(str_singular_plural_verb, numAliasesAdded == 1 ? IDSC_WAS : IDSC_WERE);
    Format(str_results, IDSC_MERGEADDED, str_singular_plural_type.c_str(),
                    str_singular_plural_verb.c_str());
    pRpt->WriteLine(str_results.c_str());
    for (size_t i = 0; i < vs_AliasesAdded.size(); i++) {
      Format(str_results, _T("\t%s"), vs_AliasesAdded[i].c_str());
      pRpt->WriteLine(str_results.c_str());
    }
  }

  if (numShortcutsAdded > 0 && pRpt != NULL) {
    std::sort(vs_ShortcutsAdded.begin(), vs_ShortcutsAdded.end(), MergeSyncGTUCompare);
    stringT str_singular_plural_type, str_singular_plural_verb;
    LoadAString(str_singular_plural_type, numShortcutsAdded == 1 ? IDSC_ENTRY : IDSC_ENTRIES);
    LoadAString(str_singular_plural_verb, numShortcutsAdded == 1 ? IDSC_WAS : IDSC_WERE);
    Format(str_results, IDSC_MERGEADDED, str_singular_plural_type.c_str(),
                    str_singular_plural_verb.c_str());
    pRpt->WriteLine(str_results.c_str());
    for (size_t i = 0; i < vs_ShortcutsAdded.size(); i++) {
      Format(str_results, _T("\t%s"), vs_ShortcutsAdded[i].c_str());
      pRpt->WriteLine(str_results.c_str());
    }
  }

  Command *pcmd2 = UpdateGUICommand::Create(this, UpdateGUICommand::WN_REDO,
                                            UpdateGUICommand::GUI_REDO_MERGESYNC);
  pmulticmds->Add(pcmd2);
  Execute(pmulticmds);

  // Tell the user we're done & provide short merge report
  stringT str_entries, str_conflicts, str_aliases, str_shortcuts;
  int totalAdded = numAdded + numConflicts + numAliasesAdded + numShortcutsAdded;
  LoadAString(str_entries, totalAdded == 1 ? IDSC_ENTRY : IDSC_ENTRIES);
  LoadAString(str_conflicts, numConflicts == 1 ? IDSC_CONFLICT : IDSC_CONFLICTS);
  LoadAString(str_aliases, numAliasesAdded == 1 ? IDSC_ALIAS : IDSC_ALIASES);
  LoadAString(str_shortcuts, numShortcutsAdded == 1 ? IDSC_SHORTCUT : IDSC_SHORTCUTS);

  Format(str_results, IDSC_MERGECOMPLETED,
                   totalAdded, str_entries.c_str(),
                   numConflicts, str_conflicts.c_str(),
                   numAliasesAdded, str_aliases.c_str(),
                   numShortcutsAdded, str_shortcuts.c_str());
  pRpt->WriteLine(str_results.c_str());

  return str_results;
}

int PWScore::MergeDependents(PWScore *pothercore, MultiCommands *pmulticmds,
                             uuid_array_t &base_uuid, uuid_array_t &new_base_uuid,
                             const bool bTitleRenamed, stringT &str_timestring,
                             const CItemData::EntryType et,
                             std::vector<StringX> &vs_added)
{
  UUIDVector dependentslist;
  UUIDVectorIter paiter;
  ItemListIter iter;
  ItemListConstIter foundPos;
  CItemData ci_temp;
  int numadded(0);
  StringX sx_merged;
  LoadAString(sx_merged, IDSC_MERGED);

  // Get all the dependents
  pothercore->GetAllDependentEntries(base_uuid, dependentslist, et);
  for (paiter = dependentslist.begin();
       paiter != dependentslist.end(); paiter++) {
    iter = pothercore->Find(*paiter);

    if (iter == pothercore->GetEntryEndIter())
      continue;

    CItemData *pci = &iter->second;
    ci_temp = (*pci);

    if (Find(*paiter) != GetEntryEndIter()) {
      ci_temp.CreateUUID();
    }

    // If the base title was renamed - we should automatically rename any dependent.
    // If we didn't, we still need to check uniqueness!
    StringX sx_newTitle = ci_temp.GetTitle();
    if (bTitleRenamed) {
      StringX sx_otherTitle(sx_newTitle);
      Format(sx_newTitle, _T("%s%s%s"), sx_otherTitle.c_str(),
                          sx_merged.c_str(), str_timestring.c_str());
      ci_temp.SetTitle(sx_newTitle);
    }
    // Check this is unique - if not - don't add this one! - its only an alias/shortcut!
    // We can't keep trying for uniqueness after adding a timestamp!
    foundPos = Find(ci_temp.GetGroup(), sx_newTitle, ci_temp.GetUser());
    if (foundPos != GetEntryEndIter())
      continue;

    Command *pcmd1 = AddEntryCommand::Create(this, ci_temp, new_base_uuid);
    pcmd1->SetNoGUINotify();
    pmulticmds->Add(pcmd1);

    if (et == CItemData::ET_ALIAS) {
      ci_temp.SetPassword(_T("[Alias]"));
      ci_temp.SetAlias();
    } else if (et == CItemData::ET_SHORTCUT) {
      ci_temp.SetPassword(_T("[Shortcut]"));
      ci_temp.SetShortcut();
    } else
      ASSERT(0);

    StringX sx_added = sx_StartChevron +
                         ci_temp.GetGroup() + sx_MiddleChevron +
                         ci_temp.GetTitle() + sx_MiddleChevron +
                         ci_temp.GetUser()  + sx_EndChevron;
    vs_added.push_back(sx_added);
    numadded++;
  }

  return numadded;
}

void PWScore::Synchronize(PWScore *pothercore,
                          const CItemData::FieldBits &bsFields, const bool &subgroup_bset,
                          const stringT &subgroup_name,
                          const int &subgroup_object, const int &subgroup_function,
                          int &numUpdated, CReport *pRpt)
{
  /*
  Purpose:
    Synchronize entries from otherCore to m_core

  Algorithm:
    Foreach entry in otherCore
      Find in m_core
        if find a match
          update requested fields
  */

  std::vector<StringX> vs_updated;
  numUpdated = 0;

  MultiCommands *pmulticmds = MultiCommands::Create(this);
  Command *pcmd1 = UpdateGUICommand::Create(this, UpdateGUICommand::WN_UNDO,
                                            UpdateGUICommand::GUI_UNDO_MERGESYNC);
  pmulticmds->Add(pcmd1);

  ItemListConstIter otherPos;
  for (otherPos = pothercore->GetEntryIter();
       otherPos != pothercore->GetEntryEndIter();
       otherPos++) {
    CItemData otherItem = pothercore->GetEntry(otherPos);
    CItemData::EntryType et = otherItem.GetEntryType();

    // Do not process Aliases and Shortcuts
    if (et == CItemData::ET_ALIAS || et == CItemData::ET_SHORTCUT)
      continue;

    if (subgroup_bset &&
        !otherItem.Matches(subgroup_name, subgroup_object, subgroup_function))
      continue;

    const StringX sx_otherGroup = otherItem.GetGroup();
    const StringX sx_otherTitle = otherItem.GetTitle();
    const StringX sx_otherUser = otherItem.GetUser();

    ItemListConstIter foundPos = Find(sx_otherGroup, sx_otherTitle, sx_otherUser);

    if (foundPos != GetEntryEndIter()) {
      // found a match
      CItemData curItem = GetEntry(foundPos);

      // Don't update if entry is protected
      if (curItem.IsProtected())
        continue;

      CItemData updItem(curItem);
      updItem.SetDisplayInfo(NULL);

      if (curItem.GetUUID() != otherItem.GetUUID()) {
        pws_os::Trace(_T("Synchronize: Mis-match UUIDs for [%s:%s:%s]\n"), sx_otherGroup.c_str(), sx_otherTitle.c_str(), sx_otherUser.c_str());
      }

      bool bUpdated(false);
      for (size_t i = 0; i < bsFields.size(); i++) {
        if (bsFields.test(i)) {
          const StringX sxValue = otherItem.GetFieldValue(static_cast<CItemData::FieldType>(i));
          if (sxValue != updItem.GetFieldValue(static_cast<CItemData::FieldType>(i))) {
            bUpdated = true;
            updItem.SetFieldValue(static_cast<CItemData::FieldType>(i), sxValue);
          }
        }
      }

      if (!bUpdated)
        continue;

      GUISetupDisplayInfo(updItem);
      updItem.SetStatus(CItemData::ES_MODIFIED);

      StringX sx_updated = sx_StartChevron +
                             sx_otherGroup + sx_MiddleChevron +
                             sx_otherTitle + sx_MiddleChevron +
                             sx_otherUser  + sx_EndChevron;
      vs_updated.push_back(sx_updated);

      Command *pcmd = EditEntryCommand::Create(this, curItem, updItem);
      pcmd->SetNoGUINotify();
      pmulticmds->Add(pcmd);

      // Update the Wizard page
      UpdateWizard(sx_updated.c_str());

      numUpdated++;
    }  // Found match via [g:t:u]
  } // iteration over other core's entries

  stringT str_results;
  if (numUpdated > 0 && pRpt != NULL) {
    std::sort(vs_updated.begin(), vs_updated.end(), MergeSyncGTUCompare);
    stringT str_singular_plural_type, str_singular_plural_verb;
    LoadAString(str_singular_plural_type, numUpdated == 1 ? IDSC_ENTRY : IDSC_ENTRIES);
    LoadAString(str_singular_plural_verb, numUpdated == 1 ? IDSC_WAS : IDSC_WERE);
    Format(str_results, IDSC_SYNCHUPDATED, str_singular_plural_type.c_str(),
                    str_singular_plural_verb.c_str());
    pRpt->WriteLine(str_results.c_str());
    for (size_t i = 0; i < vs_updated.size(); i++) {
      Format(str_results, _T("\t%s"), vs_updated[i].c_str());
      pRpt->WriteLine(str_results.c_str());
    }
  }

  Command *pcmd2 = UpdateGUICommand::Create(this, UpdateGUICommand::WN_REDO,
                                            UpdateGUICommand::GUI_REDO_MERGESYNC);
  pmulticmds->Add(pcmd2);
  Execute(pmulticmds);

  // tell the user we're done & provide short Synchronize report
  stringT str_entries;
  LoadAString(str_entries, numUpdated == 1 ? IDSC_ENTRY : IDSC_ENTRIES);
  Format(str_results, IDSC_SYNCHCOMPLETED, numUpdated, str_entries.c_str());
  pRpt->WriteLine(str_results.c_str());
}

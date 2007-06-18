/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
/// file MainEdit.cpp
//
// Edit-related methods of DboxMain
//-----------------------------------------------------------------------------

#include "PasswordSafe.h"

#include "ThisMfcApp.h"

#include "corelib/pwsprefs.h"

// dialog boxen
#include "DboxMain.h"

#include "AddDlg.h"
#include "ConfirmDeleteDlg.h"
#include "QuerySetDef.h"
#include "EditDlg.h"
#include "KeySend.h"
#include "ClearQuestionDlg.h"
#include "FindDlg.h"

#include  <Winable.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//Add an item
void
DboxMain::OnAdd() 
{
  CAddDlg dataDlg(this);
  
  if (m_core.GetUseDefUser()) {
    dataDlg.m_username = m_core.GetDefUsername();
  }
  // m_TreeViewGroup may be set by OnContextMenu, if not, try to grok it
  if (m_TreeViewGroup.IsEmpty()) {
    CItemData *itemData = NULL;
    if (m_ctlItemTree.IsWindowVisible()) { // tree view
      HTREEITEM ti = m_ctlItemTree.GetSelectedItem();
      if (ti != NULL) { // if anything selected
        itemData = (CItemData *)m_ctlItemTree.GetItemData(ti);
        if (itemData != NULL) { // leaf selected
          m_TreeViewGroup = itemData->GetGroup();
        } else { // node selected
          m_TreeViewGroup = CMyString(m_ctlItemTree.GetGroup(ti));
        }
      }
    } else { // list view
      // XXX TBD - get group name of currently selected list entry
    }
  }
  dataDlg.m_group = m_TreeViewGroup;
  m_TreeViewGroup = _T(""); // for next time
  app.DisableAccelerator();
  int rc = dataDlg.DoModal();
  app.EnableAccelerator();

  if (rc == IDOK) {
    PWSprefs *prefs = PWSprefs::GetInstance();
    //Check if they wish to set a default username
    if (!m_core.GetUseDefUser()
        && (prefs->GetPref(PWSprefs::QuerySetDef))
        && (!dataDlg.m_username.IsEmpty())) {
      CQuerySetDef defDlg(this);
      defDlg.m_message.Format(IDS_SETUSERNAME, (const CString&)dataDlg.m_username);
      int rc2 = defDlg.DoModal();
      if (rc2 == IDOK) {
        prefs->SetPref(PWSprefs::UseDefUser, true);
        prefs->SetPref(PWSprefs::DefUserName,
                       dataDlg.m_username);
        m_core.SetUseDefUser(true);
        m_core.SetDefUsername(dataDlg.m_username);
        RefreshList();
      }
    }
    //Finish Check (Does that make any geographical sense?)
    CItemData temp;
    CMyString user;
    time_t t;
    if (dataDlg.m_username.IsEmpty() && m_core.GetUseDefUser())
      user = m_core.GetDefUsername();
    else
      user = dataDlg.m_username;
    temp.CreateUUID();
    temp.SetGroup(dataDlg.m_group);
    temp.SetTitle(dataDlg.m_title);
    temp.SetUser(user);
    temp.SetPassword(dataDlg.m_password);
    temp.SetNotes(dataDlg.m_notes);
    temp.SetURL(dataDlg.m_URL);
    temp.SetAutoType(dataDlg.m_autotype);
    time(&t);
    temp.SetCTime(t);
    temp.SetLTime(dataDlg.m_tttLTime);
    if (dataDlg.m_SavePWHistory == TRUE) {
      TCHAR buffer[6];
#if _MSC_VER >= 1400
      _stprintf_s(buffer, 6, _T("1%02x00"), dataDlg.m_MaxPWHistory);
#else
      _stprintf(buffer, _T("1%02x00"), dataDlg.m_MaxPWHistory);
#endif
      temp.SetPWHistory(buffer);
    }

    AddEntry(temp);

    if (m_core.GetNumEntries() == 1) {
      // For some reason, when adding the first entry, it is not visible!
      m_ctlItemTree.SetRedraw(TRUE);
    }
    m_ctlItemList.SetFocus();
    if (prefs->GetPref(PWSprefs::SaveImmediately))
      Save();

    ChangeOkUpdate();
    uuid_array_t uuid;
    temp.GetUUID(uuid);
    m_RUEList.AddRUEntry(uuid);
  }
}

int
DboxMain::AddEntry(const CItemData &cinew)
{
  // This routine is used by Add and also Drag & Drop

  m_core.AddEntry(cinew);

  // AddEntry copies the entry, and we want to work with the inserted copy
  // Which we'll find by uuid
  uuid_array_t uuid;
  cinew.GetUUID(uuid);
  int newpos = insertItem(m_core.GetEntry(m_core.Find(uuid)));
  SelectEntry(newpos);
  FixListIndexes();
  return newpos;
}

//Add a group (tree view only)
void
DboxMain::OnAddGroup()
{
  if (m_core.IsReadOnly()) // disable in read-only mode
    return;

  if (m_ctlItemTree.IsWindowVisible()) {
    // This can be reached by right clicking over an existing group node
    // or by clicking over "whitespace".
    // If the former, add a child node to the current one
    // If the latter, add to root.
    CMyString cmys_text(MAKEINTRESOURCE(IDS_NEWGROUP));
    if (m_TreeViewGroup.IsEmpty())
      m_TreeViewGroup = cmys_text;
    else
      m_TreeViewGroup += _T(".") + cmys_text;
    HTREEITEM newGroup = m_ctlItemTree.AddGroup(m_TreeViewGroup);
    m_ctlItemTree.SelectItem(newGroup);
    m_TreeViewGroup = _T(""); // for next time
    m_ctlItemTree.EditLabel(newGroup);
  }
}

// Delete key was pressed (in list view or tree view) to delete an entry.
void
DboxMain::OnDelete() 
{
  const bool dontaskquestion = PWSprefs::GetInstance()->
    GetPref(PWSprefs::DeleteQuestion);

  bool dodelete = true;
    
  //Confirm whether to delete the item
  if (!dontaskquestion) {
    CConfirmDeleteDlg deleteDlg(this);
    int rc = deleteDlg.DoModal();
    if (rc == IDCANCEL) {
      dodelete = false;
    }
  }
  
  if (dodelete) {
	Delete();
  }
}

void
DboxMain::Delete(bool inRecursion) 
{
  if (SelItemOk() == TRUE) {
    CItemData *ci = getSelectedItem();
    ASSERT(ci != NULL);
    //  Needed for DeleteTrayRecentEntry later on
    uuid_array_t RUEuuid;
    ci->GetUUID(RUEuuid);
    DisplayInfo *di = (DisplayInfo *)ci->GetDisplayInfo();
    ASSERT(di != NULL);
    size_t curSel = di->list_index;
    // Find next in treeview, not always curSel after deletion
    HTREEITEM curTree_item = di->tree_item;
    HTREEITEM nextTree_item = m_ctlItemTree.GetNextItem(curTree_item,
                                                        TVGN_NEXT);
    // Must Find before delete from m_ctlItemList:
    ItemListIter listindex = Find(curSel);

    UnFindItem();
    m_ctlItemList.DeleteItem(curSel);
    m_ctlItemTree.DeleteFromSet(curTree_item);
    m_ctlItemTree.DeleteWithParents(curTree_item);
    delete di;

    if (ci->NumberUnknownFields() > 0)
      m_core.DecrementNumRecordsWithUnknownFields();

    m_core.RemoveEntryAt(listindex);
    FixListIndexes();
    if (m_ctlItemList.IsWindowVisible()) {
      if (m_core.GetNumEntries() > 0) {
        SelectEntry(curSel < m_core.GetNumEntries() ? 
                    curSel : m_core.GetNumEntries() - 1);
      }
      m_ctlItemList.SetFocus();
    } else {// tree view visible
      if (!inRecursion && nextTree_item != NULL) {
        m_ctlItemTree.SelectItem(nextTree_item);
      }
      m_ctlItemTree.SetFocus();
    }
    ChangeOkUpdate();
    m_RUEList.DeleteRUEntry(RUEuuid);
  } else { // !SelItemOk()
    if (m_ctlItemTree.IsWindowVisible()) {
      HTREEITEM ti = m_ctlItemTree.GetSelectedItem();
      if (ti != NULL) {
        if (!m_ctlItemTree.IsLeafNode(ti)) {
          HTREEITEM cti = m_ctlItemTree.GetChildItem(ti);

          m_ctlItemTree.SetRedraw( FALSE );

          while (cti != NULL) {
            m_ctlItemTree.SelectItem(cti);
            Delete(true); // recursion - I'm so lazy!
            cti = m_ctlItemTree.GetChildItem(ti);
          }

          m_ctlItemTree.SetRedraw( TRUE );
          m_ctlItemTree.Invalidate();

          //  delete an empty group.
          HTREEITEM parent = m_ctlItemTree.GetParentItem(ti);            
          m_ctlItemTree.DeleteItem(ti);
          m_ctlItemTree.SelectItem(parent);
        }
      }
    }  
  }
  m_TreeViewGroup = _T("");
}

void
DboxMain::OnRename() 
{
  if (m_core.IsReadOnly()) // disable in read-only mode
    return;

  // Renaming is only allowed while in Tree mode.
  if (m_ctlItemTree.IsWindowVisible()) {
    HTREEITEM hItem = m_ctlItemTree.GetSelectedItem();
    if (hItem != NULL)
      m_ctlItemTree.EditLabel(hItem);
  }
}

void
DboxMain::OnEdit() 
{
  // Note that Edit is also used for just viewing - don't want to disable
  // viewing in read-only mode
  if (SelItemOk() == TRUE) {
    CItemData *ci = getSelectedItem();
    ASSERT(ci != NULL);
    EditItem(ci);
  } else {
    // entry item not selected - perhaps here on Enter on tree item?
    // perhaps not the most elegant solution to improving non-mouse use,
    // but it works. If anyone knows how Enter/Return gets mapped to OnEdit,
    // let me know...
    CItemData *itemData = NULL;
    if (m_ctlItemTree.IsWindowVisible()) { // tree view
      HTREEITEM ti = m_ctlItemTree.GetSelectedItem();
      if (ti != NULL) { // if anything selected
        itemData = (CItemData *)m_ctlItemTree.GetItemData(ti);
        if (itemData == NULL) { // node selected
          m_ctlItemTree.Expand(ti, TVE_TOGGLE);
        }
      }
    }
  }
}

bool
DboxMain::EditItem(CItemData *ci)
{
    // List might be cleared if db locked.
    // Need to take care that we handle a rebuilt list.
    CItemData editedItem(*ci);

    CEditDlg dlg_edit(&editedItem, this);

    if (m_core.GetUseDefUser())
      dlg_edit.m_defusername = m_core.GetDefUsername();
    dlg_edit.m_Edit_IsReadOnly = m_core.IsReadOnly();

    app.DisableAccelerator();
    int rc = dlg_edit.DoModal();
    app.EnableAccelerator();

    if (rc == IDOK) {
      // Out with the old, in with the new
      uuid_array_t uuid;
      editedItem.GetUUID(uuid);
      ItemListIter listpos = Find(uuid);
      ASSERT(listpos != m_core.GetEntryEndIter());
      CItemData oldElem = GetEntryAt(listpos);
      DisplayInfo *di = (DisplayInfo *)oldElem.GetDisplayInfo();
      ASSERT(di != NULL);
      // editedItem's displayinfo will have been deleted if
      // application "locked" (Cleared list)
      DisplayInfo *ndi = new DisplayInfo;
      ndi->list_index = -1; // so that insertItem will set new values
      ndi->tree_item = 0;
      editedItem.SetDisplayInfo(ndi);

      m_core.RemoveEntryAt(listpos);
      m_core.AddEntry(editedItem);
      m_ctlItemList.DeleteItem(di->list_index);
      m_ctlItemTree.DeleteWithParents(di->tree_item);
      // AddEntry copies the entry, and we want to work with the inserted copy
      // Which we'll find by uuid
      insertItem(m_core.GetEntry(m_core.Find(uuid)));
      FixListIndexes();
      // Now delete old entry's DisplayInfo
      delete di;
      if (PWSprefs::GetInstance()->
          GetPref(PWSprefs::SaveImmediately)) {
        Save();
      }
      rc = SelectEntry(ndi->list_index);
      if (rc == LB_ERR) {
        SelectEntry(m_ctlItemList.GetItemCount() - 1);
      }
      ChangeOkUpdate();
      return true;
    } // rc == IDOK
    return false;
}


// Duplicate selected entry but make title unique
void
DboxMain::OnDuplicateEntry() 
{
  if (m_core.IsReadOnly()) // disable in read-only mode
    return;

  if (SelItemOk() == TRUE) {
    CItemData *ci = getSelectedItem();
    ASSERT(ci != NULL);
    DisplayInfo *di = (DisplayInfo *)ci->GetDisplayInfo();
    ASSERT(di != NULL);
      
    // Get information from current selected entry
    CMyString ci2_group = ci->GetGroup();
    CMyString ci2_user = ci->GetUser();
    CMyString ci2_title0 = ci->GetTitle();
    CMyString ci2_title;
      
    // Find a unique "Title"
    ItemListConstIter listpos;
    int i = 0;
    CString s_copy;
    do {
      i++;
      s_copy.Format(IDS_COPYNUMBER, i);
      ci2_title = ci2_title0 + CMyString(s_copy);
      listpos = m_core.Find(ci2_group, ci2_title, ci2_user);
    } while (listpos != m_core.GetEntryEndIter());
      
    // Set up new entry
    CItemData ci2;
    ci2.CreateUUID();
    ci2.SetGroup(ci2_group);
    ci2.SetTitle(ci2_title);
    ci2.SetUser(ci2_user);
    ci2.SetPassword( ci->GetPassword() );
    ci2.SetURL( ci->GetURL() );
    ci2.SetAutoType( ci->GetAutoType() );
    ci2.SetNotes( ci->GetNotes() );
    time_t t;
    ci->GetCTime(t);
    if ((long) t != 0)
    	ci2.SetCTime(t);
    ci->GetATime(t);
    if ((long) t != 0)
    	ci2.SetATime(t);
    ci->GetLTime(t);
    if ((long) t != 0)
    	ci2.SetLTime(t);
    ci->GetPMTime(t);
    if ((long) t != 0)
    	ci2.SetPMTime(t);
    ci->GetRMTime(t);
    if ((long) t != 0)
    	ci2.SetRMTime(t);
    CMyString tmp = ci->GetPWHistory();
    if (tmp.GetLength() >= 5)
    	    ci2.SetPWHistory(tmp);
    // Add it to the end of the list      
    m_core.AddEntry(ci2);
    di->list_index = -1; // so that insertItem will set new values
    uuid_array_t uuid;
    ci2.GetUUID(uuid);
    insertItem(m_core.GetEntry(m_core.Find(uuid)));
    FixListIndexes();
    if (PWSprefs::GetInstance()->
        GetPref(PWSprefs::SaveImmediately)) {
      Save();
    }
    int rc = SelectEntry(di->list_index);
    if (rc == LB_ERR) {
      SelectEntry(m_ctlItemList.GetItemCount() - 1);
    }
    ChangeOkUpdate();
	m_RUEList.AddRUEntry(uuid);

  }
}

void
DboxMain::OnCopyPassword()
{
  if (!SelItemOk())
    return;

  //Remind the user about clipboard security
  CClearQuestionDlg clearDlg(this);
  if (clearDlg.m_dontaskquestion == FALSE &&
      clearDlg.DoModal() == IDCANCEL)
    return;

  CItemData *ci = getSelectedItem();
  ASSERT(ci != NULL);
  ToClipboard(ci->GetPassword());
  UpdateAccessTime(ci);
  uuid_array_t RUEuuid;
  ci->GetUUID(RUEuuid);
  m_RUEList.AddRUEntry(RUEuuid);
}

void
DboxMain::OnCopyUsername()
{
  if (SelItemOk() != TRUE)
    return;

  CItemData *ci = getSelectedItem();
  ASSERT(ci != NULL);
  const CMyString username = ci->GetUser();

  if (!username.IsEmpty()) {
    ToClipboard(username);
    UpdateAccessTime(ci);
    uuid_array_t RUEuuid;
    ci->GetUUID(RUEuuid);
    m_RUEList.AddRUEntry(RUEuuid);
  }
}

void
DboxMain::OnCopyNotes()
{
  if (SelItemOk() != TRUE)
    return;

  CItemData *ci = getSelectedItem();
  ASSERT(ci != NULL);

  const CMyString notes = ci->GetNotes();
  const CMyString url = ci->GetURL();
  const CMyString autotype = ci->GetAutoType();
  CMyString clipboard_data;
  CString cs_text;

  clipboard_data = notes;
  if (!url.IsEmpty()) {
  	  cs_text.LoadString(IDS_COPYURL);
	  clipboard_data += CMyString(cs_text);
	  clipboard_data += url;
  }
  if (!autotype.IsEmpty()) {
	  cs_text.LoadString(IDS_COPYAUTOTYPE);
	  clipboard_data += CMyString(cs_text);
	  clipboard_data += autotype;
  }
  if (!clipboard_data.IsEmpty()) {
    ToClipboard(clipboard_data);
    UpdateAccessTime(ci);
    uuid_array_t RUEuuid;
    ci->GetUUID(RUEuuid);
    m_RUEList.AddRUEntry(RUEuuid);
  }
}

void
DboxMain::OnFind()
{
  // create modeless or popup existing
  CFindDlg::Doit(this, &m_lastFindCS, &m_lastFindStr, &m_bFindWrap);
}

void
DboxMain::OnClearClipboard()
{
   app.ClearClipboardData();
}

void
DboxMain::OnAutoType()
{
  if (SelItemOk() == TRUE) {
    CItemData *ci = getSelectedItem();
    ASSERT(ci != NULL);
	uuid_array_t RUEuuid;
	ci->GetUUID(RUEuuid);
	m_RUEList.AddRUEntry(RUEuuid);
    UpdateAccessTime(ci);
    // All code using ci must be before this AutoType since the
	// latter may trash *ci if lock-on-minimize
	AutoType(*ci);
  }
}

const CString DboxMain::DEFAULT_AUTOTYPE = _T("\\u\\t\\p\\n");


void
DboxMain::AutoType(const CItemData &ci)
{
    CMyString AutoCmd = ci.GetAutoType();
    const CMyString user(ci.GetUser());
    const CMyString pwd(ci.GetPassword());

    // If empty, try the database default
    if (AutoCmd.IsEmpty()) {
        AutoCmd = PWSprefs::GetInstance()->
            GetPref(PWSprefs::DefaultAutotypeString);

        // If still empty, take this default
        if (AutoCmd.IsEmpty()) {
            // checking for user and password for default settings
            if (!pwd.IsEmpty()){
                if (!user.IsEmpty())
                    AutoCmd = CMyString(DEFAULT_AUTOTYPE);
                else
                    AutoCmd = _T("\\p\\n");
            }
        }
    }

    CKeySend ks;
    // Turn off CAPSLOCK
    bool bCapsLock = false;
    if (GetKeyState(VK_CAPITAL)) {
        bCapsLock = true;
        ks.SetCapsLock(false);
    }

    CMyString tmp;
    TCHAR curChar;
    const int N = AutoCmd.GetLength();
    ks.ResetKeyboardState();

    ::BlockInput(true);

    // Note that minimizing the window before calling ci.Get*()
    // will cause garbage to be read if "lock on minimize" selected,
    // since that will clear the data [Bugs item #1026630]
    // (this is why we read user & pwd before actual use)

    // Rules are ("Minimize on Autotype" takes precedence):
    // 1. If "MinimizeOnAutotype" - minimize PWS during Autotype but do
    //    not restore it (previous default action - but a pain if locked
    //    in the system tray!)
    // 2. If "Always on Top" - hide PWS during Autotype and then make it
    //    "AlwaysOnTop" again, unless minimized!
    // 3. If not "Always on Top" - hide PWS during Autotype and show
    //    it again once finished - but behind other windows.
    bool bMinOnAuto = PWSprefs::GetInstance()->
        GetPref(PWSprefs::MinimizeOnAutotype) == TRUE;

    if (bMinOnAuto)
        ShowWindow(SW_MINIMIZE);
    else
        ShowWindow(SW_HIDE);

    Sleep(1000); // Karl Student's suggestion, to ensure focus set correctly on minimize.

    for(int n = 0; n < N; n++){
        curChar = AutoCmd[n];
        if(curChar == TCHAR('\\')) {
            n++;
            if(n < N)
                curChar=AutoCmd[n];
            switch(curChar){
                case TCHAR('\\'):
                    tmp += TCHAR('\\');
                break;
                case TCHAR('n'):
                case TCHAR('r'):
                    tmp += TCHAR('\r');
                break;
                case TCHAR('t'):
                    tmp += TCHAR('\t');
                break;
                case TCHAR('u'):
                    tmp += user;
                break;
                case TCHAR('p'):
                    tmp += pwd;
                break;
                case TCHAR('d'): {
                    // Delay is going to change - send what we have with old delay
                    ks.SendString(tmp);
                    // start collecting new delay
                    tmp = _T("");
                    int newdelay = 0;
                    int gNumIts = 0;

                    for(n++; n < N && (gNumIts < 3); ++gNumIts, n++)
                        if(isdigit(AutoCmd[n])){
                            newdelay *= 10;
                            newdelay += (AutoCmd[n] - TCHAR('0'));
                        } else
                            break; // for loop
                    n--;
                    ks.SetAndDelay(newdelay);

                    break; // case
                }
                default:
                    tmp += _T("\\") + curChar;
                    break;
            }
        } else
            tmp += curChar;
    }
    ks.SendString(tmp);
    // If we turned off CAPSLOCK, put it back
    if (bCapsLock) 
        ks.SetCapsLock(true);
 
    Sleep(100);

    ::BlockInput(false);

    // If we hid it, now show it
    if (bMinOnAuto)
      return;

  if (PWSprefs::GetInstance()->GetPref(PWSprefs::AlwaysOnTop)) {
        SetWindowPos(&wndTopMost, 0, 0, 0, 0,
                     SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
    } else {
        SetWindowPos(&wndBottom, 0, 0, 0, 0,
                     SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
    }

}

void DboxMain::AddEntries(CDDObList &in_oblist, const CMyString DropGroup)
{
  CItemData tempitem;
  CDDObject *pDDObject;
  CMyString Group, Title, User;
  POSITION pos;
  TCHAR *dot;

  for (pos = in_oblist.GetHeadPosition(); pos != NULL;) {
    pDDObject = (CDDObject *)in_oblist.GetAt(pos);

    if (in_oblist.m_bDragNode) {
      dot = (!DropGroup.IsEmpty() && !pDDObject->m_DD_Group.IsEmpty()) ? _T(".") : _T("");
      Group = DropGroup + dot + pDDObject->m_DD_Group;
    } else {
      Group = DropGroup;
    }

    Title = GetUniqueTitle(Group, pDDObject->m_DD_Title, pDDObject->m_DD_User,
                           IDS_DRAGNUMBER);

    tempitem.Clear();

    if (m_core.Find(pDDObject->m_DD_UUID) != End())
      tempitem.CreateUUID();
    else
      tempitem.SetUUID(pDDObject->m_DD_UUID);

    tempitem.SetGroup(Group);
    tempitem.SetTitle(Title);
    tempitem.SetUser(pDDObject->m_DD_User);
    tempitem.SetNotes(pDDObject->m_DD_Notes);
    tempitem.SetPassword(pDDObject->m_DD_Password);
    tempitem.SetURL(pDDObject->m_DD_URL);
    tempitem.SetAutoType(pDDObject->m_DD_AutoType);
    tempitem.SetPWHistory(pDDObject->m_DD_PWHistory);

    tempitem.SetATime(pDDObject->m_DD_ATime);
    tempitem.SetCTime(pDDObject->m_DD_CTime);
    tempitem.SetLTime(pDDObject->m_DD_LTime);
    tempitem.SetPMTime(pDDObject->m_DD_PMTime);
    tempitem.SetRMTime(pDDObject->m_DD_RMTime);

    // AddEntry copies the entry, and we want to work with the inserted copy
    // Which we'll find by uuid
    uuid_array_t uuid;
    tempitem.GetUUID(uuid);
    int newpos = insertItem(m_core.GetEntry(m_core.Find(uuid)));
    SelectEntry(newpos);
    FixListIndexes();
    if (PWSprefs::GetInstance()->
             GetPref(PWSprefs::SaveImmediately)) {
      Save();
    }
    ChangeOkUpdate();
    in_oblist.GetNext(pos);
  }

  FixListIndexes();
  RefreshList();
}

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
	  defDlg.m_message =
        _T("Would you like to set \"")
        + (const CString&)dataDlg.m_username
        + _T("\" as your default username?\n\nIt would then automatically be ")
	    + _T("put in the dialog each time you add a new item.");
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
		_stprintf_s(buffer, 6, "1%02x00", dataDlg.m_MaxPWHistory);
#else
		_stprintf(buffer, "1%02x00", dataDlg.m_MaxPWHistory);
#endif
		temp.SetPWHistory(buffer);
	}
    m_core.AddEntryToTail(temp);
    int newpos = insertItem(m_core.GetTailEntry());
    SelectEntry(newpos);
    FixListIndexes();
    m_ctlItemList.SetFocus();
    if (prefs->GetPref(PWSprefs::SaveImmediately))
        Save();

    ChangeOkUpdate();
	uuid_array_t RUEuuid;
	temp.GetUUID(RUEuuid);
	m_RUEList.AddRUEntry(RUEuuid);
  }
}

//Add a group (tree view only)
void
DboxMain::OnAddGroup()
{
  if (m_IsReadOnly) // disable in read-only mode
    return;

  if (m_ctlItemTree.IsWindowVisible()) {
    // This can be reached by right clicking over an existing group node
    // or by clicking over "whitespace".
    // If the former, add a child node to the current one
    // If the latter, add to root.
    if (m_TreeViewGroup.IsEmpty())
      m_TreeViewGroup = _T("New Group");
    else
      m_TreeViewGroup += _T(".New Group");
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
  
  if (!dodelete) {
    return;
  }

  if (SelItemOk() == TRUE) {
    CItemData *ci = getSelectedItem();
    ASSERT(ci != NULL);
	//  Needed for DeleteTrayRecentEntry later on
	uuid_array_t RUEuuid;
	ci->GetUUID(RUEuuid);
    DisplayInfo *di = (DisplayInfo *)ci->GetDisplayInfo();
    ASSERT(di != NULL);
    int curSel = di->list_index;
    // Find next in treeview, not always curSel after deletion
    HTREEITEM curTree_item = di->tree_item;
    HTREEITEM nextTree_item = m_ctlItemTree.GetNextItem(curTree_item,
                                                        TVGN_NEXT);
    POSITION listindex = Find(curSel); // Must Find before delete from m_ctlItemList

    m_ctlItemList.DeleteItem(curSel);
    m_ctlItemTree.DeleteWithParents(di->tree_item);
    delete di;
    m_core.RemoveEntryAt(listindex);
    FixListIndexes();
    if (m_ctlItemList.IsWindowVisible()) {
      if (m_core.GetNumEntries() > 0) {
        SelectEntry(curSel < m_core.GetNumEntries() ? 
                    curSel : m_core.GetNumEntries() - 1);
      }
      m_ctlItemList.SetFocus();
    } else {// tree view visible
      if (nextTree_item != NULL)
        m_ctlItemTree.SelectItem(nextTree_item);
      else
        SelectEntry(0);
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
          const bool orig_dont_ask = dontaskquestion;
          // don't question user for each leaf!
          PWSprefs::GetInstance()->
            SetPref(PWSprefs::DeleteQuestion, true);

          while (cti != NULL) {
            m_ctlItemTree.SelectItem(cti);
            OnDelete(); // recursion - I'm so lazy!
            cti = m_ctlItemTree.GetChildItem(ti);
          }

          // restore original preference after recursion
          PWSprefs::GetInstance()->
            SetPref(PWSprefs::DeleteQuestion, orig_dont_ask);

          //  delete an empty group.
          HTREEITEM parent = m_ctlItemTree.GetParentItem(ti);            
          m_ctlItemTree.DeleteItem(ti);
          m_ctlItemTree.SelectItem(parent);
        }
      }
    }  
  }
}

void
DboxMain::OnRename() 
{
  if (m_IsReadOnly) // disable in read-only mode
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
    PWHistList* pPWHistList;
    CItemData *ci = getSelectedItem();
    ASSERT(ci != NULL);
    DisplayInfo *di = (DisplayInfo *)ci->GetDisplayInfo();
    ASSERT(di != NULL);
    POSITION listpos = Find(di->list_index);
    pPWHistList = new PWHistList;

    CEditDlg dlg_edit(this);
    CMyString oldGroup, oldTitle, oldUsername, oldRealPassword, oldURL,
      oldAutoType, oldNotes, oldLTime;
    int oldMaxPWHistory;

    oldGroup = dlg_edit.m_group = ci->GetGroup();
    oldTitle = dlg_edit.m_title = ci->GetTitle();
    oldUsername = dlg_edit.m_username = ci->GetUser();
    oldRealPassword = dlg_edit.m_realpassword = ci->GetPassword();
    oldURL = dlg_edit.m_URL = ci->GetURL();
    oldAutoType = dlg_edit.m_autotype = ci->GetAutoType();
    dlg_edit.m_password = HIDDEN_PASSWORD;
    oldNotes = dlg_edit.m_notes = ci->GetNotes();
    dlg_edit.m_listindex = listpos;   // for future reference, this is not multi-user friendly
    dlg_edit.m_IsReadOnly = m_IsReadOnly;
	
    dlg_edit.m_ascCTime = ci->GetCTime();
    dlg_edit.m_ascPMTime = ci->GetPMTime();
    dlg_edit.m_ascATime = ci->GetATime();
    dlg_edit.m_ascLTime = ci->GetLTimeN();
    if (dlg_edit.m_ascLTime.GetLength() == 0)
      dlg_edit.m_ascLTime = _T("Never");
    oldLTime = dlg_edit.m_ascLTime;
    dlg_edit.m_ascRMTime = ci->GetRMTime();
    dlg_edit.m_pPWHistList = pPWHistList;

    BOOL HasHistory = FALSE;
    ci->CreatePWHistoryList(HasHistory, oldMaxPWHistory,
                            dlg_edit.m_NumPWHistory, 
                            pPWHistList, EXPORT_IMPORT);

    dlg_edit.m_MaxPWHistory = oldMaxPWHistory;
    app.DisableAccelerator();
    int rc = dlg_edit.DoModal();
    app.EnableAccelerator();

    if (rc == IDOK) {
      CMyString temptitle;
      CMyString user;
      if (dlg_edit.m_username.IsEmpty() && m_core.GetUseDefUser())
        user = m_core.GetDefUsername();
      else
        user = dlg_edit.m_username;
      time_t t;
      time(&t);
      bool bPswdChanged, bAnotherChanged, bPWHistoryCleared;
      bPswdChanged = bAnotherChanged = bPWHistoryCleared = false;
      if (oldRealPassword != dlg_edit.m_realpassword)
        bPswdChanged = true;
      else {
      	if (oldGroup != dlg_edit.m_group
      		|| oldTitle != dlg_edit.m_title
      		|| oldUsername != user
      		|| oldNotes != dlg_edit.m_notes
      		|| oldURL != dlg_edit.m_URL
      		|| oldAutoType != dlg_edit.m_autotype
            || oldLTime != dlg_edit.m_ascLTime
            || oldMaxPWHistory != dlg_edit.m_MaxPWHistory)
          bAnotherChanged = true;
      }

      if (dlg_edit.m_ClearPWHistory == TRUE) {
        pPWHistList->RemoveAll();
        if (dlg_edit.m_SavePWHistory == FALSE) {
          char buffer[6];
#if _MSC_VER >= 1400
          sprintf_s(buffer, 6, "0%02x00", dlg_edit.m_MaxPWHistory);
#else
          sprintf(buffer, "0%02x00", dlg_edit.m_MaxPWHistory);
#endif
          ci->SetPWHistory(buffer);
        }
        bPWHistoryCleared = true;
      }

      if (HasHistory && dlg_edit.m_SavePWHistory == FALSE) {
        CMyString tmp = ci->GetPWHistory();
        if (tmp.GetLength() >= 5)
          tmp.SetAt(0, '0');	// Turn it off!
        else
          tmp = _T("");
        ci->SetPWHistory(tmp);
      }

      if (bPswdChanged) {
        if (PWSprefs::GetInstance()->GetPref(PWSprefs::SavePasswordHistory) &&
            dlg_edit.m_SavePWHistory == TRUE) {
          int num = pPWHistList->GetCount();
          PWHistEntry pwh_ent;
          pwh_ent.password = oldRealPassword;
          time_t t;
          ci->GetPMTime(t);
          if ((long)t == 0L) // if never set - try creation date
            ci->GetCTime(t);
          pwh_ent.changetttdate = t;
          pwh_ent.changedate =
            PWSUtil::ConvertToDateTimeString(t, EXPORT_IMPORT);
          if (pwh_ent.changedate.IsEmpty()) {
            //                       1234567890123456789
            pwh_ent.changedate = _T("Unknown            ");
          }

          // Now add the latest
          pPWHistList->AddTail(pwh_ent);

          // Increment count
          num++;

          // Too many? remove the excess
          if (num > dlg_edit.m_MaxPWHistory) {
            for (int i = 0; i < (num - dlg_edit.m_MaxPWHistory); i++)
              pPWHistList->RemoveHead();

            num = dlg_edit.m_MaxPWHistory;
          }

          // Now create string version!
          CMyString new_PWHistory;
          CString buffer;

          buffer.Format(_T("1%02x%02x"), dlg_edit.m_MaxPWHistory, num);
          new_PWHistory = CMyString(buffer);

          POSITION listpos = pPWHistList->GetHeadPosition();
          while (listpos != NULL) {
            const PWHistEntry pwshe = pPWHistList->GetAt(listpos);

            buffer.Format(_T("%08x%04x%s"),
                          (long) pwshe.changetttdate, pwshe.password.GetLength(),
                          pwshe.password);
            new_PWHistory += CMyString(buffer);
            buffer.Empty();
            pPWHistList->GetNext(listpos);
          }
          ci->SetPWHistory(new_PWHistory);
        }

        ci->SetPMTime(t);
        ci->SetRMTime(t);
      } else {
        if (oldMaxPWHistory != dlg_edit.m_MaxPWHistory) {
          CMyString tmp = ci->GetPWHistory();
          if (tmp.GetLength() >= 5) {
            CString buffer;
            buffer.Format(_T("%02x"), dlg_edit.m_MaxPWHistory);
            tmp = tmp.Left(1) + CMyString(buffer) + tmp.Mid(3);
          } else
            tmp = _T("");
          ci->SetPWHistory(tmp);
        }
      }
		
      if (bAnotherChanged)
        ci->SetRMTime(t);

      if (!bPswdChanged && !bAnotherChanged && !bPWHistoryCleared) {	// Nothing changed!
        pPWHistList->RemoveAll();
        delete pPWHistList;
        return;
      }

      ci->SetGroup(dlg_edit.m_group);
      ci->SetTitle(dlg_edit.m_title);
      ci->SetUser(user);
      ci->SetPassword(dlg_edit.m_realpassword);
      ci->SetNotes(dlg_edit.m_notes);
      ci->SetURL(dlg_edit.m_URL);
      ci->SetAutoType(dlg_edit.m_autotype);
      if (oldLTime != dlg_edit.m_ascLTime)
        ci->SetLTime(dlg_edit.m_tttLTime);

      // Out with the old, in with the new
      CItemData editedItem(*ci); // 'cause next line deletes *ci
      m_core.RemoveEntryAt(listpos);
      m_core.AddEntryToTail(editedItem);
      m_ctlItemList.DeleteItem(di->list_index);
      m_ctlItemTree.DeleteWithParents(di->tree_item);
      di->list_index = -1; // so that insertItem will set new values
      insertItem(m_core.GetTailEntry());
      FixListIndexes();
      if (PWSprefs::GetInstance()->
          GetPref(PWSprefs::SaveImmediately)) {
        Save();
      }
      rc = SelectEntry(di->list_index);
      if (rc == LB_ERR) {
        SelectEntry(m_ctlItemList.GetItemCount() - 1);
      }
      ChangeOkUpdate();
    } // rc == IDOK
    pPWHistList->RemoveAll();
    delete pPWHistList;
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

// Duplicate selected entry but make title unique
void
DboxMain::OnDuplicateEntry() 
{
  if (m_IsReadOnly) // disable in read-only mode
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
    POSITION listpos = NULL;
    int i = 0;
    CString s_copy;
    do {
      i++;
      s_copy.Format(_T("%d"), i);
      ci2_title = ci2_title0 + _T(" Copy #") + CMyString(s_copy);
      listpos = m_core.Find(ci2_group, ci2_title, ci2_user);
    } while (listpos != NULL);
      
    // Set up new entry
    CItemData ci2;
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
    m_core.AddEntryToTail(ci2);
    di->list_index = -1; // so that insertItem will set new values
    insertItem(m_core.GetTailEntry());
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
	uuid_array_t RUEuuid;
	ci2.GetUUID(RUEuuid);
	m_RUEList.AddRUEntry(RUEuuid);

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
  if (!m_IsReadOnly && m_bMaintainDateTimeStamps) {
  	ci->SetATime();
    SetChanged(TimeStamp);
  }
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
    if (!m_IsReadOnly && m_bMaintainDateTimeStamps) {
   		ci->SetATime();
       	SetChanged(TimeStamp);
	}
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

  clipboard_data = notes;
  if (!url.IsEmpty()) {
	  clipboard_data += _T("\r\nURL: ");
	  clipboard_data += url;
  }
  if (!autotype.IsEmpty()) {
	  clipboard_data += _T("\r\nAutotype: ");
	  clipboard_data += autotype;
  }
  if (!clipboard_data.IsEmpty()) {
    ToClipboard(clipboard_data);
    if (!m_IsReadOnly && m_bMaintainDateTimeStamps) {
   		ci->SetATime();
       	SetChanged(TimeStamp);
	}
    uuid_array_t RUEuuid;
    ci->GetUUID(RUEuuid);
    m_RUEList.AddRUEntry(RUEuuid);
  }
}

void
DboxMain::OnFind()
{
  CFindDlg::Doit(this, &m_lastFindCS,
                 &m_lastFindStr); // create modeless or popup existing
  // XXX Gross hack to fix aesthetic bug in tree view
  // without this, multiple "selected" displayed
  // if treeview && there's a selected item, then
#if 0
  m_ctlItemTree.SetItemState(di->tree_item,
                             TVIS_SELECTED,
                             TVIS_DROPHILITED | TVIS_SELECTED);
#endif
}

void
DboxMain::OnClearClipboard()
{
   app.ClearClipboardData();
}

// onAutoType handles menu item ID_MENUITEM_AUTOTYPE

void
DboxMain::OnAutoType()
{
  if (SelItemOk() == TRUE) {
    CItemData *ci = getSelectedItem();
    ASSERT(ci != NULL);
	uuid_array_t RUEuuid;
	ci->GetUUID(RUEuuid);
	m_RUEList.AddRUEntry(RUEuuid);
	if (!m_IsReadOnly && m_bMaintainDateTimeStamps) {
   		ci->SetATime();
       	SetChanged(TimeStamp);
    }
    // All code using ci must be before this AutoType since the
	// latter may trash *ci if lock-on-minimize
	AutoType(*ci);
  }
}

void
DboxMain::AutoType(const CItemData &ci)
{
  CMyString AutoCmd = ci.GetAutoType();
 const CMyString user(ci.GetUser());
 const CMyString pwd(ci.GetPassword());
 if(AutoCmd.IsEmpty()){
   // checking for user and password for default settings
   if(!pwd.IsEmpty()){
     if(!user.IsEmpty())
       AutoCmd="\\u\\t\\p\\n";
     else
       AutoCmd="\\p\\n";
   }
 }
		
 CMyString tmp;
 char curChar;
 const int N = AutoCmd.GetLength();
 CKeySend ks;
 ks.ResetKeyboardState();

 // Note that minimizing the window before calling ci.Get*()
 // will cause garbage to be read if "lock on minimize" selected,
 // since that will clear the data [Bugs item #1026630]
 // (this is why we read user & pwd before actual use)

 ShowWindow(SW_MINIMIZE);
 Sleep(1000); // Karl Student's suggestion, to ensure focus set correctly on minimize.

 for(int n=0; n < N; n++){
   curChar=AutoCmd[n];
   if(curChar=='\\'){
     n++;
     if(n<N)
       curChar=AutoCmd[n];
     switch(curChar){
     case '\\':
       tmp += '\\';
       break;
     case 'n':case 'r':
       tmp += '\r';
       break;
     case 't':
       tmp += '\t';
       break;
     case 'u':
       tmp += user;
       break;
     case 'p':
       tmp += pwd;
       break;
     case 'd': {
       // Delay is going to change - send what we have with old delay
       ks.SendString(tmp);
       // start collecting new delay
       tmp = "";
       int newdelay = 0;
       int gNumIts = 0;
						
       for(n++; n<N && (gNumIts < 3); ++gNumIts, n++)
         if(isdigit(AutoCmd[n])){
           newdelay *= 10;
           newdelay += (AutoCmd[n]-'0');
         } else
           break; // for loop
       n--;							
       ks.SetAndDelay(newdelay);

       break; // case
     }
     default:
       tmp+="\\"+curChar;
       break;
     }
   }
   else
     tmp += curChar;
 }
 ks.SendString(tmp);
}

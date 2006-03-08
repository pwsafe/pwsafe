/// file DboxTray.cpp
//
// Tray-related methods of DboxMain
//-----------------------------------------------------------------------------

#include "PasswordSafe.h"
#include "ThisMfcApp.h"

#if defined(POCKET_PC)
  #include "pocketpc/resource.h"
#else
  #include <errno.h>
  #include "resource.h"
#endif

#include "DboxMain.h"
#include "corelib/pwsprefs.h"
#include "corelib/pwscore.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////// New System Tray Commands /////////////////////
#ifndef POCKET_PC
void
DboxMain::OnTrayLockUnLock()
{
	CString cMenuString;
	CMyString passkey;
	int rc;

	if (!app.GetSystemTrayState() == UNLOCKED) {  // User clicked UnLock
		rc = GetAndCheckPassword(m_core.GetCurFile(), passkey, GCP_NORMAL);  // OK, CANCEL, HELP
		if (rc != PWScore::SUCCESS)
			return;

		rc = m_core.ReadCurFile(passkey);
		if (rc == PWScore::SUCCESS) {
			m_needsreading = false;
			m_existingrestore = FALSE;
			startLockCheckTimer();
			UpdateSystemTray(UNLOCKED);
			RefreshList();
	    } else {
	    	m_needsreading = TRUE;
	    	m_existingrestore = FALSE;
	    	UpdateSystemTray(LOCKED);
	    	ClearClipboard();
	    	ShowWindow(SW_MINIMIZE);
	    	if (PWSprefs::GetInstance()->
                GetPref(PWSprefs::BoolPrefs::UseSystemTray))
	    		ShowWindow(SW_HIDE);
	      	return;
	    }
	} else {						// User clicked Lock
		UpdateSystemTray(LOCKED);
		ClearClipboard();
		ShowWindow(SW_MINIMIZE);
		ShowWindow(SW_HIDE);
	}
}

void
DboxMain::OnUpdateTrayLockUnLockCommand(CCmdUI *pCmdUI)
{
	const CString csUnLock = _T("Unlock Database");
	const CString csLock = _T("Lock Database");

	// Set text to "UnLock" or "Lock"
	if (app.GetSystemTrayState() == ThisMfcApp::UNLOCKED) {
		pCmdUI->SetText(csLock);
	} else {
		pCmdUI->SetText(csUnLock);
	}
	// If dialog visible - obviously unlocked and no need to have option to lock
	if (this->IsWindowVisible() == FALSE)
		pCmdUI->Enable(TRUE);
	else
		pCmdUI->Enable(FALSE);
}

void
DboxMain::OnTrayClearRecentEntries()
{
  ClearTrayRecentEntries();
}

void
DboxMain::OnUpdateTrayClearRecentEntries(CCmdUI *pCmdUI)
{
	if (pCmdUI->m_pSubMenu != NULL) {
        // disable or enable entire popup for "Recent Entries"
        // CCmdUI::Enable is a no-op for this case, so we
        //   must do what it would have done.
        pCmdUI->m_pMenu->EnableMenuItem(pCmdUI->m_nIndex,
            MF_BYPOSITION |
			(app.GetSystemTrayState() == ThisMfcApp::UNLOCKED ? MF_ENABLED : MF_GRAYED));
        return;
    }
    // otherwise enable
	if (m_RecentEntriesList.GetCount() != 0)
		pCmdUI->Enable(TRUE);
	else
		pCmdUI->Enable(FALSE);
}

void
DboxMain::OnTrayCopyUsername(UINT nID)
{
	POSITION pw_listpos;  // for password list

	ASSERT((nID >= ID_MENUITEM_TRAYCOPYUSERNAME1) && (nID <= ID_MENUITEM_TRAYCOPYUSERNAMEMAX));

	pw_listpos = GetPWEntryFromREList(nID - ID_MENUITEM_TRAYCOPYUSERNAME1);
	if (pw_listpos == NULL)
		return;

	CItemData &ci = m_core.GetEntryAt(pw_listpos);
	const CMyString username = ci.GetUser();
	if (!username.IsEmpty())
		ToClipboard(username);
}

void
DboxMain::OnUpdateTrayCopyUsername(CCmdUI *)
{
}

void
DboxMain::OnTrayCopyPassword(UINT nID)
{
	POSITION pw_listpos;  // for password list

	ASSERT((nID >= ID_MENUITEM_TRAYCOPYPASSWORD1) && (nID <= ID_MENUITEM_TRAYCOPYPASSWORDMAX));

	pw_listpos = GetPWEntryFromREList(nID - ID_MENUITEM_TRAYCOPYPASSWORD1);
	if (pw_listpos == NULL)
		return;

	// Copy password to clipboard
	CItemData &ci = m_core.GetEntryAt(pw_listpos);
	const CMyString curPassString = ci.GetPassword();
	ToClipboard(curPassString);
}

void
DboxMain::OnUpdateTrayCopyPassword(CCmdUI *)
{
}

void
DboxMain::OnTrayBrowse(UINT nID)
{
  POSITION pw_listpos;  // for password list

  ASSERT((nID >= ID_MENUITEM_TRAYBROWSE1) && (nID <= ID_MENUITEM_TRAYBROWSEMAX));

  pw_listpos = GetPWEntryFromREList(nID - ID_MENUITEM_TRAYBROWSE1);
  if (pw_listpos == NULL)
    return;

  // Show the embedded URL
  CItemData &ci = m_core.GetEntryAt(pw_listpos);
  CMyString browseURL = ci.GetURL();
  if (!browseURL.IsEmpty()) { // XXX factor out with OnBrowse
    HINSTANCE stat = ::ShellExecute(NULL, NULL, browseURL,
                                    NULL, _T("."), SW_SHOWNORMAL);
    if (int(stat) < 32) {
#ifdef _DEBUG
      AfxMessageBox("oops");
#endif
    }
  }
}

void
DboxMain::OnUpdateTrayBrowse(CCmdUI *pCmdUI)
{
	// UNUSED_PARAMETER(pCmdUI);
	POSITION pw_listpos;  // for password list
	
	int nID = pCmdUI->m_nID;

	ASSERT((nID >= ID_MENUITEM_TRAYBROWSE1) && (nID <= ID_MENUITEM_TRAYBROWSEMAX));

	pw_listpos = GetPWEntryFromREList(nID - ID_MENUITEM_TRAYBROWSE1);
	if (pw_listpos == NULL)
		return;

	// Has it an embedded URL
	CItemData &ci = m_core.GetEntryAt(pw_listpos);
	if (ci.GetURL().IsEmpty()) {
		pCmdUI->Enable(FALSE);
	}
}

void
DboxMain::OnTrayDeleteEntry(UINT nID)
{
	POSITION re_listpos;  // for recent entry list

	ASSERT((nID >= ID_MENUITEM_TRAYDELETE1) && (nID <= ID_MENUITEM_TRAYDELETEMAX));

	re_listpos = m_RecentEntriesList.FindIndex(nID - ID_MENUITEM_TRAYDELETE1);
	ASSERT(re_listpos != NULL);

	m_RecentEntriesList.RemoveAt(re_listpos);
}

void
DboxMain::OnUpdateTrayDeleteEntry(CCmdUI *)
{
}

void
DboxMain::OnTrayAutoType(UINT nID)
{
	POSITION pw_listpos;  // for password list

	ASSERT((nID >= ID_MENUITEM_TRAYAUTOTYPE1) && (nID <= ID_MENUITEM_TRAYAUTOTYPEMAX));

	pw_listpos = GetPWEntryFromREList(nID - ID_MENUITEM_TRAYAUTOTYPE1);
	if (pw_listpos == NULL)
		return;

	const CItemData &ci = m_core.GetEntryAt(pw_listpos);
	AutoType(ci);
}

void
DboxMain::OnUpdateTrayAutoType(CCmdUI *)
{
}

void
DboxMain::ClearTrayRecentEntries()
{
	m_RecentEntriesList.RemoveAll();
}

void
DboxMain::AddTrayRecentEntry(const CMyString &group, const CMyString &title,
                             const CMyString &user)
{
	CMyString cEntry =
      MRE_FS + group + MRE_FS + title + MRE_FS + user + MRE_FS;
	POSITION re_listpos;

	re_listpos = m_RecentEntriesList.GetHeadPosition();
	while (re_listpos != NULL)
	{
		const CMyString &re_Entry = m_RecentEntriesList.GetAt(re_listpos);
		if (re_Entry == cEntry)
			break;
		else
			m_RecentEntriesList.GetNext(re_listpos);
	}

	const int maxmruitems = 25; // XXX TBD PWSprefs::GetInstance()->
                                // GetPref(PWSprefs::IntPrefs::MaxREItems);

	if (re_listpos == NULL) {
		if (m_RecentEntriesList.GetCount() == maxmruitems)
			m_RecentEntriesList.RemoveTail();

		m_RecentEntriesList.AddHead(cEntry);
	} else {
		m_RecentEntriesList.RemoveAt(re_listpos);
		m_RecentEntriesList.AddHead(cEntry);
	}
}

void
DboxMain::RenameTrayRecentEntry(const CMyString &oldgroup, const CMyString &oldtitle,
                                const CMyString &olduser, const CMyString &newgroup,
                                const CMyString &newtitle, const CMyString &newuser)
{
	CMyString coldEntry = 
      MRE_FS + oldgroup + MRE_FS + oldtitle + MRE_FS + olduser + MRE_FS;

	POSITION re_listpos = m_RecentEntriesList.GetHeadPosition();

	while (re_listpos != NULL)
	{
		const CMyString &re_Entry = m_RecentEntriesList.GetAt(re_listpos);
		if (re_Entry == coldEntry)
			break;
		else
			m_RecentEntriesList.GetNext(re_listpos);
	}

	if (re_listpos != NULL)
      m_RecentEntriesList.RemoveAt(re_listpos);

	CMyString cnewEntry =
      MRE_FS + newgroup + MRE_FS + newtitle + MRE_FS + newuser + MRE_FS;
	m_RecentEntriesList.AddHead(cnewEntry);
}

void
DboxMain::DeleteTrayRecentEntry(const CMyString &group, const CMyString &title,
                                const CMyString &user)
{
	CMyString	cEntry =
      MRE_FS + group + MRE_FS + title + MRE_FS + user + MRE_FS;

	POSITION re_listpos = m_RecentEntriesList.GetHeadPosition();
	while (re_listpos != NULL)
	{
		const CMyString &re_Entry = m_RecentEntriesList.GetAt(re_listpos);
		if (re_Entry == cEntry) {
			m_RecentEntriesList.RemoveAt(re_listpos);
			break;
		} else
			m_RecentEntriesList.GetNext(re_listpos);
	}
}

POSITION
DboxMain::GetPWEntryFromREList(UINT nID_offset)
{
    CMyString group, title, user;
    POSITION re_listpos;  // for recent entry list
	POSITION pw_listpos;  // for password list

	re_listpos = m_RecentEntriesList.FindIndex(nID_offset);
	const CMyString &cEntry = m_RecentEntriesList.GetAt(re_listpos);

	// Entry format: >group>title>username>
    AfxExtractSubString(group, cEntry, 1, MRE_FS[0]);
    AfxExtractSubString(title, cEntry, 2, MRE_FS[0]);
    AfxExtractSubString(user, cEntry, 3, MRE_FS[0]);

	pw_listpos = app.m_core.Find(group, title, user);
	if (pw_listpos == NULL) {
		// Entry does not exist anymore!
		m_RecentEntriesList.RemoveAt(re_listpos);
		AfxMessageBox(_T("Cannot process as this entry has been deleted from the open database."));
	}
	return pw_listpos;
}

#endif

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
  #include "resource2.h"  // Menu, Toolbar & Accelerator resources
  #include "resource3.h"  // String resources
#endif

#include "DboxMain.h"
#include "RUEList.h"
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

	switch(app.GetSystemTrayState()) {
		case ThisMfcApp::LOCKED:					// User clicked UnLock
			rc = GetAndCheckPassword(m_core.GetCurFile(), passkey, GCP_NORMAL);  // OK, CANCEL, HELP
			if (rc != PWScore::SUCCESS)
				break;

			rc = m_core.ReadCurFile(passkey);
			if (rc == PWScore::SUCCESS) {
				m_needsreading = false;
				startLockCheckTimer();
				UpdateSystemTray(UNLOCKED);
				RefreshList();
			} else {
				m_needsreading = true;
				UpdateSystemTray(LOCKED);
				app.ClearClipboardData();
				ShowWindow(SW_MINIMIZE);
				if (PWSprefs::GetInstance()->
					GetPref(PWSprefs::UseSystemTray))
					ShowWindow(SW_HIDE);
			}
			break;
		case ThisMfcApp::UNLOCKED:					// User clicked Lock
			UpdateSystemTray(LOCKED);
			app.ClearClipboardData();
			ShowWindow(SW_MINIMIZE);
			ShowWindow(SW_HIDE);
			break;
		case ThisMfcApp::CLOSED:
		default:
			break;
	}
	return;
}

void
DboxMain::OnUpdateTrayLockUnLockCommand(CCmdUI *pCmdUI)
{
	CString csUnLock, csLock, csClosed;
	csUnLock.LoadString(IDS_UNLOCKSAFE);
	csLock.LoadString(IDS_LOCKSAFE);
	csClosed.LoadString(IDS_NOSAFE);

	const int i_state = app.GetSystemTrayState();
	// Set text to "UnLock" or "Lock"
	switch (i_state) {
		case ThisMfcApp::UNLOCKED:
			pCmdUI->SetText(csLock);
			break;
		case ThisMfcApp::LOCKED:
			pCmdUI->SetText(csUnLock);
			break;
		case ThisMfcApp::CLOSED:
			pCmdUI->Enable(FALSE);
			pCmdUI->SetText(csClosed);
			break;
		default:
			break;
	}

	if (i_state != ThisMfcApp::CLOSED) {
		// If dialog visible - obviously unlocked and no need to have option to lock
		if (this->IsWindowVisible() == FALSE)
			pCmdUI->Enable(TRUE);
		else
			pCmdUI->Enable(FALSE);
	}
}

void
DboxMain::OnTrayClearRecentEntries()
{
	m_RUEList.ClearEntries();
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
	if (m_RUEList.GetCount() != 0)
		pCmdUI->Enable(TRUE);
	else
		pCmdUI->Enable(FALSE);
}

void
DboxMain::OnTrayCopyUsername(UINT nID)
{
  ASSERT((nID >= ID_MENUITEM_TRAYCOPYUSERNAME1) &&
         (nID <= ID_MENUITEM_TRAYCOPYUSERNAMEMAX));

  CItemData ci;
  m_RUEList.GetPWEntry(nID - ID_MENUITEM_TRAYCOPYUSERNAME1, ci);
  if (&ci == NULL) return;
  const CMyString username = ci.GetUser();
  if (!username.IsEmpty()) {
    ToClipboard(username);
    UpdateAccessTime(&ci);	
  }
}

void
DboxMain::OnUpdateTrayCopyUsername(CCmdUI *)
{
}

void
DboxMain::OnTrayCopyPassword(UINT nID)
{
	ASSERT((nID >= ID_MENUITEM_TRAYCOPYPASSWORD1) && (nID <= ID_MENUITEM_TRAYCOPYPASSWORDMAX));

	CItemData ci;
	m_RUEList.GetPWEntry(nID - ID_MENUITEM_TRAYCOPYPASSWORD1, ci);
	if (&ci == NULL) return;
	const CMyString curPassString = ci.GetPassword();
	ToClipboard(curPassString);
    UpdateAccessTime(&ci);
}

void
DboxMain::OnUpdateTrayCopyPassword(CCmdUI *)
{
}

void
DboxMain::OnTrayCopyNotes(UINT nID)
{
  ASSERT((nID >= ID_MENUITEM_TRAYCOPYNOTES1) && (nID <= ID_MENUITEM_TRAYCOPYNOTESMAX));

  CItemData ci;
  CString cs_text;
  m_RUEList.GetPWEntry(nID - ID_MENUITEM_TRAYCOPYNOTES1, ci);
  if (&ci == NULL)
    return;

  const CMyString notes = ci.GetNotes();
  const CMyString url = ci.GetURL();
  const CMyString autotype = ci.GetAutoType();
  CMyString clipboard_data;

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
    UpdateAccessTime(&ci);
  }
}

void
DboxMain::OnUpdateTrayCopyNotes(CCmdUI *)
{
}

void
DboxMain::OnTrayBrowse(UINT nID)
{
  ASSERT((nID >= ID_MENUITEM_TRAYBROWSE1) && (nID <= ID_MENUITEM_TRAYBROWSEMAX));

  CItemData ci;
  m_RUEList.GetPWEntry(nID - ID_MENUITEM_TRAYBROWSE1, ci);
  if (&ci == NULL) return;

  CMyString browseURL = ci.GetURL();
  if (!browseURL.IsEmpty()) {
    LaunchBrowser(browseURL);
  }
  UpdateAccessTime(&ci);
}


void
DboxMain::OnUpdateTrayBrowse(CCmdUI *pCmdUI)
{
  int nID = pCmdUI->m_nID;

  ASSERT((nID >= ID_MENUITEM_TRAYBROWSE1) && (nID <= ID_MENUITEM_TRAYBROWSEMAX));

  CItemData ci;
  m_RUEList.GetPWEntry(nID - ID_MENUITEM_TRAYBROWSE1, ci);
  if (&ci == NULL) return;

  // Has it an embedded URL
  if (ci.GetURL().IsEmpty()) {
    pCmdUI->Enable(FALSE);
  }
}

void
DboxMain::OnTrayDeleteEntry(UINT nID)
{
	ASSERT((nID >= ID_MENUITEM_TRAYDELETE1) && (nID <= ID_MENUITEM_TRAYDELETEMAX));

	m_RUEList.DeleteRUEntry(nID - ID_MENUITEM_TRAYDELETE1);
}

void
DboxMain::OnUpdateTrayDeleteEntry(CCmdUI *)
{
}

void
DboxMain::OnTrayAutoType(UINT nID)
{
  ASSERT((nID >= ID_MENUITEM_TRAYAUTOTYPE1) && (nID <= ID_MENUITEM_TRAYAUTOTYPEMAX));

  CItemData ci;
  m_RUEList.GetPWEntry(nID - ID_MENUITEM_TRAYAUTOTYPE1, ci);
  if (&ci == NULL) return;
  AutoType(ci);
  UpdateAccessTime(&ci);
}

void
DboxMain::OnUpdateTrayAutoType(CCmdUI *)
{
}

#endif

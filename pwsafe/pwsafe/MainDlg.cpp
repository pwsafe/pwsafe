/// \file MainDlg.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "PasswordSafe.h"

// dialog boxen
#include "MainDlg.h"
#include "ClearQuestionDlg.h"
#include "ConfirmDeleteDlg.h"
#include "AddDlg.h"
#include "EditDlg.h"
#include "PasskeyChangeDlg.h"
#include "OptionsDlg.h"
#include "PasskeyEntry.h"
#include "PasskeySetup.h"
#include "RemindSaveDlg.h"
#include "QuerySetDef.h"
#include "QueryAddName.h"
#include "UsernameEntry.h"
#include "FileDialogExt.h"
#include "TryAgainDlg.h"

// widget override?
#include "SysColStatic.h"

#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <afxpriv.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//-----------------------------------------------------------------------------
class CAboutDlg
   : public CDialog
{
public:
   CAboutDlg()
      : CDialog(CAboutDlg::IDD)
   {}

   enum { IDD = IDD_ABOUTBOX };

protected:
   virtual void DoDataExchange(CDataExchange* pDX)    // DDX/DDV support
   {
      CDialog::DoDataExchange(pDX);
   }

protected:
   DECLARE_MESSAGE_MAP()
};

// I don't think we need this, but...
BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
CPasswordSafeDlg::CPasswordSafeDlg(CWnd* pParent)
   : CDialog(CPasswordSafeDlg::IDD, pParent)
{
   m_hIcon = app.LoadIcon(IDI_CORNERICON);
   m_pwlist.RemoveAll();

   CString temp;
   temp.LoadString(IDS_OUTPUTFILE);
   CString temp2 = app.m_curdir.m_mystring + temp;
   m_deffile = (CMyString)temp2;
   m_currfile =
      (CMyString)(app.GetProfileString("", "currentfile", temp2));
   m_currbackup =
      (CMyString)(app.GetProfileString("", "currentbackup", NULL));
   m_title = "";

   m_changed = FALSE;
   m_needsreading = TRUE;
   m_windowok = FALSE;
   m_existingrestore = FALSE;

   m_toolbarsSetup = FALSE;
}


void
CPasswordSafeDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPasswordSafeDlg, CDialog)
   ON_WM_DESTROY()
   ON_WM_PAINT()
   ON_WM_QUERYDRAGICON()
   ON_WM_SIZE()
   ON_COMMAND(ID_MENUITEM_ABOUT, OnAbout)
   ON_COMMAND(ID_MENUITEM_COPYUSERNAME, OnCopyUsername)
   ON_WM_CONTEXTMENU()
   ON_WM_VKEYTOITEM()
   ON_COMMAND(ID_MENUITEM_COPYPASSWORD, OnCopyPassword)
   ON_COMMAND(ID_MENUITEM_NEW, OnNew)
   ON_COMMAND(ID_MENUITEM_OPEN, OnOpen)
   ON_COMMAND(ID_MENUITEM_RESTORE, OnRestore)
   ON_COMMAND(ID_MENUTIME_SAVEAS, OnSaveAs)
   ON_COMMAND(ID_MENUITEM_BACKUPSAFE, OnBackupSafe)
   ON_COMMAND(ID_MENUITEM_UPDATEBACKUPS, OnUpdateBackups)
   ON_COMMAND(ID_MENUITEM_CHANGECOMBO, OnPasswordChange)
   ON_COMMAND(ID_MENUITEM_CLEARCLIPBOARD, OnClearclipboard)
   ON_COMMAND(ID_MENUITEM_DELETE, OnDelete)
   ON_COMMAND(ID_MENUITEM_EDIT, OnEdit)
   ON_COMMAND(ID_MENUITEM_OPTIONS, OnOptions)
   ON_COMMAND(ID_MENUITEM_SAVE, OnSave)
   ON_COMMAND(ID_MENUITEM_ADD, OnAdd)
   ON_COMMAND(ID_MENUITEM_EXIT, OnOK)
   ON_LBN_DBLCLK(IDC_ITEMLIST, OnCopyPassword)
   ON_COMMAND(ID_TOOLBUTTON_ADD, OnAdd)
   ON_COMMAND(ID_TOOLBUTTON_COPYPASSWORD, OnCopyPassword)
   ON_COMMAND(ID_TOOLBUTTON_COPYUSERNAME, OnCopyUsername)
   ON_COMMAND(ID_TOOLBUTTON_DELETE, OnDelete)
   ON_COMMAND(ID_TOOLBUTTON_EDIT, OnEdit)
   ON_COMMAND(ID_TOOLBUTTON_NEW, OnNew)
   ON_COMMAND(ID_TOOLBUTTON_OPEN, OnOpen)
   ON_COMMAND(ID_TOOLBUTTON_SAVE, OnSave)
   ON_BN_CLICKED(IDOK, OnEdit)
   ON_LBN_SETFOCUS(IDC_ITEMLIST, OnSetfocusItemlist)
   ON_LBN_KILLFOCUS(IDC_ITEMLIST, OnKillfocusItemlist)
   ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipText)
   ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipText)
END_MESSAGE_MAP()


BOOL
CPasswordSafeDlg::OnInitDialog()
{
   CDialog::OnInitDialog();

   if (OpenOnInit()==FALSE) // If this function fails, abort launch
      return TRUE;

   m_windowok = TRUE;
	
   // Set the icon for this dialog.  The framework does this automatically
   //  when the application's main window is not a dialog

   SetIcon(m_hIcon, TRUE);  // Set big icon
   SetIcon(m_hIcon, FALSE); // Set small icon
	
   m_listctrl = (CListBox*)GetDlgItem(IDC_ITEMLIST);
   m_listctrl->ModifyStyle(0, LVS_SHOWSELALWAYS, 0);
   RefreshList();

   ChangeOkUpdate();

   if (app.GetProfileInt("", "donebackupchange", FALSE) == FALSE)
      OnUpdateBackups();

   setupBars(); // Just to keep things a little bit cleaner
	
   return TRUE;  // return TRUE unless you set the focus to a control
}


BOOL
CPasswordSafeDlg::OpenOnInit(void)
{
   /*
     Routine to account for the differences between opening PSafe for
     the first time, and just opening a different database or
     un-minimizing the application
   */

   CMyString passkey;
   int rc;
   int rc2;

   rc = CheckPassword(m_currfile, passkey, TRUE);
   switch (rc)
   {
   case SUCCESS:
      rc2 = ReadFile(m_currfile, passkey);
      m_title = "Password Safe - " + m_currfile;
      break; 
   case CANT_OPEN_FILE:
      /*
       * If it is the default filename, assume that this is the first time
       * that they are starting Password Safe and don't confusing them.
       */
      if (m_currfile != m_deffile)
      {
         CMyString temp = m_currfile
            + "\n\nCannot open database. It likely does not exist."
            + "\nA new database will be created.";
         MessageBox(temp, "File open error.", MB_OK|MB_ICONWARNING);
      }
   case TAR_NEW:
      rc2 = New();
      break;
   case TAR_OPEN:
      rc2 = Open();
      break;
   case WRONG_PASSWORD:
      rc2 = NOT_SUCCESS;
      break;
   default:
      rc2 = NOT_SUCCESS;
      break;
   }

   if (rc2 == SUCCESS)
   {
      m_needsreading = FALSE;
      m_existingrestore = FALSE;
      return TRUE;
   }
   else
   {
      app.m_pMainWnd = NULL;
      CDialog::OnCancel();
      return FALSE;
   }
}


void
CPasswordSafeDlg::setupBars()
{
   // This code is copied from the DLGCBR32 example that comes with MFC

   const UINT statustext = IDS_STATMESSAGE;

   // Add the status bar
   if (m_statusBar.Create(this))
   {                           
      m_statusBar.SetIndicators(&statustext, 1);
      // Make a sunken or recessed border around the first pane
      m_statusBar.SetPaneInfo(0, m_statusBar.GetItemID(0), SBPS_STRETCH, NULL);
   }             

   // Add the ToolBar.
   if (!m_wndToolBar.Create(this) || !m_wndToolBar.LoadToolBar(IDR_MAINBAR))
   {
      TRACE0("Failed to create toolbar\n");
      return;      // fail to create
   }

   // TODO: Remove this if you don't want tool tips or a resizeable toolbar
   m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle()
                            | CBRS_TOOLTIPS | CBRS_FLYBY);

   // We need to resize the dialog to make room for control bars.
   // First, figure out how big the control bars are.
   CRect rcClientStart;
   CRect rcClientNow;
   GetClientRect(rcClientStart);
   RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST,
                  0, reposQuery, rcClientNow);

   // Now move all the controls so they are in the same relative
   // position within the remaining client area as they would be
   // with no control bars.
   CPoint ptOffset(rcClientNow.left - rcClientStart.left,
                   rcClientNow.top - rcClientStart.top); 

   CRect rcChild;                                 
   CWnd* pwndChild = GetWindow(GW_CHILD);
   while (pwndChild)
   {                               
      pwndChild->GetWindowRect(rcChild);
      ScreenToClient(rcChild);
      rcChild.OffsetRect(ptOffset);
      pwndChild->MoveWindow(rcChild, FALSE);
      pwndChild = pwndChild->GetNextWindow();
   }

   // Adjust the dialog window dimensions
   CRect rcWindow;
   GetWindowRect(rcWindow);
   rcWindow.right += rcClientStart.Width() - rcClientNow.Width();
   rcWindow.bottom += rcClientStart.Height() - rcClientNow.Height();
   MoveWindow(rcWindow, FALSE);

   // And position the control bars
   RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);

   // Set flag
   m_toolbarsSetup = TRUE;
}


void
CPasswordSafeDlg::OnDestroy()
{
   WinHelp(0L, HELP_QUIT);
   CDialog::OnDestroy();
}


// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void
CPasswordSafeDlg::OnPaint() 
{
   if (IsIconic())
   {
      CPaintDC dc(this); // device context for painting

      SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

      // Center icon in client rectangle
      int cxIcon = GetSystemMetrics(SM_CXICON);
      int cyIcon = GetSystemMetrics(SM_CYICON);
      CRect rect;
      GetClientRect(&rect);
      int x = (rect.Width() - cxIcon + 1) / 2;
      int y = (rect.Height() - cyIcon + 1) / 2;

      // Draw the icon
      dc.DrawIcon(x, y, m_hIcon);
   }
   else
   {
      CDialog::OnPaint();
   }
}


// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR
CPasswordSafeDlg::OnQueryDragIcon()
{
   return (HCURSOR) m_hIcon;
}

//Add an item
void
CPasswordSafeDlg::OnAdd() 
{
   CAddDlg dataDlg(this);
   if (app.GetProfileInt("", "usedefuser", FALSE) == TRUE)
   {
      CString temp = app.GetProfileString("", "defusername", "");
      dataDlg.m_username = (CMyString)temp;
      trashMemory(temp);
   }
   int rc = dataDlg.DoModal();
	
   if (rc == IDOK)
   {
      //Check if they wish to set a default username
      if ((app.GetProfileInt("", "usedefuser", FALSE) == FALSE)
          && (app.GetProfileInt("", "querysetdef", TRUE) == TRUE)
          && (dataDlg.m_username != ""))
      {
         CQuerySetDef defDlg(this);
         defDlg.m_message =
            "Would you like to set \""
            + dataDlg.m_username.m_mystring
            + "\" as your default username?\n\nIt would then automatically be "
            + "put in the dialog each time you add a new item.  Also only"
            + " non-default usernames will be displayed in the main window.";
         int rc2 = defDlg.DoModal();
         if (rc2 == IDOK)
         {
            app.WriteProfileInt("", "usedefuser", TRUE);
            app.WriteProfileString("", "defusername",
                                   dataDlg.m_username.m_mystring);
            DropDefUsernames(&m_pwlist, dataDlg.m_username);
            RefreshList();
         }
      }
      //Finish Check (Does that make any geographical sense?)
      CItemData temp;
      CMyString temptitle;
      MakeName(temptitle, dataDlg.m_title, dataDlg.m_username);
      temp.SetName(temptitle);
      temp.SetPassword(dataDlg.m_password);
      temp.SetNotes(dataDlg.m_notes);
      POSITION curPos = m_pwlist.AddTail(temp);
      int newpos = m_listctrl->AddString(temptitle);
      m_listctrl->SetCurSel(newpos);
      m_listctrl->SetFocus();
      m_changed = TRUE;
      ChangeOkUpdate();
   }
   else if (rc == IDCANCEL)
   {
   }
}

void
CPasswordSafeDlg::OnCopyPassword() 
{
   if (SelItemOk() == TRUE)
   {
      int curSel = m_listctrl->GetCurSel();
      CMyString curSelString;
      CString temp;
      m_listctrl->GetText(curSel, temp);
      curSelString = (CMyString)temp;
      trashMemory(temp);
      POSITION itemPos = Find(curSelString);
		
      CMyString curPassString;
      m_pwlist.GetAt(itemPos).GetPassword(curPassString);

      uGlobalMemSize = curPassString.GetLength()+1;
      hGlobalMemory = GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE, uGlobalMemSize);
      char* pGlobalLock = (char*)GlobalLock(hGlobalMemory);

      memcpy(pGlobalLock, curPassString, curPassString.GetLength());
		
      pGlobalLock[uGlobalMemSize-1] = '\0';
      GlobalUnlock(hGlobalMemory);	
		
      if (OpenClipboard() == TRUE)
      {
         if (EmptyClipboard()!=TRUE)
            AfxMessageBox("The clipboard was not emptied correctly");
         if (SetClipboardData(CF_TEXT, hGlobalMemory) == NULL)
            AfxMessageBox("The data was not pasted into the clipboard "
                          "correctly");
         if (CloseClipboard() != TRUE)
            AfxMessageBox("The clipboard could not be closed");
      }
      else
         AfxMessageBox("The clipboard could not be opened correctly");
		
      //Remind the user about clipboard security
      CClearQuestionDlg clearDlg(this);
      if (clearDlg.m_dontaskquestion == FALSE)
      {
         int rc = clearDlg.DoModal();
         if (rc == IDOK)
         {
         }
         else if (rc == IDCANCEL)
         {
         }
      }
   }
}


void
CPasswordSafeDlg::OnDelete() 
{
   if (SelItemOk() == TRUE)
   {
      BOOL dodelete = TRUE;
		
      //Confirm whether to delete the file
      CConfirmDeleteDlg deleteDlg(this);
      if (deleteDlg.m_dontaskquestion == FALSE)
      {
         int rc = deleteDlg.DoModal();
         if (rc == IDOK)
         {
            dodelete = TRUE;
         }
         else if (rc == IDCANCEL)
         {
            dodelete = FALSE;
         }
      }

      if (dodelete == TRUE)
      {
         m_changed = TRUE;
         int curSel = m_listctrl->GetCurSel();
         CMyString curText;
         CString temp;
         m_listctrl->GetText(curSel, temp);
         curText = (CMyString)temp;
         trashMemory(temp);
         int ctrlindex = m_listctrl->FindStringExact(-1, (LPCTSTR)curText);
         m_listctrl->DeleteString(ctrlindex);
         POSITION listindex = Find(curText);
         m_pwlist.RemoveAt(listindex);
         int rc = m_listctrl->SetCurSel(curSel);
         if (rc == LB_ERR)
         {
            m_listctrl->SetCurSel(m_listctrl->GetCount()-1);
         }
         m_listctrl->SetFocus();
         ChangeOkUpdate();
      }
   }
}


void
CPasswordSafeDlg::OnEdit() 
{
   if (SelItemOk() == TRUE)
   {
      int curSel = m_listctrl->GetCurSel();
		
      CMyString curText;
      CString temp1;
      m_listctrl->GetText(curSel, temp1);
      curText = (CMyString)temp1;
      trashMemory(temp1);

      int ctrlindex = m_listctrl->FindStringExact(-1, (LPCTSTR)curText);

      POSITION listindex = Find(curText);
      CItemData item = m_pwlist.GetAt(listindex);
      //CMyString item_name;
      //item.GetName(item_name);

      CEditDlg dlg_edit(this);
      SplitName(item.GetName(),
                dlg_edit.m_title, dlg_edit.m_username);
      //item.GetPassword(dlg_edit.m_realpassword);
      dlg_edit.m_realpassword = item.GetPassword();
      dlg_edit.m_password = dlg_edit.GetAsterisk(dlg_edit.m_realpassword);
      //item.GetNotes(dlg_edit.m_notes);
      dlg_edit.m_notes = item.GetNotes();

      int rc = dlg_edit.DoModal();

      if (rc == IDOK)
      {
         CMyString temptitle;
         MakeName(temptitle, dlg_edit.m_title, dlg_edit.m_username);
         item.SetName(temptitle);

#if 0
         // JPRFIXME - P1.2
         //Adjust for the asterisks
         if (dlg_edit.m_password.GetLength() == 0)
            item.SetPassword(dlg_edit.m_password);
         else if (dlg_edit.m_password[dlg_edit.m_password.GetLength()-1] == '*')
            item.SetPassword(dlg_edit.m_realpassword);
         else
            item.SetPassword(dlg_edit.m_password);
#endif
         item.SetPassword(dlg_edit.m_realpassword);
         item.SetNotes(dlg_edit.m_notes);

         /*
           Out with the old, in with the new
         */
         m_pwlist.RemoveAt(listindex);
         POSITION curPos = m_pwlist.AddTail(item);
         m_listctrl->DeleteString(ctrlindex);
         m_listctrl->AddString(temptitle);
         m_changed = TRUE;
      }

      m_listctrl->SetCurSel(curSel);
      m_listctrl->SetFocus();
      ChangeOkUpdate();
   }
}


void
CPasswordSafeDlg::OnOK() 
{
   int rc, rc2;

   if (m_changed == TRUE)
   {
      rc = MessageBox("Do you want to save changes to the password list?",
                             AfxGetAppName(),
                             MB_ICONQUESTION|MB_YESNOCANCEL);
      switch (rc)
      {
      case IDCANCEL:
         return;
      case IDYES:
         rc2 = Save();
         if (rc2 != SUCCESS)
            return;
      case IDNO:
         ClearClipboard();
         app.m_pMainWnd = NULL;
         break;
      }
   }
   else
   {
      ClearClipboard();
      app.m_pMainWnd = NULL;
   }

   ClearData();

   //Store current filename for next time...
   if (m_currfile!="")
      app.WriteProfileString("", "currentfile", m_currfile.m_mystring);
   else
      app.WriteProfileString("", "currentfile", NULL);

   if (m_currbackup!="")
      app.WriteProfileString("", "currentbackup", m_currbackup.m_mystring);
   else
      app.WriteProfileString("", "currentbackup", NULL);

   CDialog::OnOK();
}


void
CPasswordSafeDlg::OnCancel()
{
   OnOK();
}

void
CPasswordSafeDlg::ClearClipboard()
{
   if (OpenClipboard() != TRUE)
      AfxMessageBox("The clipboard could not be opened correctly");

   if (IsClipboardFormatAvailable(CF_TEXT) != 0)
   {
      HGLOBAL hglb = GetClipboardData(CF_TEXT); 
      if (hglb != NULL)
      {
         LPTSTR lptstr = (LPTSTR)GlobalLock(hglb); 
         if (lptstr != NULL)
         {
            trashMemory((unsigned char*)lptstr, strlen(lptstr));
            GlobalUnlock(hglb); 
         } 
      } 
   }
   if (EmptyClipboard()!=TRUE)
      AfxMessageBox("The clipboard was not emptied correctly");
   if (CloseClipboard() != TRUE)
      AfxMessageBox("The clipboard could not be closed");
}


//Finds stuff based on the .GetName() part not the entire object
POSITION
CPasswordSafeDlg::Find(CMyString lpszString)
{
   POSITION listPos = m_pwlist.GetHeadPosition();
   CMyString curthing;

   while (listPos != NULL)
   {
      m_pwlist.GetAt(listPos).GetName(curthing);
      if (curthing == lpszString)
         break;
      else
         m_pwlist.GetNext(listPos);
   }

   return listPos;
}


//Checks and sees if everything works and something is selected
BOOL
CPasswordSafeDlg::SelItemOk()
{
   int curSel = m_listctrl->GetCurSel();
   if (curSel != LB_ERR)
   {
      CMyString curText;
      CString temp;
      m_listctrl->GetText(curSel, temp);
      curText = (CMyString)temp;
      trashMemory(temp);
      int ctrlindex = m_listctrl->FindStringExact(-1, (LPCTSTR)curText);
      if (ctrlindex != LB_ERR)
      {
         POSITION listindex = Find(curText);
         if (listindex != NULL)
            return TRUE;
      }
   }
   return FALSE;
}


//Updates m_listctrl from m_pwlist
void
CPasswordSafeDlg::RefreshList()
{
   if (m_windowok == FALSE)
      return;

   //Copy the data
   m_listctrl->ResetContent();
   POSITION listPos = m_pwlist.GetHeadPosition();
   while (listPos != NULL)
   {
      CMyString temp;
      m_pwlist.GetAt(listPos).GetName(temp);
      m_listctrl->AddString(temp);
      m_pwlist.GetNext(listPos);
   }

   //Setup the selection
   if (m_listctrl->GetCount()>0)
      m_listctrl->SetCurSel(0);
}


void
CPasswordSafeDlg::OnPasswordChange() 
{
   /*
    * To change passkeys, the data is copied into a list of CMyStrings
    * and then re-put into the list with the new passkey
    */

   /*
    * CItemData should have a ChangePasskey method instead
    */

   /*
    * Here is my latest thought on this: It is definately possible to give
    * CItemData a ChangePasskey method. However, that would involve either
    * keeping two copies of the key schedule in memory at once, which would
    * then require a lot more overhead and variables than we currently have,
    * or recreating first the current and then the new schedule for each
    * item, which would be really slow. Which is why I think that we should
    * leave well enough alone. I mean, this function does work in the end.
    */
	
   CPasskeyChangeDlg changeDlg(this);
   int rc = changeDlg.DoModal();
   if (rc == IDOK)
   {
      m_changed = TRUE;
      //Copies the list into a plaintext list of CMyStrings
      CList<CMyString, CMyString> tempList;
      tempList.RemoveAll();
      POSITION listPos = m_pwlist.GetHeadPosition();
      while (listPos != NULL)
      {
         CItemData temp;
         temp = m_pwlist.GetAt(listPos);
         CMyString str;
         temp.GetName(str);
         tempList.AddTail(str);
         temp.GetPassword(str);
         tempList.AddTail(str);
         temp.GetNotes(str);
         tempList.AddTail(str);
         m_pwlist.GetNext(listPos);
      }
      m_pwlist.RemoveAll();
      listPos = tempList.GetHeadPosition();

      //Changes the global password. Eck.
      app.m_passkey = changeDlg.m_newpasskey;
		
      //Gets a new random value used for password authentication
      for (int x=0; x<8; x++)
         app.m_randstuff[x] = newrand();

      GenRandhash(changeDlg.m_newpasskey,
                  app.m_randstuff,
                  app.m_randhash);

      //Puts the list of CMyStrings back into CItemData
      while (listPos != NULL)
      {
         CItemData temp;
			
         temp.SetName(tempList.GetAt(listPos));
         tempList.GetNext(listPos);
			
         temp.SetPassword(tempList.GetAt(listPos));
         tempList.GetNext(listPos);

         temp.SetNotes(tempList.GetAt(listPos));
         tempList.GetNext(listPos);

         m_pwlist.AddTail(temp);
      }
		
      RefreshList();
   }
   else if (rc == IDCANCEL)
   {
   }
}


void
CPasswordSafeDlg::OnClearclipboard() 
{
   ClearClipboard();
}


void
CPasswordSafeDlg::OnSize(UINT nType,
                         int cx,
                         int cy) 
//Note that onsize runs before InitDialog (Gee, I love MFC)
{
   CDialog::OnSize(nType, cx, cy);

   if (nType == SIZE_MINIMIZED)
   {
      if (app.GetProfileInt("",
                            "dontaskminimizeclearyesno",
                            FALSE) == TRUE)
      {
         ClearClipboard();
      }
      if (app.GetProfileInt("", "databaseclear", FALSE) == TRUE)
      {
         BOOL dontask = app.GetProfileInt("",
                                          "dontasksaveminimize",
                                          FALSE);
         BOOL doit = TRUE;
         if ((m_changed == TRUE)
             && (dontask == FALSE))
         {
            CRemindSaveDlg remindDlg(this);

            int rc = remindDlg.DoModal();
            if (rc == IDOK)
            {
            }
            else if (rc == IDCANCEL)
            {
               doit = FALSE;
            }
         }

         if ((doit == TRUE) && (m_existingrestore == FALSE)) 
         {
            OnSave();
            ClearData();
            m_needsreading = TRUE;
         }
      }
   }
   else if (nType == SIZE_RESTORED)
   {
      if ((m_needsreading == TRUE)
          && (m_existingrestore == FALSE)
          && (m_windowok ==TRUE))
      {
         m_existingrestore = TRUE;

         CMyString passkey;
         int rc, rc2;
         CMyString temp;

         rc = CheckPassword(m_currfile, passkey);
         switch (rc)
         {
         case SUCCESS:
            rc2 = ReadFile(m_currfile, passkey);
            m_title = "Password Safe - " + m_currfile;
            break; 
         case CANT_OPEN_FILE:
            temp =
               m_currfile
               + "\n\nCannot open database. It likely does not exist."
               + "\nA new database will be created.";
            MessageBox(temp, "File open error.", MB_OK|MB_ICONWARNING);
         case TAR_NEW:
            rc2 = New();
            break;
         case TAR_OPEN:
            rc2 = Open();
            break;
         case WRONG_PASSWORD:
            rc2 = NOT_SUCCESS;
            break;
         default:
            rc2 = NOT_SUCCESS;
            break;
         }

         if (rc2 == SUCCESS)
         {
            m_needsreading = FALSE;
            m_existingrestore = FALSE;
         }
         else
         {
            app.m_pMainWnd = NULL;
            CDialog::OnCancel();
         }
      }
      RefreshList();
   }
}


void
CPasswordSafeDlg::OnSave() 
{
   Save();
}


int
CPasswordSafeDlg::Save()
{
   int rc;

   if (m_currfile == "")
      return SaveAs();

   rc = WriteFile(m_currfile);

   if (rc == CANT_OPEN_FILE)
   {
      CMyString temp = m_currfile + "\n\nCould not open file for writting!";
      MessageBox(temp, "File write error.", MB_OK|MB_ICONWARNING);
      return CANT_OPEN_FILE;
   }

   m_changed = FALSE;
   ChangeOkUpdate();
   return SUCCESS;
}


void
CPasswordSafeDlg::OnOptions() 
{
   COptionsDlg optionsDlg(this);
   BOOL currUseDefUser = optionsDlg.m_usedefuser;
   CMyString currDefUsername = optionsDlg.m_defusername;

   int rc = optionsDlg.DoModal();
   if (rc == IDOK)
   {
      if (currDefUsername != optionsDlg.m_defusername)
      {
         if (currUseDefUser == TRUE)
            MakeFullNames(&m_pwlist, currDefUsername);
         if (optionsDlg.m_usedefuser==TRUE)
            DropDefUsernames(&m_pwlist, optionsDlg.m_defusername);

         RefreshList();
      }
      else if (currUseDefUser != optionsDlg.m_usedefuser)
      {
         //Only check box has changed
         if (currUseDefUser == TRUE)
            MakeFullNames(&m_pwlist, currDefUsername);
         else
            DropDefUsernames(&m_pwlist, optionsDlg.m_defusername);
         RefreshList();
      }
   }
   else if (rc == IDCANCEL)
   {
   }
}


void
CPasswordSafeDlg::ChangeOkUpdate()
{
   if (m_windowok == FALSE)
      return;

   if (m_changed == TRUE)
      GetMenu()->EnableMenuItem(ID_MENUITEM_SAVE, MF_ENABLED);
   else if (m_changed == FALSE)
      GetMenu()->EnableMenuItem(ID_MENUITEM_SAVE, MF_GRAYED);

   /*
     This doesn't exactly belong here, but it makes sure that the
     title is fresh...
   */
   SetWindowText(LPCTSTR(m_title));
}


void
CPasswordSafeDlg::OnAbout() 
{
   CAboutDlg dlgAbout;
   dlgAbout.DoModal();
}


void
CPasswordSafeDlg::OnCopyUsername() 
{
   if (SelItemOk() == TRUE)
   {
      int curSel = m_listctrl->GetCurSel();
      CMyString curSelString;
      CString temp;
      m_listctrl->GetText(curSel, temp);
      curSelString = (CMyString)temp;
      trashMemory(temp);
      POSITION itemPos = Find(curSelString);
		
      CMyString title, junk, username;
      m_pwlist.GetAt(itemPos).GetName(title);
      SplitName(title, junk, username);

      if (username.GetLength() == 0)
      {
         AfxMessageBox("There is no username associated with this item.");
      }
      else
      {
         uGlobalMemSize = username.GetLength()+1;
         hGlobalMemory = GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE,
                                     uGlobalMemSize);
         char* pGlobalLock = (char*)GlobalLock(hGlobalMemory);

         memcpy(pGlobalLock, username, username.GetLength());

         pGlobalLock[uGlobalMemSize-1] = '\0';
         GlobalUnlock(hGlobalMemory);	
		
         if (OpenClipboard() == TRUE)
         {
            if (EmptyClipboard()!=TRUE)
               AfxMessageBox("The clipboard was not emptied correctly");
            if (SetClipboardData(CF_TEXT, hGlobalMemory) == NULL)
               AfxMessageBox("The data was not pasted into the "
                             "clipboard correctly");
            if (CloseClipboard() != TRUE)
               AfxMessageBox("The clipboard could not be closed");
         }
         else
         {
            AfxMessageBox("The clipboard could not be opened correctly");
         }
         //No need to remind the user about clipboard security
         //as this is only a username
      }
   }
}


void
CPasswordSafeDlg::OnContextMenu(CWnd* pWnd,
                                CPoint point) 
{
   CPoint local = point;
   m_listctrl->ScreenToClient(&local);

   BOOL in = TRUE;
   int item = m_listctrl->ItemFromPoint(local, in);
   if (in==FALSE)  // If the point is in the listbox...
   {
      m_listctrl->SetCurSel(item);
      m_listctrl->SetFocus();

      CMenu menu;
      if (menu.LoadMenu(IDR_POPMENU))
      {
         CMenu* pPopup = menu.GetSubMenu(0);
         ASSERT(pPopup != NULL);

         pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON,
                                point.x, point.y,
                                this); // use this window for commands
      }
   }
}


int
CPasswordSafeDlg::OnVKeyToItem(UINT nKey,
                               CListBox* pListBox,
                               UINT nIndex) 
{
   int curSel = m_listctrl->GetCurSel();

   switch (nKey)
   {
   case VK_DELETE:
      OnDelete();
      return -2;
   case VK_INSERT:
      OnAdd();
      return -2;
   // JPRFIXME P1.8
   case VK_PRIOR:  //Page up
   case VK_HOME:
      m_listctrl->SetCurSel(0);
      m_listctrl->SetFocus();
      return -2;
   case VK_NEXT:   //Page Down
   case VK_END:
      m_listctrl->SetCurSel(m_listctrl->GetCount()-1);
      m_listctrl->SetFocus();
      return -2;
   case VK_UP:
   case VK_LEFT:
      if (curSel>0)
         m_listctrl->SetCurSel(curSel-1);
      m_listctrl->SetFocus();
      return -2;
   case VK_DOWN:
   case VK_RIGHT:
      if (curSel!=(m_listctrl->GetCount()-1))
         m_listctrl->SetCurSel(curSel+1);
      m_listctrl->SetFocus();
      return -2;
   case VK_CONTROL:
      return -2;
   }
   return CDialog::OnVKeyToItem(nKey, pListBox, nIndex);
}


void
CPasswordSafeDlg::OnBackupSafe() 
{
   BackupSafe();
}


int
CPasswordSafeDlg::BackupSafe()
{
   int rc;
   CMyString tempname;

   //SaveAs-type dialog box
   while (1)
   {
      CFileDialogExt fd(FALSE,
                        "bak",
                        m_currbackup,
                        OFN_PATHMUSTEXIST|OFN_HIDEREADONLY
                        | OFN_LONGNAMES|OFN_OVERWRITEPROMPT,
                        "Password Safe Backups (*.bak)|*.bak||",
                        this);
      fd.m_ofn.lpstrTitle = "Please Choose a Name for this Backup:";

      rc = fd.DoModal();
      if (rc == IDOK)
      {
         tempname = (CMyString)fd.GetPathName();
         break;
      }
      else
         return USER_CANCEL;
   }

   rc = WriteFile(tempname);
   if (rc == CANT_OPEN_FILE)
   {
      CMyString temp = tempname + "\n\nCould not open file for writting!";
      MessageBox(temp, "File write error.", MB_OK|MB_ICONWARNING);
      return CANT_OPEN_FILE;
   }

   m_currbackup = tempname;
   return SUCCESS;
}


void
CPasswordSafeDlg::OnOpen() 
{
   Open();
}


int
CPasswordSafeDlg::Open()
{
   int rc;
   CMyString newfile, passkey, temp;

   //Open-type dialog box
   while (1)
   {
      CFileDialogExt fd(TRUE,
                        "dat",
                        NULL,
                        OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_LONGNAMES,
                        "Password Safe Databases (*.dat)|*.dat|"
                        "Password Safe Backups (*.bak)|*.bak|"
                        "All files (*.*)|*.*|"
                        "|",
                        this);
      fd.m_ofn.lpstrTitle = "Please Choose a Database to Open:";
      rc = fd.DoModal();
      if (rc == IDOK)
      {
         //Check that this file isn't already open
         newfile = (CMyString)fd.GetPathName();
         if (newfile == m_currfile)
         {
            //It is the same damn file
            MessageBox("That file is already open.",
                       "Oops!",
                       MB_OK|MB_ICONWARNING);
            continue;
         }
         break;
      }
      else
         return USER_CANCEL;
   }

   if (m_changed == TRUE)
   {
      int rc2;

      temp =
         "Do you want to save changes to the password databse: "
         + m_currfile
         + "?";
      rc = MessageBox(temp,
                      AfxGetAppName(),
                      MB_ICONQUESTION|MB_YESNOCANCEL);
      switch (rc)
      {
      case IDCANCEL:
         return USER_CANCEL;
      case IDYES:
         rc2 = Save();
         // Make sure that writting the file was successful
         if (rc2 == SUCCESS)
            break;
         else
            return CANT_OPEN_FILE;
      case IDNO:
         break;
      }
   }

   rc = CheckPassword(newfile, passkey);
   switch (rc)
   {
   case SUCCESS:
      break; // Keep going... 
   case CANT_OPEN_FILE:
      temp = m_currfile + "\n\nCan't open file. Please choose another.";
      MessageBox(temp, "File open error.", MB_OK|MB_ICONWARNING);
   case TAR_OPEN:
      return Open();
   case TAR_NEW:
      return New();
   case WRONG_PASSWORD:
      /*
        If the user just cancelled out of the password dialog, 
        assume they want to return to where they were before... 
      */
      return USER_CANCEL;
   }

   rc = ReadFile(newfile, passkey);
   if (rc == CANT_OPEN_FILE)
   {
      temp = newfile + "\n\nCould not open file for reading!";
      MessageBox(temp, "File read error.", MB_OK|MB_ICONWARNING);
      /*
        Everything stays as is... Worst case,
        they saved their file....
      */
      return CANT_OPEN_FILE;
   }

   m_currfile = newfile;
   m_changed = FALSE;
   m_title = "Password Safe - " + m_currfile;
   ChangeOkUpdate();
   RefreshList();

   return SUCCESS;
}


void
CPasswordSafeDlg::OnNew()
{
   New();
}


int
CPasswordSafeDlg::New() 
{
   int rc, rc2;

   if (m_changed==TRUE)
   {
      CMyString temp =
         "Do you want to save changes to the password database: "
         + m_currfile
         + "?";

      rc = MessageBox(temp,
                      AfxGetAppName(),
                      MB_ICONQUESTION|MB_YESNOCANCEL);
      switch (rc)
      {
      case IDCANCEL:
         return USER_CANCEL;
      case IDYES:
         rc2 = Save();
         /*
           Make sure that writting the file was successful
         */
         if (rc2 == SUCCESS)
            break;
         else
            return CANT_OPEN_FILE;
      case IDNO:
         break;
      }
   }

   rc = NewFile();
   if (rc == USER_CANCEL)
      /*
        Everything stays as is... 
        Worst case, they saved their file.... 
      */
      return USER_CANCEL;

   m_currfile = ""; //Force a save as... 
   m_changed = FALSE;
   m_title = "Password Safe - <Untitled>";
   ChangeOkUpdate();

   return SUCCESS;
}


void
CPasswordSafeDlg::OnRestore()
{
   Restore();
}


int
CPasswordSafeDlg::Restore() 
{
   int rc;
   CMyString newback, passkey, temp;

   //Open-type dialog box
   while (1)
   {
      CFileDialogExt fd(TRUE,
                        "bak",
                        m_currbackup,
                        OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_LONGNAMES,
                        "Password Safe Backups (*.bak)|*.bak||",
                        this);
      fd.m_ofn.lpstrTitle = "Please Choose a Backup to Restore:";
      rc = fd.DoModal();
      if (rc == IDOK)
      {
         newback = (CMyString)fd.GetPathName();
         break;
      }
      else
         return USER_CANCEL;
   }

   rc = CheckPassword(newback, passkey);
   switch (rc)
   {
   case SUCCESS:
      break; // Keep going... 
   case CANT_OPEN_FILE:
      temp =
         m_currfile
         + "\n\nCan't open file. Please choose another.";
      MessageBox(temp, "File open error.", MB_OK|MB_ICONWARNING);
   case TAR_OPEN:
      return Open();
   case TAR_NEW:
      return New();
   case WRONG_PASSWORD:
      /*
        If the user just cancelled out of the password dialog, 
        assume they want to return to where they were before... 
      */
      return USER_CANCEL;
   }

   if (m_changed==TRUE)
   {
      int rc2;
	
      temp = "Do you want to save changes to the password list: "
         + m_currfile + "?";

      rc = MessageBox(temp,
                      AfxGetAppName(),
                      MB_ICONQUESTION|MB_YESNOCANCEL);
      switch (rc)
      {
      case IDCANCEL:
         return USER_CANCEL;
      case IDYES:
         rc2 = Save();
         //Make sure that writting the file was successful
         if (rc2 == SUCCESS)
            break;
         else
            return CANT_OPEN_FILE;
      case IDNO:
         break;
      }
   }

   rc = ReadFile(newback, passkey);
   if (rc == CANT_OPEN_FILE)
   {
      temp = newback + "\n\nCould not open file for reading!";
      MessageBox(temp, "File read error.", MB_OK|MB_ICONWARNING);
      //Everything stays as is... Worst case, they saved their file....
      return CANT_OPEN_FILE;
   }
	
   m_currfile = ""; //Force a save as...
   m_changed = TRUE; //So that the *.dat version of the file will be saved.
   m_title = "Password Safe - <Untitled Restored Backup>";
   ChangeOkUpdate();
   RefreshList();

   return SUCCESS;
}


void
CPasswordSafeDlg::OnSaveAs()
{
   SaveAs();
}


int
CPasswordSafeDlg::SaveAs() 
{
   int rc;
   CMyString newfile;

   //SaveAs-type dialog box
   while (1)
   {
      CFileDialogExt fd(FALSE,
                        "dat",
                        m_currfile,
                        OFN_PATHMUSTEXIST|OFN_HIDEREADONLY
                        |OFN_LONGNAMES|OFN_OVERWRITEPROMPT,
                        "Password Safe Databases (*.dat)|*.dat||",
                        this);
      if (m_currfile == "")
         fd.m_ofn.lpstrTitle =
            "Please Choose a Name for the Current (Untitled) Database:";
      else
         fd.m_ofn.lpstrTitle =
            "Please Choose a New Name for the Current Database:";
      rc = fd.DoModal();
      if (rc == IDOK)
      {
         newfile = (CMyString)fd.GetPathName();
         break;
      }
      else
         return USER_CANCEL;
   }

   rc = WriteFile(newfile);
   if (rc == CANT_OPEN_FILE)
   {
      CMyString temp = newfile + "\n\nCould not open file for writting!";
      MessageBox(temp, "File write error.", MB_OK|MB_ICONWARNING);
      return CANT_OPEN_FILE;
   }

   m_currfile = newfile;
   m_changed = FALSE;
   m_title = "Password Safe - " + m_currfile;
   ChangeOkUpdate();
	
   return SUCCESS;
}


int
CPasswordSafeDlg::WriteFile(CMyString filename)
{
   int out = _open((LPCTSTR)filename,
                   _O_BINARY|_O_WRONLY|_O_SEQUENTIAL|_O_TRUNC|_O_CREAT,
                   _S_IREAD | _S_IWRITE);

   if (out == -1)
      return CANT_OPEN_FILE;

   _write(out, app.m_randstuff, 8);
   _write(out, app.m_randhash, 20);

   /*
     I know salt is just salt, but randomness always makes me
     nervous - must check this out {jpr}
    */
   unsigned char* thesalt = new unsigned char[SaltLength];
   for (int x=0; x<SaltLength; x++)
      thesalt[x] = newrand();

   _write(out, thesalt, SaltLength);
	
   unsigned char ipthing[8];
   for (x=0; x<8; x++)
      ipthing[x] = newrand();
   _write(out, ipthing, 8);

   //Write out full names
   BOOL needexpand = app.GetProfileInt("", "usedefuser", FALSE);
   CString defusername = app.GetProfileString("", "defusername", "");
   if (needexpand==TRUE)
      MakeFullNames(&m_pwlist, defusername);

   CItemData temp;
   POSITION listPos = m_pwlist.GetHeadPosition();
   CMyString tempdata;
   while (listPos != NULL)
   {
      temp = m_pwlist.GetAt(listPos);
      temp.GetName(tempdata);
      _writecbc(out, tempdata, thesalt, ipthing);
      temp.GetPassword(tempdata);
      _writecbc(out, tempdata, thesalt, ipthing);
      temp.GetNotes(tempdata);
      _writecbc(out, tempdata, thesalt, ipthing);
      m_pwlist.GetNext(listPos);
   }
   _close(out);

   delete [] thesalt;

   //Restore shortened names if necessary
   if (needexpand)
      DropDefUsernames(&m_pwlist, defusername);

   trashMemory(defusername);

   m_changed = FALSE;
   ChangeOkUpdate();

   return SUCCESS;
}


int
CPasswordSafeDlg::CheckPassword(CMyString filename,
                                CMyString& passkey,
                                BOOL first)
{
   CPasskeyEntry* passkeyDlg;
   unsigned char temprandstuff[8];
   unsigned char temprandhash[20];
   int retval;

   int in = _open((LPCTSTR)filename,
                  _O_BINARY|_O_RDONLY|_O_SEQUENTIAL,
                  S_IREAD | _S_IWRITE);
	
   if (in == -1)
      return CANT_OPEN_FILE;
					
   //Preserve the current randstuff and hash
   memcpy(temprandstuff, app.m_randstuff, 8);
   memcpy(temprandhash, app.m_randhash, 20);

   _read(in, app.m_randstuff, 8);
   _read(in, app.m_randhash, 20);
   _close(in);
	
   //The file might be deleted in CPasskeyEntry. hrrm
   passkeyDlg =  new CPasskeyEntry(this, first);
   app.m_pMainWnd = passkeyDlg;
   passkeyDlg->m_message = filename.m_mystring;
   int rc = passkeyDlg->DoModal();

   if (rc == IDOK)
   {
      passkey = passkeyDlg->m_passkey;
      retval = SUCCESS;
   }
   else /*if (rc==IDCANCEL) */ //Determine reason for cancel
   {
      int cancelreturn = passkeyDlg->GetCancelReturnValue();
      switch (cancelreturn)
      {
      case TAR_OPEN:
      case TAR_NEW:
         retval = cancelreturn;		//Return either open or new flag... 
         break;
      default:
         retval = WRONG_PASSWORD;	//Just a normal cancel
         break;
      }
   }

   //Restore the current randstuff and hash
   memcpy(app.m_randstuff, temprandstuff, 8);
   memcpy(app.m_randhash, temprandhash, 20);
   trashMemory(temprandstuff, 8);
   trashMemory(temprandhash, 20);
   delete passkeyDlg;

   return retval;
}


int
CPasswordSafeDlg::ReadFile(CMyString filename,
                           CMyString passkey)
{	
   //That passkey had better be the same one that came from CheckPassword(...)

   int in = _open((LPCTSTR)filename,
                  _O_BINARY|_O_RDONLY|_O_SEQUENTIAL, S_IREAD | _S_IWRITE);

   if (in == -1)
      return CANT_OPEN_FILE;

   ClearData(); //Before overwriting old data, but after opening the file... 
	
   _read(in, app.m_randstuff, 8);
   _read(in, app.m_randhash, 20);
   unsigned char* salt = new unsigned char[SaltLength];
   unsigned char ipthing[8];
   _read(in, salt, SaltLength);
   _read(in, ipthing, 8);
   app.m_passkey = passkey;

   CItemData temp;
   CMyString tempdata;

   int numread = 0;
   numread += _readcbc(in, tempdata, salt, ipthing);
   temp.SetName(tempdata);
   numread += _readcbc(in, tempdata, salt, ipthing);
   temp.SetPassword(tempdata);
   numread += _readcbc(in, tempdata, salt, ipthing);
   temp.SetNotes(tempdata);
   while (numread > 0)
   {
      m_pwlist.AddTail(temp);
      numread = 0;
      numread += _readcbc(in, tempdata, salt, ipthing);
      temp.SetName(tempdata);
      numread += _readcbc(in, tempdata, salt, ipthing);
      temp.SetPassword(tempdata);
      numread += _readcbc(in, tempdata, salt, ipthing);
      temp.SetNotes(tempdata);
   }
   delete [] salt;
   _close(in);

   //Shorten names if necessary
   if (app.GetProfileInt("", "usedefuser", FALSE) == TRUE)
   {
      CString temp = app.GetProfileString("", "defusername", "");
      DropDefUsernames(&m_pwlist, (CMyString)temp);
      trashMemory(temp);
   }

   //See if we should add usernames to an old version file
   if (app.GetProfileInt("", "queryaddname", TRUE) == TRUE
       && (CheckVersion(&m_pwlist) == V10))
   {
      //No splits and no defusers
      CQueryAddName dlg(this);
      int response = dlg.DoModal();
      if (response == IDOK)
      {
         CUsernameEntry dlg2(this);
         int response2 = dlg2.DoModal();
         if (response2 == IDOK)
         {
            if (dlg2.m_makedefuser == TRUE)
            {
               //MakeLongNames if this changes a set default username
               SetBlankToDef(&m_pwlist);
            }
            else
            {
               SetBlankToName(&m_pwlist, dlg2.m_username);
            }
            m_changed = TRUE;
         }
         else if (response2 == IDCANCEL)
         {
         }
      }
      else if (response == IDCANCEL)
      {
      }
   }
   return SUCCESS;
}


int
CPasswordSafeDlg::NewFile(void)
{
   CPasskeySetup passkeyDlg(this);
   app.m_pMainWnd = &passkeyDlg;
   int rc = passkeyDlg.DoModal();

   if (rc == IDCANCEL)
      return USER_CANCEL;  //User cancelled password entry

   ClearData();

   app.m_passkey = passkeyDlg.m_passkey;

   for (int x=0; x<8; x++)
      app.m_randstuff[x] = newrand();

   GenRandhash(app.m_passkey, app.m_randstuff, app.m_randhash);

   return SUCCESS;
}


void
CPasswordSafeDlg::ClearData(void)
{
   trashMemory(app.m_passkey.m_mystring);
	
   //Composed of ciphertext, so doesn't need to be overwritten
   m_pwlist.RemoveAll();
	
   //Because GetText returns a copy, we cannot do anything about the names
   if (m_windowok == TRUE)
      //Have to make sure this doesn't cause an access violation
      m_listctrl->ResetContent();
}


struct backup_t
{
   CMyString name;
   CMyString location;
};


void
CPasswordSafeDlg::OnUpdateBackups() 
{
   int rc;
   CMyString temp;
   CList<backup_t, backup_t> backuplist;

   //Collect list of backups from registry
   //This code copied almost verbatim from the old BackupDlg.cpp
   CMyString companyname;
   VERIFY(companyname.LoadString(IDS_COMPANY) != 0);
	
   //We need to use the Win32SDK method because of RegEnumKeyEx
   CMyString subkeyloc =
      (CMyString)"Software\\" 
      + companyname 
      + (CMyString) "\\Password Safe\\Backup";
   HKEY subkey;
   DWORD disposition;
   LONG result = RegCreateKeyEx(HKEY_CURRENT_USER,
                                subkeyloc, 0, NULL,
                                REG_OPTION_VOLATILE,
                                KEY_ALL_ACCESS,
                                NULL,
                                &subkey, &disposition);
   if (result != ERROR_SUCCESS)
   {
      //AfxMessageBox("There was an error opening a registry key. Sorry.");
      return;
   }

   //If the key is new, it has no data
   if (disposition == REG_CREATED_NEW_KEY)
   {
      //AfxMessageBox("There are no filenames stored in the registry. Sorry.");
      RegCloseKey(subkey);
      return;
   }
	
   //Check if the key has any items (in this case, backup listings).
   //If yes, check if user wants to update them. If no, close key and return
   if (disposition == REG_OPENED_EXISTING_KEY)
   {
      DWORD test;
      rc = IDNO;
      RegQueryInfoKey(subkey,
                      NULL, NULL, NULL, NULL, NULL, NULL,
                      &test,
                      NULL, NULL, NULL, NULL);
      if (test!=0)
      {
         temp =
            (CMyString)
            "Password Safe has detected the presence of old backup records\n"
            "from Version 1.1 of this program.  If you wish, you can update\n"
            "these files to the current version (by simply adding a .bak "
            "extension).\n"
            "\nYou will be presented with a list of file locations and the "
            "opportunity\n"
            "to save them to a text file for future reference. Also, you can "
            "rerun this\n"
            "function at any time through the \"Update V1.1 Backups...\" "
            "menu item."
            "\n\nDo you wish to proceed?";

         rc = MessageBox(LPCTSTR(temp),
                                "Update Backups",
                                MB_YESNOCANCEL|MB_ICONWARNING);
      }
      if (rc!= IDYES)
      {
         RegCloseKey(subkey);
         return;
      }
	
      //Ok, we have the go-ahead. Collect the data and update it
      int x = 0;
      int result = ERROR_SUCCESS;
      char key[_MAX_PATH];
      unsigned char value[_MAX_PATH];
      DWORD keylen = _MAX_PATH, valuelen = _MAX_PATH;
      DWORD valtype = REG_SZ;
      backup_t temp;

      result = RegEnumValue(subkey, x, key, &keylen, NULL,
                            &valtype, value, &valuelen);
      keylen = _MAX_PATH; valuelen = _MAX_PATH;
      while (result != ERROR_NO_MORE_ITEMS)
      {
         temp.name = key;
         temp.location = value;
			
         BOOL resp = CheckExtension(temp.location, (CMyString) ".bak");
         if (resp == FALSE) // File has wrong extension.
         {
            int ret = rename(temp.location, temp.location + ".bak");
            if (ret == 0) //Success
            {
               temp.location = temp.location + ".bak";
               backuplist.AddTail(temp);				
            }
            else if (errno == EACCES)
               // There is already a .bak version around
            {;}
            else if (errno == ENOENT)
               // The old version no longer exists
            {
               CMyString out =
                  "Please note that the backup named \""
                  + temp.name
                  + "\"\nno longer exists.It will be removed"
                  " from the registry.";
               MessageBox(out, "File not found.", MB_OK|MB_ICONWARNING);
               temp.location = "";
               backuplist.AddTail(temp);
            }
         }	
         else
         {
            //Test to make sure it still exists
            int ret = rename(temp.location, temp.location);
            if (ret != 0 && errno == ENOENT)
            {
               CMyString out =
                  "Please note that the backup named \""
                  + temp.name 
                  + "\"\nno longer exists. It will be removed"
                  " from the registry.";
               MessageBox(out, "File not found.", MB_OK|MB_ICONWARNING);
               temp.location = "";
               backuplist.AddTail(temp);
            }
         }
         x++;
         result = RegEnumValue(subkey, x, key, &keylen, NULL,
                               &valtype, value, &valuelen);
         keylen = _MAX_PATH;
         valuelen = _MAX_PATH;
      }

      CMyString out = "The following files were altered:\n\n";
      CMyString out2 = "";
      POSITION listpos = backuplist.GetHeadPosition();
      while (listpos != NULL)
      {
         backup_t temp = backuplist.GetAt(listpos);
         if (temp.location != "")
         {
            out2 = out2 + temp.name + "\t" + temp.location + "\n";
         }
         backuplist.GetNext(listpos);
      }

      if (out2 == "")
         out2 = "None.\n";

      CMyString out3 =
         (CMyString)
         "\nDo you want to save a text version of this list?\n\n"
         "(The file will be called changedbackups.txt\n"
         "and will be saved in the current directory)";

      rc = MessageBox(out+out2+out3,
                             "Changed Files", MB_YESNOCANCEL);
      if (rc == IDYES)
      {
         int in = _open("changedbackups.txt",
                        _O_TEXT|_O_WRONLY|_O_SEQUENTIAL|_O_APPEND|_O_CREAT,
                        _S_IREAD | _S_IWRITE);
         if (in != -1)
         {
            // No error
            _write(in, LPCTSTR(out+out2), strlen(LPCTSTR(out+out2)));
            _close(in);
         }
      }


      //Write data back to registry. This will alter the names altered and deleted
      //the names not found. Copied from backupdlg.cpp
      POSITION listPos = backuplist.GetHeadPosition();
      while (listPos != NULL)
      {
         backup_t item = backuplist.GetAt(listPos);
         if (item.location != "")
            app.WriteProfileString("Backup", item.name, item.location);
         else
            app.WriteProfileString("Backup", item.name, NULL);

         backuplist.GetNext(listPos);
      }

      //Mark that this has been done.
      app.WriteProfileInt("", "donebackupchange", TRUE);
   }

   RegCloseKey(subkey);
}	


BOOL
CPasswordSafeDlg::OnToolTipText(UINT,
                                NMHDR* pNMHDR,
                                LRESULT* pResult)
// This code is copied from the DLGCBR32 example that comes with MFC
{
   ASSERT(pNMHDR->code == TTN_NEEDTEXTA || pNMHDR->code == TTN_NEEDTEXTW);

   // allow top level routing frame to handle the message
   if (GetRoutingFrame() != NULL)
      return FALSE;

   // need to handle both ANSI and UNICODE versions of the message
   TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
   TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;
   TCHAR szFullText[256];
   CString strTipText;
   UINT nID = pNMHDR->idFrom;
   if (pNMHDR->code == TTN_NEEDTEXTA && (pTTTA->uFlags & TTF_IDISHWND) ||
       pNMHDR->code == TTN_NEEDTEXTW && (pTTTW->uFlags & TTF_IDISHWND))
   {
      // idFrom is actually the HWND of the tool
      nID = ((UINT)(WORD)::GetDlgCtrlID((HWND)nID));
   }

   if (nID != 0) // will be zero on a separator
   {
      AfxLoadString(nID, szFullText);
      // this is the command id, not the button index
      AfxExtractSubString(strTipText, szFullText, 1, '\n');
   }
#ifndef _UNICODE
   if (pNMHDR->code == TTN_NEEDTEXTA)
      lstrcpyn(pTTTA->szText, strTipText,
               (sizeof(pTTTA->szText)/sizeof(pTTTA->szText[0])));
#if 0 // build problem with new cl? - jpr
   else
      _mbstowcsz(pTTTW->szText, strTipText,
                 (sizeof(pTTTW->szText)/sizeof(pTTTW->szText[0])));
#endif // 0
#else
   if (pNMHDR->code == TTN_NEEDTEXTA)
      _wcstombsz(pTTTA->szText, strTipText,
                 (sizeof(pTTTA->szText)/sizeof(pTTTA->szText[0])));
   else
      lstrcpyn(pTTTW->szText, strTipText,
               (sizeof(pTTTW->szText)/sizeof(pTTTW->szText[0])));
#endif
   *pResult = 0;

   // bring the tooltip window above other popup windows
   ::SetWindowPos(pNMHDR->hwndFrom, HWND_TOP, 0, 0, 0, 0,
                  SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE);

   return TRUE;    // message was handled
}


void
CPasswordSafeDlg::OnSetfocusItemlist() 
{
   const UINT statustext = IDS_STATMESSAGE;

   if (m_toolbarsSetup == FALSE) {return;}

   m_statusBar.SetIndicators(&statustext, 1);	
   // Make a sunken or recessed border around the first pane
   m_statusBar.SetPaneInfo(0, m_statusBar.GetItemID(0), SBPS_STRETCH, NULL);
}


void
CPasswordSafeDlg::OnKillfocusItemlist() 
{
   const UINT statustext = IDS_STATCOMPANY;

   if (m_toolbarsSetup == FALSE)
      return;

   m_statusBar.SetIndicators(&statustext, 1);
   // Make a sunken or recessed border around the first pane
   m_statusBar.SetPaneInfo(0, m_statusBar.GetItemID(0), SBPS_STRETCH, NULL);
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

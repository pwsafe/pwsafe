/// \file PasskeyEntry.cpp
//-----------------------------------------------------------------------------

/*
  Passkey?  That's Russian for 'pass'.  You know, passkey
  down the streetsky.  [Groucho Marx]
*/

#include "PasswordSafe.h"

#include "ThisMfcApp.h"
#include "resource.h"

#include "MyString.h"

#include "SysColStatic.h"

#include "PasskeyEntry.h"
#include "TryAgainDlg.h"

#include "util.h"

#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>

//-----------------------------------------------------------------------------
CPasskeyEntry::CPasskeyEntry(CWnd* pParent,
                             const CString& a_filespec,
                             bool first)
   : CDialog(first ? CPasskeyEntry::IDD : CPasskeyEntry::IDD_BASIC,
             pParent),
     m_first(first),
     m_tries(0),
     m_status(TAR_INVALID)
{
   const int FILE_DISP_LEN = 45;	

	//{{AFX_DATA_INIT(CPasskeyEntry)
	//}}AFX_DATA_INIT

   DBGMSG("CPasskeyEntry()\n");
   if (first) {
      DBGMSG("** FIRST **\n");
   }

   m_passkey = "";

   if (a_filespec.GetLength() > FILE_DISP_LEN) {
	   m_message = a_filespec.Right(FILE_DISP_LEN - 3); // truncate for display

	   m_message.Insert(0, _T("..."));

   }

   else

   {
	   m_message = a_filespec;
   }
}


void CPasskeyEntry::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Text(pDX, IDC_PASSKEY, (CString &)m_passkey);

   //{{AFX_DATA_MAP(CPasskeyEntry)
   DDX_Text(pDX, IDC_MESSAGE, m_message);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPasskeyEntry, CDialog)
	//{{AFX_MSG_MAP(CPasskeyEntry)
   ON_BN_CLICKED(ID_HELP, OnHelp)
   ON_BN_CLICKED(ID_BROWSE, OnBrowse)
   ON_BN_CLICKED(ID_CREATE_DB, OnCreateDb)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL
CPasskeyEntry::OnInitDialog(void)
{
   CDialog::OnInitDialog();

   if (("" == m_message)
       && m_first)
   {
      //((CEdit*)GetDlgItem(IDC_PASSKEY))->SetReadOnly(TRUE);
      GetDlgItem(IDC_PASSKEY)->EnableWindow(FALSE);
      GetDlgItem(IDOK)->EnableWindow(FALSE);
      m_message = "[No current database]";
   }

   /*
    * this bit makes the background come out right on
    * the bitmaps
    */

   if (m_first)
   {
      m_Static.SubclassDlgItem(IDC_STATIC_ICON1, this);
      m_Static.ReloadBitmap(IDB_PSLOGO);
      m_Static2.SubclassDlgItem(IDC_STATIC_ICON2, this);
      m_Static2.ReloadBitmap(IDB_CLOGO);

#if 0 // this bitmap (funky "http://passwordsafe.sorcefourge.net") no longer displayed
      m_Static3.SubclassDlgItem(IDC_STATIC_ICON3, this);
      m_Static3.ReloadBitmap(IDB_CTEXT);

#endif
   }
   else
   {
      CWnd* pWnd = GetDlgItem(IDC_STATIC_ICON1);
      m_Static.SubclassWindow(pWnd->m_hWnd);
      m_Static.ReloadBitmap(IDB_CLOGO_SMALL);
   }
   
   return TRUE;
}


void
CPasskeyEntry::OnBrowse()
{
   m_status = TAR_OPEN;
   app.m_pMainWnd = NULL;
   CDialog::OnCancel();
}


void
CPasskeyEntry::OnCreateDb()
{
   m_status = TAR_NEW;
   app.m_pMainWnd = NULL;
   CDialog::OnCancel();
}


void
CPasskeyEntry::OnCancel() 
{
   app.m_pMainWnd = NULL;
   CDialog::OnCancel();
}


void
CPasskeyEntry::OnOK() 
{
   UpdateData(TRUE);

   unsigned char temphash[20]; // HashSize
   GenRandhash(m_passkey,
               app.m_randstuff,
               temphash);

   if (m_passkey == "")
   {
      AfxMessageBox("The combination cannot be blank.");
      ((CEdit*)GetDlgItem(IDC_PASSKEY))->SetFocus();
      return;
   }

   if (0 != memcmp((char*)app.m_randhash,
                   (char*)temphash,
                   20)) // HashSize
   {
      if (m_tries >= 2)
      {
         CTryAgainDlg errorDlg(this);

         int nResponse = errorDlg.DoModal();
         if (nResponse == IDOK)
         {
         }
         else if (nResponse == IDCANCEL)
         {
            m_status = errorDlg.GetCancelReturnValue();
            app.m_pMainWnd = NULL;
            CDialog::OnCancel();
         }
      }
      else
      {
         m_tries++;
         AfxMessageBox("Incorrect passkey");
         ((CEdit*)GetDlgItem(IDC_PASSKEY))->SetSel(MAKEWORD(-1, 0));
         ((CEdit*)GetDlgItem(IDC_PASSKEY))->SetFocus();
      }
   }
   else
   {
      app.m_pMainWnd = NULL;
      CDialog::OnOK();
   }
}


void
CPasskeyEntry::OnHelp() 
{
   //WinHelp(0x200B9, HELP_CONTEXT);
   ::HtmlHelp(NULL,
              "pwsafe.chm::/html/pws_combo_entry.htm",
              HH_DISPLAY_TOPIC, 0);
}


#if 0
int
CPasskeyEntry::GetCancelReturnValue()
{
   return tryagainreturnval;
}
#endif

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

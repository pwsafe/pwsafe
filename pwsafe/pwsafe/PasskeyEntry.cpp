/// \file PasskeyEntry.cpp
//-----------------------------------------------------------------------------

/*
  Passkey?  That's Russian for 'pass'.  You know, passkey
  down the streetsky.  [Groucho Marx]
*/

#include "stdafx.h"
#include "SysColStatic.h"
#include "PasswordSafe.h"
#include "PasskeyEntry.h"
#include "TryAgainDlg.h"
#include "util.h"

#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//-----------------------------------------------------------------------------
CPasskeyEntry::CPasskeyEntry(CWnd* pParent,
                             const CString& a_filespec,
                             BOOL first)
   :CDialog(first ? CPasskeyEntry::IDDFIRST : CPasskeyEntry::IDD,
            pParent)
{
   m_first = first;
   m_passkey = "";
   m_message = a_filespec;
   numtimes = 0;
   tryagainreturnval = TAR_INVALID;
}


void CPasskeyEntry::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Text(pDX, IDC_PASSKEY, m_passkey.m_mystring);
   DDX_Text(pDX, IDC_MESSAGE, m_message);
}


BEGIN_MESSAGE_MAP(CPasskeyEntry, CDialog)
   ON_BN_CLICKED(ID_HELP, OnHelp)
   ON_BN_CLICKED(ID_BROWSE, OnBrowse)
   ON_BN_CLICKED(ID_CREATE_DB, OnCreateDb)
END_MESSAGE_MAP()


BOOL
CPasskeyEntry::OnInitDialog(void)
{
   CDialog::OnInitDialog();

   if (("" == m_message)
       && (TRUE == m_first))
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

   if (m_first==TRUE)
   {
      m_Static.SubclassDlgItem(IDC_STATIC_ICON1, this);
      m_Static.ReloadBitmap(IDB_PSLOGO);
      m_Static2.SubclassDlgItem(IDC_STATIC_ICON2, this);
      m_Static2.ReloadBitmap(IDB_CLOGO);
      m_Static3.SubclassDlgItem(IDC_STATIC_ICON3, this);
      m_Static3.ReloadBitmap(IDB_CTEXT);
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
   tryagainreturnval = TAR_OPEN;
   app.m_pMainWnd = NULL;
   CDialog::OnCancel();
}


void
CPasskeyEntry::OnCreateDb()
{
   tryagainreturnval = TAR_NEW;
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
   unsigned char temphash[SaltSize];
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
                   SaltSize))
   {
      if (numtimes >= 2)
      {
         CTryAgainDlg errorDlg(this);

         int nResponse = errorDlg.DoModal();
         if (nResponse == IDOK)
         {
         }
         else if (nResponse == IDCANCEL)
         {
            tryagainreturnval = errorDlg.GetCancelReturnValue();
            app.m_pMainWnd = NULL;
            CDialog::OnCancel();
         }
      }
      else
      {
         numtimes++;
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
   WinHelp(0x200B9, HELP_CONTEXT);
}


int
CPasskeyEntry::GetCancelReturnValue()
{
   return tryagainreturnval;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

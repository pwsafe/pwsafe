/// \file PasskeyEntry.cpp
//-----------------------------------------------------------------------------

/*
  Passkey?  That's Russian for 'pass'.  You know, passkey
  down the streetsky.  [Groucho Marx]
*/

#include "PasswordSafe.h"

#include "ThisMfcApp.h"
#include "resource.h"

#include "corelib/MyString.h"

#include "SysColStatic.h"

#include "PasskeyEntry.h"
#include "TryAgainDlg.h"
#include "DboxMain.h" // for CheckPassword()

//-----------------------------------------------------------------------------
CPasskeyEntry::CPasskeyEntry(CWnd* pParent,
                             const CString& a_filespec,
                             bool first)
   : CDialog(first ? CPasskeyEntry::IDD : CPasskeyEntry::IDD_BASIC,
             pParent),
     m_first(first),
     m_filespec(a_filespec),
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
// m_message = a_filespec.Right(FILE_DISP_LEN - 3); // truncate for display
// m_message.Insert(0, _T("..."));
// changed by karel@VanderGucht.de to see beginning + ending of 'a_filespec'
      m_message =  a_filespec.Left(FILE_DISP_LEN/2-5) + " ... " + a_filespec.Right(FILE_DISP_LEN/2);

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

   if ( m_first )
	DDX_Control(pDX, IDC_STATIC_LOGOTEXT, m_ctlLogoText);

   //{{AFX_DATA_MAP(CPasskeyEntry)
	DDX_Control(pDX, IDC_STATIC_LOGO, m_ctlLogo);
	DDX_Control(pDX, IDOK, m_ctlOK);
	DDX_Control(pDX, IDC_PASSKEY, m_ctlPasskey);
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
  DboxMain* pParent = (DboxMain*) GetParent();
  ASSERT(pParent != NULL);
  CDialog::OnInitDialog();
  if (pParent->CheckPassword(m_filespec, m_passkey) != PWScore::SUCCESS)
    if (("" == m_message)
	&& m_first)
      {
	m_ctlPasskey.EnableWindow(FALSE);
	m_ctlOK.EnableWindow(FALSE);
	m_message = "[No current database]";
      }

  /*
   * this bit makes the background come out right on
   * the bitmaps
   */

  if (m_first)
    {
      m_ctlLogoText.ReloadBitmap(IDB_PSLOGO);
      m_ctlLogo.ReloadBitmap(IDB_CLOGO);
    }
  else
    {
      m_ctlLogo.ReloadBitmap(IDB_CLOGO_SMALL);
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

  if (m_passkey.IsEmpty())
    {
      AfxMessageBox("The combination cannot be blank.");
      m_ctlPasskey.SetFocus();
      return;
    }

  DboxMain* pParent = (DboxMain*) GetParent();
  ASSERT(pParent != NULL);
  if (pParent->CheckPassword(m_filespec, m_passkey) != PWScore::SUCCESS)
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
	  m_ctlPasskey.SetSel(MAKEWORD(-1, 0));
	  m_ctlPasskey.SetFocus();
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

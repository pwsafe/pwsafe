/// \file DboxMain.cpp
//-----------------------------------------------------------------------------

#include "PasswordSafe.h"

#include "ThisMfcApp.h"
#include "resource.h"

// dialog boxen
#include "DboxMain.h"
#include "OptionsSecurity.h"
#include "OptionsDisplay.h"
#include "OptionsUsername.h"
#include "OptionsPasswordPolicy.h"
#include "OptionsMisc.h"

#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <afxpriv.h>


#define not(x) ((x) ? 0 : 1)

void
DboxMain::OnOptions() 
{
   CPropertySheet optionsDlg("Options", this);

   COptionsDisplay         display;
   COptionsSecurity        security;
   COptionsPasswordPolicy  passwordpolicy;
   COptionsUsername        username;
   COptionsMisc            misc;

   /*
   **  Initialize the property pages values.
   */
   display.m_alwaysontop = m_bAlwaysOnTop;
   display.m_pwshowinedit = app.GetProfileInt("", "showpwdefault", FALSE);
   display.m_pwshowinlist = app.GetProfileInt("", "showpwinlist", FALSE);

   security.m_clearclipboard = (app.GetProfileInt("", "dontaskminimizeclearyesno", FALSE));
   security.m_lockdatabase = (app.GetProfileInt("", "databaseclear", FALSE));
   security.m_confirmsaveonminimize = not(app.GetProfileInt("", "dontasksaveminimize", FALSE));
   security.m_confirmcopy = not(app.GetProfileInt("", "dontaskquestion", FALSE));

   passwordpolicy.m_pwlendefault = app.GetProfileInt("", "pwlendefault", 8);
   passwordpolicy.m_pwuselowercase = app.GetProfileInt("", "pwuselowercase", TRUE);
   passwordpolicy.m_pwuseuppercase = app.GetProfileInt("", "pwuseuppercase", TRUE);
   passwordpolicy.m_pwusedigits = app.GetProfileInt("", "pwusedigits", TRUE);
   passwordpolicy.m_pwusesymbols = app.GetProfileInt("", "pwusesymbols", TRUE);
   passwordpolicy.m_pweasyvision = app.GetProfileInt("", "pweasyvision", FALSE);

   username.m_usedefuser = app.GetProfileInt("", "usedefuser", FALSE);
   username.m_defusername = app.GetProfileString("", "defusername", "");
   username.m_querysetdef = app.GetProfileInt("", "querysetdef", TRUE);

   misc.m_confirmdelete = not(app.GetProfileInt("", "deletequestion", FALSE));
   misc.m_saveimmediately = app.GetProfileInt("", "saveimmediately", TRUE);

   optionsDlg.AddPage( &display );
   optionsDlg.AddPage( &security );
   optionsDlg.AddPage( &passwordpolicy );
   optionsDlg.AddPage( &username );
   optionsDlg.AddPage( &misc );

   /*
   **  Save some values for use after the dialog completes.
   */
   BOOL      currUseDefUser = username.m_usedefuser;
   CMyString currDefUsername = username.m_defusername;

   /*
   **  Remove the "Apply Now" button.
   */
   optionsDlg.m_psh.dwFlags |= PSH_NOAPPLYNOW;
   int rc = optionsDlg.DoModal();

   if (rc == IDOK)
   {
      /*
      **  First save all the options.
      */
      app.WriteProfileInt("", "alwaysontop",    display.m_alwaysontop);
      app.WriteProfileInt("", "showpwdefault",  display.m_pwshowinedit);
      app.WriteProfileInt("", "showpwinlist",   display.m_pwshowinlist);

      app.WriteProfileInt("", "dontaskminimizeclearyesno",  security.m_clearclipboard);
      app.WriteProfileInt("", "databaseclear",              security.m_lockdatabase);
      app.WriteProfileInt("", "dontasksaveminimize",    not(security.m_confirmsaveonminimize));
      app.WriteProfileInt("", "dontaskquestion",        not(security.m_confirmcopy));

      app.WriteProfileInt("", "pwlendefault",   passwordpolicy.m_pwlendefault);
      app.WriteProfileInt("", "pwuselowercase", passwordpolicy.m_pwuselowercase);
      app.WriteProfileInt("", "pwuseuppercase", passwordpolicy.m_pwuseuppercase);
      app.WriteProfileInt("", "pwusedigits",    passwordpolicy.m_pwusedigits);
      app.WriteProfileInt("", "pwusesymbols",   passwordpolicy.m_pwusesymbols);
      app.WriteProfileInt("", "pweasyvision",   passwordpolicy.m_pweasyvision);

      app.WriteProfileInt("", "usedefuser",     username.m_usedefuser);
      app.WriteProfileString("", "defusername", username.m_defusername);
      app.WriteProfileInt("", "querysetdef",    username.m_querysetdef);

      app.WriteProfileInt("", "deletequestion",   not(misc.m_confirmdelete));
      app.WriteProfileInt("", "saveimmediately",      misc.m_saveimmediately);

      /*
      **  Now update the application according to the options.
      */
      pwchars.SetPool(passwordpolicy.m_pweasyvision,
                      passwordpolicy.m_pwuselowercase,
                      passwordpolicy.m_pwuseuppercase,
                      passwordpolicy.m_pwusedigits,
                      passwordpolicy.m_pwusesymbols);

      m_bAlwaysOnTop = display.m_alwaysontop;
      UpdateAlwaysOnTop();

      bool bOldShowPasswordInList = m_bShowPasswordInList;
      m_bShowPasswordInList = app.GetProfileInt("", "showpwinlist", FALSE)? true: false;
      if (currDefUsername != (CMyString)username.m_defusername)
      {
         if (TRUE == currUseDefUser)
            MakeFullNames(&m_pwlist, currDefUsername);
         if (TRUE == username.m_usedefuser)
            DropDefUsernames(&m_pwlist, username.m_defusername);

         RefreshList();
      }
      else if (currUseDefUser != username.m_usedefuser)
      {
         //Only check box has changed
         if (TRUE == currUseDefUser)
            MakeFullNames(&m_pwlist, currDefUsername);
         else
            DropDefUsernames(&m_pwlist, username.m_defusername);
         RefreshList();
      }
      else if ((bOldShowPasswordInList + m_bShowPasswordInList) == 1)
      {
         RefreshList();
      }
   }
   else if (rc == IDCANCEL)
   {
   }
}


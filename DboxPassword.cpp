/// \file DboxPassword.cpp
//-----------------------------------------------------------------------------

#include "PasswordSafe.h"

#include "resource.h"
#include "OptionsPasswordPolicy.h"
#include "corelib/Util.h"
#include "corelib/PWCharPool.h"
#include "corelib/PWSprefs.h"

#include "DboxMain.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
//-----------------------------------------------------------------------------

// generate a random password, using the default settings or user overrides
// the generated password will be copied to the clipboard. Doing
// this leaves a problem where the user can generate a random password, have
// the password copied to the clipboard and then change the password. This could
// be avoided by putting the password into the clipboard when the entry is saved
// but that would be annoying when generating a new entry.

bool
DboxMain::MakeRandomPassword( CDialog * const pDialog, CMyString& password)
{
  bool is_override = (pDialog->IsDlgButtonChecked(IDC_OVERRIDE_POLICY) == BST_CHECKED);
  CMyString temp;

  PWSprefs *prefs = PWSprefs::GetInstance();

  COptionsPasswordPolicy  passwordpolicy;
  passwordpolicy.m_pwlendefault = prefs->
    GetPref(PWSprefs::IntPrefs::PWLenDefault);
  passwordpolicy.m_pwuselowercase = prefs->
    GetPref(PWSprefs::BoolPrefs::PWUseLowercase);
  passwordpolicy.m_pwuseuppercase = prefs->
    GetPref(PWSprefs::BoolPrefs::PWUseUppercase);
  passwordpolicy.m_pwusedigits = prefs->
    GetPref(PWSprefs::BoolPrefs::PWUseDigits);
  passwordpolicy.m_pwusesymbols = prefs->
    GetPref(PWSprefs::BoolPrefs::PWUseSymbols);
  passwordpolicy.m_pwusehexdigits = prefs->
    GetPref(PWSprefs::BoolPrefs::PWUseHexDigits);
  passwordpolicy.m_pweasyvision = prefs->
    GetPref(PWSprefs::BoolPrefs::PWEasyVision);

  if (is_override) {
    // Start with existing password policy
    CPropertySheet optionsDlg(_T("Password Policy Override"), pDialog);

    // Display COptionsPasswordPolicy page
    optionsDlg.AddPage(&passwordpolicy);
    optionsDlg.m_psh.dwFlags |= PSH_NOAPPLYNOW; // remove "Apply Now" button

    // If the user cancels the dialog, the values will be left
    // at the default values, so the password will be generated
    // properly. That means we can actually ignore the return value here.
    (void)optionsDlg.DoModal();
  }

  while(1) {
    CPasswordCharPool pwchars(
			      passwordpolicy.m_pwlendefault,
			      passwordpolicy.m_pwuselowercase,
			      passwordpolicy.m_pwuseuppercase,
			      passwordpolicy.m_pwusedigits,
			      passwordpolicy.m_pwusesymbols,
			      passwordpolicy.m_pwusehexdigits,
			      passwordpolicy.m_pweasyvision);

    temp = pwchars.MakePassword();

    int nResponse;
    CMyString msg;
    msg = _T("The randomly generated password is: \"")
      + temp
      + _T("\" \n(without the quotes). Use it (Yes), or generate another (No)?");
    nResponse = pDialog->MessageBox(msg, AfxGetAppName(),
				    MB_ICONQUESTION|MB_YESNOCANCEL);

    if( IDYES == nResponse ) {
      password = temp;
      ToClipboard( password );
      return true;
    } else if( IDCANCEL == nResponse ) {
      return false;
    }
    ASSERT( IDNO == nResponse );
  }
}




#include "PWSprefs.h"

#include <AfxWin.h> // for AfxGetApp()
#include <strstream>
#include <LMCons.h> // for UNLEN
#include "PWSfile.h"
#include "SysInfo.h"
#include "XMLprefs.h"
#include "util.h"

using namespace std;

#if defined(POCKET_PC)
const LPCTSTR PWS_REG_POSITION = _T("Position");
const LPCTSTR PWS_REG_OPTIONS = _T("Options");
#else
const LPCTSTR PWS_REG_POSITION = _T("");
const LPCTSTR PWS_REG_OPTIONS = _T("");
#endif

PWSprefs *PWSprefs::self = NULL;

// 1st parameter = name of preference
// 2nd parameter = default value
// 3rd parameter if 'true' means value stored in db, if 'false' means application related
const PWSprefs::boolPref PWSprefs::m_bool_prefs[NumBoolPrefs] = {
	{_T("alwaysontop"), false, false},								// application
	{_T("showpwdefault"), false, true},								// database
	{_T("showpwinlist"), false, true},								// database
	{_T("sortascending"), true, true},								// database
	{_T("usedefuser"), false, true},								// database
	{_T("saveimmediately"), true, true},							// database
	{_T("pwuselowercase"), true, true},								// database
	{_T("pwuseuppercase"), true, true},								// database
	{_T("pwusedigits"), true, true},								// database
	{_T("pwusesymbols"), false, true},								// database
	{_T("pwusehexdigits"), false, true},							// database
	{_T("pweasyvision"), false, true},								// database
	{_T("dontaskquestion"), false, false},							// application
	{_T("deletequestion"), false, false},							// application
	{_T("DCShowsPassword"), false, false},							// application
	{_T("DontAskMinimizeClearYesNo"), true, false},					// application
	{_T("DatabaseClear"), false, false},							// application
	{_T("DontAskSaveMinimize"), false, false},						// application
	{_T("QuerySetDef"), true, false},								// application
	{_T("UseNewToolbar"), true, false},								// application
	{_T("UseSystemTray"), true, false},								// application
	{_T("LockOnWindowLock"), true, false},							// application
	{_T("LockOnIdleTimeout"), true, false},							// application
	{_T("EscExits"), true, false},									// application
	{_T("IsUTF8"), false, true},									// database
	{_T("HotKeyEnabled"), false, false},							// application
	{_T("MRUOnFileMenu"), true, false},								// application
	{_T("DisplayExpandedAddEditDlg"), true, true},					// database
	{_T("MaintainDateTimeStamps"), false, true},					// database
	{_T("SavePasswordHistory"), false, true},						// database
	{_T("FindWraps"), false, false},								// application
	{_T("ShowNotesDefault"), false, true},							// database
	{_T("BackupBeforeEverySave"), false, false},					// application
};

// Default value = -1 means set at runtime
// Extra two values for Integer - min and max acceptable values (ignored if = -1)
const PWSprefs::intPref PWSprefs::m_int_prefs[NumIntPrefs] = {
	{_T("column1width"), (unsigned int)-1, false, -1, -1}, 					// application
	{_T("column2width"), (unsigned int)-1, false, -1, -1}, 					// application
	{_T("column3width"), (unsigned int)-1, false, -1, -1}, 					// application
	{_T("column4width"), (unsigned int)-1, false, -1, -1}, 					// application
	{_T("sortedcolumn"), 0, false, 0, 7},									// application
	{_T("pwlendefault"), 8, true, 4, 1024},									// database
	// maxmruitems maximum = (ID_FILE_MRU_ENTRYMAX - ID_FILE_MRU_ENTRY1 + 1)
	{_T("maxmruitems"), 4, false, 0, 20},									// application
	{_T("IdleTimeout"), 5, true, 1, 120},									// database
	{_T("DoubleClickAction"), DoubleClickCopyPassword, false, minDCA, maxDCA},	// application
	{_T("HotKey"), 0, false, -1, -1}, // 0=disabled, >0=keycode.			// application
	// MaxREItems maximum = (ID_TRAYRECENT_ENTRYMAX - ID_TRAYRECENT_ENTRY1 + 1)
	{_T("MaxREItems"), 25, false, 0, 25},									// application
	{_T("TreeDisplayStatusAtOpen"), AllCollapsed, true, minTDS, maxTDS},	// database
	{_T("NumPWHistoryDefault"), 3, true, 0, 255},							// database
	// Default and user specified
	{_T("BackupPrefix"), 0, false, 0, 1},									// application
	// Specified by supported masks
	{_T("BackupSuffix"), 0, false, minBKSFX, maxBKSFX},						// application
	// default, user specified sub-directory and user specified other location
	{_T("BackupLocation"), 0, false, 0, 2},									// application
	{_T("BackupMaxIncremented"), 3, false, 1, 999},							// application
	{_T("UseDefaultBrowser"), 0, false, 0, 1},								// application
};

const PWSprefs::stringPref PWSprefs::m_string_prefs[NumStringPrefs] = {
	{_T("currentbackup"), _T(""), false},							// application
	{_T("currentfile"), _T(""), false},								// application
	{_T("lastview"), _T("tree"), true},								// database
	{_T("defusername"), _T(""), true},								// database
	{_T("treefont"), _T(""), false},								// application
	{_T("BackupPrefixValue"), _T(""), false},						// application
	{_T("BackupSubDirectoryValue"), _T(""), false},					// application
	{_T("BackupOtherLocationValue"), _T(""), false},				// application
	{_T("OtherBrowser"), _T(""), false},							// application
};


PWSprefs *PWSprefs::GetInstance()
{
  if (self == NULL) {
    self = new PWSprefs();
  }
  return self;
}

void PWSprefs::DeleteInstance()
{
  delete self->m_XML_Config; // should have ~PWSprefs
  delete self;
  self = NULL;
  SysInfo::DeleteInstance();
}

PWSprefs::PWSprefs() : m_app(::AfxGetApp())
{
  ASSERT(m_app != NULL);

  m_prefs_changed[DB_PREF] = false;
  m_prefs_changed[APP_PREF] = false;

  for (int i = 0; i < NumBoolPrefs; i++)
    m_boolChanged[i] = false;

  for (int i = 0; i < NumIntPrefs; i++)
    m_intChanged[i] = false;

  for (int i = 0; i < NumStringPrefs; i++)
    m_stringChanged[i] = false;

  InitializePreferences();
}

bool
PWSprefs::CheckRegistryExists()
{
	bool bExists;
	HKEY hSubkey;
	const CString csSubkey = _T("Software\\") + CString(m_app->m_pszRegistryKey);
	bExists = (::RegOpenKeyEx(HKEY_CURRENT_USER,
					csSubkey,
					0L,
					KEY_READ,
					&hSubkey) == ERROR_SUCCESS);
	if (bExists)
		::RegCloseKey(hSubkey);

	return bExists;
}

bool PWSprefs::GetPref(BoolPrefs pref_enum) const
{
	return m_boolValues[pref_enum];
}

unsigned int PWSprefs::GetPref(IntPrefs pref_enum) const
{
	return m_intValues[pref_enum];
}

CMyString PWSprefs::GetPref(StringPrefs pref_enum) const
{
	return m_stringValues[pref_enum];
}

// Following for case where default value is determined at runtime
unsigned int PWSprefs::GetPref(IntPrefs pref_enum, unsigned int defVal) const
{
    return m_intValues[pref_enum] == (unsigned int)-1 ? defVal : m_intValues[pref_enum];
}

void PWSprefs::GetPrefRect(long &top, long &bottom,
                           long &left, long &right) const
{
	// this is never stored in db
	switch (m_ConfigOptions) {
		case CF_REGISTRY:
			top = m_app->GetProfileInt(PWS_REG_POSITION, _T("top"), -1);
			bottom = m_app->GetProfileInt(PWS_REG_POSITION, _T("bottom"), -1);
			left = m_app->GetProfileInt(PWS_REG_POSITION, _T("left"), -1);
			right = m_app->GetProfileInt(PWS_REG_POSITION, _T("right"), -1);
			break;
		case CF_FILE_RO:
		case CF_FILE_RW:
		case CF_FILE_RW_NEW:
			m_XML_Config->SetKeepXMLLock(true);
			top = m_XML_Config->Get(m_csHKCU_POS, _T("top"), -1);
			bottom = m_XML_Config->Get(m_csHKCU_POS, _T("bottom"), -1);
			left = m_XML_Config->Get(m_csHKCU_POS, _T("left"), -1);
			right = m_XML_Config->Get(m_csHKCU_POS, _T("right"), -1);
			m_XML_Config->SetKeepXMLLock(false);
			break;
		case CF_NONE:
		default:
			top = bottom = left = right = -1;
			break;
	}
}

void PWSprefs::SetPref(BoolPrefs pref_enum, bool value)
{
	// ONLY save in memory - written out at database save (to database and config destination)
	m_prefs_changed[m_bool_prefs[pref_enum].isStoredinDB ? DB_PREF : APP_PREF] |=
		(m_boolValues[pref_enum] != value);

	if (m_boolValues[pref_enum] != value) { // Only if changed
		m_boolValues[pref_enum] = value;
		m_boolChanged[pref_enum] = true;
	}
}

void PWSprefs::SetPref(IntPrefs pref_enum, unsigned int value)
{
	// ONLY save in memory - written out at database save (to database and config destination)
	m_prefs_changed[m_int_prefs[pref_enum].isStoredinDB ? DB_PREF : APP_PREF] |=
		(m_intValues[pref_enum] != value);

	if (m_intValues[pref_enum] != value) { // Only if changed
		m_intValues[pref_enum] = value;
		m_intChanged[pref_enum] = true;
	}
}

void PWSprefs::SetPref(StringPrefs pref_enum, const CMyString &value)
{
	// ONLY save in memory - written out at database save (to database and config destination)
	m_prefs_changed[m_string_prefs[pref_enum].isStoredinDB ? DB_PREF : APP_PREF] |=
		(m_stringValues[pref_enum] != value);

	if (m_stringValues[pref_enum] != value) { // Only if changed
		m_stringValues[pref_enum] = value;
		m_stringChanged[pref_enum] = true;
	}
}

bool PWSprefs::WritePref(const CMyString &name, bool val)
{
	// Used to save to config destination at database save and application termination
	bool bRetVal(false);
	switch (m_ConfigOptions) {
		case CF_REGISTRY:
			bRetVal = (m_app->WriteProfileInt(PWS_REG_OPTIONS, name, val ? 1 : 0) == TRUE);
			break;
		case CF_FILE_RW:
		case CF_FILE_RW_NEW:
			bRetVal = (m_XML_Config->Set(m_csHKCU_PREF, name, val ? 1 : 0) == 0);
			break;
		case CF_FILE_RO:
		case CF_NONE:
		default:
			break;
	}
	UpdateTimeStamp();
	return bRetVal;
}

bool PWSprefs::WritePref(const CMyString &name, unsigned int val)
{
	// Used to save to config destination at database save and application termination
	bool bRetVal(false);
	switch (m_ConfigOptions) {
		case CF_REGISTRY:
			bRetVal = (m_app->WriteProfileInt(PWS_REG_OPTIONS, name, val) == TRUE);
			break;
		case CF_FILE_RW:
		case CF_FILE_RW_NEW:
			bRetVal = (m_XML_Config->Set(m_csHKCU_PREF, name, val) == 0);
			break;
		case CF_FILE_RO:
		case CF_NONE:
		default:
			break;
	}
	UpdateTimeStamp();
	return bRetVal;
}

bool PWSprefs::WritePref(const CMyString &name, const CMyString &val)
{
	// Used to save to config destination at database save and application termination
	bool bRetVal(false);
	switch (m_ConfigOptions) {
		case CF_REGISTRY:
			bRetVal = (m_app->WriteProfileString(PWS_REG_OPTIONS, name, val) == TRUE);
			break;
		case CF_FILE_RW:
		case CF_FILE_RW_NEW:
			bRetVal = (m_XML_Config->Set(m_csHKCU_PREF, name, val) == 0);
			break;
		case CF_FILE_RO:
		case CF_NONE:
		default:
			break;
	}
	UpdateTimeStamp();
	return bRetVal;
}

bool PWSprefs::DeletePref(const CMyString &name)
{
	bool bRetVal = true;
	switch (m_ConfigOptions) {
		case CF_REGISTRY:
			m_app->WriteProfileInt(PWS_REG_OPTIONS, name, NULL);
			break;
		case CF_FILE_RW:
		case CF_FILE_RW_NEW:
			bRetVal = (m_XML_Config->DeleteSetting(m_csHKCU_PREF, name) == TRUE);
			break;
		case CF_FILE_RO:
		case CF_NONE:
		default:
			break;
	}
	UpdateTimeStamp();
	return bRetVal;
}

void PWSprefs::SetKeepXMLLock(bool state)
{
  m_XML_Config->SetKeepXMLLock(state);
}


void PWSprefs::SetPrefRect(long top, long bottom,
                           long left, long right)
{
  UpdateTimeStamp();

  switch (m_ConfigOptions) {
  case CF_REGISTRY:
    m_app->WriteProfileInt(PWS_REG_POSITION, _T("top"), top);
    m_app->WriteProfileInt(PWS_REG_POSITION, _T("bottom"), bottom);
    m_app->WriteProfileInt(PWS_REG_POSITION, _T("left"), left);
    m_app->WriteProfileInt(PWS_REG_POSITION, _T("right"), right);
    break;
  case CF_FILE_RW:
  case CF_FILE_RW_NEW:
    {
      CString obuff;
      m_XML_Config->SetKeepXMLLock(true);
      obuff.Format("%d", top);
      VERIFY(m_XML_Config->Set(m_csHKCU_POS, _T("top"), obuff) == 0);
      obuff.Format("%d", bottom);
      VERIFY(m_XML_Config->Set(m_csHKCU_POS, _T("bottom"), obuff) == 0);
      obuff.Format("%d", left);
      VERIFY(m_XML_Config->Set(m_csHKCU_POS, _T("left"), obuff) == 0);
      obuff.Format("%d", right);
      VERIFY(m_XML_Config->Set(m_csHKCU_POS, _T("right"), obuff) == 0);
      m_XML_Config->SetKeepXMLLock(false);
    }
    break;
  case CF_FILE_RO:
  case CF_NONE:
  default:
    break;
  }

}

CMyString PWSprefs::Store()
{
	/*
	 * Create a string of values that are (1) different from the defaults, &&
     * (2) are isStoredinDB
	 * String is of the form "X nn vv X nn vv..." Where X=[BIS] for binary,
     * integer and string, resp.,
	 * nn is the numeric value of the enum, and vv is the value,
     * {1,0} for bool, unsigned integer for int, and quoted string for String.
	 */

	CMyString retval(_T(""));
	ostrstream os;

	for (int i = 0; i < NumBoolPrefs; i++)
		if (m_boolValues[i] != m_bool_prefs[i].defVal &&
			m_bool_prefs[i].isStoredinDB)
			os << "B " << i << ' ' << (m_boolValues[i] ? 1 : 0) << ' ';

	for (int i = 0; i < NumIntPrefs; i++)
		if (m_intValues[i] != m_int_prefs[i].defVal &&
			m_int_prefs[i].isStoredinDB)
			os << "I " << i << ' ' << m_intValues[i] << ' ';

	for (int i = 0; i < NumStringPrefs; i++)
		if (m_stringValues[i] != m_string_prefs[i].defVal &&
			m_string_prefs[i].isStoredinDB)
			os << "S " << i << " \"" << LPCTSTR(m_stringValues[i]) << "\" ";

	os << ends;
	retval = os.str();
	delete[] os.str(); // reports memory leaks in spite of this!
	return retval;
}

void PWSprefs::Load(const CMyString &prefString)
{
	// Set default values for preferences stored in Database
	for (int i = 0; i < NumBoolPrefs; i++)
		if (m_bool_prefs[i].isStoredinDB)
			m_boolValues[i] = m_bool_prefs[i].defVal != 0;

	for (int i = 0; i < NumIntPrefs; i++)
		if (m_int_prefs[i].isStoredinDB)
			m_intValues[i] = m_int_prefs[i].defVal;

	for (int i = 0; i < NumStringPrefs; i++)
		if (m_string_prefs[i].isStoredinDB)
	 		m_stringValues[i] = CMyString(m_string_prefs[i].defVal);

	if (prefString.GetLength() == 0)
		return;

	// parse prefString, updating current values
	istrstream is(prefString);
	char type;
  int index, ival;
  unsigned int iuval;
  CMyString msval;

  const int N = prefString.GetLength(); // safe upper limit on string size
  char *buf = new char[N];

	while (is) {
		is >> type >> index;
		switch (type) {
			case 'B':
				// Need to get value - even of not understood or wanted
				is >> ival;
				// forward compatibility and check whether still in DB
				if (index < NumBoolPrefs && m_bool_prefs[index].isStoredinDB) {
					ASSERT(ival == 0 || ival == 1);
					m_boolValues[index] = (ival != 0);
				}
				break;
			case 'I':
				// Need to get value - even of not understood or wanted
				is >> iuval;
				// forward compatibility and check whether still in DB
				if (index < NumIntPrefs && m_int_prefs[index].isStoredinDB)
					m_intValues[index] = iuval;
				break;
			case 'S':
				// Need to get value - even of not understood or wanted
				is.ignore(2, '\"'); // skip over space and leading "
				is.get(buf, N, '\"'); // get string value
				// forward compatibility and check whether still in DB
				if (index < NumStringPrefs && m_string_prefs[index].isStoredinDB) {
					msval= buf;
					m_stringValues[index] = msval;
				}
				break;
			default:
				continue; // forward compatibility (also last space)
		} // switch
	} // while
	delete[] buf;
}

void PWSprefs::UpdateTimeStamp()
{
	if (m_ConfigOptions == CF_FILE_RW || m_ConfigOptions == CF_FILE_RW_NEW) {
		time_t time_now;
		time(&time_now);
		const CMyString now = PWSUtil::ConvertToDateTimeString(time_now, TMC_XML);

		m_XML_Config->Set(m_csHKCU, _T("LastUpdated"), now);
	}
}

void PWSprefs::InitializePreferences()
{
	// Does the registry entry exist for this user?
	m_bRegistryKeyExists = CheckRegistryExists();

	// Find out name of config file (should it exist).
	TCHAR path_buffer[_MAX_PATH];
	TCHAR drive[_MAX_DRIVE];
	TCHAR dir[_MAX_DIR];

	GetModuleFileName(NULL, path_buffer, _MAX_PATH);

#if _MSC_VER >= 1400
	_tsplitpath_s(path_buffer, drive, _MAX_DRIVE, dir, _MAX_DIR, NULL, 0, NULL, 0);
	_tmakepath_s(path_buffer, _MAX_PATH, drive, dir, _T("pwsafe"), _T("cfg"));
#else
	_tsplitpath(path_buffer, drive, dir, NULL, NULL);
	_tmakepath(path_buffer, drive, dir, _T("pwsafe"), _T("cfg"));
#endif

	m_configfilename = (CString)path_buffer;

	// Set up all the XML "keys"
    const SysInfo *si = SysInfo::GetInstance();

	// Get the name of the computer
	m_csHKCU = si->GetCurrentHost() + _T("\\");

	// Get the user name
	m_csHKCU += si->GetCurrentUser();

	m_csHKCU_MRU  = m_csHKCU + _T("\\MRU");
	m_csHKCU_POS  = m_csHKCU + _T("\\Position");
	m_csHKCU_PREF = m_csHKCU + _T("\\Preferences");
	
	m_XML_Config = new CXMLprefs ();
	m_XML_Config->SetConfigFile(m_configfilename);

	CFile iniFile;
	CFileException fileException;
	CFileStatus fileStatus; 

	// Does it exist?
	m_bConfigFileExists = CFile::GetStatus(m_configfilename, fileStatus) == TRUE;

	CMyString locker(_T(""));
	// We only loop on getting the lock in this one place
	// as we need to know what to use
	bool bgotconfiglock;
	for (int iLoop = 0; iLoop < 10; iLoop++) {
      bgotconfiglock = PWSfile::LockFile(m_configfilename, locker, false);
      if (bgotconfiglock)
        break;
      Sleep(200);  // try max of 10 * 0.2 seconds = 2 secs
	}

	if (!bgotconfiglock) {
		CString cs_msg = _T("Unable to get configuration file lock.\n\n");
		cs_msg += _T("Will use registry.");
		AfxMessageBox(cs_msg, MB_OK);
		m_ConfigOptions = CF_REGISTRY;
		if (m_bRegistryKeyExists)
			LoadProfileFromRegistry();
		else
			LoadProfileFromDefaults();
		return;
	}

	m_ConfigOptions = CF_NONE;
	// checking for: CFileException::accessDenied
	if (m_bConfigFileExists) {
		// File exists: check we can write to it
		if (iniFile.Open(m_configfilename, CFile::modeWrite,
					&fileException)) {
			// it exists and we can use it!
			iniFile.Close();
			m_ConfigOptions = CF_FILE_RW;
			// Are we already in it?
			CString tmp = m_XML_Config->Get(m_csHKCU, _T("LastUpdated"), _T(""));
			time_t tt;
			if (!PWSUtil::VerifyXMLDateTimeString(tmp, tt)) {
				// No - try and load from registry
				if (m_bRegistryKeyExists)
					LoadProfileFromRegistry();
				else
					LoadProfileFromDefaults();
				// Now save to file
				SaveProfileToXML();
			} else {
				// Yes - load previous values
				LoadProfileFromFile();
			}
		} else {
			// it exists but we can't write to it!
			FileError(fileException.m_cause);
			// Are we already in it?
			CString tmp = m_XML_Config->Get(m_csHKCU, _T("LastUpdated"), _T(""));
			time_t tt;
			if (!PWSUtil::VerifyXMLDateTimeString(tmp, tt)) {
				// No - use registry
				m_ConfigOptions = CF_REGISTRY;
				if (m_bRegistryKeyExists)
					LoadProfileFromRegistry();
				else
					LoadProfileFromDefaults();
			} else {
				// Yes - load previous values
				m_ConfigOptions = CF_FILE_RO;
				LoadProfileFromFile();
			}
		}
	} else {
		// File doesn't exist: check we can create it and write to it
		if (iniFile.Open(m_configfilename, CFile::modeCreate | CFile::modeWrite,
					&fileException)) {
			iniFile.Close();
			m_ConfigOptions = CF_FILE_RW_NEW;
			DeleteFile(m_configfilename); // Since we deleted it!
			LoadProfileFromRegistry();
			SaveProfileToXML();
		} else {
			m_ConfigOptions = CF_REGISTRY;
			if (m_bRegistryKeyExists)
				LoadProfileFromRegistry();
			else
				LoadProfileFromDefaults();
			FileError(fileException.m_cause);
		}
	}

    PWSfile::UnlockFile(m_configfilename, false);
	
	CString cs_msg;
	switch (m_ConfigOptions) {
		case CF_REGISTRY:
			cs_msg = _T("Unable to create a new XML configuration file or add your settings to an existing XML configuration file.\n\nAll application configuration settings will be saved in your registry entry on this computer.\n\nDatabase related settings will be stored in the open database when it is closed, assuming it is not read-only or locked by another user.");
			break;
		case CF_FILE_RW:
		case CF_FILE_RW_NEW:
			m_XML_Config->SetReadWriteStatus(true);
			break;
		case CF_FILE_RO:
			m_XML_Config->SetReadWriteStatus(false);
			cs_msg = _T("Unable to update your entries in the existing XML configuration file.\n\nAll application configuration settings will be loaded from this file but no changes will be saved.\n\nDatabase related settings will be stored in the open database when it is closed, assuming it is not read-only or locked by another user.");
			break;
		case CF_NONE:
		default:
			cs_msg = _T("Error - unable to determine settings configuration!\n\nNo application settings will be saved.\n\nDatabase related settings will be stored in the open database when it is closed, assuming it is not read-only or locked by another user.");
			break;
	}
	if (!cs_msg.IsEmpty())
		AfxMessageBox(cs_msg, MB_OK);
}

void PWSprefs::LoadProfileFromDefaults()
{
	// Default values only
	for (int i = 0; i < NumBoolPrefs; i++)
		m_boolValues[i] = m_bool_prefs[i].defVal != 0;

	for (int i = 0; i < NumIntPrefs; i++)
		m_intValues[i] = m_int_prefs[i].defVal;

	for (int i = 0; i < NumStringPrefs; i++)
	 		m_stringValues[i] = CMyString(m_string_prefs[i].defVal);
}

void PWSprefs::LoadProfileFromRegistry()
{
	// Read in values from registry
	int i;
	// Defensive programming, if not "0", then "TRUE", all other values = FALSE
	for (i = 0; i < NumBoolPrefs; i++)
		m_boolValues[i] = m_app->GetProfileInt(PWS_REG_OPTIONS,
						 m_bool_prefs[i].name,
						 m_bool_prefs[i].defVal) != 0;

	// Defensive programming, if outside the permitted range, then set to default
	for (i = 0; i < NumIntPrefs; i++) {
		const int iVal = m_app->GetProfileInt(PWS_REG_OPTIONS,
						m_int_prefs[i].name,
						m_int_prefs[i].defVal);

		if (m_int_prefs[i].minVal != -1 && iVal < m_int_prefs[i].minVal)
			m_intValues[i] = m_int_prefs[i].defVal;
		else if (m_int_prefs[i].maxVal != -1 && iVal > m_int_prefs[i].maxVal)
			m_intValues[i] = m_int_prefs[i].defVal;
		else m_intValues[i] = iVal;
	}

	// Defensive programming not applicable.
	for (int i = 0; i < NumStringPrefs; i++)
		m_stringValues[i] = CMyString(m_app->GetProfileString(PWS_REG_OPTIONS,
					m_string_prefs[i].name,
					m_string_prefs[i].defVal));

	/*
		The following is "defensive" code because there was "a code ordering
		issue" in V3.02 and earlier.  PWSprefs.cpp and PWSprefs.h differed in
		the order of the HotKey and DoubleClickAction preferences.
		This is to protect the application should a HotKey value be assigned
		to DoubleClickAction.
		Note: HotKey also made an "Application preference" from a "Database
		preference".
	*/

	if (m_intValues[HotKey] > 0 && m_intValues[HotKey] <= 3) {
		m_boolValues[HotKeyEnabled] = false;
		m_intValues[DoubleClickAction] = m_intValues[HotKey];
		m_intValues[HotKey] = 0;
		m_prefs_changed[APP_PREF] = true;
	}

	if (m_intValues[DoubleClickAction] > 3) {
		m_intValues[DoubleClickAction] = 1;
		m_prefs_changed[APP_PREF] = true;
	}
	// End of "defensive" code
}

void PWSprefs::LoadProfileFromFile()
{
	// Read in values from file

	// Defensive programming, if not "0", then "TRUE", all other values = FALSE
	for (int i = 0; i < NumBoolPrefs; i++)
		m_boolValues[i] = m_XML_Config->Get(m_csHKCU_PREF,
					 m_bool_prefs[i].name,
					 m_bool_prefs[i].defVal) != 0;

	// Defensive programming, if outside the permitted range, then set to default
	for (int i = 0; i < NumIntPrefs; i++) {
		const int iVal = m_XML_Config->Get(m_csHKCU_PREF,
					m_int_prefs[i].name,
					m_int_prefs[i].defVal);

		if (m_int_prefs[i].minVal != -1 && iVal < m_int_prefs[i].minVal)
			m_intValues[i] = m_int_prefs[i].defVal;
		else if (m_int_prefs[i].maxVal != -1 && iVal > m_int_prefs[i].maxVal)
			m_intValues[i] = m_int_prefs[i].defVal;
		else m_intValues[i] = iVal;
	}

	// Defensive programming not applicable.
	for (int i = 0; i < NumStringPrefs; i++)
		m_stringValues[i] = CMyString(m_XML_Config->Get(m_csHKCU_PREF,
					m_string_prefs[i].name,
					m_string_prefs[i].defVal));
}

void PWSprefs::SaveApplicationPreferences()
{
	if (m_prefs_changed[APP_PREF] == false)
		return;

	if (m_ConfigOptions == CF_FILE_RW ||
	    m_ConfigOptions == CF_FILE_RW_NEW)
		m_XML_Config->SetKeepXMLLock(true);

	UpdateTimeStamp();
	// Write in values to XML file
	for (int i = 0; i < NumBoolPrefs; i++) {
		if (!m_bool_prefs[i].isStoredinDB && m_boolChanged[i]) {
			if (m_boolValues[i] != m_bool_prefs[i].defVal) {
				WritePref(m_bool_prefs[i].name, m_boolValues[i]);
			} else {
				DeletePref(m_bool_prefs[i].name);
			}
			m_boolChanged[i] = false;
		}
	}

	for (int i = 0; i < NumIntPrefs; i++) {
		if (!m_int_prefs[i].isStoredinDB && m_intChanged[i]) {
			if (m_intValues[i] != m_int_prefs[i].defVal) {
				WritePref(m_int_prefs[i].name, m_intValues[i]);
			} else {
				DeletePref(m_int_prefs[i].name);
			}
			m_intChanged[i] = false;
		}
	}

	for (int i = 0; i < NumStringPrefs; i++) {
		if (!m_string_prefs[i].isStoredinDB && m_stringChanged[i]) {
			if (m_stringValues[i] != m_string_prefs[i].defVal) {
				WritePref(m_string_prefs[i].name, m_stringValues[i]);
			} else {
				DeletePref(m_string_prefs[i].name);
			}
			m_stringChanged[i] = false;
		}
	}
	
	if (m_ConfigOptions == CF_FILE_RW ||
	    m_ConfigOptions == CF_FILE_RW_NEW)
		m_XML_Config->SetKeepXMLLock(false);

	m_prefs_changed[APP_PREF] = false;
}

void PWSprefs::SaveProfileToXML()
{
	m_XML_Config->SetKeepXMLLock(true);

	UpdateTimeStamp();
	// Write in values to XML file
	for (int i = 0; i < NumBoolPrefs; i++) {
		if (!m_bool_prefs[i].isStoredinDB) {
			if (m_boolValues[i] != m_bool_prefs[i].defVal) {
				VERIFY(m_XML_Config->Set(m_csHKCU_PREF,
						 m_bool_prefs[i].name,
						 m_boolValues[i] ? 1 : 0) == 0);
			} else {
				DeletePref(m_bool_prefs[i].name);
			}
		}
	}

	for (int i = 0; i < NumIntPrefs; i++) {
		if (!m_int_prefs[i].isStoredinDB) {
			if (m_intValues[i] != m_int_prefs[i].defVal) {
				VERIFY(m_XML_Config->Set(m_csHKCU_PREF,
						m_int_prefs[i].name,
						m_intValues[i]) == 0);
			} else {
				DeletePref(m_int_prefs[i].name);
			}
		}
	}

	for (int i = 0; i < NumStringPrefs; i++) {
		if (!m_string_prefs[i].isStoredinDB) {
			if (m_stringValues[i] != m_string_prefs[i].defVal) {
				VERIFY(m_XML_Config->Set(m_csHKCU_PREF,
						m_string_prefs[i].name,
						m_stringValues[i]) == 0);
			} else {
				DeletePref(m_string_prefs[i].name);
			}
		}
	}

	m_XML_Config->SetKeepXMLLock(false);
}

void PWSprefs::DeleteMRUFromXML(const CString &csSubkey)
{
	m_XML_Config->DeleteSetting(m_csHKCU_MRU, csSubkey);
}

CString PWSprefs::ReadMRUFromXML(const CString &csSubkey)
{
	return m_XML_Config->Get(m_csHKCU_MRU, csSubkey, _T(""));
}

void PWSprefs::WriteMRUToXML(const CString &csSubkey, const CString &csMRUFilename)
{
	if (csMRUFilename.IsEmpty()) return;

	// Don't remember backup files
	if (csMRUFilename.Right(4) == _T(".bak") ||
		csMRUFilename.Right(5) == _T(".bak~") ||
		csMRUFilename.Right(5) == _T(".ibak") ||
		csMRUFilename.Right(6) == _T(".ibak~"))
		return;

	m_XML_Config->Set(m_csHKCU_MRU, csSubkey, csMRUFilename);
}

void PWSprefs::DeleteRegistryEntries()
{
	HKEY hSubkey;
	const CString csSubkey = _T("Software\\") + CString(m_app->m_pszRegistryKey);

	LONG dw = RegOpenKeyEx(HKEY_CURRENT_USER,
							csSubkey,
							NULL,
							KEY_ALL_ACCESS,
							&hSubkey);
	ASSERT(dw == ERROR_SUCCESS);

	dw = m_app->DelRegTree(hSubkey, m_app->m_pszAppName);
	ASSERT(dw == ERROR_SUCCESS);

	dw = RegCloseKey(hSubkey);
	ASSERT(dw == ERROR_SUCCESS);
}

void PWSprefs::FileError(const int &icause)
{
	CString file_error_msgs[15] =
		{_T("No error occurred."),
		_T("An unspecified error occurred."),
		_T("The file was not found."),
		_T("All or part of the path is invalid."),
		_T("Too many open files."),
		_T("Access to the file was denied: probably not sufficient security rights."),
		_T("There was an attempt to use an invalid file handle."),
		_T("The current working directory cannot be removed."),
		_T("There are no more directory entries."),
		_T("There was an error trying to set the file pointer."),
		_T("There was a hardware error."),
		_T("Sharing violation: file probably open in another application."),
		_T("There was an attempt to lock a region that was already locked."),
		_T("The disk is full."),
		_T("The end of file was reached.")};
	
	ASSERT(icause >= 0 && icause <= 14);
	AfxMessageBox(file_error_msgs[icause] + _T("\n\nThe configuration file will not be used."), MB_OK);
}

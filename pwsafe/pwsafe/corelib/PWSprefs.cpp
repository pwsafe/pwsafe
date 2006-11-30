/*
 * Copyright (c) 2003-2006 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
#include "PWSprefs.h"
#include "corelib.h"
#include <AfxWin.h> // for AfxGetApp()
#include <sstream>
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
	{_T("BackupBeforeEverySave"), true, false},					// application
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
	// Specified by supported masks
	{_T("BackupSuffix"), 0, false, minBKSFX, maxBKSFX},						// application
	{_T("BackupMaxIncremented"), 1, false, 1, 999},							// application
};

const PWSprefs::stringPref PWSprefs::m_string_prefs[NumStringPrefs] = {
	{_T("currentbackup"), _T(""), false},							// application
	{_T("currentfile"), _T(""), false},								// application
	{_T("lastview"), _T("tree"), false},							// application
	{_T("defusername"), _T(""), true},								// database
	{_T("treefont"), _T(""), false},								// application
	{_T("BackupPrefixValue"), _T(""), false},						// application
	{_T("BackupDir"), _T(""), false},                               // application
	{_T("AltBrowser"), _T(""), false},								// application
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
  delete self;
  self = NULL;
  SysInfo::DeleteInstance();
}

PWSprefs::PWSprefs() : m_app(::AfxGetApp()), m_XML_Config(NULL)
{
  ASSERT(m_app != NULL);
  int i;

  m_prefs_changed[DB_PREF] = false;
  m_prefs_changed[APP_PREF] = false;

  for (i = 0; i < NumBoolPrefs; i++)
    m_boolChanged[i] = false;

  for (i = 0; i < NumIntPrefs; i++)
    m_intChanged[i] = false;

  for (i = 0; i < NumStringPrefs; i++)
    m_stringChanged[i] = false;

  m_rect.top = m_rect.bottom = m_rect.left = m_rect.right = -1;
  m_rect.changed = false;

  m_MRUitems = new CString[m_int_prefs[MaxMRUItems].maxVal];
  InitializePreferences();
}

PWSprefs::~PWSprefs()
{
    delete m_XML_Config;
    delete[] m_MRUitems;
}

bool
PWSprefs::CheckRegistryExists() const
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
    top = m_rect.top;
    bottom = m_rect.bottom;
    left = m_rect.left;
    right = m_rect.right;
}

int PWSprefs::GetMRUList(CString *MRUFiles)
{
    ASSERT(MRUFiles != NULL);

    if (m_ConfigOptions == CF_NONE || m_ConfigOptions == CF_REGISTRY)
        return 0;
    
    const int n = GetPref(PWSprefs::MaxMRUItems);
    for (int i = 0; i < n; i++)
        MRUFiles[i] = m_MRUitems[i];

    return n;
}

int PWSprefs::SetMRUList(const CString *MRUFiles, int n, int max_MRU)
{
    ASSERT(MRUFiles != NULL);

    if (m_ConfigOptions == CF_NONE || m_ConfigOptions == CF_REGISTRY ||
        m_ConfigOptions == CF_FILE_RO)
        return 0;

    int i, cnt;
    bool changed = false;
    // remember the ones in use
    for (i = 0, cnt = 1; i < n; i++) {
        if (MRUFiles[i].IsEmpty() ||
            // Don't remember backup files
            MRUFiles[i].Right(4) == _T(".bak") ||
            MRUFiles[i].Right(5) == _T(".bak~") ||
            MRUFiles[i].Right(5) == _T(".ibak") ||
            MRUFiles[i].Right(6) == _T(".ibak~"))
            continue;
        if (m_MRUitems[cnt-1] != MRUFiles[i]) {
            m_MRUitems[cnt-1] = MRUFiles[i];
            changed = true;
        }
        cnt++;
    }
    // Remove any not in use    
    for (i = cnt - 1; i < max_MRU; i++) {
        if (!m_MRUitems[i].IsEmpty()) {
            m_MRUitems[i] = _T("");
            changed = true;
        }
    }
    if (changed)
        m_prefs_changed[APP_PREF] = true;
    return n;
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
	return bRetVal;
}
void PWSprefs::SetPrefRect(long top, long bottom,
                           long left, long right)
{
    if (m_rect.top != top) {
        m_rect.top = top; m_rect.changed = true;
    }
    if (m_rect.bottom != bottom) {
        m_rect.bottom = bottom; m_rect.changed = true;
    }
    if (m_rect.left != left) {
        m_rect.left = left; m_rect.changed = true;
    }
    if (m_rect.right != right) {
        m_rect.right = right; m_rect.changed = true;
    }
    if (m_rect.changed)
        m_prefs_changed[APP_PREF] = true;
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

	CString retval(_T(""));
#ifdef _UNICODE
	wostringstream os;
#else
	ostrstream os;
#endif

	for (int i = 0; i < NumBoolPrefs; i++)
		if (m_boolValues[i] != m_bool_prefs[i].defVal &&
			m_bool_prefs[i].isStoredinDB)
			os << _T("B ") << i << TCHAR(' ') << (m_boolValues[i] ? 1 : 0) << TCHAR(' ');

	for (int i = 0; i < NumIntPrefs; i++)
		if (m_intValues[i] != m_int_prefs[i].defVal &&
			m_int_prefs[i].isStoredinDB)
			os << _T("I ") << i << TCHAR(' ') << m_intValues[i] << TCHAR(' ');

	for (int i = 0; i < NumStringPrefs; i++)
		if (m_stringValues[i] != m_string_prefs[i].defVal &&
			m_string_prefs[i].isStoredinDB)
			os << _T("S ") << i << _T(" \"") << LPCTSTR(m_stringValues[i]) << _T("\" ");

	os << ends;
	retval = os.str();
	delete[] os.str(); // reports memory leaks in spite of this!
	return CMyString(retval);
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
#ifdef _UNICODE
	wistringstream is(prefString);
#else
	istrstream is(prefString);
#endif
	char type;
  int index, ival;
  unsigned int iuval;
  CMyString msval;

  const int N = prefString.GetLength(); // safe upper limit on string size
  char *buf = new char[N];

	while (is) {
		is >> type >> index;
		switch (type) {
			case TCHAR('B'):
				// Need to get value - even of not understood or wanted
				is >> ival;
				// forward compatibility and check whether still in DB
				if (index < NumBoolPrefs && m_bool_prefs[index].isStoredinDB) {
					ASSERT(ival == 0 || ival == 1);
					m_boolValues[index] = (ival != 0);
				}
				break;
			case TCHAR('I'):
				// Need to get value - even of not understood or wanted
				is >> iuval;
				// forward compatibility and check whether still in DB
				if (index < NumIntPrefs && m_int_prefs[index].isStoredinDB)
					m_intValues[index] = iuval;
				break;
			case TCHAR('S'):
				// Need to get value - even of not understood or wanted
				is.ignore(2, TCHAR('\"')); // skip over space and leading "
				is.get(buf, N, TCHAR('\"')); // get string value
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
/*
 * 1. If the config file exists, use it, ignore registry (common case)
 * 2. If no config file and old (*) registry tree, import registry prefs,
 *    create config file. (1st run on upgrade)
 * 3. If no config file and no registry, create config file. (virgin install)
 *
 * (*) Old == "Counterpane Systems" reg key
 * - User can delete old registry key explicitly via options.
 * - "No config file" also means config file exists but no entry for
 *   host/user
 * - If config file can't be created, fallback to "Password Safe" registry
 */
    // Find out name of config file (should it exist).
    TCHAR path_buffer[_MAX_PATH];
    TCHAR drive[_MAX_DRIVE];
    TCHAR dir[_MAX_DIR];

    GetModuleFileName(NULL, path_buffer, _MAX_PATH);

#if _MSC_VER >= 1400
    _tsplitpath_s(path_buffer, drive, _MAX_DRIVE, dir, _MAX_DIR,
                  NULL, 0, NULL, 0);
    _tmakepath_s(path_buffer, _MAX_PATH, drive, dir, _T("pwsafe"), _T("cfg"));
#else
    _tsplitpath(path_buffer, drive, dir, NULL, NULL);
    _tmakepath(path_buffer, drive, dir, _T("pwsafe"), _T("cfg"));
#endif

    m_configfilename = (CString)path_buffer;

    // Start with fallback position: hardcoded defaults
    LoadProfileFromDefaults();
    m_ConfigOptions = CF_NONE;

    // Actually, "config file exists" means:
    // 1. File exists &&
    // 2. host/user key found.

    // 1. Does config file exist (and if, so, can we write to it?)?
    bool isRO = false;
    bool configFileExists = PWSfile::FileExists(m_configfilename, isRO);
    if (configFileExists)
        m_ConfigOptions = (isRO) ? CF_FILE_RO : CF_FILE_RW;
    else 
        m_ConfigOptions = CF_FILE_RW_NEW;

    const SysInfo *si = SysInfo::GetInstance();
    // Set up XML "keys": host/user
    m_csHKCU = si->GetCurrentHost() + _T("\\");
    m_csHKCU += si->GetCurrentUser();
    // set up other keys
    m_csHKCU_MRU  = m_csHKCU + _T("\\MRU");
    m_csHKCU_POS  = m_csHKCU + _T("\\Position");
    m_csHKCU_PREF = m_csHKCU + _T("\\Preferences");

    // Does the registry entry exist for this user?
    m_bRegistryKeyExists = CheckRegistryExists();

    // 2. host/user key found?
    if (configFileExists) {
        if (!LoadProfileFromFile()) {
            // Config file exists, but host/user not in it
            if (!isRO) { // we can create one
                ImportOldPrefs(); // get pre-3.05, if any
                // If we didn't have r/w but now do:
                LoadProfileFromRegistry();
            } else { // isRO
                // awkward situation, config file exists, we're not in it,
                // can't write to it either.
                // Would a warning to user be appropriate?
                if (CheckRegistryExists())
                    LoadProfileFromRegistry();
                else
                    ImportOldPrefs();
            } // isRO
        } // host/user not found
    } else { // File doesn't exist
        ImportOldPrefs();
        LoadProfileFromRegistry();
        // can we create one? If not, fallback to registry
        // We assume that if we can create a lock file, we can create
        // a config file in the same directory
        CMyString locker;
        if (PWSfile::LockFile(m_configfilename, locker, false)) {
            PWSfile::UnlockFile(m_configfilename, false);
        } else {
            m_ConfigOptions = CF_REGISTRY; // CF_FILE_RW_NEW -> CF_REGISTRY
        }
    }

    CString cs_msg;
    switch (m_ConfigOptions) {
        case CF_REGISTRY:
            cs_msg.LoadString(IDSC_CANTCREATEXMLCFG);
            break;
        case CF_FILE_RW:
        case CF_FILE_RW_NEW:
            break;
        case CF_FILE_RO:
            cs_msg.LoadString(IDSC_CANTUPDATEXMLCFG);
            break;
        case CF_NONE:
        default:
            cs_msg.LoadString(IDSC_CANTDETERMINECFG);
            break;
    }
    if (!cs_msg.IsEmpty())
        TRACE(cs_msg);
}

void PWSprefs::LoadProfileFromDefaults()
{
    // set prefs to hardcoded values
    int i;
	// Default values only
	for (i = 0; i < NumBoolPrefs; i++)
		m_boolValues[i] = m_bool_prefs[i].defVal != 0;

	for (i = 0; i < NumIntPrefs; i++)
		m_intValues[i] = m_int_prefs[i].defVal;

	for (i = 0; i < NumStringPrefs; i++)
        m_stringValues[i] = CMyString(m_string_prefs[i].defVal);
}

void PWSprefs::LoadProfileFromRegistry()
{
	// Read in values from registry
    if (!CheckRegistryExists())
        return; // Avoid creating keys if none already, as
    //             GetProfile* creates keys if not found!

    // Note that default values are now current values,
    // as they've been set in LoadProfileFromDefaults, and
    // may have been overridden by ImportOldPrefs()
	int i;
	// Defensive programming, if not "0", then "TRUE", all other values = FALSE
	for (i = 0; i < NumBoolPrefs; i++)
		m_boolValues[i] = m_app->GetProfileInt(PWS_REG_OPTIONS,
                                               m_bool_prefs[i].name,
                                               m_boolValues[i]) != 0;

	// Defensive programming, if outside the permitted range, then set to default
	for (i = 0; i < NumIntPrefs; i++) {
		const int iVal = m_app->GetProfileInt(PWS_REG_OPTIONS,
                                              m_int_prefs[i].name,
                                              m_intValues[i]);

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
                                                              m_stringValues[i]));

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

    // Load last main window size & pos:
    m_rect.top = m_app->GetProfileInt(PWS_REG_POSITION,
                                      _T("top"), -1);
    m_rect.bottom = m_app->GetProfileInt(PWS_REG_POSITION,
                                         _T("bottom"), -1);
    m_rect.left = m_app->GetProfileInt(PWS_REG_POSITION,
                                       _T("left"), -1);
    m_rect.right = m_app->GetProfileInt(PWS_REG_POSITION,
                                        _T("right"), -1);
}

bool PWSprefs::LoadProfileFromFile()
{
    /*
     * Called from InitializePreferences() at startup,
     * attempts to read in application preferences
     * from pref file.
     * Returns false if couldn't read (unlikely, since exists),
     * or if no host/user section
     * found.
     */
    bool retval;
    CString ts, csSubkey;

    m_XML_Config = new CXMLprefs(m_configfilename);
	if (!m_XML_Config->Load()) {
        retval = false;
        goto exit;
    }

    // Are we (host/user) already in the config file?
    ts = m_XML_Config->Get(m_csHKCU, _T("LastUpdated"), _T(""));
    time_t tt;
    if (!PWSUtil::VerifyXMLDateTimeString(ts, tt)) {
        // No, nothing to load, return false
        retval = false;
        goto exit;
    }

    int i;
	// Defensive programming, if not "0", then "TRUE", all other values = FALSE
	for (i = 0; i < NumBoolPrefs; i++)
		m_boolValues[i] = m_XML_Config->Get(m_csHKCU_PREF,
                                            m_bool_prefs[i].name,
                                            m_bool_prefs[i].defVal) != 0;

	// Defensive programming, if outside the permitted range, then set to default
	for (i = 0; i < NumIntPrefs; i++) {
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
	for (i = 0; i < NumStringPrefs; i++)
		m_stringValues[i] = CMyString(m_XML_Config->Get(m_csHKCU_PREF,
                                                        m_string_prefs[i].name,
                                                        m_string_prefs[i].defVal));

    // Load last main window size & pos:
    m_rect.top = m_XML_Config->Get(m_csHKCU_POS, _T("top"), -1);
	m_rect.bottom = m_XML_Config->Get(m_csHKCU_POS, _T("bottom"), -1);
	m_rect.left = m_XML_Config->Get(m_csHKCU_POS, _T("left"), -1);
    m_rect.right = m_XML_Config->Get(m_csHKCU_POS, _T("right"), -1);

    // Load most recently used file list
    const int nMRUItems = m_intValues[MaxMRUItems];
    for (i = nMRUItems; i > 0; i--) {
        csSubkey.Format(_T("Safe%02d"), i);
        m_MRUitems[i-1] = m_XML_Config->Get(m_csHKCU_MRU, csSubkey, _T(""));
    }
    retval = true;
  exit:
    delete m_XML_Config;
    m_XML_Config = NULL;
    return retval;
}

void PWSprefs::SaveApplicationPreferences()
{
	if (!m_prefs_changed[APP_PREF])
		return;

	if (m_ConfigOptions == CF_FILE_RW ||
	    m_ConfigOptions == CF_FILE_RW_NEW) {
        // Load prefs file in case it was changed elsewhere
        // Here we need to explicitly lock from before
        // load to after store
        m_XML_Config = new CXMLprefs(m_configfilename);
		if (!m_XML_Config->Lock()) {
            // punt to registry!
            m_ConfigOptions = CF_REGISTRY;
            delete m_XML_Config;
            m_XML_Config = NULL;
        } else { // acquired lock
            m_XML_Config->Load();
        }
    }
    UpdateTimeStamp();
	
	// Write values to XML file or registry
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

    if (m_rect.changed) {
        switch (m_ConfigOptions) {
            case CF_REGISTRY:
                m_app->WriteProfileInt(PWS_REG_POSITION,
                                       _T("top"), m_rect.top);
                m_app->WriteProfileInt(PWS_REG_POSITION,
                                       _T("bottom"), m_rect.bottom);
                m_app->WriteProfileInt(PWS_REG_POSITION,
                                       _T("left"), m_rect.left);
                m_app->WriteProfileInt(PWS_REG_POSITION,
                                       _T("right"), m_rect.right);
                break;
            case CF_FILE_RW:
            case CF_FILE_RW_NEW: {
                CString obuff;
                obuff.Format(_T("%d"), m_rect.top);
                VERIFY(m_XML_Config->Set(m_csHKCU_POS, _T("top"), obuff) == 0);
                obuff.Format(_T("%d"), m_rect.bottom);
                VERIFY(m_XML_Config->Set(m_csHKCU_POS, _T("bottom"), obuff) == 0);
                obuff.Format(_T("%d"), m_rect.left);
                VERIFY(m_XML_Config->Set(m_csHKCU_POS, _T("left"), obuff) == 0);
                obuff.Format(_T("%d"), m_rect.right);
                VERIFY(m_XML_Config->Set(m_csHKCU_POS, _T("right"), obuff) == 0);
            }
                break;
            case CF_FILE_RO:
            case CF_NONE:
            default:
                break;
        }
        m_rect.changed = false;
    } // m_rect.changed

	if (m_ConfigOptions == CF_FILE_RW ||
	    m_ConfigOptions == CF_FILE_RW_NEW) {
        int i;
        const int n = GetPref(PWSprefs::MaxMRUItems);
        CString csSubkey;
        for (i = 0; i < n; i++)
            if (!m_MRUitems[i].IsEmpty()) {
                csSubkey.Format(_T("Safe%02d"), i+1);
                m_XML_Config->Set(m_csHKCU_MRU, csSubkey, m_MRUitems[i]);

            }
        // Remove any not in use
        for (i = n-1; i < m_int_prefs[MaxMRUItems].maxVal; i++) {
            csSubkey.Format(_T("Safe%02d"), i + 1);
            m_XML_Config->DeleteSetting(m_csHKCU_MRU, csSubkey);
        }
    }


	if (m_ConfigOptions == CF_FILE_RW ||
	    m_ConfigOptions == CF_FILE_RW_NEW) {
        m_XML_Config->Store();
        m_XML_Config->Unlock();
        delete m_XML_Config;
        m_XML_Config = NULL;
    }

	m_prefs_changed[APP_PREF] = false;
}

bool PWSprefs::OfferDeleteRegistry() const
{
    return (m_ConfigOptions == CF_FILE_RW &&
            (m_bRegistryKeyExists || OldPrefsExist()));
}

void PWSprefs::DeleteRegistryEntries()
{
    DeleteOldPrefs();
	HKEY hSubkey;
	const CString csSubkey = _T("Software\\") + CString(m_app->m_pszRegistryKey);

	LONG dw = RegOpenKeyEx(HKEY_CURRENT_USER,
							csSubkey,
							NULL,
							KEY_ALL_ACCESS,
							&hSubkey);
	if (dw != ERROR_SUCCESS) {
        return; // may have been called due to OldPrefs
    }

	dw = m_app->DelRegTree(hSubkey, m_app->m_pszAppName);
	ASSERT(dw == ERROR_SUCCESS);

	dw = RegCloseKey(hSubkey);
	ASSERT(dw == ERROR_SUCCESS);
    m_bRegistryKeyExists = false;
}

int PWSprefs::GetConfigIndicator() const
{
    switch (m_ConfigOptions) {
	   	case CF_NONE: return IDSC_CONFIG_NONE; break;
	    case CF_REGISTRY: return IDSC_CONFIG_REGISTRY; break;
	    case CF_FILE_RW:
		case CF_FILE_RW_NEW: return IDSC_CONFIG_FILE_RW; break;
	    case CF_FILE_RO: return IDSC_CONFIG_FILE_RO; break;
    	default: ASSERT(0); return 0;
    }
}

// Old registry handling code:
const CString OldSubKey(_T("Software\\Counterpane Systems"));

bool PWSprefs::OldPrefsExist() const
{
    bool bExists;
	HKEY hSubkey;
	bExists = (::RegOpenKeyEx(HKEY_CURRENT_USER,
                              OldSubKey,
                              0L,
                              KEY_READ,
                              &hSubkey) == ERROR_SUCCESS);
	if (bExists)
		::RegCloseKey(hSubkey);

	return bExists;
}

void PWSprefs::ImportOldPrefs()
{
	HKEY hSubkey;
    CString OldAppKey = OldSubKey + _T("\\Password Safe");
	LONG dw = ::RegOpenKeyEx(HKEY_CURRENT_USER,
                             OldAppKey,
                             NULL,
                             KEY_ALL_ACCESS,
                             &hSubkey);
	if (dw != ERROR_SUCCESS) {
        return;
    }
    // Iterate over app preferences (those not stored
    // in database, read values and store if exist
    int i;
    LONG rv;
    DWORD dwType;
	for (i = 0; i < NumBoolPrefs; i++)
        if (!m_bool_prefs[i].isStoredinDB) {
            DWORD vData, DataLen(sizeof(vData));
            rv = ::RegQueryValueEx(hSubkey,
                                   m_bool_prefs[i].name,
                                   NULL,
                                   &dwType,
                                   LPBYTE(&vData),
                                   &DataLen
                                   );
            if (rv == ERROR_SUCCESS && dwType == REG_DWORD)
                SetPref(BoolPrefs(i), (vData != 0));
        }
	for (i = 0; i < NumIntPrefs; i++)
		if (!m_int_prefs[i].isStoredinDB) {
            DWORD vData, DataLen(sizeof(vData));
            rv = ::RegQueryValueEx(hSubkey,
                                   m_int_prefs[i].name,
                                   NULL,
                                   &dwType,
                                   LPBYTE(&vData),
                                   &DataLen
                                   );
            if (rv == ERROR_SUCCESS && dwType == REG_DWORD)
                SetPref(IntPrefs(i), vData);
        }
	for (i = 0; i < NumStringPrefs; i++)
		if (!m_string_prefs[i].isStoredinDB) {
            DWORD DataLen = 0;
            rv = ::RegQueryValueEx(hSubkey,
                                   m_string_prefs[i].name,
                                   NULL,
                                   &dwType,
                                   NULL,
                                   &DataLen
                                   );
            if (rv == ERROR_SUCCESS && dwType == REG_SZ) {
                DataLen++;
                BYTE *pData = new BYTE[DataLen];
                ::memset(pData, 0, DataLen);
                rv = ::RegQueryValueEx(hSubkey,
                                       m_string_prefs[i].name,
                                       NULL,
                                       &dwType,
                                       pData,
                                       &DataLen
                                       );
                if (rv == ERROR_SUCCESS)
                    SetPref(StringPrefs(i), CMyString(pData));
                delete[] pData;
            } // Get the value
        } // pref in registry

    // Last but not least, rectangle
    long rectVals[4] = {-1, -1, -1, -1};
    TCHAR *rectNames[4] = {_T("top"), _T("bottom"), _T("left"), _T("right")};
    for (i = 0; i < 4; i++) {
        DWORD vData, DataLen(sizeof(vData));
        rv = ::RegQueryValueEx(hSubkey,
                               rectNames[i],
                               NULL,
                               &dwType,
                               LPBYTE(&vData),
                               &DataLen
                               );
        if (rv == ERROR_SUCCESS && dwType == REG_DWORD)
            rectVals[i] = vData;
    }
    SetPrefRect(rectVals[0], rectVals[1], rectVals[2], rectVals[3]);

	dw = ::RegCloseKey(hSubkey);
	ASSERT(dw == ERROR_SUCCESS);
}


void PWSprefs::DeleteOldPrefs()
{
	HKEY hSubkey;
	LONG dw = ::RegOpenKeyEx(HKEY_CURRENT_USER,
                             OldSubKey,
                             NULL,
                             KEY_ALL_ACCESS,
                             &hSubkey);
	if (dw != ERROR_SUCCESS) {
        return;
    }

	dw = m_app->DelRegTree(hSubkey, OldSubKey);
	ASSERT(dw == ERROR_SUCCESS);

	dw = ::RegCloseKey(hSubkey);
	ASSERT(dw == ERROR_SUCCESS);
}

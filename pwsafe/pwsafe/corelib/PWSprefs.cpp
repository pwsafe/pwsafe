/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#include "PWSprefs.h"
#include "os/typedefs.h"
#include "os/pws_tchar.h"
#include "corelib.h"
#include "PWSfile.h"
#include "SysInfo.h"
#include "XMLprefs.h"
#include "Util.h"
#include "PWSdirs.h"
#include "VerifyFormat.h"
#include "StringXStream.h"
#ifdef _WIN32
#include <AfxWin.h> // for AfxGetApp()
#include <LMCons.h> // for UNLEN
#endif

using namespace std;

#if defined(POCKET_PC)
const LPCTSTR PWS_REG_POSITION = _T("Position");
const LPCTSTR PWS_REG_OPTIONS = _T("Options");
#else
const LPCTSTR PWS_REG_POSITION = _T("");
const LPCTSTR PWS_REG_OPTIONS = _T("");
#endif

HANDLE s_cfglockFileHandle = INVALID_HANDLE_VALUE;
int s_cfgLockCount = 0;

PWSprefs *PWSprefs::self = NULL;
stringT PWSprefs::m_configfilename; // may be set before singleton created

// 1st parameter = name of preference
// 2nd parameter = default value
// 3rd parameter if stored in database, application or obsolete
const PWSprefs::boolPref PWSprefs::m_bool_prefs[NumBoolPrefs] = {
  {_T("AlwaysOnTop"), false, ptApplication},                // application
  {_T("ShowPWDefault"), false, ptDatabase},                 // database
  {_T("ShowPasswordInTree"), false, ptDatabase},            // database
  {_T("SortAscending"), true, ptDatabase},                  // database
  {_T("UseDefaultUser"), false, ptDatabase},                // database
  {_T("SaveImmediately"), true, ptDatabase},                // database
  {_T("PWUseLowercase"), true, ptDatabase},                 // database
  {_T("PWUseUppercase"), true, ptDatabase},                 // database
  {_T("PWUseDigits"), true, ptDatabase},                    // database
  {_T("PWUseSymbols"), false, ptDatabase},                  // database
  {_T("PWUseHexDigits"), false, ptDatabase},                // database
  {_T("PWUseEasyVision"), false, ptDatabase},               // database
  {_T("dontaskquestion"), false, ptApplication},            // application
  {_T("deletequestion"), false, ptApplication},             // application
  {_T("DCShowsPassword"), false, ptApplication},            // application
  {_T("DontAskMinimizeClearYesNo"), true, ptObsolete},      // obsolete in 3.13 - replaced by 2 separate entries
  {_T("DatabaseClear"), false, ptApplication},              // application
  {_T("DontAskSaveMinimize"), false, ptObsolete},           // obsolete in 3.02
  {_T("QuerySetDef"), true, ptApplication},                 // application
  {_T("UseNewToolbar"), true, ptApplication},               // application
  {_T("UseSystemTray"), true, ptApplication},               // application
  {_T("LockOnWindowLock"), true, ptApplication},            // application
  {_T("LockOnIdleTimeout"), true, ptApplication},           // application
  {_T("EscExits"), true, ptApplication},                    // application
  {_T("IsUTF8"), false, ptDatabase},                        // database
  {_T("HotKeyEnabled"), false, ptApplication},              // application
  {_T("MRUOnFileMenu"), true, ptApplication},               // application
  {_T("DisplayExpandedAddEditDlg"), true, ptDatabase},      // database
  {_T("MaintainDateTimeStamps"), false, ptDatabase},        // database
  {_T("SavePasswordHistory"), false, ptDatabase},           // database
  {_T("FindWraps"), false, ptObsolete},                     // obsolete in 3.11
  {_T("ShowNotesDefault"), false, ptDatabase},              // database
  {_T("BackupBeforeEverySave"), true, ptApplication},       // application
  {_T("PreExpiryWarn"), false, ptApplication},              // application
  {_T("ExplorerTypeTree"), false, ptApplication},           // application
  {_T("ListViewGridLines"), false, ptApplication},          // application
  {_T("MinimizeOnAutotype"), true, ptApplication},          // application
  {_T("ShowUsernameInTree"), true, ptDatabase},             // database
  {_T("PWMakePronounceable"), false, ptDatabase},           // database - 3.12 password policy
  {_T("ClearClipoardOnMinimize"), true, ptObsolete},        // obsolete in 3.14 - typos
  {_T("ClearClipoardOneExit"), true, ptObsolete},           // obsolete in 3.14 - typos
  {_T("ShowToolbar"), true, ptApplication},                 // application
  {_T("ShowNotesAsToolTipsInViews"), false, ptApplication}, // application
  {_T("DefaultOpenRO"), false, ptApplication},              // application
  {_T("MultipleInstances"), true, ptApplication},           // application
  {_T("ShowDragbar"), true, ptApplication},                 // application
  {_T("ClearClipboardOnMinimize"), true, ptApplication},    // application
  {_T("ClearClipboardOnExit"), true, ptApplication},        // application
};

// Default value = -1 means set at runtime
// Extra two values for Integer - min and max acceptable values (ignored if = -1)
const PWSprefs::intPref PWSprefs::m_int_prefs[NumIntPrefs] = {
  {_T("column1width"), (unsigned int)-1, ptApplication, -1, -1},    // application
  {_T("column2width"), (unsigned int)-1, ptApplication, -1, -1},    // application
  {_T("column3width"), (unsigned int)-1, ptApplication, -1, -1},    // application
  {_T("column4width"), (unsigned int)-1, ptApplication, -1, -1},    // application
  {_T("sortedcolumn"), 0, ptApplication, 0, 15},                    // application
  {_T("PWDefaultLength"), 8, ptDatabase, 4, 1024},                  // database
  // maxmruitems maximum = (ID_FILE_MRU_ENTRYMAX - ID_FILE_MRU_ENTRY1 + 1)
  {_T("maxmruitems"), 4, ptApplication, 0, 20},                     // application
  {_T("IdleTimeout"), 5, ptDatabase, 1, 120},                       // database
  {_T("DoubleClickAction"), DoubleClickCopyPassword, ptApplication,
                            minDCA, maxDCA},                        // application
  {_T("HotKey"), 0, ptApplication, -1, -1}, // 0=disabled, >0=keycode. // application
  // MaxREItems maximum = (ID_TRAYRECENT_ENTRYMAX - ID_TRAYRECENT_ENTRY1 + 1)
  {_T("MaxREItems"), 25, ptApplication, 0, 25},                     // application
  {_T("TreeDisplayStatusAtOpen"), AllCollapsed, ptDatabase,
                                  minTDS, maxTDS},                  // database
  {_T("NumPWHistoryDefault"), 3, ptDatabase, 0, 255},               // database
  // Specified by supported masks
  {_T("BackupSuffix"), 0, ptApplication, minBKSFX, maxBKSFX},       // application
  {_T("BackupMaxIncremented"), 1, ptApplication, 1, 999},           // application
  {_T("PreExpiryWarnDays"), 1, ptApplication, 1, 30},               // application
  {_T("ClosedTrayIconColour"), stiBlack, ptApplication,
                               stiBlack, stiYellow},                // application
  {_T("PWDigitMinLength"), 0, ptDatabase, 0, 1024},                 // database
  {_T("PWLowercaseMinLength"), 0, ptDatabase, 0, 1024},             // database
  {_T("PWSymbolMinLength"), 0, ptDatabase, 0, 1024},                // database
  {_T("PWUppercaseMinLength"), 0, ptDatabase, 0, 1024},             // database
};

const PWSprefs::stringPref PWSprefs::m_string_prefs[NumStringPrefs] = {
  {_T("currentbackup"), _T(""), ptApplication},                     // application
  {_T("currentfile"), _T(""), ptApplication},                       // application
  {_T("lastview"), _T("tree"), ptApplication},                      // application
  {_T("DefaultUsername"), _T(""), ptDatabase},                      // database
  {_T("treefont"), _T(""), ptApplication},                          // application
  {_T("BackupPrefixValue"), _T(""), ptApplication},                 // application
  {_T("BackupDir"), _T(""), ptApplication},                         // application
  {_T("AltBrowser"), _T(""), ptApplication},                        // application
  {_T("ListColumns"), _T(""), ptApplication},                       // application
  {_T("ColumnWidths"), _T(""), ptApplication},                      // application
  {_T("DefaultAutotypeString"), _T(""), ptDatabase},                // database
  {_T("AltBrowserCmdLineParms"), _T(""), ptApplication},            // application
  {_T("MainToolBarButtons"), _T(""), ptApplication},                // application
  {_T("PasswordFont"), _T(""), ptApplication},                      // application
  {_T("TreeListSampleText"), _T("AaBbYyZz 0O1IlL"), ptApplication}, // application
  {_T("PswdSampleText"), _T("AaBbYyZz 0O1IlL"), ptApplication},     // application
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

PWSprefs::PWSprefs() : m_XML_Config(NULL)
{
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

  m_MRUitems = new stringT[m_int_prefs[MaxMRUItems].maxVal];
  InitializePreferences();
}

PWSprefs::~PWSprefs()
{
  delete m_XML_Config;
  delete[] m_MRUitems;
}

bool PWSprefs::CheckRegistryExists() const
{
  bool bExists = false;
#ifdef _WIN32
  HKEY hSubkey;
  const stringT csSubkey = _T("Software\\") + stringT(::AfxGetApp()->m_pszRegistryKey);
  bExists = (::RegOpenKeyEx(HKEY_CURRENT_USER,
                            csSubkey.c_str(),
                            0L,
                            KEY_READ,
                            &hSubkey) == ERROR_SUCCESS);
  if (bExists)
    ::RegCloseKey(hSubkey);
#endif /* _WIN32 */
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

StringX PWSprefs::GetPref(StringPrefs pref_enum) const
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

int PWSprefs::GetMRUList(stringT *MRUFiles)
{
  ASSERT(MRUFiles != NULL);

  if (m_ConfigOptions == CF_NONE || m_ConfigOptions == CF_REGISTRY)
    return 0;

  const int n = GetPref(PWSprefs::MaxMRUItems);
  for (int i = 0; i < n; i++)
    MRUFiles[i] = m_MRUitems[i];

  return n;
}

int PWSprefs::SetMRUList(const stringT *MRUFiles, int n, int max_MRU)
{
  ASSERT(MRUFiles != NULL);

  if (m_ConfigOptions == CF_NONE || m_ConfigOptions == CF_REGISTRY ||
    m_ConfigOptions == CF_FILE_RO)
    return 0;

  int i, cnt;
  bool changed = false;
  // remember the ones in use
  for (i = 0, cnt = 1; i < n; i++) {
    if (MRUFiles[i].empty() ||
      // Don't remember backup files
        MRUFiles[i].substr(MRUFiles[i].length() - 4) == _T(".bak") ||
      MRUFiles[i].substr(MRUFiles[i].length() - 5) == _T(".bak~") ||
      MRUFiles[i].substr(MRUFiles[i].length() - 5) == _T(".ibak") ||
      MRUFiles[i].substr(MRUFiles[i].length() - 6) == _T(".ibak~"))
      continue;
    if (m_MRUitems[cnt-1] != MRUFiles[i]) {
      m_MRUitems[cnt-1] = MRUFiles[i];
      changed = true;
    }
    cnt++;
  }
  // Remove any not in use    
  for (i = cnt - 1; i < max_MRU; i++) {
    if (!m_MRUitems[i].empty()) {
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
  m_prefs_changed[m_bool_prefs[pref_enum].pt == ptDatabase ? DB_PREF : APP_PREF] |=
    (m_boolValues[pref_enum] != value);

  if (m_boolValues[pref_enum] != value) { // Only if changed
    m_boolValues[pref_enum] = value;
    m_boolChanged[pref_enum] = true;
  }
}

void PWSprefs::SetPref(IntPrefs pref_enum, unsigned int value)
{
  // ONLY save in memory - written out at database save (to database and config destination)
  m_prefs_changed[m_int_prefs[pref_enum].pt == ptDatabase ? DB_PREF : APP_PREF] |=
    (m_intValues[pref_enum] != value);

  if (m_intValues[pref_enum] != value) { // Only if changed
    m_intValues[pref_enum] = value;
    m_intChanged[pref_enum] = true;
  }
}

void PWSprefs::SetPref(StringPrefs pref_enum, const StringX &value)
{
  // ONLY save in memory - written out at database save (to database and config destination)
  m_prefs_changed[m_string_prefs[pref_enum].pt == ptDatabase ? DB_PREF : APP_PREF] |=
    (m_stringValues[pref_enum] != value);

  if (m_stringValues[pref_enum] != value) { // Only if changed
    m_stringValues[pref_enum] = value;
    m_stringChanged[pref_enum] = true;
  }
}

bool PWSprefs::WritePref(const StringX &name, bool val)
{
  // Used to save to config destination at database save and application termination
  bool bRetVal(false);
  switch (m_ConfigOptions) {
  case CF_REGISTRY:
#ifdef _WIN32
    bRetVal = (::AfxGetApp()->WriteProfileInt(PWS_REG_OPTIONS, name.c_str(),
                                      val ? 1 : 0) == TRUE);
#endif /* _WIN32 */
    break;
  case CF_FILE_RW:
  case CF_FILE_RW_NEW:
    bRetVal = (m_XML_Config->Set(m_csHKCU_PREF, name.c_str(),
                                 val ? 1 : 0) == 0);
    break;
  case CF_FILE_RO:
  case CF_NONE:
  default:
    break;
  }
  return bRetVal;
}

bool PWSprefs::WritePref(const StringX &name, unsigned int val)
{
  // Used to save to config destination at database save and application termination
  bool bRetVal(false);
  switch (m_ConfigOptions) {
    case CF_REGISTRY:
#ifdef _WIN32
      bRetVal = (::AfxGetApp()->WriteProfileInt(PWS_REG_OPTIONS, name.c_str(),
                                        val) == TRUE);
#endif /* _WIN32 */
      break;
    case CF_FILE_RW:
    case CF_FILE_RW_NEW:
      bRetVal = (m_XML_Config->Set(m_csHKCU_PREF, name.c_str(), val) == 0);
      break;
    case CF_FILE_RO:
    case CF_NONE:
    default:
      break;
  }
  return bRetVal;
}

bool PWSprefs::WritePref(const StringX &name, const StringX &val)
{
  // Used to save to config destination at database save and application termination
  bool bRetVal(false);
  switch (m_ConfigOptions) {
    case CF_REGISTRY:
#ifdef _WIN32
      bRetVal = (::AfxGetApp()->WriteProfileString(PWS_REG_OPTIONS, name.c_str(),
                                           val.c_str()) == TRUE);
#endif /* _WIN32 */
      break;
    case CF_FILE_RW:
    case CF_FILE_RW_NEW:
      bRetVal = (m_XML_Config->Set(m_csHKCU_PREF,
                                   name.c_str(), val.c_str()) == 0);
      break;
    case CF_FILE_RO:
    case CF_NONE:
    default:
      break;
  }
  return bRetVal;
}

bool PWSprefs::DeletePref(const StringX &name)
{
  bool bRetVal = true;
  switch (m_ConfigOptions) {
    case CF_REGISTRY:
    case CF_FILE_RW_NEW:
      // The following is not correct.  Not important just now.
      //if (m_bRegistryKeyExists) {
      //  bRetVal = ::AfxGetApp()->WriteProfileInt(PWS_REG_OPTIONS, name, NULL) == TRUE;
      //}
      break;
    case CF_FILE_RW:
      bRetVal = (m_XML_Config->DeleteSetting(m_csHKCU_PREF,
                                             name.c_str()) == TRUE);
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

StringX PWSprefs::Store()
{
  /*
  * Create a string of values that are (1) different from the defaults, &&
  * (2) are storage in the database (pt == ptDatabase)
  * String is of the form "X nn vv X nn vv..." Where X=[BIS] for binary,
  * integer and string, resp.,
  * nn is the numeric value of the enum, and vv is the value,
  * {1,0} for bool, unsigned integer for int, and delimited string for String.
  */

  oStringXStream os;
  int i;

  for (i = 0; i < NumBoolPrefs; i++) {
    if (m_boolValues[i] != m_bool_prefs[i].defVal &&
        m_bool_prefs[i].pt == ptDatabase)
      os << _T("B ") << i << TCHAR(' ') << (m_boolValues[i] ? 1 : 0) << TCHAR(' ');
  }

  for (i = 0; i < NumIntPrefs; i++) {
    if (m_intValues[i] != m_int_prefs[i].defVal &&
        m_int_prefs[i].pt == ptDatabase)
      os << _T("I ") << i << TCHAR(' ') << m_intValues[i] << TCHAR(' ');
  }

  TCHAR delim;
  const TCHAR Delimiters[] = _T("\"\'#?!%&*+=:;@~<>?,.{}[]()\xbb");
  const int NumDelimiters = sizeof(Delimiters)/ sizeof(Delimiters[0]) - 1;

  for (i = 0; i < NumStringPrefs; i++) {
    if (m_stringValues[i] != m_string_prefs[i].defVal &&
        m_string_prefs[i].pt == ptDatabase) {
      const StringX svalue = m_stringValues[i];
      delim = _T(' ');
      for (int j = 0; j < NumDelimiters; j++) {
        if (svalue.find(Delimiters[j]) ==StringX::npos) {
          delim = Delimiters[j];
          break;
        }
      }
      if (delim == _T(' '))
        continue;  // We tried, but just can't save it!

      os << _T("S ") << i << _T(' ') << delim << m_stringValues[i] <<
        delim << _T(' ');
    }
  }

  os << ends;
  return os.str();
}

void PWSprefs::Load(const StringX &prefString)
{
  // Set default values for preferences stored in Database
  int i;
  for (i = 0; i < NumBoolPrefs; i++) {
    if (m_bool_prefs[i].pt == ptDatabase)
        m_boolValues[i] = m_bool_prefs[i].defVal != 0;
  }

  for (i = 0; i < NumIntPrefs; i++) {
    if (m_int_prefs[i].pt == ptDatabase)
        m_intValues[i] = m_int_prefs[i].defVal;
  }

  for (i = 0; i < NumStringPrefs; i++) {
    if (m_string_prefs[i].pt == ptDatabase)
        m_stringValues[i] = m_string_prefs[i].defVal;
  }

  if (prefString.empty())
    return;

  // parse prefString, updating current values
  iStringXStream is(prefString);

  TCHAR type, delim[1];
  int index, ival;
  unsigned int iuval;

  const int N = prefString.length(); // safe upper limit on string size
  TCHAR *buf = new TCHAR[N];

  while (is) {
    is >> type >> index;
    if (is.eof())
      break;
    switch (type) {
      case TCHAR('B'):
        // Need to get value - even of not understood or wanted
        is >> ival;
        // forward compatibility and check whether still in DB
        if (index < NumBoolPrefs && m_bool_prefs[index].pt == ptDatabase) {
          ASSERT(ival == 0 || ival == 1);
          m_boolValues[index] = (ival != 0);
        }
        break;
      case TCHAR('I'):
        // Need to get value - even of not understood or wanted
        is >> iuval;
        // forward compatibility and check whether still in DB
        if (index < NumIntPrefs && m_int_prefs[index].pt == ptDatabase)
          m_intValues[index] = iuval;
        break;
      case TCHAR('S'):
        // Need to get value - even of not understood or wanted
        is.ignore(1, TCHAR(' ')); // skip over space
        is.get(delim[0]);         // get string delimiter
        is.get(buf, N, delim[0]); // get string value
        is.ignore(1, TCHAR(' ')); // skip over trailing delimiter
        // forward compatibility and check whether still in DB
        if (index < NumStringPrefs && m_string_prefs[index].pt == ptDatabase) {
          m_stringValues[index] = buf;
        }
        break;
      default:
        // Can't be forward compatibility as don't know how to process other newer types!
        continue;
    } // switch
    is.ignore(1, TCHAR(' ')); // skip over space after each entry
  } // while
  delete[] buf;
}

void PWSprefs::UpdateTimeStamp()
{
  if (m_ConfigOptions == CF_FILE_RW || m_ConfigOptions == CF_FILE_RW_NEW) {
    time_t time_now;
    time(&time_now);
    const StringX now = PWSUtil::ConvertToDateTimeString(time_now, TMC_XML);

    m_XML_Config->Set(m_csHKCU, _T("LastUpdated"), now.c_str());
  }
}

static void xmlify(charT t, stringT &name)
{
  if (!_istalpha(name[0]))
    name = t + name;
  int N = name.length();
  for (int i = 0; i < N; i++)
    if (!_istalnum(name[i]) &&
        name[i] != charT('_') &&
        name[i] != charT('-') &&
        name[i] != charT(':') &&
        name[i] != charT('.'))
      name[i] = charT('_');
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

  // change to config dir
  // dirs' d'tor will put us back when we leave
  // needed for case where m_configfilename was passed relatively
  PWSdirs dirs(PWSdirs::GetConfigDir());

  // Set path & name of config file
  if (m_configfilename.empty()) {
    m_configfilename = PWSdirs::GetConfigDir().c_str();
    m_configfilename += _T("pwsafe.cfg");
  }
  // Start with fallback position: hardcoded defaults
  LoadProfileFromDefaults();
  m_ConfigOptions = CF_NONE;

  // Actually, "config file exists" means:
  // 1. File exists &&
  // 2. host/user key found.

  // 1. Does config file exist (and if, so, can we write to it?)?
  bool isRO = false;
  bool configFileExists = PWSfile::FileExists(m_configfilename.c_str(), isRO);
  if (configFileExists)
    m_ConfigOptions = (isRO) ? CF_FILE_RO : CF_FILE_RW;
  else 
    m_ConfigOptions = CF_FILE_RW_NEW;

  const SysInfo *si = SysInfo::GetInstance();
  // Set up XML "keys": host/user
  // ensure that they start with letter,
  // and otherwise conforms with
  // http://www.w3.org/TR/2000/REC-xml-20001006#NT-Name
  stringT hn = si->GetEffectiveHost();
  xmlify(charT('H'), hn);
  stringT un = si->GetEffectiveUser();
  xmlify(charT('u'), un);
  m_csHKCU =  hn.c_str(); m_csHKCU += _T("\\");
  m_csHKCU += un.c_str();
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
        if (m_bRegistryKeyExists)
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
    StringX locker;
    if (LockCFGFile(m_configfilename, locker)) {
      UnlockCFGFile(m_configfilename);
    } else {
      m_ConfigOptions = CF_REGISTRY; // CF_FILE_RW_NEW -> CF_REGISTRY
    }
  }

  stringT cs_msg;
  switch (m_ConfigOptions) {
    case CF_REGISTRY:
      LoadAString(cs_msg, IDSC_CANTCREATEXMLCFG);
      break;
    case CF_FILE_RW:
    case CF_FILE_RW_NEW:
      break;
    case CF_FILE_RO:
      LoadAString(cs_msg, IDSC_CANTUPDATEXMLCFG);
      break;
    case CF_NONE:
    default:
      LoadAString(cs_msg, IDSC_CANTDETERMINECFG);
      break;
  }
  if (!cs_msg.empty())
    TRACE(cs_msg.c_str());

  // Check someone has introduced a conflict & silently resolve.
  if ((m_intValues[DoubleClickAction] == DoubleClickCopyPasswordMinimize) &&
    m_boolValues[ClearClipboardOnMinimize]) {
      m_intValues[DoubleClickAction] = DoubleClickCopyPassword;
      m_intChanged[DoubleClickAction] = true;
  }
}

void PWSprefs::SetDatabasePrefsToDefaults()
{
  // set prefs to hardcoded values
  int i;
  // Default values only
  for (i = 0; i < NumBoolPrefs; i++)
    if (m_bool_prefs[i].pt == ptDatabase)
        m_boolValues[i] = m_bool_prefs[i].defVal != 0;

  for (i = 0; i < NumIntPrefs; i++)
    if (m_int_prefs[i].pt == ptDatabase)
        m_intValues[i] = m_int_prefs[i].defVal;

  for (i = 0; i < NumStringPrefs; i++)
    if (m_string_prefs[i].pt == ptDatabase)
        m_stringValues[i] = m_string_prefs[i].defVal;
}

void PWSprefs::LoadProfileFromDefaults()
{
  // set prefs to hardcoded values
  int i;
  // Default values only
  for (i = 0; i < NumBoolPrefs; i++) {
    m_boolValues[i] = m_bool_prefs[i].defVal != 0;
  }

  for (i = 0; i < NumIntPrefs; i++) {
    m_intValues[i] = m_int_prefs[i].defVal;
  }

  for (i = 0; i < NumStringPrefs; i++) {
    m_stringValues[i] = m_string_prefs[i].defVal;
  }
}

void PWSprefs::LoadProfileFromRegistry()
{
#ifdef _WIN32
  // Read in values from registry
  if (!m_bRegistryKeyExists)
    return; // Avoid creating keys if none already, as
  //             GetProfile* creates keys if not found!

  m_prefs_changed[APP_PREF] = true;

  // Note that default values are now current values,
  // as they've been set in LoadProfileFromDefaults, and
  // may have been overridden by ImportOldPrefs()
  int i;
  // Defensive programming, if not "0", then "TRUE", all other values = FALSE
  for (i = 0; i < NumBoolPrefs; i++) {
    m_boolValues[i] = ::AfxGetApp()->GetProfileInt(PWS_REG_OPTIONS,
                                           m_bool_prefs[i].name,
                                           m_boolValues[i]) != 0;
    // Make sure we write them all out to the config file the first time
    m_boolChanged[i] = true;
  }

  { // encapsulate in braces to avoid compiler issues w.r.t.
    // initializations and goto
    // silently convert pre-3.14 ClearClipoardOn{Minimize,eExit} typos
    // to correct spelling while maintain preference's value.
    BOOL bccom = GetPref(ClearClipboardOnMinimize) ? TRUE : FALSE;
    BOOL bccoe = GetPref(ClearClipboardOnExit) ? TRUE : FALSE;
    
    bool bccom2 = ::AfxGetApp()->GetProfileInt(PWS_REG_OPTIONS,
                                               _T("ClearClipoardOnMinimize"), // deliberate!
                                               bccom) != 0;
    bool bccoe2 = ::AfxGetApp()->GetProfileInt(PWS_REG_OPTIONS,
                                               _T("ClearClipoardOneExit"), // deliberate!
                                               bccoe) != 0;
    
    // If old (mis-spelt) name was there, use its value. Since the
    // default above was the new (correct) spelling, it has priority
    m_boolValues[ClearClipboardOnMinimize] = bccom2;
    m_boolValues[ClearClipboardOnExit] = bccoe2;
    // end of silent conversion
  }
  // Defensive programming, if outside the permitted range, then set to default
  for (i = 0; i < NumIntPrefs; i++) {
    const int iVal = ::AfxGetApp()->GetProfileInt(PWS_REG_OPTIONS,
                                          m_int_prefs[i].name,
                                          m_intValues[i]);

    if (m_int_prefs[i].minVal != -1 && iVal < m_int_prefs[i].minVal)
      m_intValues[i] = m_int_prefs[i].defVal;
    else if (m_int_prefs[i].maxVal != -1 && iVal > m_int_prefs[i].maxVal)
      m_intValues[i] = m_int_prefs[i].defVal;
    else m_intValues[i] = iVal;

    // Make sure we write them all out to the config file the first time
    m_intChanged[i] = true;
  }

  // Defensive programming not applicable.
  for (i = 0; i < NumStringPrefs; i++) {
    m_stringValues[i] = ::AfxGetApp()->GetProfileString(PWS_REG_OPTIONS,
                                                m_string_prefs[i].name,
                                                m_stringValues[i].c_str());

    // Make sure we write them all out to the config file the first time
    m_stringChanged[i] = true;
  }

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
  m_rect.top = ::AfxGetApp()->GetProfileInt(PWS_REG_POSITION,
                                    _T("top"), -1);
  m_rect.bottom = ::AfxGetApp()->GetProfileInt(PWS_REG_POSITION,
                                       _T("bottom"), -1);
  m_rect.left = ::AfxGetApp()->GetProfileInt(PWS_REG_POSITION,
                                     _T("left"), -1);
  m_rect.right = ::AfxGetApp()->GetProfileInt(PWS_REG_POSITION,
                                      _T("right"), -1);
#endif /* _WIN32 */
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
  stringT ts, csSubkey;

  m_XML_Config = new CXMLprefs(m_configfilename.c_str());
  if (!m_XML_Config->Load()) {
    retval = false;
    goto exit;
  }

  // Are we (host/user) already in the config file?
  ts = m_XML_Config->Get(m_csHKCU, _T("LastUpdated"), _T(""));
  time_t tt;
  if (!VerifyXMLDateTimeString(ts, tt)) {
    // No, nothing to load, return false
    retval = false;
    goto exit;
  }

  int i;
  // Defensive programming, if not "0", then "TRUE", all other values = FALSE
  for (i = 0; i < NumBoolPrefs; i++) {
    m_boolValues[i] = m_XML_Config->Get(m_csHKCU_PREF,
                                        m_bool_prefs[i].name,
                                        m_bool_prefs[i].defVal) != 0;
  }

  { // encapsulate in braces to avoid compiler issues w.r.t.
    // initializations and goto
    // silently convert pre-3.14 ClearClipoardOn{Minimize,eExit} typos
    // to correct spelling while maintain preference's value.
    bool bccom = GetPref(ClearClipboardOnMinimize);
    bool bccoe = GetPref(ClearClipboardOnExit);

    bool bccom2 = m_XML_Config->Get(m_csHKCU_PREF,
                                    _T("ClearClipoardOnMinimize"), // deliberate!
                                    bccom) != 0;
    bool bccoe2 = m_XML_Config->Get(m_csHKCU_PREF,
                                    _T("ClearClipoardOneExit"), // deliberate!
                                    bccoe) != 0;

    // If old (mis-spelt) name was there, use its value. Since the
    // default above was the new (correct) spelling, it has priority
    m_boolValues[ClearClipboardOnMinimize] = bccom2;
    m_boolValues[ClearClipboardOnExit] = bccoe2;
  }
  // Now delete them so we don't have to do this again, as they would
  // override the user's intention, if they changed them using the
  // correctly spelt versions.
  if (DeletePref(_T("ClearClipoardOnMinimize"))) {
    m_boolChanged[ClearClipboardOnMinimize] = true;
    m_prefs_changed[APP_PREF] = true;
  }
  if (DeletePref(_T("ClearClipoardOneExit"))) {
    m_boolChanged[ClearClipboardOnExit] = true;
    m_prefs_changed[APP_PREF] = true;
  }
  // end of silent conversion

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
  for (i = 0; i < NumStringPrefs; i++) {
    m_stringValues[i] = m_XML_Config->Get(m_csHKCU_PREF.c_str(),
                                          m_string_prefs[i].name,
                                          m_string_prefs[i].defVal).c_str();
  }

  // Load last main window size & pos:
  m_rect.top = m_XML_Config->Get(m_csHKCU_POS, _T("top"), -1);
  m_rect.bottom = m_XML_Config->Get(m_csHKCU_POS, _T("bottom"), -1);
  m_rect.left = m_XML_Config->Get(m_csHKCU_POS, _T("left"), -1);
  m_rect.right = m_XML_Config->Get(m_csHKCU_POS, _T("right"), -1);

  // Load most recently used file list
  for (i = m_intValues[MaxMRUItems]; i > 0; i--) {
    Format(csSubkey, _T("Safe%02d"), i);
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
  int i;
  if (!m_prefs_changed[APP_PREF])
    return;

  // change to config dir
  // dirs' d'tor will put us back when we leave
  // needed for case where m_configfilename was passed relatively
  PWSdirs dirs(PWSdirs::GetConfigDir());

  if (m_ConfigOptions == CF_FILE_RW ||
      m_ConfigOptions == CF_FILE_RW_NEW) {
      // Load prefs file in case it was changed elsewhere
      // Here we need to explicitly lock from before
      // load to after store
    m_XML_Config = new CXMLprefs(m_configfilename.c_str());
      if (!m_XML_Config->Lock()) {
        // punt to registry!
        m_ConfigOptions = CF_REGISTRY;
        delete m_XML_Config;
        m_XML_Config = NULL;
      } else { // acquired lock
        // if file exists, load to get other values
        if (PWSfile::FileExists(m_configfilename.c_str()))
          m_XML_Config->Load();
      }
  }
  UpdateTimeStamp();

  // Write values to XML file or registry
  for (i = 0; i < NumBoolPrefs; i++) {
    if (m_bool_prefs[i].pt == ptApplication && m_boolChanged[i]) {
      if (m_boolValues[i] != m_bool_prefs[i].defVal) {
        WritePref(m_bool_prefs[i].name, m_boolValues[i]);
      } else {
        DeletePref(m_bool_prefs[i].name);
      }
      m_boolChanged[i] = false;
    }
    if (m_bool_prefs[i].pt == ptObsolete)
      DeletePref(m_bool_prefs[i].name);
  }

  for (i = 0; i < NumIntPrefs; i++) {
    if (m_int_prefs[i].pt == ptApplication && m_intChanged[i]) {
      if (m_intValues[i] != m_int_prefs[i].defVal) {
        WritePref(m_int_prefs[i].name, m_intValues[i]);
      } else {
        DeletePref(m_int_prefs[i].name);
      }
      m_intChanged[i] = false;
    }
    if (m_int_prefs[i].pt == ptObsolete)
      DeletePref(m_int_prefs[i].name);
  }

  for (i = 0; i < NumStringPrefs; i++) {
    if (m_string_prefs[i].pt == ptApplication && m_stringChanged[i]) {
      if (m_stringValues[i] != m_string_prefs[i].defVal) {
        WritePref(m_string_prefs[i].name, m_stringValues[i]);
      } else {
        DeletePref(m_string_prefs[i].name);
      }
      m_stringChanged[i] = false;
    }
    if (m_string_prefs[i].pt == ptObsolete)
      DeletePref(m_string_prefs[i].name);
  }

  if (m_rect.changed) {
    switch (m_ConfigOptions) {
      case CF_REGISTRY:
#ifdef _WIN32
        ::AfxGetApp()->WriteProfileInt(PWS_REG_POSITION,
                               _T("top"), m_rect.top);
        ::AfxGetApp()->WriteProfileInt(PWS_REG_POSITION,
                               _T("bottom"), m_rect.bottom);
        ::AfxGetApp()->WriteProfileInt(PWS_REG_POSITION,
                               _T("left"), m_rect.left);
        ::AfxGetApp()->WriteProfileInt(PWS_REG_POSITION,
                               _T("right"), m_rect.right);
#endif /* _WIN32 */
        break;
      case CF_FILE_RW:
      case CF_FILE_RW_NEW:
      {
        stringT obuff;
        Format(obuff, _T("%d"), m_rect.top);
         VERIFY(m_XML_Config->Set(m_csHKCU_POS, _T("top"), obuff) == 0);
        Format(obuff, _T("%d"), m_rect.bottom);
        VERIFY(m_XML_Config->Set(m_csHKCU_POS, _T("bottom"), obuff) == 0);
        Format(obuff, _T("%d"), m_rect.left);
        VERIFY(m_XML_Config->Set(m_csHKCU_POS, _T("left"), obuff) == 0);
        Format(obuff, _T("%d"), m_rect.right);
        VERIFY(m_XML_Config->Set(m_csHKCU_POS, _T("right"), obuff) == 0);
        break;
      }
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
    // Delete ALL entries
    m_XML_Config->DeleteSetting(m_csHKCU_MRU, _T(""));
    // Now put back the ones we want
    stringT csSubkey;
    for (i = 0; i < n; i++)
      if (!m_MRUitems[i].empty()) {
        Format(csSubkey, _T("Safe%02d"), i+1);
        m_XML_Config->Set(m_csHKCU_MRU, csSubkey, m_MRUitems[i]);
      }
  }

  if (m_ConfigOptions == CF_FILE_RW ||
    m_ConfigOptions == CF_FILE_RW_NEW) {
      if (m_XML_Config->Store()) // can't be new after succ. store
        m_ConfigOptions = CF_FILE_RW;
      m_XML_Config->Unlock();
      delete m_XML_Config;
      m_XML_Config = NULL;
  }

  m_prefs_changed[APP_PREF] = false;
}

bool PWSprefs::OfferDeleteRegistry() const
{
#ifdef _WIN32
  return (m_ConfigOptions == CF_FILE_RW &&
    (m_bRegistryKeyExists || OldPrefsExist()));
#else
  return false;
#endif /* _WIN32 */
}

void PWSprefs::DeleteRegistryEntries()
{
#ifdef _WIN32
  DeleteOldPrefs();
  HKEY hSubkey;
  const stringT csSubkey = _T("Software\\") + stringT(::AfxGetApp()->m_pszRegistryKey);

  LONG dw = RegOpenKeyEx(HKEY_CURRENT_USER,
                         csSubkey.c_str(),
                         NULL,
                         KEY_ALL_ACCESS,
                         &hSubkey);
  if (dw != ERROR_SUCCESS) {
    return; // may have been called due to OldPrefs
  }

  dw = ::AfxGetApp()->DelRegTree(hSubkey, ::AfxGetApp()->m_pszAppName);
  ASSERT(dw == ERROR_SUCCESS);

  dw = RegCloseKey(hSubkey);
  ASSERT(dw == ERROR_SUCCESS);
  m_bRegistryKeyExists = false;
#endif /* _WIN32 */
}

int PWSprefs::GetConfigIndicator() const
{
  switch (m_ConfigOptions) {
    case CF_NONE:
      return IDSC_CONFIG_NONE;
    case CF_REGISTRY:
      return IDSC_CONFIG_REGISTRY;
    case CF_FILE_RW:
    case CF_FILE_RW_NEW:
      return IDSC_CONFIG_FILE_RW;
    case CF_FILE_RO: 
      return IDSC_CONFIG_FILE_RO;
    default:
      ASSERT(0);
      return 0;
  }
}

// Old registry handling code:
const stringT OldSubKey(_T("Counterpane Systems"));
const stringT Software(_T("Software"));

bool PWSprefs::OldPrefsExist() const
{
  bool bExists = false;
#ifdef _WIN32
  HKEY hSubkey;
  stringT key = Software + _T("\\") + OldSubKey;
  bExists = (::RegOpenKeyEx(HKEY_CURRENT_USER,
                            key.c_str(),
                            0L,
                            KEY_READ,
                            &hSubkey) == ERROR_SUCCESS);
  if (bExists)
    ::RegCloseKey(hSubkey);
#endif /* _WIN32 */
  return bExists;
}

void PWSprefs::ImportOldPrefs()
{
#ifdef _WIN32
  HKEY hSubkey;
  stringT OldAppKey = Software + _T("\\") + OldSubKey + _T("\\Password Safe");
  LONG dw = ::RegOpenKeyEx(HKEY_CURRENT_USER,
                           OldAppKey.c_str(),
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
  for (i = 0; i < NumBoolPrefs; i++) {
    if (m_bool_prefs[i].pt != ptDatabase) {
      DWORD vData, DataLen(sizeof(vData));
      rv = ::RegQueryValueEx(hSubkey,
                             m_bool_prefs[i].name,
                             NULL,
                             &dwType,
                             LPBYTE(&vData),
                             &DataLen);
      if (rv == ERROR_SUCCESS && dwType == REG_DWORD) {
        if (m_bool_prefs[i].pt == ptApplication)
          SetPref(BoolPrefs(i), (vData != 0));
        else { // Obsolete entries - but currently only need to deal with one!
          if (i == DontAskMinimizeClearYesNo) {
            SetPref(BoolPrefs(ClearClipboardOnMinimize), (vData != 0));
            SetPref(BoolPrefs(ClearClipboardOnExit), (vData != 0));
          }
        }
      }
    }
  }

  for (i = 0; i < NumIntPrefs; i++) {
    if (m_int_prefs[i].pt == ptApplication) {
      DWORD vData, DataLen(sizeof(vData));
      rv = ::RegQueryValueEx(hSubkey,
                             m_int_prefs[i].name,
                             NULL,
                             &dwType,
                             LPBYTE(&vData),
                             &DataLen);
      if (rv == ERROR_SUCCESS && dwType == REG_DWORD)
        SetPref(IntPrefs(i), vData);
    }
  }

  for (i = 0; i < NumStringPrefs; i++) {
    if (m_string_prefs[i].pt == ptApplication) {
      DWORD DataLen = 0;
      rv = ::RegQueryValueEx(hSubkey,
                             m_string_prefs[i].name,
                             NULL,
                             &dwType,
                             NULL,
                             &DataLen);
      if (rv == ERROR_SUCCESS && dwType == REG_SZ) {
        DataLen++;
        TCHAR *pData = new TCHAR[DataLen];
        ::memset(pData, 0, DataLen);
        rv = ::RegQueryValueEx(hSubkey,
                               m_string_prefs[i].name,
                               NULL,
                               &dwType,
                               LPBYTE(pData),
                               &DataLen);

        if (rv == ERROR_SUCCESS)
          SetPref(StringPrefs(i), pData);

        delete[] pData;
      } // Get the value
    } // pref in registry
  }

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
                           &DataLen);
    if (rv == ERROR_SUCCESS && dwType == REG_DWORD)
      rectVals[i] = vData;
  }
  SetPrefRect(rectVals[0], rectVals[1], rectVals[2], rectVals[3]);

  dw = ::RegCloseKey(hSubkey);
  ASSERT(dw == ERROR_SUCCESS);
#endif /* _WIN32 */
}

void PWSprefs::DeleteOldPrefs()
{
#ifdef _WIN32
  HKEY hSubkey;
  LONG dw = ::RegOpenKeyEx(HKEY_CURRENT_USER,
                           Software.c_str(),
                           NULL,
                           KEY_ALL_ACCESS,
                           &hSubkey);
  if (dw != ERROR_SUCCESS) {
    TRACE(_T("PWSprefs::DeleteOldPrefs: RegOpenKeyEx failed\n"));
    return;
  }

  dw = ::AfxGetApp()->DelRegTree(hSubkey, OldSubKey.c_str());
  if (dw != ERROR_SUCCESS) {
    TRACE(_T("PWSprefs::DeleteOldPrefs: DelRegTree failed\n"));
  }
  dw = ::RegCloseKey(hSubkey);
  if (dw != ERROR_SUCCESS) {
    TRACE(_T("PWSprefs::DeleteOldPrefs: RegCloseKey failed\n"));
  }
#endif /* _WIN32 */
}

stringT PWSprefs::GetXMLPreferences()
{
  stringT retval(_T(""));
  ostringstreamT os;

  os << _S("\t<Preferences>") << endl;
  int i;
  for (i = 0; i < NumBoolPrefs; i++) {
    if (m_boolValues[i] != m_bool_prefs[i].defVal &&
        m_bool_prefs[i].pt == ptDatabase)
      os << "\t\t<" << m_bool_prefs[i].name << ">" << (m_boolValues[i] ? "1</" : "0</") << 
      m_bool_prefs[i].name << ">" << endl;
  }

  for (i = 0; i < NumIntPrefs; i++) {
    if (m_intValues[i] != m_int_prefs[i].defVal &&
        m_int_prefs[i].pt == ptDatabase) {
      os << "\t\t<" << m_int_prefs[i].name << ">" ;
      if (i == TreeDisplayStatusAtOpen) {
        switch (m_intValues[i]) {
          case AllExpanded:
            os << "AllExpanded";
            break;
          case AsPerLastSave:
            os << "AsPerLastSave";
            break;
          case AllCollapsed:
          default:
            os << "AllCollapsed";
            break;
        }
      } else
        os << m_intValues[i];

      os << "</" << m_int_prefs[i].name << ">" << endl;
    }
  }

  for (i = 0; i < NumStringPrefs; i++) {
    if (m_stringValues[i] != m_string_prefs[i].defVal &&
        m_string_prefs[i].pt == ptDatabase) {
      StringX::size_type p = m_stringValues[i].find(_T("]]>")); // special handling required
      if (p == StringX::npos) {
        // common case
        os << "\t\t<" << m_string_prefs[i].name << "><![CDATA[" <<
          m_stringValues[i] << "]]></" << 
          m_string_prefs[i].name << ">" << endl;
      } else {
        // value has "]]>" sequence(s) that need(s) to be escaped
        // Each "]]>" splits the field into two CDATA sections, one ending with
        // ']]', the other starting with '>'
        const StringX value = m_stringValues[i];
        os << "\t\t<" << m_string_prefs[i].name << ">";
        int from = 0, to = p + 2;
        do {
          StringX slice = value.substr(from, (to - from));
          os << "<![CDATA[" << slice << "]]><![CDATA[";
          from = to;
          p = value.find(_T("]]>"), from); // are there more?
          if (p == StringX::npos) {
            to = value.length();
            slice = value.substr(from, (to - from));
          } else {
            to = p + 2;
            slice = value.substr(from, (to - from));
            from = to;
            to = value.length();
          }
          os <<  slice << "]]>";
        } while (p != StringX::npos);
        os << "</" << m_string_prefs[i].name << ">" << endl;      
      }
    }
  }
  os << "\t</Preferences>" << endl << endl;
  os << ends;
  retval = os.str().c_str();
  return retval;
}

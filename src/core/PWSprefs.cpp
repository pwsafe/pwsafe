/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#include "PWSprefs.h"
#include "core.h"
#include "PWSfile.h"
#include "SysInfo.h"
#include "XMLprefs.h"
#include "Util.h"
#include "PWSdirs.h"
#include "VerifyFormat.h"
#include "StringXStream.h"
#include "UTF8Conv.h"
#include "PWPolicy.h"

#include "os/typedefs.h"
#include "os/debug.h"
#include "os/pws_tchar.h"
#include "os/file.h"
#include "os/dir.h"
#include "os/registry.h"
#include "os/logit.h"

#include <fstream>
#include <algorithm>
#include <iomanip>

#ifdef _WIN32
#include <LMCons.h> // for UNLEN
#endif

using namespace std;

const LPCTSTR PWS_REG_POSITION = _T("");
const LPCTSTR PWS_REG_OPTIONS = _T("");

// For Old registry handling:
const stringT Software(_T("Software"));
const stringT OldSubKey(_T("Counterpane Systems"));

HANDLE s_cfglockFileHandle = INVALID_HANDLE_VALUE;

PWSprefs *PWSprefs::self = nullptr;
stringT PWSprefs::m_configfilename = _T(""); // may be set before singleton created
PWSprefs::ConfigOption PWSprefs::m_ConfigOption = PWSprefs::CF_NONE;
Reporter *PWSprefs::m_pReporter = nullptr;
bool PWSprefs::m_userSetCfgFile = false; // true iff user set config file (-g command line)

// One place for the config filename:
const stringT PWSprefs::cfgFileName = _T("pwsafe.cfg");

/*
 Note: to change a preference between application and database, the way
 to do it is to set the current one as obsolete and define a new one.

 Do not change the current type.

 NOTE: Database preferences are exported & imported via XML. Don't forget
 updating these routines and the schema.
*/

// 1st parameter = name of preference
// 2nd parameter = default value
// 3rd parameter if stored in database, application or obsolete
const PWSprefs::boolPref PWSprefs::m_bool_prefs[NumBoolPrefs] = {
  {_T("AlwaysOnTop"), false, ptApplication},                // application
  {_T("ShowPWDefault"), false, ptDatabase},                 // database
  {_T("ShowPasswordInTree"), false, ptDatabase},            // database
  {_T("SortAscending"), true, ptObsolete},                  // obsolete in 3.40 - replaced by new app pref
  {_T("UseDefaultUser"), false, ptDatabase},                // database
  {_T("SaveImmediately"), true, ptDatabase},                // database
  {_T("PWUseLowercase"), true, ptDatabase},                 // database
  {_T("PWUseUppercase"), true, ptDatabase},                 // database
  {_T("PWUseDigits"), true, ptDatabase},                    // database
  {_T("PWUseSymbols"), true, ptDatabase},                   // database
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
  #ifdef __linux__
  {_T("UseSystemTray"), false, ptApplication},              // application
  #else
  {_T("UseSystemTray"), true, ptApplication},               // application
  #endif
  {_T("LockOnWindowLock"), true, ptApplication},            // application
  {_T("LockOnIdleTimeout"), true, ptObsolete},              // obsolete in 3.19 - replaced by Database equivalent
  {_T("EscExits"), true, ptApplication},                    // application
  {_T("IsUTF8"), false, ptDatabase},                        // database - not used???
  {_T("HotKeyEnabled"), false, ptApplication},              // application
  {_T("MRUOnFileMenu"), true, ptApplication},               // application
  {_T("DisplayExpandedAddEditDlg"), true, ptObsolete},      // obsolete in 3.18
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
  {_T("ShowFindToolBarOnOpen"), false, ptApplication},      // application
  {_T("NotesWordWrap"), false, ptApplication},              // application
  {_T("LockDBOnIdleTimeout"), true, ptDatabase},            // database
  {_T("HighlightChanges"), true, ptApplication},            // application
  {_T("HideSystemTray"), false, ptApplication},             // application
  {_T("UsePrimarySelectionForClipboard"), false, ptApplication}, //application
  {_T("CopyPasswordWhenBrowseToURL"), false, ptDatabase},   // database
  {_T("UseAltAutoType"), false, ptApplication},             //application
  {_T("IgnoreHelpLoadError"), false, ptApplication},        //application
  {_T("VKPlaySound"), false, ptApplication},                //application
  {_T("ListSortAscending"), true, ptApplication},           //application
  {_T("EnableWindowTransparency"), false, ptApplication },  //application
};

// Default value = -1 means set at runtime
// Extra two values for Integer - min and max acceptable values (ignored if = -1)
const PWSprefs::intPref PWSprefs::m_int_prefs[NumIntPrefs] = {
  {_T("column1width"), static_cast<unsigned int>(-1), ptApplication, -1, -1}, // application
  {_T("column2width"), static_cast<unsigned int>(-1), ptApplication, -1, -1}, // application
  {_T("column3width"), static_cast<unsigned int>(-1), ptApplication, -1, -1}, // application
  {_T("column4width"), static_cast<unsigned int>(-1), ptApplication, -1, -1}, // application
  {_T("sortedcolumn"), 1, ptApplication, 0, 15},                    // application
  {_T("PWDefaultLength"), 12, ptDatabase, 4, 1024},                 // database
  // maxmruitems maximum = (ID_FILE_MRU_ENTRYMAX - ID_FILE_MRU_ENTRY1 + 1)
  {_T("maxmruitems"), 4, ptApplication, 0, 20},                     // application
  {_T("IdleTimeout"), 5, ptDatabase, 1, 600},                       // database
  {_T("DoubleClickAction"), DoubleClickCopyPassword, ptApplication,
                            minDCA, maxDCA},                        // application
  {_T("HotKey"), 0, ptApplication, -1, -1}, // 0=disabled, >0=keycode. // application
  // MaxREItems maximum = (ID_TRAYRECENT_ENTRYMAX - ID_TRAYRECENT_ENTRY1 + 1)
  {_T("MaxREItems"), 25, ptApplication, 0, 25},                     // application
  {_T("TreeDisplayStatusAtOpen"), AllCollapsed, ptDatabase,
                                  minTDS, maxTDS},                  // database
  {_T("NumPWHistoryDefault"), 3, ptDatabase, 0, 255},               // database
  // Specified by supported masks
  {_T("BackupSuffix"), BKSFX_IncNumber, ptApplication, minBKSFX, maxBKSFX}, // application
  {_T("BackupMaxIncremented"), 3, ptApplication, 1, 999},           // application
  {_T("PreExpiryWarnDays"), 1, ptApplication, 1, 30},               // application
  {_T("ClosedTrayIconColour"), 0, ptObsolete, -1, -1},              // obsolete in 3.43.1
  {_T("PWDigitMinLength"), 0, ptDatabase, 0, 1024},                 // database
  {_T("PWLowercaseMinLength"), 0, ptDatabase, 0, 1024},             // database
  {_T("PWSymbolMinLength"), 0, ptDatabase, 0, 1024},                // database
  {_T("PWUppercaseMinLength"), 0, ptDatabase, 0, 1024},             // database
  {_T("OptShortcutColumnWidth"), 92, ptApplication, 10, 512},       // application
  {_T("ShiftDoubleClickAction"), DoubleClickCopyUsername, ptApplication,
                            minDCA, maxDCA},                        // application
  {_T("DefaultAutotypeDelay"), 10, ptApplication,
                            1, 60000},                              // application
  {_T("DlgOrientation"), AUTO, ptApplication, AUTO, WIDE},          // application
  {_T("TimedTaskChainDelay"), 100, ptApplication, -1, -1},          // application
  {_T("AutotypeSelectAllKeyCode"), 0, ptApplication, 0, 255},       // application
  {_T("AutotypeSelectAllModMask"), 0, ptApplication, 0, 255},       // application
  {_T("TreeFontPtSz"), 0, ptApplication, 0, -1},                    // application
  {_T("PasswordFontPtSz"), 0, ptApplication, 0, -1},                // application
  {_T("NotesFontPtSz"), 0, ptApplication, 0, -1},                   // application
  {_T("AddEditFontPtSz"), 0, ptApplication, 0, -1},                 // application
  {_T("VKFontPtSz"), 0, ptApplication, 0, -1},                      // application
  {_T("WindowTransparency"), 0, ptApplication, 0, 50},              // application
  {_T("DefaultExpiryDays"), 90, ptApplication, 1, 3650},            // application
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
  {_T("LastUsedKeyboard"), _T(""), ptApplication},                  // application
  {_T("VKeyboardFontName"), _T(""), ptApplication},                 // application
  {_T("VKSampleText"), _T("AaBbYyZz 0O1IlL"), ptApplication},       // application
  {_T("AltNotesEditor"), _T(""), ptApplication},                    // application
  {_T("LanguageFile"), _T(""), ptApplication},                      // application
  {_T("DefaultSymbols"), _T(""), ptDatabase},                       // database
  {_T("NotesFont"), _T(""), ptApplication},                         // application
  {_T("NotesSampleText"), _T("AaBbYyZz 0O1IlL"), ptApplication},    // application
  {_T("AutotypeTaskDelays"), _T("100,100,100"), ptApplication},     // application
  {_T("AddEditFont"), _T(""), ptApplication },                      // application
  {_T("AddEditSampleText"), _T("AaBbYyZz 0O1IlL"), ptApplication},  // application
  {_T("AltNotesEditorCmdLineParms"), _T(""), ptApplication},        // application
};

PWSprefs *PWSprefs::GetInstance()
{
  if (self == nullptr) {
    self = new PWSprefs();
  }
  return self;
}

void PWSprefs::DeleteInstance()
{
  delete self;
  self = nullptr;
  SysInfo::DeleteInstance();
}

bool PWSprefs::SetConfigFile(const stringT &fn)
{
  m_configfilename = fn;
  m_userSetCfgFile = true;
  return pws_os::FileExists(fn);
}

PWSprefs::PWSprefs() : m_pXML_Config(nullptr)
{
  int i;

  m_prefs_changed[DB_PREF] = m_prefs_changed[APP_PREF] = m_prefs_changed[SHC_PREF] = false;

  for (i = 0; i < NumBoolPrefs; i++) m_boolChanged[i] = false;
  for (i = 0; i < NumIntPrefs; i++) m_intChanged[i] = false;
  for (i = 0; i < NumStringPrefs; i++) m_stringChanged[i] = false;

  m_rect.top = m_rect.bottom = m_rect.left = m_rect.right = -1;
  m_rect.changed = false;
  m_PSSrect.top = m_PSSrect.bottom = m_PSSrect.left = m_PSSrect.right = -1;
  m_PSSrect.changed = false;

  m_MRUitems = new stringT[m_int_prefs[MaxMRUItems].maxVal];
  InitializePreferences();
}

PWSprefs::~PWSprefs()
{
  delete m_pXML_Config;
  delete[] m_MRUitems;
}

bool PWSprefs::CheckRegistryExists() const
{
  return pws_os::RegCheckExists();
}

bool PWSprefs::GetPref(BoolPrefs pref_enum, const bool bUseCopy) const
{
  return bUseCopy ? m_boolCopyValues[pref_enum] : m_boolValues[pref_enum];
}

unsigned int PWSprefs::GetPref(IntPrefs pref_enum, const bool bUseCopy) const
{
  return bUseCopy ? m_intCopyValues[pref_enum] : m_intValues[pref_enum];
}

StringX PWSprefs::GetPref(StringPrefs pref_enum, const bool bUseCopy) const
{
  return bUseCopy ? m_stringCopyValues[pref_enum] : m_stringValues[pref_enum];
}

bool PWSprefs::GetPrefDefVal(BoolPrefs pref_enum) const
{
  return m_bool_prefs[pref_enum].defVal;
}

unsigned int PWSprefs::GetPrefDefVal(IntPrefs pref_enum) const
{
  return m_int_prefs[pref_enum].defVal;
}

StringX PWSprefs::GetPrefDefVal(StringPrefs pref_enum) const
{
  return m_string_prefs[pref_enum].defVal;
}

int PWSprefs::GetPrefMinVal(IntPrefs pref_enum) const
{
  return m_int_prefs[pref_enum].minVal;
}

int PWSprefs::GetPrefMaxVal(IntPrefs pref_enum) const
{
  return m_int_prefs[pref_enum].maxVal;
}

StringX PWSprefs::GetAllBoolPrefs(const bool bUseCopy)
{
  PWS_LOGIT;

  oStringXStream osxs;
  for (int i = 0; i < NumBoolPrefs; i++) {
    osxs << setw(1) << i << TCHAR(' ')
         << setw(1) << m_bool_prefs[i].ptype << TCHAR(' ')
         << setw(1) <<
         ((bUseCopy ? m_boolCopyValues[i] : m_boolValues[i]) ? 1 : 0) << TCHAR(' ');
  }
  return osxs.str();
}

StringX PWSprefs::GetAllIntPrefs(const bool bUseCopy)
{
  PWS_LOGIT;

  oStringXStream osxs;
  for (int i = 0; i < NumIntPrefs; i++) {
    osxs << setw(1) << i << TCHAR(' ')
         << setw(1) << m_int_prefs[i].ptype << TCHAR(' ')
         << setw(4) << setfill(TCHAR('0')) << hex <<
         (bUseCopy ? m_intCopyValues[i] : m_intValues[i]) << dec << TCHAR(' ');
  }
  return osxs.str();
}

StringX PWSprefs::GetAllStringPrefs(const bool bUseCopy)
{
  PWS_LOGIT;

  // Here we must restrict most string preferences as they contain user data
  int SafeStringPrefs[] = {
    LastView, TreeFont, BackupPrefixValue, ListColumns,
    ColumnWidths, MainToolBarButtons, PasswordFont,
    TreeListSampleText, PswdSampleText,
    LastUsedKeyboard, VKeyboardFontName, VKSampleText, LanguageFile
  };

  TCHAR delim;
  const TCHAR Delimiters[] = _T("\"\'#?!%&*+=:;@~<>?,.{}[]()\xab\xbb");
  const size_t NumDelimiters = sizeof(Delimiters) / sizeof(Delimiters[0]) - 1;

  oStringXStream osxs;
  for (size_t i = 0; i < sizeof(SafeStringPrefs) / sizeof(SafeStringPrefs[0]); i++) {
    const int k = SafeStringPrefs[i];
    delim = TCHAR(' ');
    for (size_t j = 0; j < NumDelimiters; j++) {
      if (bUseCopy) {
        if (m_stringCopyValues[k].find(Delimiters[j]) == StringX::npos) {
          delim = Delimiters[j];
          break;
        }
      } else {
        if (m_stringValues[k].find(Delimiters[j]) == StringX::npos) {
          delim = Delimiters[j];
          break;
        }
      }
    }
    if (delim == TCHAR(' '))
      continue;  // We tried, but just can't save it!

    osxs << setw(1) << k << TCHAR(' ')
         << setw(1) << m_string_prefs[k].ptype << TCHAR(' ')
         << delim <<
         (bUseCopy ? m_stringCopyValues[k].c_str() : m_stringValues[k].c_str()) <<
         delim << TCHAR(' ');
  }
  return osxs.str();
}

// Following for case where default value is determined at runtime
unsigned int PWSprefs::GetPref(IntPrefs pref_enum, unsigned int defVal,
                               const bool bUseCopy) const
{
  return m_intValues[pref_enum] == static_cast<unsigned int>(-1) ? defVal :
      (bUseCopy ? m_intCopyValues[pref_enum] : m_intValues[pref_enum]);
}

void PWSprefs::GetPrefRect(long &top, long &bottom,
                           long &left, long &right) const
{
  top = m_rect.top;
  bottom = m_rect.bottom;
  left = m_rect.left;
  right = m_rect.right;
}

void PWSprefs::GetPrefPSSRect(long &top, long &bottom,
                              long &left, long &right) const
{
  top = m_PSSrect.top;
  bottom = m_PSSrect.bottom;
  left = m_PSSrect.left;
  right = m_PSSrect.right;
}

int PWSprefs::GetMRUList(stringT *MRUFiles) const
{
  ASSERT(MRUFiles != nullptr);

  if (m_ConfigOption == CF_NONE || m_ConfigOption == CF_REGISTRY)
    return 0;

  const int n = GetPref(PWSprefs::MaxMRUItems);
  for (int i = 0; i < n; i++)
    MRUFiles[i] = m_MRUitems[i];

  return n;
}

int PWSprefs::SetMRUList(const stringT *MRUFiles, int n, int max_MRU)
{
  ASSERT(n == 0 || MRUFiles != nullptr); // if n is zero, wx passes nullptr

  if (m_ConfigOption == CF_NONE || m_ConfigOption == CF_REGISTRY ||
      m_ConfigOption == CF_FILE_RO)
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
    if (m_MRUitems[cnt - 1] != MRUFiles[i]) {
      m_MRUitems[cnt - 1] = MRUFiles[i];
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

PWPolicy PWSprefs::GetDefaultPolicy(const bool bUseCopy) const
{
  PWPolicy pwp;
  if (GetPref(PWUseLowercase, bUseCopy))
    pwp.flags |= PWPolicy::UseLowercase;
  if (GetPref(PWUseUppercase, bUseCopy))
    pwp.flags |= PWPolicy::UseUppercase;
  if (GetPref(PWUseDigits, bUseCopy))
    pwp.flags |= PWPolicy::UseDigits;
  if (GetPref(PWUseSymbols, bUseCopy))
    pwp.flags |= PWPolicy::UseSymbols;
  if (GetPref(PWUseHexDigits, bUseCopy))
    pwp.flags |= PWPolicy::UseHexDigits;
  if (GetPref(PWUseEasyVision, bUseCopy))
    pwp.flags |= PWPolicy::UseEasyVision;
  if (GetPref(PWMakePronounceable, bUseCopy))
    pwp.flags |= PWPolicy::MakePronounceable;

  pwp.length = GetPref(PWDefaultLength, bUseCopy);
  pwp.digitminlength = GetPref(PWDigitMinLength, bUseCopy);
  pwp.lowerminlength = GetPref(PWLowercaseMinLength, bUseCopy);
  pwp.symbolminlength = GetPref(PWSymbolMinLength, bUseCopy);
  pwp.upperminlength = GetPref(PWUppercaseMinLength, bUseCopy);
  pwp.symbols = GetPref(DefaultSymbols, bUseCopy);
  return pwp;
}

void PWSprefs::SetDefaultPolicy(const PWPolicy &pol, const bool bUseCopy)
{
  SetPref(PWUseLowercase,
          (pol.flags & PWPolicy::UseLowercase) != 0, bUseCopy);
  SetPref(PWUseUppercase,
          (pol.flags & PWPolicy::UseUppercase) != 0, bUseCopy);
  SetPref(PWUseDigits,
          (pol.flags & PWPolicy::UseDigits) != 0, bUseCopy);
  SetPref(PWUseSymbols,
          (pol.flags & PWPolicy::UseSymbols) != 0, bUseCopy);
  SetPref(PWUseHexDigits,
          (pol.flags & PWPolicy::UseHexDigits) != 0, bUseCopy);
  SetPref(PWUseEasyVision,
          (pol.flags & PWPolicy::UseEasyVision) != 0, bUseCopy);
  SetPref(PWMakePronounceable,
          (pol.flags & PWPolicy::MakePronounceable) != 0, bUseCopy);

  SetPref(PWDefaultLength, pol.length, bUseCopy);
  SetPref(PWDigitMinLength, pol.digitminlength, bUseCopy);
  SetPref(PWLowercaseMinLength, pol.lowerminlength, bUseCopy);
  SetPref(PWSymbolMinLength, pol.symbolminlength, bUseCopy);
  SetPref(PWUppercaseMinLength, pol.upperminlength, bUseCopy);
  SetPref(DefaultSymbols, pol.symbols, bUseCopy);
}

void PWSprefs::SetupCopyPrefs()
{
  // Set up copy of current preferences
  for (int i = 0; i < NumBoolPrefs; i++) {
    m_boolCopyValues[i] = m_boolValues[i];
  }

  for (int i = 0; i < NumIntPrefs; i++) {
    m_intCopyValues[i] = m_intValues[i];
  }

  for (int i = 0; i < NumStringPrefs; i++) {
    m_stringCopyValues[i] = m_stringValues[i];
  }
}

void PWSprefs::UpdateFromCopyPrefs(const PWSprefs::PrefType ptype)
{
  PWS_LOGIT;

  // Update real preferences from copy values
  for (int i = 0; i < NumBoolPrefs; i++) {
    if (ptype == ptAll || m_bool_prefs[i].ptype == ptype) {
      // ONLY save in memory - written out at database save (to database and config destination)
      m_prefs_changed[m_bool_prefs[i].ptype == ptDatabase ? DB_PREF : APP_PREF] |=
                               (m_boolValues[i] != m_boolCopyValues[i]);

      if (m_boolValues[i] != m_boolCopyValues[i]) { // Only if changed
        m_boolValues[i] = m_boolCopyValues[i];
        m_boolChanged[i] = true;
      }
    }
  }

  for (int i = 0; i < NumIntPrefs; i++) {
    if (ptype == ptAll || m_int_prefs[i].ptype == ptype) {
      // ONLY save in memory - written out at database save (to database and config destination)
      m_prefs_changed[m_int_prefs[i].ptype == ptDatabase ? DB_PREF : APP_PREF] |=
                               (m_intValues[i] != m_intCopyValues[i]);

      if (m_intValues[i] != m_intCopyValues[i]) { // Only if changed
        m_intValues[i] = m_intCopyValues[i];
        m_intChanged[i] = true;
      }
    }
  }

  for (int i = 0; i < NumStringPrefs; i++) {
    if (ptype == ptAll || m_string_prefs[i].ptype == ptype) {
      // ONLY save in memory - written out at database save (to database and config destination)
      m_prefs_changed[m_string_prefs[i].ptype == ptDatabase ? DB_PREF : APP_PREF] |=
                               (m_stringValues[i] != m_stringCopyValues[i]);

      if (m_stringValues[i] != m_stringCopyValues[i]) { // Only if changed
        m_stringValues[i] = m_stringCopyValues[i];
        m_stringChanged[i] = true;
      }
    }
  }
}

void PWSprefs::SetPref(BoolPrefs pref_enum, bool value, const bool bUseCopy)
{
  if (bUseCopy) {
    // This updates a copy of the DB preferences.  Used when changing Options/New Database
    m_boolCopyValues[pref_enum] = value;
    return;
  }

  // ONLY save in memory - written out at database save (to database and config destination)
  m_prefs_changed[m_bool_prefs[pref_enum].ptype == ptDatabase ? DB_PREF : APP_PREF] |=
    (m_boolValues[pref_enum] != value);

  if (m_boolValues[pref_enum] != value) { // Only if changed
    m_boolValues[pref_enum] = value;
    m_boolChanged[pref_enum] = true;
  }
}

void PWSprefs::SetPref(IntPrefs pref_enum, unsigned int value, const bool bUseCopy)
{
  if (bUseCopy) {
    // This updates a copy of the DB preferences.  Used when changing Options/New Database
    m_intCopyValues[pref_enum] = value;
    return;
  }

  // ONLY save in memory - written out at database save (to database and config destination)
  m_prefs_changed[m_int_prefs[pref_enum].ptype == ptDatabase ? DB_PREF : APP_PREF] |=
    (m_intValues[pref_enum] != value);

  if (m_intValues[pref_enum] != value) { // Only if changed
    m_intValues[pref_enum] = value;
    m_intChanged[pref_enum] = true;
  }
}

void PWSprefs::SetPref(StringPrefs pref_enum, const StringX &value, const bool bUseCopy)
{
  if (bUseCopy) {
    // This updates a copy of the DB preferences.  Used when changing Options/New Database
    m_stringCopyValues[pref_enum] = value;
    return;
  }

  // ONLY save in memory - written out at database save (to database and config destination)
  m_prefs_changed[m_string_prefs[pref_enum].ptype == ptDatabase ? DB_PREF : APP_PREF] |=
    (m_stringValues[pref_enum] != value);

  if (m_stringValues[pref_enum] != value) { // Only if changed
    m_stringValues[pref_enum] = value;
    m_stringChanged[pref_enum] = true;
  }
}

void PWSprefs::ResetPref(BoolPrefs pref_enum)
{
  PWS_LOGIT_ARGS("BoolPref: %ls", m_bool_prefs[pref_enum].name);

  m_boolValues[pref_enum] = m_bool_prefs[pref_enum].defVal;
  m_boolChanged[pref_enum] = true;
  m_prefs_changed[m_bool_prefs[pref_enum].ptype == ptDatabase ? DB_PREF : APP_PREF] = true;
}

void PWSprefs::ResetPref(IntPrefs pref_enum)
{
  PWS_LOGIT_ARGS("IntegerPref: %ls", m_int_prefs[pref_enum].name);

  m_intValues[pref_enum] = m_int_prefs[pref_enum].defVal;
  m_intChanged[pref_enum] = true;
  m_prefs_changed[m_int_prefs[pref_enum].ptype == ptDatabase ? DB_PREF : APP_PREF] = true;
}

void PWSprefs::ResetPref(StringPrefs pref_enum)
{
  PWS_LOGIT_ARGS("StringPref: %ls", m_string_prefs[pref_enum].name);

  m_stringValues[pref_enum] = m_string_prefs[pref_enum].defVal;
  m_stringChanged[pref_enum] = true;
  m_prefs_changed[m_string_prefs[pref_enum].ptype == ptDatabase ? DB_PREF : APP_PREF] = true;
}

bool PWSprefs::WritePref(const StringX &name, bool val)
{
  PWS_LOGIT_ARGS("Bool Name: %ls", name.c_str());

  // Used to save to config destination at database save and application termination
  bool bRetVal(false);
  switch (m_ConfigOption) {
    case CF_REGISTRY:
      bRetVal = pws_os::RegWriteValue(PWS_REG_OPTIONS, name.c_str(), val);
      break;
    case CF_FILE_RW:
    case CF_FILE_RW_NEW:
      bRetVal = (m_pXML_Config->Set(m_csHKCU_PREF, name.c_str(),
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
  PWS_LOGIT_ARGS("Integer Name: %ls", name.c_str());

  // Used to save to config destination at database save and application termination
  bool bRetVal(false);
  switch (m_ConfigOption) {
    case CF_REGISTRY:
      bRetVal = pws_os::RegWriteValue(PWS_REG_OPTIONS, name.c_str(), int(val));
      break;
    case CF_FILE_RW:
    case CF_FILE_RW_NEW:
      bRetVal = (m_pXML_Config->Set(m_csHKCU_PREF, name.c_str(), val) == 0);
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
  PWS_LOGIT_ARGS("String Name: %ls", name.c_str());

  // Used to save to config destination at database save and application termination
  bool bRetVal(false);
  switch (m_ConfigOption) {
    case CF_REGISTRY:
      bRetVal = pws_os::RegWriteValue(PWS_REG_OPTIONS, name.c_str(), val.c_str());
      break;
    case CF_FILE_RW:
    case CF_FILE_RW_NEW:
      bRetVal = (m_pXML_Config->Set(m_csHKCU_PREF,
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
  PWS_LOGIT_ARGS("Name: %ls", name.c_str());

  bool bRetVal(false);
  switch (m_ConfigOption) {
    case CF_REGISTRY:
      bRetVal = pws_os::RegDeleteEntry(name.c_str());
      break;
    case CF_FILE_RW:
      bRetVal = m_pXML_Config->DeleteSetting(m_csHKCU_PREF, name.c_str());
      break;
    case CF_FILE_RW_NEW:
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

void PWSprefs::SetPrefPSSRect(long top, long bottom,
                              long left, long right)
{
  if (m_PSSrect.top != top) {
    m_PSSrect.top = top; m_PSSrect.changed = true;
  }
  if (m_PSSrect.bottom != bottom) {
    m_PSSrect.bottom = bottom; m_PSSrect.changed = true;
  }
  if (m_PSSrect.left != left) {
    m_PSSrect.left = left; m_PSSrect.changed = true;
  }
  if (m_PSSrect.right != right) {
    m_PSSrect.right = right; m_PSSrect.changed = true;
  }
  if (m_PSSrect.changed)
    m_prefs_changed[APP_PREF] = true;
}

// std::sort for shortcuts
struct shortcut_less {
  bool operator ()(st_prefShortcut const& a, st_prefShortcut const& b) const {
    return (a.id < b.id);
  }
};

bool equal_shortcuts(st_prefShortcut a, st_prefShortcut b)
{
  return (a.id == b.id &&
          a.siVirtKey == b.siVirtKey &&
          a.cPWSModifier == b.cPWSModifier &&
          a.Menu_Name == b.Menu_Name);
}

void PWSprefs::SetPrefShortcuts(const std::vector<st_prefShortcut> &vShortcuts)
{
  std::vector<st_prefShortcut> vSortedShortcuts(vShortcuts);
  std::sort(vSortedShortcuts.begin(), vSortedShortcuts.end(), shortcut_less());
  if (m_vShortcuts.size() == vSortedShortcuts.size() &&
      std::equal(m_vShortcuts.begin(), m_vShortcuts.end(),
                 vSortedShortcuts.begin(), equal_shortcuts))
    return;

  m_vShortcuts = vSortedShortcuts;
  m_prefs_changed[SHC_PREF] = true;
}

StringX PWSprefs::Store(bool bUseCopy)
{
  PWS_LOGIT_ARGS("bUseCopy=%ls", bUseCopy ? _T("true") : _T("false"));
  /*
  * Create a string of values that are (1) different from the defaults, &&
  * (2) are storage in the database (ptype == ptDatabase)
  * String is of the form "X nn vv X nn vv..." Where X=[BIS] for binary,
  * integer and string, resp.,
  * nn is the numeric value of the enum, and vv is the value,
  * {1,0} for bool, unsigned integer for int, and delimited string for String.
  */

  bool *p_boolValues;
  unsigned int *p_intValues;
  StringX *p_stringValues;

  if (bUseCopy) {
    p_boolValues = m_boolCopyValues;
    p_intValues = m_intCopyValues;
    p_stringValues = m_stringCopyValues;
  } else {
    p_boolValues = m_boolValues;
    p_intValues = m_intValues;
    p_stringValues = m_stringValues;
  }

  oStringXStream os;

  for (int i = 0; i < NumBoolPrefs; i++, p_boolValues++) {
    if (*p_boolValues != m_bool_prefs[i].defVal &&
        m_bool_prefs[i].ptype == ptDatabase) {
      os << _T("B ") << i << TCHAR(' ') << (*p_boolValues ? 1 : 0) << TCHAR(' ');
    }
  }

  for (int i = 0; i < NumIntPrefs; i++, p_intValues++) {
    if (*p_intValues != m_int_prefs[i].defVal &&
        m_int_prefs[i].ptype == ptDatabase) {
      os << _T("I ") << i << TCHAR(' ') << *p_intValues << TCHAR(' ');
    }
  }

  TCHAR delim;
  const TCHAR Delimiters[] = _T("\"\'#?!%&*+=:;@~<>?,.{}[]()\xbb");
  const int NumDelimiters = sizeof(Delimiters) / sizeof(Delimiters[0]) - 1;

  for (int i = 0; i < NumStringPrefs; i++, p_stringValues++) {
    if (*p_stringValues != m_string_prefs[i].defVal &&
        m_string_prefs[i].ptype == ptDatabase) {
      const StringX svalue = *p_stringValues;
      delim = TCHAR(' ');
      for (int j = 0; j < NumDelimiters; j++) {
        if (svalue.find(Delimiters[j]) == StringX::npos) {
          delim = Delimiters[j];
          break;
        }
      }
      if (delim == TCHAR(' '))
        continue;  // We tried, but just can't save it!

      os << _T("S ") << i << TCHAR(' ') << delim << *p_stringValues <<
        delim << TCHAR(' ');
    }
  }

  // Now add preferences not known to this release (current types B, I & S)
  for (size_t i = 0; i < m_vUnknownBPrefs.size(); i++) {
    //
    os << _T("B ") << m_vUnknownBPrefs[i].index << TCHAR(' ') <<
        (m_vUnknownBPrefs[i].bValue ? 1 : 0) << TCHAR(' ');
  }

  for (size_t i = 0; i < m_vUnknownIPrefs.size(); i++) {
    //
    os << _T("I ") << m_vUnknownIPrefs[i].index << TCHAR(' ') <<
        m_vUnknownIPrefs[i].iValue << TCHAR(' ');
  }

  for (size_t i = 0; i < m_vUnknownSPrefs.size(); i++) {
    //
    os << _T("S ") << m_vUnknownSPrefs[i].index << TCHAR(' ') <<
        m_vUnknownSPrefs[i].delim <<
        m_vUnknownSPrefs[i].sValue.c_str() <<
        m_vUnknownSPrefs[i].delim << TCHAR(' ');
  }

  return os.str();
}

void PWSprefs::Load(const StringX &prefString, bool bUseCopy)
{
  PWS_LOGIT_ARGS("bUseCopy=%ls", bUseCopy ? _T("true") : _T("false"));

  bool *p_boolValues;
  unsigned int *p_intValues;
  StringX *p_stringValues;

  if (bUseCopy) {
    p_boolValues = m_boolCopyValues;
    p_intValues = m_intCopyValues;
    p_stringValues = m_stringCopyValues;
  } else {
    p_boolValues = m_boolValues;
    p_intValues = m_intValues;
    p_stringValues = m_stringValues;
  }

  // Set default values for preferences stored in Database
  int i; bool *pbool; unsigned int *pint; StringX *pstr;

  for (i = 0, pbool = p_boolValues; i < NumBoolPrefs; i++, pbool++) {
    if (m_bool_prefs[i].ptype == ptDatabase) {
      *pbool = m_bool_prefs[i].defVal != 0;
    }
  }

  for (i = 0, pint = p_intValues; i < NumIntPrefs; i++, pint++) {
    if (m_int_prefs[i].ptype == ptDatabase) {
      *pint = m_int_prefs[i].defVal;
    }
  }

  for (i = 0, pstr = p_stringValues; i < NumStringPrefs; i++, pstr++) {
    if (m_string_prefs[i].ptype == ptDatabase) {
      *pstr = m_string_prefs[i].defVal;
    }
  }

  ClearUnknownPrefs();

  if (prefString.empty())
    return;

  // parse prefString, updating current values
  iStringXStream is(prefString);

  TCHAR type, delim[1];
  int index, ival;
  unsigned int iuval;

  const size_t N = prefString.length(); // safe upper limit on string size
  auto *buf = new TCHAR[N];

  while (is) {
    is >> type >> index;
    if (is.eof())
      break;

    switch (type) {
      case TCHAR('B'):
        // Need to get value - even if not understood or wanted
        is >> ival;
        // forward compatibility and check whether still in DB
        if (index < NumBoolPrefs) {
          if (m_bool_prefs[index].ptype == ptDatabase) {
            ASSERT(ival == 0 || ival == 1);
            p_boolValues[index] = (ival != 0);
          }
        } else {
          st_BP stxBP;
          ASSERT(ival == 0 || ival == 1);
          stxBP.index = index;
          stxBP.bValue = ival == 1;
          m_vUnknownBPrefs.push_back(stxBP);
        }
        break;
      case TCHAR('I'):
        // Need to get value - even if not understood or wanted
        is >> iuval;
        // forward compatibility and check whether still in DB
        if (index < NumIntPrefs) {
          if (m_int_prefs[index].ptype == ptDatabase) {
            p_intValues[index] = iuval;
          }
        } else {
          st_IP stxIP;
          stxIP.index = index;
          stxIP.iValue = iuval;
          m_vUnknownIPrefs.push_back(stxIP);
        }
        break;
      case TCHAR('S'):
        // Need to get value - even if not understood or wanted
        is.ignore(1, TCHAR(' ')); // skip over space
        is.get(delim[0]);         // get string delimiter
        is.get(buf, N, delim[0]); // get string value
        is.ignore(1, TCHAR(' ')); // skip over trailing delimiter
        // forward compatibility and check whether still in DB
        if (index < NumStringPrefs) {
          if (m_string_prefs[index].ptype == ptDatabase) {
            p_stringValues[index] = buf;
          }
        } else {
          st_SP stxSP;
          stxSP.index = index;
          stxSP.delim = delim[0];
          stxSP.sValue = buf;
          m_vUnknownSPrefs.push_back(stxSP);
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

void PWSprefs::GetDefaultUserInfo(const StringX &sxDBPreferences,
                                  bool &bIsDefUserSet, StringX &sxDefUserValue)
{
  if (sxDBPreferences.empty()) {
    bIsDefUserSet = m_bool_prefs[UseDefaultUser].defVal != 0;
    sxDefUserValue = m_string_prefs[DefaultUsername].defVal;
  } else {
    // Use Copy
    Load(sxDBPreferences, true);
    bIsDefUserSet = m_boolCopyValues[UseDefaultUser] != 0;
    sxDefUserValue = m_stringCopyValues[DefaultUsername];
  }
}

void PWSprefs::UpdateTimeStamp()
{
  if (m_ConfigOption == CF_FILE_RW || m_ConfigOption == CF_FILE_RW_NEW) {
    time_t time_now;
    time(&time_now);
    const StringX now = PWSUtil::ConvertToDateTimeString(time_now, PWSUtil::TMC_XML);

    m_pXML_Config->Set(m_csHKCU, _T("LastUpdated"), now.c_str());
  }
}

void PWSprefs::XMLify(charT t, stringT &name)
{
  if (!_istalpha(name[0]))
    name = t + name;
  size_t N = name.length();
  for (size_t i = 0; i < N; i++)
    if (!_istalnum(name[i]) &&
        name[i] != charT('_') &&
        name[i] != charT('-') &&
        name[i] != charT(':') &&
        name[i] != charT('.'))
      name[i] = charT('_');
}

void PWSprefs::FindConfigFile()
{
  /**
   * 0. If user specified file (via command line), that's that.
   *
   * 1. Look for it in exe dir (pre-3.22 location). If it's there,
   *    check that hostname/user are in file (could have been migrated)
   * 2. If not in exe dir (or hostname/user not there), set file to
   *    config dir - we'll either read from there or create a new one
   */

  const stringT sExecDir = PWSdirs::GetExeDir();

  // Set path & name of config file
  if (!m_userSetCfgFile) { // common case
    const stringT sCnfgDir = PWSdirs::GetConfigDir();
    m_configfilename = sExecDir + cfgFileName;
    if (pws_os::FileExists(m_configfilename)) {
      // old (exe dir) exists, is host/user there?
      if (LoadProfileFromFile())
        return;
    }
    // not in exe dir or host/user not there
    m_configfilename = sCnfgDir + cfgFileName;
  } else { // User specified config file via SetConfigFile()
    // As per pre-use of Local AppData directory,
    // If file name's relative, it's expected to be in the
    // same directory as the executable
    stringT sDrive, sDir, sFile, sExt;
    pws_os::splitpath(m_configfilename, sDrive, sDir, sFile, sExt);
    if (sDrive.empty() || sDir.empty())
      m_configfilename = sExecDir + sFile + sExt;
  }
}

void PWSprefs::InitializePreferences()
{
  PWS_LOGIT;

  // Set up XML "keys": host/user ensure that they start with letter,
  // and otherwise conforms with http://www.w3.org/TR/2000/REC-xml-20001006#NT-Name
  const SysInfo *si = SysInfo::GetInstance();
  stringT hn = si->GetEffectiveHost();
  XMLify(charT('H'), hn);
  stringT un = si->GetEffectiveUser();
  XMLify(charT('u'), un);
  m_csHKCU = _T("Pwsafe_Settings\\");
  m_csHKCU += hn.c_str();
  m_csHKCU += _T("\\");
  m_csHKCU += un.c_str();

  // set up other keys
  m_csHKCU_MRU  = m_csHKCU + _T("\\MRU");
  m_csHKCU_POS  = m_csHKCU + _T("\\Position");
  m_csHKCU_PREF = m_csHKCU + _T("\\Preferences");
  m_csHKCU_SHCT = m_csHKCU + _T("\\Shortcuts");

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

  // Start with fallback position: hardcoded defaults
  LoadProfileFromDefaults();
  m_ConfigOption = CF_NONE;
  bool isRO(true);

  FindConfigFile(); // sets m_configfilename
  bool configFileExists = pws_os::FileExists(m_configfilename, isRO);
  if (configFileExists) {
    m_ConfigOption = (isRO) ? CF_FILE_RO : CF_FILE_RW;
  } else {
    // Doesn't exist but can we write to the directory?
    // Try and create the file (and delete afterwards if we succeeded)
    FILE *testfile = pws_os::FOpen(m_configfilename, _T("w"));
    if (testfile != nullptr) {
      fclose(testfile);
      pws_os::DeleteAFile(m_configfilename);
      m_ConfigOption = CF_FILE_RW_NEW;
      isRO = false;
    }
  }
  pws_os::Trace(_T("PWSprefs - using %ls config file: %ls [%ls]\n"),
            configFileExists ? L"existing" : L"",
            m_configfilename.c_str(),
            isRO ? L"R/O" : L"R/W");

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
    stringT locker;
    if (LockCFGFile(m_configfilename, locker)) {
      UnlockCFGFile(m_configfilename);
    } else {
      m_ConfigOption = CF_REGISTRY; // CF_FILE_RW_NEW -> CF_REGISTRY
    }
  }

  stringT cs_msg;
  switch (m_ConfigOption) {
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
    pws_os::Trace0(cs_msg.c_str());

  // Check someone has introduced a conflict & silently resolve.
  if ((m_intValues[DoubleClickAction] == DoubleClickCopyPasswordMinimize) &&
      m_boolValues[ClearClipboardOnMinimize]) {
    m_intValues[DoubleClickAction] = DoubleClickCopyPassword;
    m_intChanged[DoubleClickAction] = true;
  }
}

void PWSprefs::SetDatabasePrefsToDefaults(const bool bUseCopy)
{
  PWS_LOGIT_ARGS("bUseCopy=%ls", bUseCopy ? L"true" : L"false");

  // Set Database prefs to hardcoded values
  int i;
  // Default values only
  for (i = 0; i < NumBoolPrefs; i++)
    if (m_bool_prefs[i].ptype == ptDatabase) {
      if (bUseCopy)
        m_boolCopyValues[i] = m_bool_prefs[i].defVal != 0;
      else
        m_boolValues[i] = m_bool_prefs[i].defVal != 0;
    }
  for (i = 0; i < NumIntPrefs; i++)
    if (m_int_prefs[i].ptype == ptDatabase) {
      if (bUseCopy)
        m_intCopyValues[i] = m_int_prefs[i].defVal;
      else
        m_intValues[i] = m_int_prefs[i].defVal;
    }
  for (i = 0; i < NumStringPrefs; i++)
    if (m_string_prefs[i].ptype == ptDatabase) {
      if (bUseCopy)
        m_stringCopyValues[i] = m_string_prefs[i].defVal;
      else
        m_stringValues[i] = m_string_prefs[i].defVal;
    }
}

void PWSprefs::LoadProfileFromDefaults()
{
  PWS_LOGIT;

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
  PWS_LOGIT;

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
    m_boolValues[i] = pws_os::RegReadValue(PWS_REG_OPTIONS,
                                           m_bool_prefs[i].name,
                                           m_boolValues[i]);
    // Make sure we write them all out to the config file the first time
    m_boolChanged[i] = true;
  }

  { // encapsulate in braces to avoid compiler issues w.r.t.
    // initializations and goto
    // silently convert pre-3.14 ClearClipoardOn{Minimize,eExit} typos
    // to correct spelling while maintain preference's value.
    bool bccom = GetPref(ClearClipboardOnMinimize);
    bool bccoe = GetPref(ClearClipboardOnExit);

    bool bccom2 = pws_os::RegReadValue(PWS_REG_OPTIONS,
                                       _T("ClearClipoardOnMinimize"), // deliberate!
                                       bccom) != 0;
    bool bccoe2 = pws_os::RegReadValue(PWS_REG_OPTIONS,
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
    const int iVal = pws_os::RegReadValue(PWS_REG_OPTIONS,
                                          m_int_prefs[i].name,
                                          int(m_intValues[i]));

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
    m_stringValues[i] = pws_os::RegReadValue(PWS_REG_OPTIONS,
                                             m_string_prefs[i].name,
                                             m_stringValues[i].c_str()).c_str();

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
  m_rect.top = pws_os::RegReadValue(PWS_REG_POSITION, _T("top"), -1);
  m_rect.bottom = pws_os::RegReadValue(PWS_REG_POSITION, _T("bottom"), -1);
  m_rect.left = pws_os::RegReadValue(PWS_REG_POSITION, _T("left"), -1);
  m_rect.right = pws_os::RegReadValue(PWS_REG_POSITION, _T("right"), -1);

  // Load last Password subset window size & pos:
  m_PSSrect.top = pws_os::RegReadValue(PWS_REG_POSITION, _T("PSS_top"), -1);
  m_PSSrect.bottom = pws_os::RegReadValue(PWS_REG_POSITION, _T("PSS_bottom"), -1);
  m_PSSrect.left = pws_os::RegReadValue(PWS_REG_POSITION, _T("PSS_left"), -1);
  m_PSSrect.right = pws_os::RegReadValue(PWS_REG_POSITION, _T("PSS_right"), -1);
}

bool PWSprefs::LoadProfileFromFile()
{
  PWS_LOGIT;

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

  m_pXML_Config = new CXMLprefs(m_configfilename);
  if (!m_pXML_Config->XML_Load()) {
    if (!m_pXML_Config->getReason().empty() &&
        m_pReporter != nullptr)
      (*m_pReporter)(m_pXML_Config->getReason()); // show what went wrong
    retval = false;
    goto exit;
  }

  // Are we (host/user) already in the config file?
  ts = m_pXML_Config->Get(m_csHKCU, _T("LastUpdated"), _T(""));
  time_t tt;
  if (!VerifyXMLDateTimeString(ts, tt)) {
    // No, nothing to load, return false
    retval = false;
    goto exit;
  }

  // LockOnIdleTimeout is now a Database preference -
  // Silently delete it from XML file (actually doesn't delete
  // it here but marks as changed so will be deleted when saved).
  // Set updated so that the XML file is rewritten without it
  if (DeletePref(_T("LockOnIdleTimeout"))) {
    m_prefs_changed[APP_PREF] = true;
  }

  int i;
  // Defensive programming, if not "0", then "TRUE", all other values = FALSE
  for (i = 0; i < NumBoolPrefs; i++) {
    m_boolValues[i] = m_pXML_Config->Get(m_csHKCU_PREF,
                                        m_bool_prefs[i].name,
                                        m_bool_prefs[i].defVal) != 0;
  }

  { // encapsulate in braces to avoid compiler issues w.r.t.
    // initializations and goto
    // silently convert pre-3.14 ClearClipoardOn{Minimize,eExit} typos
    // to correct spelling while maintain preference's value.
    bool bccom = GetPref(ClearClipboardOnMinimize);
    bool bccoe = GetPref(ClearClipboardOnExit);

    bool bccom2 = m_pXML_Config->Get(m_csHKCU_PREF,
                                    _T("ClearClipoardOnMinimize"), // deliberate!
                                    bccom) != 0;
    bool bccoe2 = m_pXML_Config->Get(m_csHKCU_PREF,
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
    const int iVal = m_pXML_Config->Get(m_csHKCU_PREF,
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
    m_stringValues[i] = m_pXML_Config->Get(m_csHKCU_PREF.c_str(),
                                          m_string_prefs[i].name,
                                          m_string_prefs[i].defVal).c_str();
  }

  // Load last main window size & pos:
  m_rect.top = m_pXML_Config->Get(m_csHKCU_POS, _T("top"), -1);
  m_rect.bottom = m_pXML_Config->Get(m_csHKCU_POS, _T("bottom"), -1);
  m_rect.left = m_pXML_Config->Get(m_csHKCU_POS, _T("left"), -1);
  m_rect.right = m_pXML_Config->Get(m_csHKCU_POS, _T("right"), -1);

  m_PSSrect.top = m_pXML_Config->Get(m_csHKCU_POS, _T("PSS_top"), -1);
  m_PSSrect.bottom = m_pXML_Config->Get(m_csHKCU_POS, _T("PSS_bottom"), -1);
  m_PSSrect.left = m_pXML_Config->Get(m_csHKCU_POS, _T("PSS_left"), -1);
  m_PSSrect.right = m_pXML_Config->Get(m_csHKCU_POS, _T("PSS_right"), -1);

  // Load most recently used file list
  for (i = m_intValues[MaxMRUItems]; i > 0; i--) {
    Format(csSubkey, L"Safe%02d", i);
    m_MRUitems[i-1] = m_pXML_Config->Get(m_csHKCU_MRU, csSubkey, L"");
  }

  m_vShortcuts = m_pXML_Config->GetShortcuts(m_csHKCU_SHCT);
  std::sort(m_vShortcuts.begin(), m_vShortcuts.end(), shortcut_less());
  retval = true;

exit:
  delete m_pXML_Config;
  m_pXML_Config = nullptr;
  return retval;
}

void PWSprefs::SaveApplicationPreferences()
{
  PWS_LOGIT;

  int i;
  if (!m_prefs_changed[APP_PREF])
    return;

  if (m_ConfigOption == CF_FILE_RW ||
      m_ConfigOption == CF_FILE_RW_NEW) {
    // Load prefs file in case it was changed elsewhere
    // Here we need to explicitly lock from before
    // load to after store
    m_pXML_Config = new CXMLprefs(m_configfilename);
    stringT locker;
    if (!m_pXML_Config->Lock(locker)) {
      // punt to registry!
      m_ConfigOption = CF_REGISTRY;
      delete m_pXML_Config;
      m_pXML_Config = nullptr;
    } else { // acquired lock
      // if file exists, load to get other values
      if (pws_os::FileExists(m_configfilename))
        m_pXML_Config->XML_Load(); // we ignore failures here. why bother?
    }
  }
  UpdateTimeStamp();

  // Write values to XML file or registry
  for (i = 0; i < NumBoolPrefs; i++) {
    if (m_bool_prefs[i].ptype == ptApplication && m_boolChanged[i]) {
      if (m_boolValues[i] != m_bool_prefs[i].defVal) {
        WritePref(m_bool_prefs[i].name, m_boolValues[i]);
      } else {
        DeletePref(m_bool_prefs[i].name);
      }
      m_boolChanged[i] = false;
    }
    if (m_bool_prefs[i].ptype == ptObsolete) {
      DeletePref(m_bool_prefs[i].name);
    }
  }

  for (i = 0; i < NumIntPrefs; i++) {
    if (m_int_prefs[i].ptype == ptApplication && m_intChanged[i]) {
      if (m_intValues[i] != m_int_prefs[i].defVal) {
        WritePref(m_int_prefs[i].name, m_intValues[i]);
      } else {
        DeletePref(m_int_prefs[i].name);
      }
      m_intChanged[i] = false;
    }
    if (m_int_prefs[i].ptype == ptObsolete) {
      DeletePref(m_int_prefs[i].name);
    }
  }

  for (i = 0; i < NumStringPrefs; i++) {
    if (m_string_prefs[i].ptype == ptApplication && m_stringChanged[i]) {
      if (m_stringValues[i] != m_string_prefs[i].defVal) {
        WritePref(m_string_prefs[i].name, m_stringValues[i]);
      } else {
        DeletePref(m_string_prefs[i].name);
      }
      m_stringChanged[i] = false;
    }
    if (m_string_prefs[i].ptype == ptObsolete) {
      DeletePref(m_string_prefs[i].name);
    }
  }

  if (m_rect.changed) {
    switch (m_ConfigOption) {
      case CF_REGISTRY:
        pws_os::RegWriteValue(PWS_REG_POSITION, _T("top"),
                              int(m_rect.top));
        pws_os::RegWriteValue(PWS_REG_POSITION, _T("bottom"),
                              int(m_rect.bottom));
        pws_os::RegWriteValue(PWS_REG_POSITION, _T("left"),
                              int(m_rect.left));
        pws_os::RegWriteValue(PWS_REG_POSITION, _T("right"),
                              int(m_rect.right));
        break;
      case CF_FILE_RW:
      case CF_FILE_RW_NEW:
      {
        stringT obuff;
        Format(obuff, L"%d", m_rect.top);
        VERIFY(m_pXML_Config->Set(m_csHKCU_POS, L"top", obuff) == 0);
        Format(obuff, L"%d", m_rect.bottom);
        VERIFY(m_pXML_Config->Set(m_csHKCU_POS, L"bottom", obuff) == 0);
        Format(obuff, L"%d", m_rect.left);
        VERIFY(m_pXML_Config->Set(m_csHKCU_POS, L"left", obuff) == 0);
        Format(obuff, L"%d", m_rect.right);
        VERIFY(m_pXML_Config->Set(m_csHKCU_POS, L"right", obuff) == 0);
        break;
      }
      case CF_FILE_RO:
      case CF_NONE:
      default:
        break;
    }
    m_rect.changed = false;
  } // m_rect.changed

  if (m_PSSrect.changed) {
    switch (m_ConfigOption) {
      case CF_REGISTRY:
        pws_os::RegWriteValue(PWS_REG_POSITION, _T("PSS_top"),
                              int(m_PSSrect.top));
        pws_os::RegWriteValue(PWS_REG_POSITION, _T("PSS_bottom"),
                              int(m_PSSrect.bottom));
        pws_os::RegWriteValue(PWS_REG_POSITION, _T("PSS_left"),
                              int(m_PSSrect.left));
        pws_os::RegWriteValue(PWS_REG_POSITION, _T("PSS_right"),
                              int(m_PSSrect.right));
        break;
      case CF_FILE_RW:
      case CF_FILE_RW_NEW:
      {
        stringT obuff;
        Format(obuff, L"%d", m_PSSrect.top);
        VERIFY(m_pXML_Config->Set(m_csHKCU_POS, L"PSS_top", obuff) == 0);
        Format(obuff, L"%d", m_PSSrect.bottom);
        VERIFY(m_pXML_Config->Set(m_csHKCU_POS, L"PSS_bottom", obuff) == 0);
        Format(obuff, L"%d", m_PSSrect.left);
        VERIFY(m_pXML_Config->Set(m_csHKCU_POS, L"PSS_left", obuff) == 0);
        Format(obuff, L"%d", m_PSSrect.right);
        VERIFY(m_pXML_Config->Set(m_csHKCU_POS, L"PSS_right", obuff) == 0);
        break;
      }
      case CF_FILE_RO:
      case CF_NONE:
      default:
        break;
    }
    m_rect.changed = false;
  } // m_rect.changed

  if (m_ConfigOption == CF_FILE_RW ||
      m_ConfigOption == CF_FILE_RW_NEW) {
    int j;
    const int n = GetPref(PWSprefs::MaxMRUItems);
    // Delete ALL MRU entries
    m_pXML_Config->DeleteSetting(m_csHKCU_MRU, _T(""));
    // Now put back the ones we want
    stringT csSubkey;
    for (j = 0; j < n; j++) {
      if (!m_MRUitems[j].empty()) {
        Format(csSubkey, L"Safe%02d", j + 1);
        m_pXML_Config->Set(m_csHKCU_MRU, csSubkey, m_MRUitems[j]);
      }
    }

    // Since we may have changed the MRU - update timestamp in config file
    UpdateTimeStamp();
  }

  if (m_ConfigOption == CF_FILE_RW ||
      m_ConfigOption == CF_FILE_RW_NEW) {
    if (m_pXML_Config->XML_Store(m_csHKCU_PREF)) // can't be new after succ. store
      m_ConfigOption = CF_FILE_RW;
    else
    if (!m_pXML_Config->getReason().empty() &&
        m_pReporter != nullptr)
      (*m_pReporter)(m_pXML_Config->getReason()); // show what went wrong

    m_pXML_Config->Unlock();
    delete m_pXML_Config;
    m_pXML_Config = nullptr;
  }

  m_prefs_changed[APP_PREF] = false;
}

void PWSprefs::SaveShortcuts()
{
  if (!m_prefs_changed[SHC_PREF])
    return;

  if (m_ConfigOption == CF_FILE_RW ||
      m_ConfigOption == CF_FILE_RW_NEW) {
    // Load prefs file in case it was changed elsewhere
    // Here we need to explicitly lock from before
    // load to after store
    m_pXML_Config = new CXMLprefs(m_configfilename);
    stringT locker;
    if (!m_pXML_Config->Lock(locker)) {
      // punt to registry!
      m_ConfigOption = CF_REGISTRY;
      delete m_pXML_Config;
      m_pXML_Config = nullptr;
    } else { // acquired lock
      // if file exists, load to get other values
      if (pws_os::FileExists(m_configfilename))
        m_pXML_Config->XML_Load(); // we ignore failures here. why bother?
    }
  }

  if (m_ConfigOption == CF_FILE_RW ||
      m_ConfigOption == CF_FILE_RW_NEW) {
    // Delete ALL shortcut entries
    m_pXML_Config->DeleteSetting(m_csHKCU_SHCT, _T(""));
    // Now put back the ones we want
    if (!m_vShortcuts.empty())
      m_pXML_Config->SetShortcuts(m_csHKCU_SHCT, m_vShortcuts);

    // Since we may have changed the shortcuts - update timestamp in config file
    UpdateTimeStamp();
  }

  if (m_ConfigOption == CF_FILE_RW ||
      m_ConfigOption == CF_FILE_RW_NEW) {
    if (m_pXML_Config->XML_Store(m_csHKCU_PREF)) // can't be new after succ. store
      m_ConfigOption = CF_FILE_RW;
    else
    if (!m_pXML_Config->getReason().empty() &&
        m_pReporter != nullptr)
      (*m_pReporter)(m_pXML_Config->getReason()); // show what went wrong

    m_pXML_Config->Unlock();
    delete m_pXML_Config;
    m_pXML_Config = nullptr;
  }
  m_prefs_changed[SHC_PREF] = false;
}

bool PWSprefs::OfferDeleteRegistry() const
{
  return (m_ConfigOption == CF_FILE_RW &&
    (m_bRegistryKeyExists || OldPrefsExist()));
}

void PWSprefs::DeleteRegistryEntries()
{
  pws_os::RegDeleteSubtree(OldSubKey.c_str());
  if (pws_os::DeleteRegistryEntries())
    m_bRegistryKeyExists = false;
}

int PWSprefs::GetConfigIndicator() const
{
  switch (m_ConfigOption) {
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

bool PWSprefs::OldPrefsExist() const
{
  return pws_os::RegCheckExists(OldSubKey.c_str());
}

void PWSprefs::ImportOldPrefs()
{
  if (!pws_os::RegOpenSubtree(OldSubKey.c_str()))
    return;

  // Iterate over app preferences (those not stored
  // in database), read values and store if found.
  int i;
  for (i = 0; i < NumBoolPrefs; i++) {
    if (m_bool_prefs[i].ptype != ptDatabase) {
      bool value;
      if (pws_os::RegReadSTValue(m_bool_prefs[i].name, value)) {
        if (m_bool_prefs[i].ptype == ptApplication)
          SetPref(BoolPrefs(i), value);
        else { // Obsolete entries - but currently only need to deal with one!
          if (i == DontAskMinimizeClearYesNo) {
            SetPref(BoolPrefs(ClearClipboardOnMinimize), value);
            SetPref(BoolPrefs(ClearClipboardOnExit), value);
          }
        }
      }
    }
  } // NumBoolPrefs

  for (i = 0; i < NumIntPrefs; i++) {
    if (m_int_prefs[i].ptype == ptApplication) {
      int value;
      if (pws_os::RegReadSTValue(m_int_prefs[i].name, value)) {
        SetPref(IntPrefs(i), value);
      }
    }
  } // NumIntPrefs

  for (i = 0; i < NumStringPrefs; i++) {
    if (m_string_prefs[i].ptype == ptApplication) {
      stringT value;
      if (pws_os::RegReadSTValue(m_string_prefs[i].name,
                                 value))
        SetPref(StringPrefs(i), value.c_str());
    } // Get the value
  } // NumStringPrefs

  // Last but not least, rectangle
  long rectVals[4] = {-1, -1, -1, -1};
  const TCHAR *rectNames[4] = {_T("top"), _T("bottom"), _T("left"), _T("right")};
  for (i = 0; i < 4; i++) {
    int value;
    if (pws_os::RegReadSTValue(rectNames[i], value))
      rectVals[i] = value;
  }
  SetPrefRect(rectVals[0], rectVals[1], rectVals[2], rectVals[3]);

  bool closedOK = pws_os::RegCloseSubtree();
  VERIFY(closedOK);
}

void PWSprefs::DeleteOldPrefs()
{
  pws_os::RegDeleteSubtree(OldSubKey.c_str());
}

stringT PWSprefs::GetXMLPreferences()
{
  PWS_LOGIT;

  stringT retval(_T(""));
  ostringstreamT os;

  os << _S("\t<Preferences>") << endl;
  int i;
  for (i = 0; i < NumBoolPrefs; i++) {
    if (m_boolValues[i] != m_bool_prefs[i].defVal &&
        m_bool_prefs[i].ptype == ptDatabase)
      os << "\t\t<" << m_bool_prefs[i].name << ">" << (m_boolValues[i] ? "1</" : "0</") <<
      m_bool_prefs[i].name << ">" << endl;
  }

  for (i = 0; i < NumIntPrefs; i++) {
    if (m_intValues[i] != m_int_prefs[i].defVal &&
        m_int_prefs[i].ptype == ptDatabase) {
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
        m_string_prefs[i].ptype == ptDatabase) {
      stringT sTemp = PWSUtil::GetSafeXMLString(m_stringValues[i]);
      os << "\t\t<" << m_string_prefs[i].name << ">" << sTemp << "</" <<
          m_string_prefs[i].name << ">" << endl;
    }
  }
  os << "\t</Preferences>" << endl << endl;
  retval = os.str().c_str();
  return retval;
}

bool PWSprefs::LockCFGFile(const stringT &filename, stringT &locker)
{
  return pws_os::LockFile(filename, locker,
                          s_cfglockFileHandle);
}

void PWSprefs::UnlockCFGFile(const stringT &filename)
{
  return pws_os::UnlockFile(filename, s_cfglockFileHandle);
}

bool PWSprefs::IsLockedCFGFile(const stringT &filename)
{
  return pws_os::IsLockedFile(filename);
}

void PWSprefs::ClearUnknownPrefs()
{
  m_vUnknownBPrefs.clear();
  m_vUnknownIPrefs.clear();
  m_vUnknownSPrefs.clear();
}

stringT PWSprefs::GetDCAdescription(int dca)
{
  const int ids[maxDCA+1] = {
    IDSC_STATCOPYPASSWORD,    // DoubleClickCopyPassword = 0
    IDSC_STATVIEWEDIT,        // DoubleClickViewEdit = 1
    IDSC_STATAUTOTYPE,        // DoubleClickAutoType = 2
    IDSC_STATBROWSE,          // DoubleClickBrowse = 3
    IDSC_STATCOPYNOTES,       // DoubleClickCopyNotes = 4
    IDSC_STATCOPYUSERNAME,    // DoubleClickCopyUsername = 5
    IDSC_STATCOPYPASSWORDMIN, // DoubleClickCopyPasswordMinimize = 6
    IDSC_STATBROWSEPLUS,      // DoubleClickBrowsePlus = 7
    IDSC_STATRUN,             // DoubleClickRun = 8
    IDSC_STATSENDEMAIL,       // DoubleClickSendEmail = 9
  };

  int id = (dca >= minDCA && dca <= maxDCA) ? ids[dca] : IDSC_STATCOMPANY;

  stringT retval;
  LoadAString(retval, id);
  return retval;
}

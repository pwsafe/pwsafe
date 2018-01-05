/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/*
  THIS IS A COPY OF "A SUBSET" OF THE PREFERENCES OUT OF CORE'S PWSPREFS.CPP

  THIS IS NEEDED "ONLY" TO GET THE PREFERENCE "NAME"

    COPY TAKEN V3.34.2 - LAST CHANGED 2014-08-20
      SHA-1: 855071b427bc31e7bfb4ddda4043cd9d9ea26c1f

  Note when copying from the core version of PWSprefs.cpp, remove the prefix 'm_' in the following:
  PWSprefs::m_bool_prefs[NumIntPrefs] to PWSprefs::bool_prefs[NumIntPrefs]
  PWSprefs::m_int_prefs[NumIntPrefs] to PWSprefs::int_prefs[NumIntPrefs]
  PWSprefs::m_string_prefs[NumIntPrefs] to PWSprefs::string_prefs[NumIntPrefs]

*/

#include "stdafx.h"

#include "Copy_PWS_prefs.h"

const TCHAR* PWSprefs::stringTypes[] = {
   _T("ptObsolete"), _T("ptDatabase"), _T("ptApplication"), _T("ptAll")
};

const TCHAR* PWSprefs::stringDisplay[] = {
   _T("AllCollapsed"), _T("AllExpanded"), _T("AsPerLastSave"),
};

const TCHAR* PWSprefs::stringCfgLoc[] = { 
  _T("CF_NONE"), _T("CF_REGISTRY"), _T("CF_FILE_RO"), _T("CF_FILE_RW"), _T("CF_FILE_RW_NEW")
};

// 1st parameter = name of preference
// 2nd parameter = default value
// 3rd parameter if stored in database, application or obsolete
const PWSprefs::boolPref PWSprefs::bool_prefs[NumBoolPrefs] = {
  {_T("AlwaysOnTop"), false, ptApplication},                // application
  {_T("ShowPWDefault"), false, ptDatabase},                 // database
  {_T("ShowPasswordInTree"), false, ptDatabase},            // database
  {_T("SortAscending"), true, ptDatabase},                  // database
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
  {_T("UseSystemTray"), true, ptApplication},               // application
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
};

// Default value = -1 means set at runtime
// Extra two values for Integer - min and max acceptable values (ignored if = -1)
const PWSprefs::intPref PWSprefs::int_prefs[NumIntPrefs] = {
  {_T("column1width"), static_cast<unsigned int>(-1), ptApplication, -1, -1},    // application
  {_T("column2width"), static_cast<unsigned int>(-1), ptApplication, -1, -1},    // application
  {_T("column3width"), static_cast<unsigned int>(-1), ptApplication, -1, -1},    // application
  {_T("column4width"), static_cast<unsigned int>(-1), ptApplication, -1, -1},    // application
  {_T("sortedcolumn"), 0, ptApplication, 0, 15},                    // application
  {_T("PWDefaultLength"), 12, ptDatabase, 4, 1024},                  // database
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
  {_T("BackupSuffix"), BKSFX_IncNumber, ptApplication, minBKSFX, maxBKSFX}, // application
  {_T("BackupMaxIncremented"), 3, ptApplication, 1, 999},           // application
  {_T("PreExpiryWarnDays"), 1, ptApplication, 1, 30},               // application
  {_T("ClosedTrayIconColour"), stiBlack, ptApplication,
                               stiBlack, stiYellow},                // application
  {_T("PWDigitMinLength"), 0, ptDatabase, 0, 1024},                 // database
  {_T("PWLowercaseMinLength"), 0, ptDatabase, 0, 1024},             // database
  {_T("PWSymbolMinLength"), 0, ptDatabase, 0, 1024},                // database
  {_T("PWUppercaseMinLength"), 0, ptDatabase, 0, 1024},             // database
  {_T("OptShortcutColumnWidth"), 92, ptApplication, 10, 512},       // application
  {_T("ShiftDoubleClickAction"), DoubleClickCopyUsername, ptApplication,
                            minDCA, maxDCA},                        // application
  {_T("DefaultAutotypeDelay"), 10, ptApplication,
                            1, 60000},                              // application
};

const PWSprefs::stringPref PWSprefs::string_prefs[NumStringPrefs] = {
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
};

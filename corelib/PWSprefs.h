// PWSprefs.h
//-----------------------------------------------------------------------------

#ifndef PWSprefs_h
#define PWSprefs_h
/*
 * A class to abstract away the persistent storage mechanism used to store and
 * retrieve user preferences. Pre-2.03 implementations used the Windows 
 * registry. People have asked for preferences to be stored along with the 
 * database, so that the same preferences can be shared across computers
 * (e.g., using disk-on-key).
 * We need to be careful about compatability here, so: 
 * Loading preferences will be done first from the registry, then from
 * the file. This way, exisitng prefs will be imported first time, and file
 * prefs will have priority over locals. Storing prefs will be done both to
 * registry and to file. Of course, the registry part is Windows-specific.
 *
 * IMPORTANT: When adding a new preference, the new enum MUST be before last,
 * that is, right before the Num*Prefs enum. This is because the prefs are
 * identified in storage by their type and index.
 */

#include "MyString.h"

class PWSprefs {
 public:
  static PWSprefs *GetInstance(); // singleton

  // prefString is stored on file, format described in PWSprefs.cpp
  void Load(const CMyString &prefString);
  CMyString Store(); // returns string for saving in file

  enum  BoolPrefs {AlwaysOnTop, ShowPWDefault, ShowPWInList, SortAscending,
		   UseDefUser, SaveImmediately, PWUseLowercase, PWUseUppercase,
		   PWUseDigits, PWUseSymbols, PWUseHexDigits, PWEasyVision,
		   DontAskQuestion, DeleteQuestion, DCShowsPassword,
		   DontAskMinimizeClearYesNo, DatabaseClear,
		   DontAskSaveMinimize, QuerySetDef,
		   UseNewToolbar, UseSystemTray, 
		   LockOnWindowLock, LockOnIdleTimeout,
		   EscExits, NumBoolPrefs};
  enum  IntPrefs {Column1Width, Column2Width, Column3Width, Column4Width,
		  SortedColumn, PWLenDefault, MaxMRUItems, IdleTimeout,
		  NumIntPrefs};
  enum  StringPrefs {CurrentBackup, CurrentFile, LastView, DefUserName,
		     NumStringPrefs};

  bool IsChanged() const {return m_changed;}
  void ClearChanged() {m_changed = false;}

  bool GetPref(BoolPrefs pref_enum) const;
  unsigned int GetPref(IntPrefs pref_enum) const;
  // Following for case where default value is determined @ runtime
  unsigned int GetPref(IntPrefs pref_enum, unsigned int defVal) const;
  CMyString GetPref(StringPrefs pref_enum) const;

  // Special case
  void GetPrefRect(long &top, long &bottom,
		   long &left, long &right) const;

  void SetPref(BoolPrefs pref_enum, bool value);
  void SetPref(IntPrefs pref_enum, unsigned int value);
  void SetPref(StringPrefs pref_enum, const CMyString &value);
  // Special case
  void SetPrefRect(long top, long bottom,
		   long left, long right);

 private:
  PWSprefs();

  bool GetBoolPref(const CMyString &name, bool defVal) const;
  unsigned int GetIntPref(const CMyString &name, unsigned int defVal) const;
  CMyString GetStringPref(const CMyString &name, const CMyString &defVal) const;

  void SetPref(const CMyString &name, bool val);
  void SetPref(const CMyString &name, unsigned int val);
  void SetPref(const CMyString &name, const CMyString &val);

  static PWSprefs *self; // singleton

  CWinApp *m_app;
  bool m_changed;
  // below, isPersistent means stored in db, !isPersistent means use registry only
  static const struct boolPref {
    TCHAR *name; bool defVal; bool isPersistent;} m_bool_prefs[NumBoolPrefs];
  static const struct intPref {
    TCHAR *name; unsigned int defVal; bool isPersistent;} m_int_prefs[NumIntPrefs];
  static const struct stringPref {
    TCHAR *name; TCHAR *defVal; bool isPersistent;} m_string_prefs[NumStringPrefs];
  // current values, loaded/stored from db
  bool m_boolValues[NumBoolPrefs];
  unsigned int m_intValues[NumIntPrefs];
  CMyString m_stringValues[NumStringPrefs];
};
#endif /* PWSprefs_h */

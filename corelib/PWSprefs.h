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
 */

#include "MyString.h"

class PWSprefs {
 public:
  static PWSprefs *GetInstance(); // singleton

  // prefString is stored on file, format described in PWSprefs.cpp
  void Load(const CMyString &prefString);
  CMyString &Store(); // returns string for saving in file
  bool IsChanged() const {return m_changed;}

  enum  BoolPrefs {AlwaysOnTop, ShowPWDefault, ShowPWInList, SortAscending,
		   UseDefUser, SaveImmediately, PWUseLowercase, PWUseUppercase,
		   PWUseDigits, PWUseSymbols, PWUseHexDigits, PWEasyVision,
		   DontAskQuestion, DeleteQuestion, DCShowsPassword,
		   DontAskMinimizeClearYesNo, DatabaseClear,
		   DontAskSaveMinimize, QuerySetDef,
		   UseNewToolbar, UseSystemTray, 
		   NumBoolPrefs};
  enum  IntPrefs {Column1Width, Column2Width, Column3Width, Column4Width,
		  SortedColumn, PWLenDefault, MaxMRUItems,
		  NumIntPrefs};
  enum  StringPrefs {CurrentBackup, CurrentFile, LastView, DefUserName,
		     NumStringPrefs};

  bool GetPref(BoolPrefs pref_enum) const {
    return GetBoolPref(m_bool_prefs[pref_enum].name,
		       m_bool_prefs[pref_enum].defVal);}

  unsigned int GetPref(IntPrefs pref_enum) const {
    return GetIntPref(m_int_prefs[pref_enum].name,
		      m_int_prefs[pref_enum].defVal);}

  // Following for case where default value is determined @ runtime
  unsigned int GetPref(IntPrefs pref_enum, unsigned int defVal) const {
    return GetIntPref(m_int_prefs[pref_enum].name, defVal);}

  CMyString GetPref(StringPrefs pref_enum) const {
    return GetStringPref(m_string_prefs[pref_enum].name,
			 m_string_prefs[pref_enum].defVal);}

  // Special case
  void GetPrefRect(long &top, long &bottom,
		   long &left, long &right) const;

  void SetPref(BoolPrefs pref_enum, bool value)
    {SetPref(m_bool_prefs[pref_enum].name, value);}

  void SetPref(IntPrefs pref_enum, unsigned int value)
    {SetPref(m_int_prefs[pref_enum].name, value);}

  void SetPref(StringPrefs pref_enum, const CMyString &value)
    {SetPref(m_string_prefs[pref_enum].name, value);}

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
  static const struct boolPref {
    TCHAR *name; bool defVal;} m_bool_prefs[NumBoolPrefs];
  static const struct intPref {
    TCHAR *name; unsigned int defVal;} m_int_prefs[NumIntPrefs];
  static const struct stringPref {
    TCHAR *name; TCHAR *defVal;} m_string_prefs[NumStringPrefs];
};
#endif /* PWSprefs_h */

#if 0
// raw material - exisitng prefs
app.GetProfileInt(_T(""), _T("dontaskminimizeclearyesno"), FALSE) == TRUE)
app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("alwaysontop"), FALSE);
app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("column1width"),
app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("column2width"),
app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("column3width"),
app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("column4width"),
app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("databaseclear"), FALSE));
app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("dcshowspassword"), FALSE);
app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("deletequestion"), FALSE));
app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("deletequestion"), FALSE);
app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("dontaskminimizeclearyesno"), FALSE));
app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("dontaskquestion"), FALSE));
app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("dontasksaveminimize"), FALSE);
app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("maxmruitems"), 4);
app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("pweasyvision"), FALSE);
app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("pwlendefault"), 8);
app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("pwusedigits"), TRUE);
app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("pwusehexdigits"), FALSE);
app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("pwuselowercase"), TRUE);
app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("pwusesymbols"), FALSE);
app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("pwuseuppercase"), TRUE);
app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("querysetdef"), TRUE);
app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("saveimmediately"), FALSE);
app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("saveimmediately"), TRUE);
app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("showpwdefault"), FALSE);
app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("showpwinlist"), FALSE);
app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("showpwinlist"), FALSE);
app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("sortascending"), 1)
app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("sortedcolumn"), 0);
app.GetProfileInt(_T(PWS_REG_OPTIONS), _T("usedefuser"), FALSE);
app.GetProfileInt(_T(PWS_REG_OPTIONS),_T("dontasksaveminimize"), FALSE);
app.GetProfileInt(_T(PWS_REG_POSITION), _T("bottom"), -1);
app.GetProfileInt(_T(PWS_REG_POSITION), _T("left"), -1);
app.GetProfileInt(_T(PWS_REG_POSITION), _T("right"), -1);
app.GetProfileInt(_T(PWS_REG_POSITION), _T("top"), -1);
app.GetProfileString(_T(PWS_REG_OPTIONS), _T("currentbackup"), NULL);
app.GetProfileString(_T(PWS_REG_OPTIONS), _T("currentfile")));
app.GetProfileString(_T(PWS_REG_OPTIONS), _T("defusername"), _T(""));
app.GetProfileString(_T(PWS_REG_OPTIONS), _T("lastview"),

#endif

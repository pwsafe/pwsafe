#include <AfxWin.h> // for AfxGetApp()
#include "PWSprefs.h"

#if defined(POCKET_PC)
const LPCTSTR PWS_REG_POSITION = _T("Position");
const LPCTSTR PWS_REG_OPTIONS = _T("Options");
#else
const LPCTSTR PWS_REG_POSITION = _T("");
const LPCTSTR PWS_REG_OPTIONS = _T("");
#endif

PWSprefs *PWSprefs::self = NULL;

const PWSprefs::boolPref PWSprefs::m_bool_prefs[NumBoolPrefs] = {
  {_T("alwaysontop"), false},
  {_T("showpwdefault"), false},
  {_T("showpwinlist"), false},
  {_T("sortascending"), true},
  {_T("usedefuser"), false},
  {_T("saveimmediately"), false},
  {_T("pwuselowercase"), true},
  {_T("pwuseuppercase"), true},
  {_T("pwusedigits"), true},
  {_T("pwusesymbols"), false},
  {_T("pwusehexdigits"), false},
  {_T("pweasyvision"), false},
  {_T("dontaskquestion"), false},
  {_T("deletequestion"), false},
  {_T("DCShowsPassword"), false},
  {_T("DontAskMinimizeClearYesNo"), false},
  {_T("DatabaseClear"), false},
  {_T("DontAskSaveMinimize"), false},
  {_T("QuerySetDef"), true},
  {_T("UseNewToolbar"), true},
};

const PWSprefs::intPref PWSprefs::m_int_prefs[NumIntPrefs] = {
  {_T("column1width"), (unsigned int)-1}, // default unused - set @ runtime
  {_T("column2width"), (unsigned int)-1}, // default unused - set @ runtime
  {_T("column3width"), (unsigned int)-1}, // default unused - set @ runtime
  {_T("column4width"), (unsigned int)-1}, // default unused - set @ runtime
  {_T("sortedcolumn"), 0},
  {_T("pwlendefault"), 8},
  {_T("maxmruitems"), 4},
};

const PWSprefs::stringPref PWSprefs::m_string_prefs[NumStringPrefs] = {
  {_T("currentbackup"), _T("")},
  {_T("currentfile"), _T("")},
  {_T("lastview"), _T("list")},
  {_T("defusername"), _T("")},
};


PWSprefs *PWSprefs::GetInstance()
{
  if (self == NULL)
    self = new PWSprefs;
  return self;
}

PWSprefs::PWSprefs() : m_changed(false), m_app(::AfxGetApp())
{
  ASSERT(m_app != NULL);
}


bool PWSprefs::GetBoolPref(const CMyString &name, bool defVal) const
{
  return m_app->GetProfileInt(PWS_REG_OPTIONS, name, defVal) != 0;
}

unsigned int PWSprefs::GetIntPref(const CMyString &name, unsigned int defVal) const
{
  return m_app->GetProfileInt(PWS_REG_OPTIONS, name, defVal);
}

CMyString PWSprefs::GetStringPref(const CMyString &name, const CMyString &defVal) const
{
  CMyString retval = m_app->GetProfileString(PWS_REG_OPTIONS, name, defVal);
  return retval;
}

void PWSprefs::GetPrefRect(long &top, long &bottom,
			   long &left, long &right) const
{
  top = m_app->GetProfileInt(_T(PWS_REG_POSITION), _T("top"), -1);
  bottom = m_app->GetProfileInt(_T(PWS_REG_POSITION), _T("bottom"), -1);
  left = m_app->GetProfileInt(_T(PWS_REG_POSITION), _T("left"), -1);
  right = m_app->GetProfileInt(_T(PWS_REG_POSITION), _T("right"), -1);
}


void PWSprefs::SetPref(const CMyString &name, bool val)
{
  m_app->WriteProfileInt(PWS_REG_OPTIONS, name, val ? 1 : 0);
}

void PWSprefs::SetPref(const CMyString &name, unsigned int val)
{
  m_app->WriteProfileInt(PWS_REG_OPTIONS, name, val);
}

void PWSprefs::SetPref(const CMyString &name, const CMyString &val)
{
  m_app->WriteProfileString(PWS_REG_OPTIONS, name, val);
}

void PWSprefs::SetPrefRect(long top, long bottom,
			   long left, long right)
{
  m_app->WriteProfileInt(_T(PWS_REG_POSITION), _T("top"), top);
  m_app->WriteProfileInt(_T(PWS_REG_POSITION), _T("bottom"), bottom);
  m_app->WriteProfileInt(_T(PWS_REG_POSITION), _T("left"), left);
  m_app->WriteProfileInt(_T(PWS_REG_POSITION), _T("right"), right);
}

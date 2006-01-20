#include "PWSprefs.h"
#include <AfxWin.h> // for AfxGetApp()
#pragma warning(push,3) // sad that VC6 cannot cleanly compile standard headers
#include <strstream>
#pragma warning(pop)
#pragma warning(disable : 4786)
using namespace std;

#if defined(POCKET_PC)
const LPCTSTR PWS_REG_POSITION = _T("Position");
const LPCTSTR PWS_REG_OPTIONS = _T("Options");
#else
const LPCTSTR PWS_REG_POSITION = _T("");
const LPCTSTR PWS_REG_OPTIONS = _T("");
#endif

PWSprefs *PWSprefs::self = NULL;

const PWSprefs::boolPref PWSprefs::m_bool_prefs[NumBoolPrefs] = {
  {_T("alwaysontop"), false, true},
  {_T("showpwdefault"), false, true},
  {_T("showpwinlist"), false, true},
  {_T("sortascending"), true, true},
  {_T("usedefuser"), false, true},
  {_T("saveimmediately"), false, true},
  {_T("pwuselowercase"), true, true},
  {_T("pwuseuppercase"), true, true},
  {_T("pwusedigits"), true, true},
  {_T("pwusesymbols"), false, true},
  {_T("pwusehexdigits"), false, true},
  {_T("pweasyvision"), false, true},
  {_T("dontaskquestion"), false, true},
  {_T("deletequestion"), false, true},
  {_T("DCShowsPassword"), false, true},
  {_T("DontAskMinimizeClearYesNo"), true, true},
  {_T("DatabaseClear"), false, true},
  {_T("DontAskSaveMinimize"), false, true},
  {_T("QuerySetDef"), true, true},
  {_T("UseNewToolbar"), true, true},
  {_T("UseSystemTray"), true, true},
  {_T("LockOnWindowLock"), true, true},
  {_T("LockOnIdleTimeout"), true, true},
  {_T("EscExits"), true, true},
  {_T("IsUTF8"), false, true},
  {_T("HotKeyEnabled"), false, true},
  {_T("MRUOnFileMenu"), true, true},
};

const PWSprefs::intPref PWSprefs::m_int_prefs[NumIntPrefs] = {
  {_T("column1width"), (unsigned int)-1, false}, // default unused - set @ runtime
  {_T("column2width"), (unsigned int)-1, false}, // default unused - set @ runtime
  {_T("column3width"), (unsigned int)-1, false}, // default unused - set @ runtime
  {_T("column4width"), (unsigned int)-1, false}, // default unused - set @ runtime
  {_T("sortedcolumn"), 0, true},
  {_T("pwlendefault"), 8, true},
  {_T("maxmruitems"), 4, true},
  {_T("IdleTimeout"), 5, true},
  {_T("HotKey"), 0, true}, // zero means disabled, !=0 is key code.
  {_T("DoubleClickAction"), PWSprefs::DoubleClickCopy, true},
};

const PWSprefs::stringPref PWSprefs::m_string_prefs[NumStringPrefs] = {
  {_T("currentbackup"), _T(""), true},
  {_T("currentfile"), _T(""), false},
  {_T("lastview"), _T("tree"), true},
  {_T("defusername"), _T(""), true},
  {_T("treefont"), _T(""), true},
};


PWSprefs *PWSprefs::GetInstance()
{
  if (self == NULL)
    self = new PWSprefs;
  return self;
}

PWSprefs::PWSprefs() : m_app(::AfxGetApp()), m_changed(false)
{
  ASSERT(m_app != NULL);
  // Start by reading in from registry
  int i;
  // Read in values from registry
  for (i = 0; i < NumBoolPrefs; i++)
    m_boolValues[i] = m_app->GetProfileInt(PWS_REG_OPTIONS,
					   m_bool_prefs[i].name,
					   m_bool_prefs[i].defVal) != 0;
  for (i = 0; i < NumIntPrefs; i++)
    m_intValues[i] = m_app->GetProfileInt(PWS_REG_OPTIONS,
					  m_int_prefs[i].name,
					  m_int_prefs[i].defVal);
  for (i = 0; i < NumStringPrefs; i++)
    m_stringValues[i] = CMyString(m_app->GetProfileString(PWS_REG_OPTIONS,
							  m_string_prefs[i].name,
							  m_string_prefs[i].defVal));
}


bool PWSprefs::GetPref(BoolPrefs pref_enum) const
{
  return m_bool_prefs[pref_enum].isPersistent ? m_boolValues[pref_enum] :
    GetBoolPref(m_bool_prefs[pref_enum].name, m_bool_prefs[pref_enum].defVal);
}

bool PWSprefs::GetBoolPref(const CMyString &name, bool defVal) const
{
  return m_app->GetProfileInt(PWS_REG_OPTIONS, name, defVal) != 0;
}

unsigned int PWSprefs::GetPref(IntPrefs pref_enum) const
{
  return m_int_prefs[pref_enum].isPersistent ? m_intValues[pref_enum] :
    GetIntPref(m_int_prefs[pref_enum].name, m_int_prefs[pref_enum].defVal);
}

unsigned int PWSprefs::GetPref(IntPrefs pref_enum, unsigned int defVal) const
{
  if (m_int_prefs[pref_enum].isPersistent)
    return m_intValues[pref_enum] == (unsigned int)-1 ? defVal : m_intValues[pref_enum];
  else
    return GetIntPref(m_int_prefs[pref_enum].name, defVal);
}

unsigned int PWSprefs::GetIntPref(const CMyString &name, unsigned int defVal) const
{
  return m_app->GetProfileInt(PWS_REG_OPTIONS, name, defVal);
}

CMyString PWSprefs::GetPref(StringPrefs pref_enum) const
{
  return m_string_prefs[pref_enum].isPersistent ? m_stringValues[pref_enum] :
    GetStringPref(m_string_prefs[pref_enum].name, m_string_prefs[pref_enum].defVal);
}

CMyString PWSprefs::GetStringPref(const CMyString &name, const CMyString &defVal) const
{
  CMyString retval = m_app->GetProfileString(PWS_REG_OPTIONS, name, defVal);
  return retval;
}

void PWSprefs::GetPrefRect(long &top, long &bottom,
			   long &left, long &right) const
{
  // this is never store in db
  top = m_app->GetProfileInt(PWS_REG_POSITION, _T("top"), -1);
  bottom = m_app->GetProfileInt(PWS_REG_POSITION, _T("bottom"), -1);
  left = m_app->GetProfileInt(PWS_REG_POSITION, _T("left"), -1);
  right = m_app->GetProfileInt(PWS_REG_POSITION, _T("right"), -1);
}


void PWSprefs::SetPref(BoolPrefs pref_enum, bool value)
{
  m_changed |= (m_boolValues[pref_enum] != value && m_int_prefs[pref_enum].isPersistent);
  m_boolValues[pref_enum] = value;
  SetPref(m_bool_prefs[pref_enum].name, value);
}

void PWSprefs::SetPref(const CMyString &name, bool val)
{
  m_app->WriteProfileInt(PWS_REG_OPTIONS, name, val ? 1 : 0);
}

void PWSprefs::SetPref(IntPrefs pref_enum, unsigned int value)
{
  m_changed |= (m_intValues[pref_enum] != value && m_int_prefs[pref_enum].isPersistent);
  m_intValues[pref_enum] = value;
  SetPref(m_int_prefs[pref_enum].name, value);
}

void PWSprefs::SetPref(const CMyString &name, unsigned int val)
{
  m_app->WriteProfileInt(PWS_REG_OPTIONS, name, val);
}

void PWSprefs::SetPref(StringPrefs pref_enum, const CMyString &value)
{
  m_changed |= (m_stringValues[pref_enum] != value && m_string_prefs[pref_enum].isPersistent);
  m_stringValues[pref_enum] = value;
  SetPref(m_string_prefs[pref_enum].name, value);
}

void PWSprefs::SetPref(const CMyString &name, const CMyString &val)
{
  m_app->WriteProfileString(PWS_REG_OPTIONS, name, val);
}

void PWSprefs::SetPrefRect(long top, long bottom,
			   long left, long right)
{
  m_app->WriteProfileInt(PWS_REG_POSITION, _T("top"), top);
  m_app->WriteProfileInt(PWS_REG_POSITION, _T("bottom"), bottom);
  m_app->WriteProfileInt(PWS_REG_POSITION, _T("left"), left);
  m_app->WriteProfileInt(PWS_REG_POSITION, _T("right"), right);
}

CMyString PWSprefs::Store()
{
  /*
   * Create a string of values that are (1) different from the defaults, && (2) are isPersistent
   * String is of the form "X nn vv X nn vv..." Where X=[BIS] for binary, integer and string, resp.,
   * nn is the numeric value of the enum, and vv is the value, {1.0} for bool, unsigned integer
   * for int, and quoted string for String.
   */
  CMyString retval(_T(""));
  ostrstream os;
  int i;
  for (i = 0; i < NumBoolPrefs; i++)
    if (m_boolValues[i] != m_bool_prefs[i].defVal &&
	m_bool_prefs[i].isPersistent)
      os << "B " << i << ' ' << (m_boolValues[i] ? 1 : 0) << ' ';
  for (i = 0; i < NumIntPrefs; i++)
    if (m_intValues[i] != m_int_prefs[i].defVal &&
	m_int_prefs[i].isPersistent)
      os << "I " << i << ' ' << m_intValues[i] << ' ';
  for (i = 0; i < NumStringPrefs; i++)
    if (m_stringValues[i] != m_string_prefs[i].defVal &&
	m_string_prefs[i].isPersistent)
      os << "S " << i << " \"" << LPCTSTR(m_stringValues[i]) << "\" ";
  os << ends;
  retval = os.str();
  delete[] os.str(); // reports memory leaks in spite of this!
  return retval;
}

void PWSprefs::Load(const CMyString &prefString)
{
  // parse prefString, updating current values
  istrstream is(prefString);
  char type;
  int index;

  while (is) {
    is >> type >> index;
    switch (type) {
    case 'B':
      if (index >= NumBoolPrefs) {
	continue; // forward compatibility
      } else {
	int val;
	is >> val;
	ASSERT(val == 0 || val == 1);
	m_boolValues[index] = (val != 0);
      }
      break;
    case 'I':
      if (index >= NumIntPrefs) {
	continue; // forward compatibility
      } else {
	unsigned int val;
	is >> val;
	m_intValues[index] = val;
      }
      break;
    case 'S':
      if (index >= NumStringPrefs) {
	continue; // forward compatibility
      } else {
	const int N = prefString.GetLength(); // safe upper limit on string size
	char *buf = new char[N];
	is.ignore(2, '\"'); // skip over space and leading "
	is.get(buf, N, '\"'); // get string value
	CMyString val(buf);
	m_stringValues[index] = val;
	delete[] buf;
      }
      break;
    default:
      continue; // forward compatibility (also last space)
    } // switch
  } // while
}

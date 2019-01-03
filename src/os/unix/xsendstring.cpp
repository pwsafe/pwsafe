/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/*
 * xsendstring - send a bunch of keystrokes to the app having current input
 *focus
 *
 * Calls X library functions defined in Xt and Xtst
 *
 * To-Do list:
 * +. Initialize all the params of XKeyEvent
 * +  __STD_ISO_10646__ check
 * +  Remap an unused keycode to a keysym of XKeysymToKeycode fails
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <vector>
#include <errno.h>
#include <limits.h>

#include <memory>
#include <algorithm>
#include <functional>
#include <fstream>
#include <sstream>
#include <map>

#include <X11/Intrinsic.h> // in libxt-dev
#include <X11/keysym.h>
#include <X11/extensions/XTest.h> // in libxtst-dev

#include "./xsendstring.h"
#include "../sleep.h"
#include "../../core/PwsPlatform.h" // for NumberOf()
#include "../../core/StringX.h"
#include "./unicode2keysym.h"

// X errors are reported via asynchronous callbacks, which we store here
std::ostringstream g_xerrormsg;

std::ostream &operator<<(std::ostream &os, unsigned char c) {
  return os << std::showbase << std::hex << static_cast<int>(c);
}
// When something goes wrong, we write our error message to g_xerrormsg
// above and throw this exception. We also throw this exception if we
// notice that it has some data (that got there by XErrorHandler below)
class autotype_exception : public std::exception {
  std::string msg; // we get this from g_xerrormsg above
public:
  autotype_exception() : msg(g_xerrormsg.str()) {}
  const char *what() const throw() override { return msg.c_str(); }
};

/*
 * ErrorHandler will be called when X detects an error. This function
 * just saves the error message text
 */
int ErrorHandler(Display *my_dpy, XErrorEvent *event) {
  char xmsg[512] = { 0 };

  XGetErrorText(my_dpy, event->error_code, xmsg, NumberOf(xmsg) - 1);

  g_xerrormsg << "X error (" << event->request_code << "): " << xmsg
              << std::endl;
  return 0;
}

class XErrorHandlerInstaller {

  typedef int(XErrorHandler)(Display *, XErrorEvent *);
  XErrorHandler *m_pfnOldErrorHandler;

public:
  XErrorHandlerInstaller()
      : m_pfnOldErrorHandler(XSetErrorHandler(ErrorHandler)) {}

  ~XErrorHandlerInstaller() { XSetErrorHandler(m_pfnOldErrorHandler); }
};

enum class KeyEventType { PRESS, RELEASE };

// A simple helper class
class AutotypeEvent : public XKeyEvent {
public:
  AutotypeEvent(Display *disp, KeyEventType ktype, int kcode, int kstate,
                Time ktime = CurrentTime) {
    display = disp;
    int revert_to;
    XGetInputFocus(display, &window, &revert_to);
    subwindow = None;
    x = y = x_root = y_root = 1;
    same_screen = True;
    state = kstate;
    keycode = kcode;
    type = ktype == KeyEventType::PRESS? KeyPress: KeyRelease;
    time = ktime;
  }

  bool IsPressed() const { return type == KeyPress; }
};

// helper method that throws if keysym is not mapped to any KeyCode
KeyCode KeySymToKeyCode(Display *disp, KeySym sym);

// These two classes help with RAII
class XKeyboardMapping {
  KeySym *symlist;
  int keysyms_per_keycode;

public:
  XKeyboardMapping(Display *disp, KeyCode code)
      : symlist(XGetKeyboardMapping(disp, code, 1, &keysyms_per_keycode)) {
    if (!symlist) {
      g_xerrormsg << "Could not get X keyboard mapping for keycode " << std::hex
                  << std::showbase << code;
      throw autotype_exception();
    }
  }
  ~XKeyboardMapping() { XFree(symlist); }
  size_t IndexOf(KeySym sym) const {
    return std::find(symlist, symlist + keysyms_per_keycode, sym) - symlist;
  }
};

////////////////////////////////////////////////
// X ModMap has a fixed mapping between the index (0 - 8)
// of a modifier in the modmap, and the mask associated
// with that modifier. And we need to search the modmap
// by both. This class encapsulates that mapping between
// the index and mask of the modifier, the class invariant
// being a valid mapping between the two
class XModPos {
public:
  enum ModMapIndex : unsigned char {
    MODMAP_INDEX_SHIFT = ShiftMapIndex,
    MODMAP_INDEX_LOCK = LockMapIndex,
    MODMAP_INDEX_CONTROL = ControlMapIndex,
    MODMAP_INDEX_MOD1 = Mod1MapIndex,
    MODMAP_INDEX_MOD2 = Mod2MapIndex,
    MODMAP_INDEX_MOD3 = Mod3MapIndex,
    MODMAP_INDEX_MOD4 = Mod4MapIndex,
    MODMAP_INDEX_MOD5 = Mod5MapIndex,
  };

private:
  unsigned char m_index;

public:
  explicit XModPos(ModMapIndex i) : m_index(i) {}

  unsigned char index() const { return m_index; }
  unsigned char mask() const { return 1 << m_index; }

  bool last() const { return m_index == MODMAP_INDEX_MOD5; }

  XModPos operator++() { return operator+=(1); }

  XModPos &operator+=(unsigned char offset) {
    if ((m_index + offset) <= MODMAP_INDEX_MOD5) {
      m_index += offset;
    } else {
      g_xerrormsg << "attempt to offset modmap index at " << m_index << " by "
                  << offset;
      throw autotype_exception();
    }
    return *this;
  }
};

class XModMap {
  XModifierKeymap *modmap;

public:
  explicit XModMap(Display *disp) : modmap(XGetModifierMapping(disp)) {
    if (!disp) {
      g_xerrormsg << "Could not get X modifier map";
      throw autotype_exception();
    }
  }
  ~XModMap() { XFreeModifiermap(modmap); }

  KeyCode ModifierKeyCode(XModPos pos) const {
    const KeyCode *start =
        modmap->modifiermap + pos.index() * modmap->max_keypermod;
    const KeyCode *pkc = std::find_if_not(start, start + modmap->max_keypermod,
                                          [](KeyCode c) { return c == 0; });
    if (pkc == start + modmap->max_keypermod) {
      // This is not an error. A KeyCode doesn't have to be mapped to each
      // modifier index
      return 0;
    }
    return *pkc;
  }

  XModPos IndexOf(KeyCode code) const {
    const auto keys = modmap->modifiermap,
               keys_end = modmap->modifiermap + 8 * modmap->max_keypermod;
    const auto keyptr =
        std::find_if(keys, keys_end, [code](KeyCode k) { return k == code; });
    if (keyptr != keys_end) {
      XModPos pos{ XModPos::MODMAP_INDEX_SHIFT };
      pos += ((keyptr - keys) / modmap->max_keypermod - ShiftMapIndex);
      return pos;
    }
    g_xerrormsg << "Could not find KeyCode " << code << " in modifier map";
    throw autotype_exception();
  }
};

//////////////////////////////////////////////////////////////////////////////////////
// ModifierKey
//
// A keycode and the mask it generates, deduced from XModifierMap. You can
// either create it from a fixed position in the XModifierMap, or from a
// KeySym that is mapped to some KeyCode that's there in the XModifierMap
//
struct ModifierKey {
  KeyCode code;
  int mask;
  using klist = std::vector<KeyEventType>;
  klist set_events, unset_events;

  ModifierKey(klist set_ev, klist unset_ev)
      : code(0), mask(0), set_events(std::move(set_ev)),
        unset_events(std::move(unset_ev)) {}

public:
  ModifierKey(Display *disp, XModPos pos, klist set_events = { KeyEventType::PRESS },
              klist unset_events = { KeyEventType::RELEASE });
  ModifierKey(Display *disp, KeySym ks, klist set_events = { KeyEventType::PRESS },
              klist unset_events = { KeyEventType::RELEASE });
  ModifierKey(Display *disp, KeyCode code, int mask,
              klist set_events = { KeyEventType::PRESS },
              klist unset_events = { KeyEventType::RELEASE });

  bool IsValid() const {
    return code != 0 && (mask != 0 && (mask == 1 || mask % 2 == 0));
  }
};

ModifierKey::ModifierKey(Display *disp, XModPos pos, klist set_events,
                         klist unset_events)
    : ModifierKey(set_events, unset_events) {
  XModMap modmap(disp);
  // we can cheat safely now that index is known to be one of 1 - 8
  mask = pos.mask();
  code = modmap.ModifierKeyCode(pos);
  if (code == 0) {
    // if we are asked for a modifier at a specific index, it better be mapped
    g_xerrormsg << "No KeyCode mapped to index " << pos.index();
    throw autotype_exception();
  }
}

ModifierKey::ModifierKey(Display *disp, KeySym ks, klist set_events,
                         klist unset_events)
    : ModifierKey(set_events, unset_events) {
  XModMap modmap(disp);
  code = KeySymToKeyCode(disp, ks);
  const XModPos pos = modmap.IndexOf(code);
  mask = pos.mask();
}

ModifierKey::ModifierKey(Display *disp, KeyCode kcode, int kmask,
                         klist set_events, klist unset_events)
    : ModifierKey(set_events, unset_events) {
  code = kcode;
  mask = kmask;
}

//////////////////////////////////////////////////////////////////////////////////////
// AutotypeMethodBase
//
// Base class for the various ways to emulate keystrokes that result in a
// keyboard event interpreted by the receiving application as the intended
// character.
//
// This class takes care of wchar_t => KeySym => (KeyCode, Modifiers)
// combination, and generates the key-up & key-down events in the required
// order to generate the character being auto-typed.  The derived classes
// only need to implement the virtual GenerateKeyEvent() member that
// emulates a single keystroke

class AutotypeMethodBase {
protected:
  Display *m_display;

public:
  explicit AutotypeMethodBase(Display *display) : m_display(display) {}

  // virtual, because derived classes will have clean-ups of their own
  virtual ~AutotypeMethodBase() {}

  virtual void GenerateKeyEvent(AutotypeEvent *ev) = 0;
};

//////////////////////////////////////////////////////////////////////////
// AutotypeMethodXTEST
//
// Emulates keystrokes using the XTEST extension.
//
class AutotypeMethodXTEST : public AutotypeMethodBase {
public:
  explicit AutotypeMethodXTEST(Display *display) : AutotypeMethodBase(display) {
    XTestGrabControl(display, true);
  }
  ~AutotypeMethodXTEST() override { XTestGrabControl(m_display, false); }

protected:
  void GenerateKeyEvent(AutotypeEvent *ev) override {
    XTestFakeKeyEvent(ev->display, ev->keycode, ev->IsPressed()? True: False,
                      CurrentTime);
  }
};

//////////////////////////////////////////////////////////////////////////
// AutotypeMethodSendKeys
//
// Emulates keystrokes using the XSendEvent method
//
class AutotypeMethodSendKeys : public AutotypeMethodBase {
public:
  explicit AutotypeMethodSendKeys(Display *display) : AutotypeMethodBase(display) {}
  ~AutotypeMethodSendKeys() override { XSync(m_display, False); }

protected:
  void GenerateKeyEvent(AutotypeEvent *ev) override {
    XSendEvent(ev->display, ev->window, TRUE,
               ev->IsPressed() ? KeyPressMask : KeyReleaseMask,
               reinterpret_cast<XEvent *>(ev));
  }
};

//////////////////////////////////////////////////////////////////////////
// AutotypeLogger
//
// This logs the keystrokes to the file provided
//
class AutotypeLogger : public AutotypeMethodBase {
  std::ofstream atlog;
  XComposeStatus cstat{};

public:
  explicit AutotypeLogger(Display *display, const char *filename)
      : AutotypeMethodBase(display), atlog(filename) {}
  ~AutotypeLogger() override {}

protected:
  void GenerateKeyEvent(AutotypeEvent *ev) override {
    char buf[256]{};
    KeySym ks{NoSymbol};
    int nRet = XLookupString(ev, buf, NumberOf(buf)-1, &ks, &cstat);
    if (nRet < 0 || static_cast<unsigned int>(nRet) >= NumberOf(buf)) nRet = 0;
    buf[nRet] = 0;
    const char *strks = XKeysymToString(ks);
    if (!strks) strks = "NULL";
    atlog << ev->keycode << (ev->IsPressed() ? " pressed" : " released")
          << " with mask " << ev->state << " => char [" << buf << ']'
          << ", keysym [" << ks << "], name [" << strks << ']' << std::endl;
  }
};

//////////////////////////////////////////////////////////////////////////
// Factory method for generating the correct type of autotype method, based
// on
//   1. User preference
//   2. XTEST extension availability
//
AutotypeMethodBase *
GetAutotypeMethod(Display *disp, pws_os::AutotypeMethod method_preference) {
  // To enable logging for autotype, uncomment the following block of code
  // Then run pwsafe in a terminal like this:
  //
  //    PASSWORDSAFE_AUTOTYPE_LOG=/tmp/pwsafe-autotype.log /path/to/pwsafe
  //
  // Then send the /tmp/pwsafe-autotype.log file to devs.
  //
  // IMPORTANT: remember NOT to use your actual passwords for this purpose.
  // Your password can be deciphered from the log file. Delete the
  // /tmp/pwsafe-autotype.log file after sending it to devs. Also, put the
  // comments back on and rebuild once you are done testing
  //

  /*
  const char *atlog = getenv("PASSWORDSAFE_AUTOTYPE_LOG");
  if (atlog) {
    return new AutotypeLogger(disp, atlog);
  }
  */

  int major_opcode, first_event, first_error;
  AutotypeMethodBase *method_ptr = nullptr;
  if (method_preference != pws_os::ATMETHOD_XSENDKEYS &&
      XQueryExtension(disp, "XTEST", &major_opcode, &first_event,
                      &first_error)) {
    method_ptr = new AutotypeMethodXTEST(disp);
  } else {
    method_ptr = new AutotypeMethodSendKeys(disp);
  }
  return method_ptr;
}

///////////////////////////////////////////////////////
// ModifierFactory
//
// Creates and caches Modifier keys from KeyCode/KeySym

class ModifierFactory {
  Display *m_display;

  // helper class for on-demand creation & caching of mods
  template <typename Param> class ModifierCreator {
    Display *m_display;
    Param m_p;
    ModifierKey *m_key = nullptr;

  public:
    ModifierCreator(Display *d, Param p) : m_display{ d }, m_p{ p } {}
    operator ModifierKey &() {
      if (!m_key) {
        m_key = new ModifierKey(m_display, m_p);
      }
      return *m_key;
    }

    ModifierCreator(const ModifierCreator &) = delete;
    ModifierCreator(ModifierCreator &&) = delete;
    ModifierCreator &operator=(const ModifierCreator &) = delete;
    ModifierCreator &operator=(ModifierCreator &&) = delete;

    ~ModifierCreator() { delete m_key; }
  };

  ModifierCreator<XModPos> m_shift, m_ctrl;
  ModifierCreator<KeySym> m_mode_switch, m_level3_shift;

public:
  explicit ModifierFactory(Display *disp)
      : m_display{ disp },
        m_shift{ disp, XModPos{ XModPos::MODMAP_INDEX_SHIFT } },
        m_ctrl{ disp, XModPos{ XModPos::MODMAP_INDEX_CONTROL } },
        m_mode_switch{ disp, XK_Mode_switch },
        m_level3_shift{ disp, XK_ISO_Level3_Shift } {}

  std::vector<ModifierKey> GetModifiersForKeySym(KeyCode code, KeySym sym);

  ModifierKey Control() { return m_ctrl; }
};

std::vector<ModifierKey> ModifierFactory::GetModifiersForKeySym(KeyCode code,
                                                                KeySym sym) {
  XKeyboardMapping symlist(m_display, code);
  const size_t keysym_index = symlist.IndexOf(sym);

  switch (keysym_index) {

  case 0: // plain key
    return {};

  case 1:
    return { m_shift };

  case 2:
    return { m_mode_switch };

  case 3:
    return { m_shift, m_mode_switch };

  case 4:
    return { m_level3_shift };

  case 5:
    return { m_shift, m_level3_shift };

  case 6:
    return { m_mode_switch, m_level3_shift };

  case 7:
    return { m_shift, m_mode_switch, m_level3_shift };

  default:
    g_xerrormsg << "Modifier keys for generating keysym " << sym
                << " fall in unsupported " << keysym_index << " level";
    throw autotype_exception();
  }
}

KeySym _wchar2keysym(wchar_t wc) {
  if (wc < 0x100) {
    if (wc >= 0x20) return wc;
    switch (wc) {
    case L'\t':
      return XK_Tab;
    case L'\r':
      return XK_Return;
    case L'\n':
      return XK_Linefeed;
    case '\010':
      return XK_BackSpace;
    case '\177':
      return XK_Delete;
    case '\033':
      return XK_Escape;
    default:
      return NoSymbol;
    }
  }
  if (wc > 0x10ffff || (wc > 0x7e && wc < 0xa0)) return NoSymbol;
  KeySym sym = unicode2keysym(wc);
  if (sym != NoSymbol) return sym;
  // For everything else, there's Mastercard :)
  return wc | 0x01000000;
}

KeySym _wchar2string2keysym(wchar_t wc) {
  char ucstr[16] = { 0 };
  snprintf(ucstr, 16, "U%04X", wc);
  return XStringToKeysym(ucstr);
}

KeySym WcharToKeySym(Display *disp, wchar_t wc) {
  using std::endl;
  // Try a regular conversion first
  KeySym sym = _wchar2keysym(wc);

  // Failing which, get it algorithmically
  if (sym == NoSymbol || !XKeysymToKeycode(disp, sym)) {
    sym = _wchar2string2keysym(wc);
  }

  // its a no-go
  if (sym == NoSymbol) {
    g_xerrormsg
        << std::hex << std::showbase
        << "Could not convert wchar_t value to X KeySym. Aborting autotype."
        << endl
        << "If \'xmodmap -pk\' does not list this KeySym, you probably need "
        << "to install an appropriate keyboard layout." << endl
        << "wchar_t value: " << wc << endl;
    throw autotype_exception();
  }

  return sym;
}

KeyCode KeySymToKeyCode(Display *disp, KeySym sym) {
  const KeyCode code = XKeysymToKeycode(disp, sym);
  if (!code) {
    using std::endl;
    const char *symStr = XKeysymToString(sym);
    g_xerrormsg
        << std::hex << std::showbase
        << "Could not convert X KeySym to X keycode. Aborting autotype." << endl
        << "If \'xmodmap -pk\' does not list this KeySym, you probably need "
        << "to install an appropriate keyboard layout." << endl
        << "KeySym value : " << sym << endl
        << "KeySym string: " << (symStr ? symStr : "NULL") << endl;
    throw autotype_exception();
  }
  return code;
}

template <class ContIter, class ModIter>
void SequenceAutotypeEvents(ContIter ci, ModIter mbegin, ModIter mend,
                            Display *disp, KeyCode code, bool emulateMods) {

  using mem_evt_set = decltype(ModifierKey::set_events);
  auto add_mods = [&](const mem_evt_set ModifierKey::* ms) {
    std::for_each(mbegin, mend, [&](const ModifierKey &m) {
      std::for_each((m.*ms).begin(), (m.*ms).end(), [&](KeyEventType kt) {
          (*ci)++ = AutotypeEvent{ disp, kt, m.code, 0 };
      });
    });
  };

  std::for_each(mbegin, mend,
                [](const ModifierKey &m) { assert(m.IsValid()); });

  if (emulateMods) {
    add_mods(&ModifierKey::set_events);
  }

  int mask = 0;
  std::for_each(mbegin, mend,
                [&mask](const ModifierKey &m) { mask |= m.mask; });

  assert(std::distance(mbegin, mend) == 0 || mask != 0);

  (*ci)++ = AutotypeEvent{ disp, KeyEventType::PRESS, code, mask };
  (*ci)++ = AutotypeEvent{ disp, KeyEventType::RELEASE, code, mask };

  if (emulateMods) {
    add_mods(&ModifierKey::unset_events);
  }
}

// This class helps me not include Xlib headers in xsendstring.h
struct wchar2xevent_map_ptr {
  using wchar2event_map =
      std::map<StringX::value_type, std::vector<AutotypeEvent> >;
  wchar2event_map m_map;
  wchar2event_map *operator->() { return &m_map; }
};

/*
 * DoSendString - actually sends a string to the X Window having input focus
 *
 * The main task of this function is to convert the wchar_t values
 * into X Keyboard events. Keyboard events consist of keycodes and modifier
 * flags like Shift, Alt, etc. The KeyCodes can have any random values
 * and are not contiguous like the ascii/unicode values are, and their mapping
 * to wchar_t values are system-dependent.
 *
 * wchar_t => KeySym (eg. XK_A, or XK_Colon)
 * KeySym  => keycode + index (of keysym into that keycode's keysym list.
 *                             Each keycode can generate 1-8 keysyms in
 *                             combination with modifier keys like Shift)
 * index   => Modifier key + modifier mask
 *
 * Then we can generate X Keyboard events for each wchar_t. Sometimes, we
 * also need to generate separate keyboard events for the modifier keys
 * themselves.
 *
 * Modifier keys like Shift are press-and-hold, which means we need to generate
 * a keydown event for the modifier key before the main key, and a keyup event
 * afterwards. Some modifier keys (e.g. latch, lock, etc) behave differently,
 * but we don't need those as we support only 8 keysyms per keycode. I'm not
 * aware how those key events are actually generated by users on their
 *keyboards.
 */
void CKeySendImpl::DoSendString(const StringX &str, unsigned delayMS,
                                bool emulateMods) {
  using std::vector;
  g_xerrormsg.str(std::string());

  // convert all the chars into keycodes and required shift states first
  // Abort if any of the characters cannot be converted
  typedef vector<AutotypeEvent> AutotypeEventVector;
  AutotypeEventVector keypresses;

  for (const auto &chr : str) {

    // throw away 'vertical tab' chars which are only used on Windows to send a
    // shift+tab as a workaround for some issues with IE
    if (chr == _T('\v')) continue;

    using maptype = wchar2xevent_map_ptr::wchar2event_map;

    wchar2xevent_map_ptr &charmap = *m_wcharmap;
    maptype::const_iterator itr = charmap->lower_bound(chr);
    if (itr != charmap->end() && charmap->key_comp()(itr->first, chr)) {
      keypresses.insert(keypresses.end(), itr->second.begin(),
                        itr->second.end());
    } else {
      const KeySym sym = WcharToKeySym(m_display, chr);
      const KeyCode code = KeySymToKeyCode(m_display, sym);

      const std::vector<ModifierKey> modkeys =
          m_modFactory->GetModifiersForKeySym(code, sym);
      std::for_each(modkeys.begin(), modkeys.end(),
                    [](const ModifierKey &m) { assert(m.IsValid()); });

      const AutotypeEventVector::size_type count = keypresses.size();

      SequenceAutotypeEvents(std::back_inserter(keypresses), modkeys.begin(),
                             modkeys.end(), m_display, code, emulateMods);

      charmap->insert(itr,
                      maptype::value_type{ chr, { keypresses.begin() + count,
                                                  keypresses.end() } });

      // hack to know after which keys to sleep
      keypresses.push_back(AutotypeEvent{ m_display, KeyEventType::PRESS, 0, 0 });
    }
  }

  for (auto k : keypresses) {
    if (k.keycode) {
      m_method->GenerateKeyEvent(&k);
    } else {
      XFlush(m_display);
      pws_os::sleep_ms(delayMS);
    }
  }
}

void CKeySendImpl::SendString(const StringX &str, unsigned delayMS) {
  DoSendString(str, delayMS, m_emulateModsSeparately);
}

void CKeySendImpl::SelectAll(unsigned delayMS, int code /*= 0*/,
                             int mask /*= 0*/) {
  assert(mask >= 0 && mask <= 255);

  std::vector<ModifierKey> modkeys;

  XModMap modmap(m_display);

  if (code) {
    // user configured something, so use it even if mask is 0
    // mask could have multiple bits, each bit would require its own modkey
    for (XModPos pos{ XModPos::MODMAP_INDEX_SHIFT }; !pos.last(); ++pos) {
      if (mask & pos.mask()) {
        KeyCode kc = modmap.ModifierKeyCode(pos);
        if (kc) {
          // note that the press & hold behavior is hardcoded here
          modkeys.push_back(ModifierKey{ m_display, kc, pos.mask() });
        } else {
          g_xerrormsg << "No KeyCode mapped to mask bit " << pos.index()
                      << " for generating Select-All event";
          throw autotype_exception();
        }
      }
    }
  } else {
    // Ctrl-A it is!
    code = KeySymToKeyCode(m_display, XK_A);
    modkeys.push_back( m_modFactory->Control() );
  }

  std::for_each(modkeys.cbegin(), modkeys.cend(),
                [](const ModifierKey &m) { assert(m.IsValid()); });

  std::vector<AutotypeEvent> selectAllEvents;
  SequenceAutotypeEvents(std::back_inserter(selectAllEvents), modkeys.cbegin(),
                         modkeys.cend(), m_display, code,
                         m_emulateModsSeparately);

  for (auto k : selectAllEvents) {
    m_method->GenerateKeyEvent(&k);
  }
  XFlush(m_display);
  // sleep only after all keys for select-all are typed
  pws_os::sleep_ms(delayMS);
}

CKeySendImpl::CKeySendImpl(pws_os::AutotypeMethod method)
    : m_display(XOpenDisplay(nullptr)) {
  if (m_display) {
    m_errHandlerInstaller = new XErrorHandlerInstaller;
    m_method = GetAutotypeMethod(m_display, method);
    m_modFactory = new ModifierFactory(m_display);
    m_wcharmap = new wchar2xevent_map_ptr;
  } else {
    g_xerrormsg << "Could not open X display for autotyping";

    throw autotype_exception();
  }
}

CKeySendImpl::~CKeySendImpl() {
  delete m_method;
  delete m_errHandlerInstaller;
  delete m_modFactory;
  delete m_wcharmap;
  XCloseDisplay(m_display);
}

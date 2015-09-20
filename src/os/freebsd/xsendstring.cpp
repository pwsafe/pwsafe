/*
* Copyright (c) 2003-2015 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/*
 * xsendstring - send a bunch of keystrokes to the app having current input focus
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

#include <X11/Intrinsic.h> // in libxt-dev
#include <X11/keysym.h>
#include <X11/extensions/XTest.h> // in libxtst-dev

#include "./xsendstring.h"
#include "../sleep.h"
#include "../../core/PwsPlatform.h" // for NumberOf()
#include "../../core/StringX.h"
#include "./unicode2keysym.h"

// We convert all chars in a string to a keycode and a set of modifier keys (like Shift)
typedef struct _KeyPress {
  KeyCode code;
  unsigned int state;
} KeyPressInfo;

// X errors are reported via asynchronous callbacks, which we store here
struct AutotypeGlobals
{
  Boolean      error_detected;
  char      errorString[1024];
} atGlobals  = { False, {0} };

// We throw this exception when we detect something in atGlobals above
class autotype_exception: public std::exception
{
  public:
  virtual const char* what() const throw() {
    return atGlobals.errorString;
  }
};

/*
 * ErrorHandler will be called when X detects an error. This function
 * just sets a global flag and saves the error message text
 */
int ErrorHandler(Display *my_dpy, XErrorEvent *event)
{
  char xmsg[512] = {0};

  atGlobals.error_detected = TRUE;
  XGetErrorText(my_dpy, event->error_code, xmsg, NumberOf(xmsg) - 1);
  snprintf(atGlobals.errorString, NumberOf(atGlobals.errorString)-1, "X error (%d): %s", event->request_code, xmsg);
  return 0;
}



// A simple helper class
class AutotypeEvent: public XKeyEvent {
public:
  AutotypeEvent(Display *disp)
  {
    display = disp;
    int    revert_to;
    XGetInputFocus(display, &window, &revert_to);
    subwindow = None;
    x = y = x_root = y_root = 1;
    same_screen = True;
  }
};

//////////////////////////////////////////////////////////////////////////////////////
// AutotypeMethodBase
//
// Base class for the various ways to emulate keystrokes that result in a keyboard
// event interpreted by the receiving application as the intended character.
//
// This class takes care of wchar_t => KeySym => (KeyCode, Modifiers) combination, and
// generates the key-up & key-down events in the required order to generate the
// character being auto-typed.  The derived classes only need to implement the
// virtual GenerateKeyEvent() member that emulates a single keystroke

class AutotypeMethodBase
{
  // We use this array to look up modifier keycodes from modifier masks
  struct ModInfo {
    // These enums are uses as array index in SetModifiers() method below
    // Don't change their values!!
    enum ModType {Shift /*press and hold*/, Lock, Latch, Unknown};

    int mask;
    int ModMapIndex;
    KeySym sym;
    KeyCode key;
    ModType type;

  }  m_mods[8] =    {{ShiftMask,   ShiftMapIndex,   XK_Shift_L,   0, ModInfo::Shift},
                     {LockMask,    LockMapIndex,    XK_Caps_Lock, 0, ModInfo::Lock},
                     {ControlMask, ControlMapIndex, XK_Control_L, 0, ModInfo::Shift},
                     {Mod1Mask,    Mod1MapIndex,    NoSymbol,     0, ModInfo::Unknown},
                     {Mod2Mask,    Mod2MapIndex,    NoSymbol,     0, ModInfo::Unknown},
                     {Mod3Mask,    Mod3MapIndex,    NoSymbol,     0, ModInfo::Unknown},
                     {Mod4Mask,    Mod4MapIndex,    NoSymbol,     0, ModInfo::Unknown},
                     {Mod5Mask,    Mod5MapIndex,    NoSymbol,     0, ModInfo::Unknown},
                   };

  void InitModInfo();

protected:
  Display *m_display;
  bool     m_emulateMods = true;

public:
  AutotypeMethodBase(Display *display, bool emulateMods):
			m_display(display), m_emulateMods(emulateMods) {
	XSetErrorHandler(ErrorHandler);
	atGlobals.error_detected = false;
  InitModInfo();
  }

  // virtual, because derived classes will have clean-ups of their own
  virtual ~AutotypeMethodBase() {
	  XSetErrorHandler(NULL);
  }

  // This function does the most heavy lifting, using the bare minimum api-specific
  // support from derived classes implemented by overriding GenerateKeyEvent
  void operator()(unsigned int keycode, unsigned int state, Time event_time = CurrentTime);
  void operator()(XKeyEvent &ev);

  bool EmulatesMods() const { return m_emulateMods; }
  void EmulateMods(bool emulate) { m_emulateMods = emulate; }

protected:
  // This is not exposed as it will probably not do what you think.  Use SendKeyEvent above
  virtual void GenerateKeyEvent(XKeyEvent *ev) = 0;

  void PressModifiers(int masks) { SetModifiers(masks, true); }
  void ReleaseModifiers(int masks) { SetModifiers(masks, false); }
  void SetModifiers(int masks, bool set);
};

///////////////////////////////////////////////////////////////////////
// For each modifier, finds the keycode that generates that modifier
// and the type (shift, latch, etc) because that dictates what kind
// of key events are required to use that modifier.
//
// The KeySym is generated in the process and is needed for initialization,
// but is not used afterwards
void AutotypeMethodBase::InitModInfo()
{
  XModifierKeymap* modmap = XGetModifierMapping(m_display);
  if (modmap) {
    for(auto& m: m_mods) {
      if (m.key == 0) {
        if (m.sym != NoSymbol) {
          // If we know the KeySym associated with the modifier, things are easy
          m.key = XKeysymToKeycode(m_display, m.sym);
        }
        if (!m.key) {
          // Either the KeySym is unknown, or XKeysymToKeycode failed above
          // Look up the KeyCode in the XModifierKeymap
          const auto keys = modmap->modifiermap + m.ModMapIndex*modmap->max_keypermod;
          const auto keyptr = std::find_if(keys, keys + modmap->max_keypermod, [](KeyCode k){ return k != 0; });
          m.key = (keyptr == keys + modmap->max_keypermod? 0: *keyptr);
        }
      }

      if (m.sym == NoSymbol && m.key != 0) {
        // We know the KeyCode.  Now we need the KeySym to know what kind of key events
        // to generate with that KeyCode
        int keysyms_per_keycode = 0;
        KeySym* symlist = XGetKeyboardMapping(m_display, m.key, 1, &keysyms_per_keycode);
        if (symlist) {
          // The keysym for a modifier must not require a modifier itself, so the first
          // KeySym must not be empty
          assert(symlist[0] != NoSymbol);
          m.sym = symlist[0];
          XFree(symlist);
        }
      }
      if (m.type == ModInfo::Unknown && m.sym != NoSymbol) {
        // The modifier "type" tells us the sequence of up or down key events
        // to generate for that modifier
        switch (m.sym) {
          // These are known
          case XK_Meta_L: case XK_Meta_R: case XK_Alt_L: case XK_Alt_R:
          case XK_Super_L: case XK_Super_R: case XK_Hyper_L: case XK_Hyper_R:
            m.type = ModInfo::Shift;
            break;
          default:
          // Now we go by what it "sounds" like
            const char *symstr = XKeysymToString(m.sym);
            if (symstr) {
              const size_t s_len = strlen(symstr);
              auto ends_with = [symstr, s_len](const char* e, size_t e_len) {
                return s_len >= e_len && strncmp(symstr + s_len - e_len, e, e_len) == 0;
              };
              m.type =  ends_with("Latch", 5)? ModInfo::Latch
                       : ends_with("Lock", 4)? ModInfo::Lock
                       : ends_with("Shift", 5) || ends_with("Switch", 6)? ModInfo::Shift
                       : ModInfo::Unknown;
            }
            break;
        }
      }
    }
    XFreeModifiermap(modmap);
  }
  // Note that we may not have initialized every Modifier, but that may be ok
  // because we may not actually use that modifier.  We can only tell while'
  // autotyping
}

void AutotypeMethodBase::operator()(unsigned int keycode, unsigned int state,
							Time event_time /*= CurrentTime*/)
{
	XKeyEvent ev;
	ev.display = m_display;
	ev.keycode = keycode;
	ev.state = state;
	ev.time = event_time;
	operator()(ev);
}

void AutotypeMethodBase::operator()(XKeyEvent &ev)
{
  if (!ev.display)
    ev.display = m_display;
  else {
    assert( ev.display == m_display);
  }

  if (m_emulateMods && ev.state) {
    PressModifiers(ev.state);
  }

	ev.type = KeyPress;
	GenerateKeyEvent(&ev);
	ev.type = KeyRelease;
	GenerateKeyEvent(&ev);

  if (m_emulateMods && ev.state) {
    ReleaseModifiers(ev.state);
  }

  XFlush(m_display);
}

///////////////////////////////////////////////////////////
// Generate the modifier key events for all the modifier masks
// (one event per mask).  For each mask, generate the keystrokes
// (up, down, both, etc) for the keycodes corresponding to that
// modifier based on the "type" of the modifier
void AutotypeMethodBase::SetModifiers(int masks, bool set)
{
  // Shift modifiers require you to press the key for setting it, or release
  // while unsetting
  const int shift_events[] =  { (set? KeyPress: KeyRelease), 0};

  // Lock modifiers (like CAPSLOCK) require you to press and release the key
  // for both setting & unsetting
  const int lock_events[] = {KeyPress, KeyRelease, 0};

  // Latch modifiers require you to press and release the key for setting it
  // it unsets itself as soon as any other key is pressed
  const int latch_events[] = {KeyPress, KeyRelease, KeyPress, KeyRelease, 0};

  // Note that these are indexed by the ModInfo::ModType enum values
  const int* key_event_types[] = {shift_events, lock_events, latch_events, shift_events /*Unknown*/};

  for (auto mod: m_mods) {
    if (mod.mask & masks) {
      const auto *events = key_event_types[mod.type];
      for( const auto *e = events; *e; e++) {
        XKeyEvent ev{};
        ev.type = *e;
        ev.display = m_display;
        ev.keycode = mod.key;
        ev.time = CurrentTime;
        GenerateKeyEvent(&ev);
      }
    }
  }
}

//////////////////////////////////////////////////////////////////////////
// AutotypeMethodXTEST
//
// Emulates keystrokes using the XTEST extension.
//
class AutotypeMethodXTEST: public AutotypeMethodBase
{
public:
	AutotypeMethodXTEST(Display *display, bool emulateMods):
	AutotypeMethodBase(display, emulateMods){ XTestGrabControl(display, true); }
	~AutotypeMethodXTEST() { XTestGrabControl(m_display, false); }

protected:
  virtual void GenerateKeyEvent(XKeyEvent *ev) {
    XTestFakeKeyEvent(ev->display, ev->keycode, ev->type == KeyPress, CurrentTime);
  }

};

//////////////////////////////////////////////////////////////////////////
// AutotypeMethodSendKeys
//
// Emulates keystrokes using the XSendEvent method
//
class AutotypeMethodSendKeys: public AutotypeMethodBase
{
public:
	AutotypeMethodSendKeys(Display *display, bool emulateMods):
				AutotypeMethodBase(display, emulateMods){}
	~AutotypeMethodSendKeys() { XSync(m_display, False); }
protected:
  virtual void GenerateKeyEvent(XKeyEvent *ev) {
	XSendEvent(ev->display, ev->window, TRUE,
					ev->type == KeyPress? KeyPressMask: KeyReleaseMask, 
					reinterpret_cast<XEvent *>(ev));
  }
};


//////////////////////////////////////////////////////////////////////////
// Factory method for generating the correct type of autotype method, based
// on
//   1. User preference
//   2. XTEST extension availability
//
AutotypeMethodBase* GetAutotypeMethod(Display* disp,
									pws_os::AutotypeMethod method_preference)
{
  int major_opcode, first_event, first_error;
  AutotypeMethodBase *method_ptr = nullptr;
  if (method_preference != pws_os::ATMETHOD_XSENDKEYS &&
			XQueryExtension(disp, "XTEST", &major_opcode, &first_event, &first_error)) {
    method_ptr = new AutotypeMethodXTEST(disp, true); // true => emulate modifiers independently
  }
  else {
	method_ptr = new AutotypeMethodSendKeys(disp, false); // false => no emulation for modifier keys
  }
  return method_ptr;
}

// Helper method to check if an X extension is available
bool XExtensionAvailable(Display *disp, const char* ext)
{
  int major_opcode, first_event, first_error;
  return XQueryExtension(disp, ext, &major_opcode, &first_event, &first_error) == True;
}

// Calculates the masks (actually, shifts) used by the modifier with the KeySym "sym"
int FindModifierMask(Display* disp, KeySym sym)
{
  int modmask = 0;
  XModifierKeymap* modmap = XGetModifierMapping(disp);
  if (modmap) {
    const int last = 8*modmap->max_keypermod;
    //begin at 4th row, where Mod1 starts
    for (int i = Mod1MapIndex*modmap->max_keypermod; i < last && !modmask; i++) {
      //
      const KeyCode kc = modmap->modifiermap[i];
      if (!kc)
        continue;
      int keysyms_per_keycode = 0;
      // For each keycode attached to this modifier, get a list of all keysyms
      // attached with this keycode. If any of those keysyms is what we are looking
      // for, then this is the modifier to use
      KeySym* symlist = XGetKeyboardMapping(disp, kc, 1, &keysyms_per_keycode);
      if ( symlist) {
        for (int j = 0; j < keysyms_per_keycode; j++) {
          if (sym == symlist[j]) {
            modmask = (i / modmap->max_keypermod);
            break;
          }
        }
        XFree(symlist);
      }
    }
    XFreeModifiermap(modmap);
  }
  return modmask;
}

int CalcModifiersForKeysym(KeyCode code, KeySym sym, Display* disp)
{
  int modifiers = 0;
  int keysyms_per_keycode = 0;
  KeySym* symlist = XGetKeyboardMapping(disp, code, 1, &keysyms_per_keycode);
  if (symlist != NULL && keysyms_per_keycode > 0) {
    // Supported everywhere.  Note: order is important
    std::vector<int> masks = {0, ShiftMask};
    // These aren't necessarily supported in all systems.  Once again, order is important
    for ( const auto s: {XK_Mode_switch, XK_ISO_Level3_Shift}) {
      const int modshift = FindModifierMask(disp, s);
      // May repeat.  Only consider it if we haven't added it already
      if (modshift && std::find(masks.begin(), masks.end(), 1 << modshift) == masks.end()) {
        std::vector<int> extra_masks;
        // OR each element of mask with "1 << modshift" & insert the result in mask
        std::transform(masks.begin(), masks.end(), std::back_inserter(extra_masks),
                        std::bind(std::bit_or<int>(), std::placeholders::_1, 1 << modshift));
        masks.insert(masks.end(), extra_masks.begin(), extra_masks.end());
      }
    }
    // Get the index of the symbol we are searching for
    const auto max_keysym_index = std::min(masks.size(), static_cast<size_t>(keysyms_per_keycode));
    // return the modifiers at the same index
    const size_t match_index = std::find(symlist, symlist + max_keysym_index, sym) - symlist;
    if ( match_index != max_keysym_index)
        modifiers = masks[match_index];
    XFree(symlist);
  }
  return modifiers;
}

KeySym wchar2keysym(wchar_t wc)
{
  if (wc < 0x100) {
    if (wc >= 0x20)
      return wc;
    switch(wc) {
      case L'\t': return XK_Tab;
      case L'\r': return XK_Return;
      case L'\n': return XK_Linefeed;
      case '\010': return XK_BackSpace;
      case '\177': return XK_Delete;
      case '\033': return XK_Escape;
      default:
        return NoSymbol;
    }
  }
  if (wc > 0x10ffff || (wc > 0x7e && wc < 0xa0))
    return NoSymbol;
  KeySym sym = unicode2keysym(wc);
  if (sym != NoSymbol)
    return sym;
  //For everything else, there's Mastercard :)
  return wc | 0x01000000;
}

//converts a  single wchar_t to a byte string [i.e. char*]
class wchar2bytes
{
private:
  //MB_CUR_MAX is a function call, not a constant
  char* bytes;
public:
  wchar2bytes(wchar_t wc):  bytes(new char[MB_CUR_MAX*2 + sizeof(wchar_t)*2 + 2 + 1]) {
    mbstate_t ps;
    memset(&ps, 0, sizeof(ps));//initialize mbstate
    size_t n;
    if ((n = wcrtomb(bytes, wc, &ps)) == size_t(-1))
      snprintf(bytes, NumberOf(bytes), "U+%04X", int(wc));
    else
      bytes[n] = 0;
  }
  ~wchar2bytes() { delete [] bytes; }
  const char* str() const {return bytes;}
};

/*
 * DoSendString - actually sends a string to the X Window having input focus
 *
 * The main task of this function is to convert the ascii char values
 * into X KeyCodes.  But they need to be converted to X KeySyms first
 * and then to the keycodes.  The KeyCodes can have any random values
 * and are not contiguous like the ascii values are.
 *
 * Some escape sequences can be converted to the appropriate KeyCodes
 * by this function.  See the code below for details
 */
void CKeySendImpl::DoSendString(const StringX& str, unsigned delayMS, bool emulateMods)
{
  atGlobals.error_detected = false;
  atGlobals.errorString[0] = 0;

  AutotypeEvent event(m_display);

  // convert all the chars into keycodes and required shift states first
  // Abort if any of the characters cannot be converted
  typedef std::vector<KeyPressInfo> KeyPressInfoVector;
  KeyPressInfoVector keypresses;

  for (StringX::const_iterator srcIter = str.begin(); srcIter != str.end(); ++srcIter) {

    //throw away 'vertical tab' chars which are only used on Windows to send a shift+tab
    //as a workaround for some issues with IE
    if (*srcIter == _T('\v'))
      continue;

    //Try a regular conversion first
    KeySym sym = wchar2keysym(*srcIter);

    if (NoSymbol != sym) {
      KeyPressInfo keypress = {0, 0};
      if ((keypress.code = XKeysymToKeycode(event.display, sym)) != 0) {
        //non-zero return value implies sym -> code was successful
        keypress.state |= CalcModifiersForKeysym(keypress.code, sym, event.display);
        keypresses.push_back(keypress);
      }
      else {
        const char* symStr = XKeysymToString(sym);
        snprintf(atGlobals.errorString, NumberOf(atGlobals.errorString),
              "Could not get keycode for key char(%s) - sym(%#X) - str(%s). Aborting autotype.\n\nIf \'xmodmap -pk\' does not list this KeySym, you probably need to install an appropriate keyboard layout.",
                          wchar2bytes(*srcIter).str(), static_cast<int>(sym), symStr ? symStr : "NULL");
        atGlobals.error_detected = True;
        return;
      }
    }
    else {
      snprintf(atGlobals.errorString, NumberOf(atGlobals.errorString),
              "Cannot convert '%s' [U+%04X] to keysym. Aborting autotype", wchar2bytes(*srcIter).str(), int(*srcIter));
      atGlobals.error_detected = True;
      return;
    }
  }

  m_method->EmulateMods(emulateMods);

  for (KeyPressInfoVector::const_iterator itr = keypresses.begin(); itr != keypresses.end()
                              && !atGlobals.error_detected; ++itr) {
    event.keycode = itr->code;
    event.state = itr->state;
    event.time = CurrentTime;

    (*m_method)(event);
    pws_os::sleep_ms(delayMS);
  }
}


void CKeySendImpl::SendString(const StringX& str, unsigned delayMS)
{
  atGlobals.error_detected = false;
  atGlobals.errorString[0] = 0;

  DoSendString(str, delayMS, m_emulateModsSeparately);

  if (atGlobals.error_detected)
    throw autotype_exception();
}

void CKeySendImpl::SelectAll(unsigned delayMS)
{
  AutotypeEvent event(m_display);
  event.keycode = XKeysymToKeycode(event.display, XK_a);
  event.state = ControlMask;
  event.time = CurrentTime;
  (*m_method)(event);
  pws_os::sleep_ms(delayMS);
}

CKeySendImpl::CKeySendImpl(pws_os::AutotypeMethod method): m_display(XOpenDisplay(NULL))
{
  if (m_display) {
    m_method = GetAutotypeMethod(m_display, method);
  }
  else {
    if (!atGlobals.error_detected)
      atGlobals.error_detected = true;
    if (!atGlobals.errorString[0])
      strncpy(atGlobals.errorString, "Could not open X display for autotyping", NumberOf(atGlobals.errorString));

    throw autotype_exception();
  }
}

CKeySendImpl::~CKeySendImpl()
{
  delete m_method;
  XCloseDisplay(m_display);
}

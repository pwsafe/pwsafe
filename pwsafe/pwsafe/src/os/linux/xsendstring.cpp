/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/*
 * xsendkeys - send a bunch of keystrokes to the app having current input focus
 *
 * Calls X library functions defined in Xt and Xtst
 *
 * +. Initialize all the params of XKeyEvent
 * +. Test for \133 etc
 * +  More escape sequences from http://msdn.microsoft.com/en-us/library/h21280bw%28VS.80%29.aspx
 * +  XGetErrorText and sprintf overflow
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <time.h>


#include <X11/Intrinsic.h> // in libxt-dev
#include <X11/keysym.h>
#include <X11/extensions/XTest.h> // in libxtst-dev

#include "./xsendstring.h"
#include "../sleep.h"
#include "../../corelib/PwsPlatform.h" // for NumberOf()

namespace { // anonymous namespace for hiding
  //           local variables and functions

struct AutotypeGlobals
{
  Boolean      error_detected;
  char         errorString[1024];
  KeyCode     lshiftCode;
  pws_os::AutotypeMethod  method;
  Boolean      LiteralKeysymsInitialized;
} atGlobals  = { False, {0}, 0, pws_os::ATMETHOD_AUTO, False };

/*
 * ErrorHandler will be called when X detects an error. This function
 * just sets a global flag and saves the error message text
 */
int ErrorHandler(Display *my_dpy, XErrorEvent *event)
{
  char xmsg[512] = {0};

  atGlobals.error_detected = TRUE;
  XGetErrorText(my_dpy, event->error_code, xmsg, NumberOf(xmsg) - 1);
  snprintf(atGlobals.errorString, NumberOf(atGlobals.errorString)-1, "X error (%d): %s\n", event->request_code, xmsg);
  return 0;
}

int ShiftRequired(char* keystring)
{
  if (isupper(keystring[0]))
    return 1;

  switch(keystring[0]) {
    case '~': case '!': case '@': case '#': case '$': case '%': 
    case '^': case '&': case '*': case '(': case ')': case '_': 
    case '+': case '{': case '}': case '|': case ':': case '"': 
    case '<': case '>': case '?':
      return 1;
    default:
      return 0;
  }
}

/*
 * - characters which need to be manually converted to KeySyms
 */

static struct {
    char ch;
    const char* keystr;
  KeySym sym;
} LiteralKeysyms[] =
{
    { ' ',    "space",        NoSymbol },
    { '\t',   "Tab",          NoSymbol },
    { '\n',   "Linefeed",     NoSymbol },
    { '\r',   "Return",       NoSymbol },
    { '\e',   "Escape",       NoSymbol },
    { '\010', "BackSpace",    NoSymbol },  /* \b doesn't work */
    { '\177', "Delete",       NoSymbol },
    { '\27',  "Escape",       NoSymbol },  /* \e doesn't work */
    { '!',    "exclam",       NoSymbol },
    { '#',    "numbersign",   NoSymbol },
    { '%',    "percent",      NoSymbol },
    { '$',    "dollar",       NoSymbol },
    { '&',    "ampersand",    NoSymbol },
    { '"',    "quotedbl",     NoSymbol },
    { '\'',   "apostrophe",   NoSymbol },
    { '(',    "parenleft",    NoSymbol },
    { ')',    "parenright",   NoSymbol },
    { '*',    "asterisk",     NoSymbol },
    { '=',    "equal",        NoSymbol },
    { '+',    "plus",         NoSymbol },
    { ',',    "comma",        NoSymbol },
    { '-',    "minus",        NoSymbol },
    { '.',    "period",       NoSymbol },
    { '/',    "slash",        NoSymbol },
    { ':',    "colon",        NoSymbol },
    { ';',    "semicolon",    NoSymbol },
    { '<',    "less",            44    }, /* we get '>' instead of '<' unless we hardcode this ???*/
    { '>',    "greater",      NoSymbol },
    { '?',    "question",     NoSymbol },
    { '@',    "at",           NoSymbol },
    { '[',    "bracketleft",  NoSymbol },
    { ']',    "bracketright", NoSymbol },
    { '\\',   "backslash",    NoSymbol },
    { '^',    "asciicircum",  NoSymbol },
    { '_',    "underscore",   NoSymbol },
    { '`',    "grave",        NoSymbol },
    { '{',    "braceleft",    NoSymbol },
    { '|',    "bar",          NoSymbol },
    { '}',    "braceright",   NoSymbol },
    { '~',    "asciitilde",   NoSymbol },
};

void InitLiteralKeysyms(void)
{
  size_t idx;
  for (idx = 0; idx < NumberOf(LiteralKeysyms); ++idx)
    if (LiteralKeysyms[idx].sym == NoSymbol)
      LiteralKeysyms[idx].sym = XStringToKeysym(LiteralKeysyms[idx].keystr);

  atGlobals.lshiftCode = XKeysymToKeycode(XOpenDisplay(NULL), XK_Shift_L);
}

KeySym GetLiteralKeysym(char* keystring)
{
  size_t idx;
  for (idx = 0; idx < NumberOf(LiteralKeysyms); ++idx)
    if (keystring[0] ==  LiteralKeysyms[idx].ch )
      return LiteralKeysyms[idx].sym;

  return NoSymbol;
}

void XTest_SendEvent(XKeyEvent *event)
{
  XTestFakeKeyEvent(event->display, event->keycode, event->type == KeyPress, 0);
}

void XSendKeys_SendEvent(XKeyEvent *event)
{
    XSendEvent(event->display, event->window, TRUE, KeyPressMask, (XEvent *)event);
}

void XSendKeys_SendKeyEvent(XKeyEvent* event)
{
  event->type = KeyPress;
  XSendKeys_SendEvent(event);

  event->type = KeyRelease;
  XSendKeys_SendEvent(event);
  
  XFlush(event->display);
}

void XTest_SendKeyEvent(XKeyEvent* event)
{
  XKeyEvent shiftEvent;

  /* must simulate the shift-press for CAPS and shifted keypresses manually */
  if (event->state & ShiftMask) {
    memcpy(&shiftEvent, event, sizeof(shiftEvent));

    shiftEvent.keycode = atGlobals.lshiftCode;
    shiftEvent.type = KeyPress;

    XTest_SendEvent(&shiftEvent);
  }

  event->type = KeyPress;
  XTest_SendEvent(event);

  event->type = KeyRelease;
  XTest_SendEvent(event);
  
  if (event->state & ShiftMask) {
    shiftEvent.type = KeyRelease;
    XTest_SendEvent(&shiftEvent);
  }

  XFlush(event->display);
}

Bool UseXTest(void)
{
  int major_opcode, first_event, first_error;
  static Bool useXTest;
  static int checked = 0;
    
  if (!checked) {
    useXTest = XQueryExtension(XOpenDisplay(0), "XTEST", &major_opcode, &first_event, &first_error);
    checked = 1;
  }
  return useXTest;
}

void InitKeyEvent(XKeyEvent* event)
{
  int    revert_to;
  event->display = XOpenDisplay(NULL);
  XGetInputFocus(event->display, &event->window, &revert_to);

  event->subwindow = None;
  event->x = event->y = event->x_root = event->y_root = 1;
  event->same_screen = TRUE;
}

}; // anonymous namespace

/* 
 * SendString - sends a string to the X Window having input focus
 *
 * The main task of this function is to convert the ascii char values 
 * into X KeyCodes.  But they need to be converted to X KeySyms first
 * and then to the keycodes.  The KeyCodes can have any random values
 * and are not contiguous like the ascii values are.
 *
 * Some escape sequences can be converted to the appropriate KeyCodes 
 * by this function.  See the code below for details
 */

void pws_os::SendString(const char* str, AutotypeMethod method, unsigned delayMS)
{
  typedef struct _KeyPress {
    KeyCode code;
    unsigned int state;
  } KeyPressInfo;

  XKeyEvent event;
  KeyPressInfo* keypresses;
  size_t nsrc, ndest;
    char keystring[2];

  if (!str || !*str) return;

  InitKeyEvent(&event);

  /* no of bytes in strlen, not the same as number of characters to be converted
   * which could be different due to escape sequences like \r, \133, etc */
  nsrc = strlen(str);

  /* convert all the chars into keycodes and required shift states first
   * Abort if any of the characters cannot be converted */
  keypresses = (KeyPressInfo*)malloc(sizeof(KeyPressInfo)*nsrc);
  if (!keypresses)
    return;

  memset(keypresses, 0, sizeof(KeyPressInfo)*nsrc);

  ndest = 0;
  while (*str) {
    if (*str == '\\') {
      ++str;
      switch(*str) {
        case 't':
          keystring[0] = '\t';
          break;

        case 'n':
          keystring[0] = '\n';
          break;

        case 'r':
          keystring[0] = '\r';
          break;

        case 'b':
          keystring[0] = '\b';
          break;

        case '\\':
          keystring[0] = '\\';
          break;

        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
        {
          int number, nchars;
                    sscanf(str, "%d%n", &number, &nchars); /* nchars is set by sscanf to the number of source chars processed */
          keystring[0] = number;
          str += nchars - 1; /* incremented below for the entire while loop*/
          break;
        }

        default:
          keystring[0] = '\\';
          str--;
          break; 
      }
    }
    else {
      keystring[0] = *str;
    }

    if (!atGlobals.LiteralKeysymsInitialized) {
      InitLiteralKeysyms();
      atGlobals.LiteralKeysymsInitialized = True;
    }

    keystring[1] = '\0';

    /* Try a regular conversion first */
    KeySym sym = XStringToKeysym(keystring);

    /* Failing which, use our hard-coded special names for certain keys */
    if (NoSymbol != sym || (sym = GetLiteralKeysym(keystring)) != NoSymbol) {
      keypresses[ndest].code = XKeysymToKeycode(event.display, sym);
      if (keypresses[ndest].code) { /* non-zero return value implies sym -> code was successful */
        if (ShiftRequired(keystring))
          keypresses[ndest].state |= ShiftMask;
        ndest++;
      }
      else {
        sprintf(atGlobals.errorString, "Could not get keycode for keysym(%s). Aborting...\n", XKeysymToString(sym));
        break;
      }
    }
    else {
      sprintf(atGlobals.errorString, "Cannot convert '%d' to keysym. Aborting ...\n", (int)keystring[0]);  
      break;
    }
    ++str;
  }

  if (*str) {
    /* some of the input chars were unprocessed */
    sprintf(atGlobals.errorString, "char at approximate index(%u), ascii(%d), symbol(%c) couldn't be converted to keycode\n", ndest, (int)*str, *str);
  }
  else {
    size_t n;

    XSetErrorHandler(ErrorHandler);
    atGlobals.error_detected = False;

    if (method == ATMETHOD_XTEST || (method == ATMETHOD_AUTO && UseXTest())) {

      XTestGrabControl(event.display, True);

      for (n = 0; n < ndest && !atGlobals.error_detected; ++n) {
        event.keycode = keypresses[n].code;
        event.state = keypresses[n].state;
        event.time = CurrentTime;

        XTest_SendKeyEvent(&event);
        pws_os::sleep_ms(delayMS);
      }

      XTestGrabControl(event.display, False);
    }
    else {
      for (n = 0; n < ndest && !atGlobals.error_detected; ++n) {
        event.keycode = keypresses[n].code;
        event.state = keypresses[n].state;
        event.time = CurrentTime;

        XSendKeys_SendKeyEvent(&event);
        pws_os::sleep_ms(delayMS);
      }
      XSync(event.display, False);
    }

    XSetErrorHandler(NULL);
  }
  free(keypresses);
}

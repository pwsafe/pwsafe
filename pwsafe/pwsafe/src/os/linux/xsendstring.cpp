/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
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
 * +. Initialize all the params of XKeyEvent
 * +  More escape sequences from http://msdn.microsoft.com/en-us/library/h21280bw%28VS.80%29.aspx
 * +  XGetErrorText and sprintf overflow
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


#include <X11/Intrinsic.h> // in libxt-dev
#include <X11/keysym.h>
#include <X11/extensions/XTest.h> // in libxtst-dev

#include "./xsendstring.h"
#include "../sleep.h"
#include "../../core/PwsPlatform.h" // for NumberOf()
#include "../../core/StringX.h"

namespace { // anonymous namespace for hiding
  //           local variables and functions
typedef struct _KeyPress {
  KeyCode code;
  unsigned int state;
} KeyPressInfo;

struct AutotypeGlobals
{
	Boolean			error_detected;
	char			errorString[1024];
	KeyCode 		lshiftCode;
  pws_os::AutotypeMethod	method;
	Boolean			LiteralKeysymsInitialized;
} atGlobals	= { False, {0}, 0, pws_os::ATMETHOD_AUTO, False };

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
    { ' ', 		"space", 		NoSymbol },
    { '\t', 	"Tab",  		NoSymbol },
    { '\n', 	"Linefeed", 	NoSymbol },
    { '\r', 	"Return", 		NoSymbol },
    { '\010', 	"BackSpace", 	NoSymbol },  /* \b doesn't work */
    { '\177', 	"Delete", 		NoSymbol },
    { '\033', 	"Escape", 		NoSymbol },  /* \e doesn't work and \e is non-iso escape sequence*/
    { '!', 		"exclam", 		NoSymbol },
    { '#', 		"numbersign", 	NoSymbol },
    { '%', 		"percent", 		NoSymbol },
    { '$', 		"dollar", 		NoSymbol },
    { '&', 		"ampersand", 	NoSymbol },
    { '"', 		"quotedbl", 	NoSymbol },
    { '\'', 	"apostrophe", 	NoSymbol },
    { '(', 		"parenleft", 	NoSymbol },
    { ')', 		"parenright", 	NoSymbol },
    { '*', 		"asterisk", 	NoSymbol },
    { '=', 		"equal", 		NoSymbol },
    { '+', 		"plus", 		NoSymbol },
    { ',', 		"comma", 		NoSymbol },
    { '-', 		"minus", 		NoSymbol },
    { '.', 		"period", 		NoSymbol },
    { '/', 		"slash", 		NoSymbol },
    { ':', 		"colon", 		NoSymbol },
    { ';', 		"semicolon", 	NoSymbol },
    { '<', 		"less", 		44 	 }, /* I don't understand why we get '>' instead of '<' unless we hardcode this */
    { '>', 		"greater", 		NoSymbol },
    { '?', 		"question", 	NoSymbol },
    { '@', 		"at", 			NoSymbol },
    { '[', 		"bracketleft", 	NoSymbol },
    { ']', 		"bracketright", NoSymbol },
    { '\\', 	"backslash", 	NoSymbol },
    { '^', 		"asciicircum", 	NoSymbol },
    { '_', 		"underscore", 	NoSymbol },
    { '`', 		"grave", 		NoSymbol },
    { '{', 		"braceleft", 	NoSymbol },
    { '|', 		"bar", 			NoSymbol },
    { '}', 		"braceright", 	NoSymbol },
    { '~', 		"asciitilde", 	NoSymbol },
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
    XSendEvent(event->display, event->window, TRUE, KeyPressMask, reinterpret_cast<XEvent *>(event));
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
	int	  revert_to;
	event->display = XOpenDisplay(NULL);
	XGetInputFocus(event->display, &event->window, &revert_to);

	event->subwindow = None;
	event->x = event->y = event->x_root = event->y_root = 1;
	event->same_screen = TRUE;
}

} // anonymous namespace

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

void pws_os::SendString(const StringX& str, AutotypeMethod method, unsigned delayMS)
{

  if (!atGlobals.LiteralKeysymsInitialized) {
    InitLiteralKeysyms();
    atGlobals.LiteralKeysymsInitialized = True;
  }

  XKeyEvent event;
  InitKeyEvent(&event);

  // convert all the chars into keycodes and required shift states first
  // Abort if any of the characters cannot be converted
  typedef std::vector<KeyPressInfo> KeyPressInfoVector;
  KeyPressInfoVector keypresses;
  
  for (StringX::const_iterator srcIter = str.begin(); srcIter != str.end(); ++srcIter) {

    //This array holds the multibyte representation of the current (wide) char, plus NULL
    char keystring[MB_LEN_MAX + 1] = {0};

    mbstate_t state = {0};

    size_t ret = wcrtomb(keystring, *srcIter, &state);
    if (ret < 0) {
      snprintf(atGlobals.errorString, NumberOf(atGlobals.errorString), 
              "char at index(%u), value(%d) couldn't be converted to keycode. %s\n",
                  (unsigned int)std::distance(str.begin(), srcIter), (int)*srcIter, strerror(errno));
      atGlobals.error_detected = True;
      return;
    }

    ASSERT(ret < (NumberOf(keystring)-1));
    
    //Try a regular conversion first
    KeySym sym = XStringToKeysym(keystring);

    //Failing which, use our hard-coded special names for certain keys
    if (NoSymbol != sym || (sym = GetLiteralKeysym(keystring)) != NoSymbol) {
      KeyPressInfo keypress = {0};
      if ((keypress.code = XKeysymToKeycode(event.display, sym)) != 0) {
        //non-zero return value implies sym -> code was successful
        if (ShiftRequired(keystring)) {
          keypress.state |= ShiftMask;
        }
        keypresses.push_back(keypress);
      }
      else {
        const char* symStr = XKeysymToString(sym);
        snprintf(atGlobals.errorString, NumberOf(atGlobals.errorString), 
              "Could not get keycode for key char(%s) - sym(%d) - str(%s). Aborting autotype\n", 
                          keystring, static_cast<int>(sym), symStr ? symStr : "NULL");
        atGlobals.error_detected = True;
        return;
      }
    }
    else {
      snprintf(atGlobals.errorString, NumberOf(atGlobals.errorString), 
              "Cannot convert '%s' to keysym. Aborting autotype\n", keystring);
      atGlobals.error_detected = True;
      return;
    }
  }

  XSetErrorHandler(ErrorHandler);
  atGlobals.error_detected = False;

  bool useXTEST = (UseXTest() && method != ATMETHOD_XSENDKEYS);
  void (*KeySendFunction)(XKeyEvent*);

  if ( useXTEST) {
    KeySendFunction = XTest_SendKeyEvent;
    XTestGrabControl(event.display, True);
  }
  else {
    KeySendFunction = XSendKeys_SendKeyEvent;
  }
  
  for (KeyPressInfoVector::const_iterator itr = keypresses.begin(); itr != keypresses.end()
                              && !atGlobals.error_detected; ++itr) {
    event.keycode = itr->code;
    event.state = itr->state;
    event.time = CurrentTime;

    KeySendFunction(&event);
    pws_os::sleep_ms(delayMS);
  }

  if (useXTEST) {
    XTestGrabControl(event.display, False);
  }
  else {
    XSync(event.display, False);
  }

  XSetErrorHandler(NULL);
}

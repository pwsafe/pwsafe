/*
* Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/*
 * macsendstring - sends keyboard events to the app in focus
 *
 */
#include "./macsendstring.h"

#include <ApplicationServices/ApplicationServices.h> // OSX-only header
#include <Carbon/Carbon.h>
#include "../sleep.h"
#include "../../core/PwsPlatform.h"
#include "../debug.h"

void SendString(CFStringRef str, unsigned delayMS)
{
  //virtual keycodes copied from 10.6 SDK.  Could not find them in 10.4 SDK
  enum { VK_RETURN = 0x24 /*KVK_Return*/, VK_TAB = 0x30 /*kVK_Tab*/, VK_SPACE = 0x31/*kVK_Space*/};
  
  //A list of chars for which we must specify the virtual keycode
  static CFStringRef specialChars = CFSTR("\n\t ");
  
  CFStringRef verticalTab = CFSTR("\v");

  //each keycode must correspond to the correct char in 'specialChars'
  CGKeyCode specialKeyCodes[] = {VK_RETURN, VK_TAB, VK_SPACE };
  
  for (unsigned i = 0, len = CFStringGetLength(str); i < len; ++i) {
    //The next char to send
    UniChar c = CFStringGetCharacterAtIndex(str, i);
    
    //throw away 'vertical tab' chars which are only used on Windows to send a shift+tab
    //as a workaround for some issues with IE 
    if (CFStringGetCharacterAtIndex(verticalTab, 0) == c)
      continue;
    
    //see if we need to specify the virtual ekycode for this char
    CGKeyCode vKey = 0; //0 = kVK_ANSI_A, but I don't know of a more appropriate default value
    for (unsigned j = 0, n = CFStringGetLength(specialChars); j < n; ++j) {
      if ( CFStringGetCharacterAtIndex(specialChars, j) == c) {
        vKey = specialKeyCodes[j];
        break;
      }
    }
    
    CGEventRef keyDown = CGEventCreateKeyboardEvent(NULL, vKey, true);
    CGEventRef keyUp   = CGEventCreateKeyboardEvent(NULL, vKey, false);
    
    if (keyDown && keyUp) {
      //may be we should not do this if we found the virtual keycode?
      CGEventKeyboardSetUnicodeString(keyDown, 1, &c);
      CGEventKeyboardSetUnicodeString(keyUp, 1, &c);
      
      CGEventPost(kCGSessionEventTap, keyDown);
      CGEventPost(kCGSessionEventTap, keyUp);
      
      pws_os::sleep_ms(delayMS);
      
      CFRelease(keyDown);
      CFRelease(keyUp);
    }
    else {
      if (keyDown) CFRelease(keyDown);
      if (keyUp) CFRelease(keyUp);
      pws_os::IssueError(_T("Out of memory trying to allocate CGEventRef"));
      return;
    }
  }
}

void pws_os::SendString(const char* str, unsigned delayMS)
{
	//GetApplicationTextEncoding call here certainly seems wrong.  We should store the keyboard layout
	//and string encoding in password db.  But this works for now.
	CFStringRef cfstr = CFStringCreateWithCString(kCFAllocatorDefault, str, GetApplicationTextEncoding());
	SendString(cfstr, delayMS);
	CFRelease(cfstr);
}

bool pws_os::MacSimulateApplicationSwitch(unsigned delayMS)
{
  enum { VK_CMD = 55, VK_TAB = 48 };

  struct {
    CGKeyCode virtualKey;
    bool down;
    bool mask;
  } KeySequence[]={ {VK_CMD, true, true},
                    {VK_TAB, true, true},
                    {VK_TAB, false, true},
                    {VK_CMD, false, false}
                  };

  for (size_t idx = 0; idx < NumberOf(KeySequence); ++idx) {
    CGEventRef keystroke = CGEventCreateKeyboardEvent(NULL, KeySequence[idx].virtualKey, KeySequence[idx].down);
    if (keystroke) {
      if (KeySequence[idx].mask) {
        CGEventSetFlags(keystroke, kCGEventFlagMaskCommand);
      }
      CGEventPost(kCGSessionEventTap, keystroke);
      CFRelease(keystroke);
      pws_os::sleep_ms(delayMS);
    }
    else {
      return false;
    }
  }
  
  return true;
}

#if 0
int main (int argc, const char * argv[]) 
{
    for (int i = 1; i < argc; ++i)
	{
		printf("Sending \"%s\", switch to another application in 5 seconds...\n", argv[i]);
		sleep(5);
		SendString(argv[i], 5000);
	}
    return 0;
}

#endif

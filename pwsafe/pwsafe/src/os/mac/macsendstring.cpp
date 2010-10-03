/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
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

void SendString(CFStringRef str, unsigned delayMS)
{
  CGEventRef keyDown = CGEventCreateKeyboardEvent(NULL, 0, true);
  CGEventRef keyUp   = CGEventCreateKeyboardEvent(NULL, 0, false);

  if (keyDown && keyUp) {
    for (unsigned i = 0, len = CFStringGetLength(str); i < len; ++i) {
      CGEventSetFlags(keyDown, 0);
      CGEventSetFlags(keyUp, 0);
      
      UniChar c = CFStringGetCharacterAtIndex(str, i);
      
      CGEventKeyboardSetUnicodeString(keyDown, 1, &c);
      CGEventKeyboardSetUnicodeString(keyUp, 1, &c);
      
      CGEventPost(kCGSessionEventTap, keyDown);
      CGEventPost(kCGSessionEventTap, keyUp);

      pws_os::sleep_ms(delayMS);
    }
  }
  
  if (keyDown) CFRelease(keyDown);
  if (keyUp) CFRelease(keyUp);
}

void pws_os::SendString(const char* str, unsigned delayMS)
{
  //GetApplicationTextEncoding call here certainly seems wrong.  We should stoe the keyboard layout
  //and string encoding in password db.  But this works for now.
  CFStringRef cfstr = CFStringCreateWithCString(kCFAllocatorDefault, str, GetApplicationTextEncoding());
  SendString(cfstr, delayMS);
  CFRelease(cfstr);
}

#if 0
int main (int argc, const char * argv[]) 
{
  for (int i = 1; i < argc; ++i) {
    printf("Sending \"%s\", switch to another application in 5 seconds...\n", argv[i]);
    sleep(5);
    SendString(argv[i], 5000);
  }
  return 0;
}

#endif

/*
 * Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file SystemTrayMenuId.h
 *
 */

/*
 * This file contains the logic behind generating the command-ids
 * used by the dynamically generated system tray menu.
 *
 * Each item in the recently-used entry generates one sub-menu.  Each
 * of these sub-menus can have a menu entry to 
 *
 *    1. Copy password to clipboard
 *    2. Copy Username to clipboard (if present)
 *    3. Copy Notes to clipboard (if present)
 *    4. Perform Autotype
 *    5. Copy URL to clipboard (if present)
 *    6. Copy email to clipboard (if present)
 *    7. Browse to URL (if present)
 *    8. Browse to URL and Autotype (if present)
 *    9. Send email (if email is present)
 *   10. Run command  (if there is a command)
 *   11. Delete from Recent Entry list (same for all items)
 *
 * I'm not sure how many recently-used items could there be, but
 * there has to be a limit (probably 100). The default seems 25
 * 
 * Precautions to be taken are:
 *
 *   1. The ids must not conflict with wxWidget's pre-defined ids,
 *      that is, they must fall outside wxID_LOWEST(4999) - wxID_HIGHEST (5999)
 *      see wx/defs.h
 *      
 *   2. The ids must not conflict with the dialogblocks generated ids
 *      defined in passwordsafeframe.h, and some of the hand-defined values
 *      which follow the dialogblocks ids in the same file.  These values
 *      start from 10000 and don't have a defined upper bound as of yet
 *
 * The other problem to be solved is to somehow relate these command-ids to
 * the recently-used items that the submenu refers to, since there seems to 
 * be no way to associate any kind of callback data with menus and menu 
 * events (like MENUINFO and WM_MENUCOMMAND in Win32 api. The only thing we 
 * have when a menuitem is clicked is the command-id.
 *
 * So a part of the command-id has to represent the recently-used item and
 * the other part has to represent the operation to perform (copy password, 
 * autotype, etc).
 * 
 * The solution is to use the lowest 16 bits of the command-id as follows
 *
 *                 0 0 0 0  0 0 0 0    0 0 0 0  0 0 0 0    
 *                          |     |    |              |
 *                            opn       RUE array index
 *
 * The 4 bits for operation values make it possible to represent 16 operations,
 * meaning, each sub-menu can have a max of 16 items.  But we must start with
 * 1 and not zero, so that at least one of the bits is always on, else the
 * command-id for index 0 would be 0 for the first operation
 *
 * The 8 bits for RUE array index allow us to have a max of 256 recently-used
 * entries in the array, which should be good enough
 *
 * So the lowest value of our id is 000100000000 (256) and the highest is
 * 00001111 11111111 (4095), well clear of all the ranges defined in precautions
 * listed above
 */

#include "../../os/typedefs.h"

enum { MIN_RUE_COMMAND_ID = 256, MAX_RUE_COMMAND_ID = 4095 };

inline bool IsRUECommand(int id) { 
  return id >= MIN_RUE_COMMAND_ID && id <= MAX_RUE_COMMAND_ID;
}

typedef enum { RUE_COPYPASSWORD = 1, /* must start with 1, see above comments */
               RUE_COPYUSERNAME,
               RUE_COPYNOTES,
               RUE_AUTOTYPE,
               RUE_COPYURL,
               RUE_COPYEMAIL,
               RUE_BROWSE,
               RUE_BROWSEAUTOTYPE,
               RUE_SENDEMAIL,
               RUE_RUNCOMMAND,
               RUE_DELETERUEENTRY} RUEOperation;

inline uint16 MakeCommandId(RUEOperation opn, size_t index) {
  wxASSERT( index < 256 );
  const uint16 id = MAKEWORD(index, opn);
  wxASSERT(IsRUECommand(id));
  return id;
}

inline BYTE GetRUEIndex(int id) {
  wxASSERT(IsRUECommand(id));
  return LOBYTE(id);
}

inline RUEOperation GetRUEOperation(int id) {
  wxASSERT(IsRUECommand(id));
  const BYTE opn = (HIBYTE(LOWORD(id)) & 0xF);
  const RUEOperation operations[] = { RUE_COPYPASSWORD,
                                      RUE_COPYUSERNAME,
                                      RUE_COPYNOTES,
                                      RUE_AUTOTYPE,
                                      RUE_COPYURL,
                                      RUE_COPYEMAIL,
                                      RUE_BROWSE,
                                      RUE_BROWSEAUTOTYPE,
                                      RUE_SENDEMAIL,
                                      RUE_RUNCOMMAND,
                                      RUE_DELETERUEENTRY} ;
  wxASSERT(opn > 0 && opn <= NumberOf(operations));
  return operations[opn-1];
}

inline int GetFrameCommandId(RUEOperation opn) {
  const int frameCommands[] =  { ID_COPYPASSWORD,
                                 ID_COPYUSERNAME,
                                 ID_COPYNOTESFLD,
                                 ID_AUTOTYPE,
                                 ID_COPYURL,
                                 ID_COPYEMAIL,
                                 ID_BROWSEURL,
                                 ID_BROWSEURLPLUS,
                                 ID_SENDEMAIL,
                                 ID_RUNCOMMAND };
                           
  wxASSERT(opn > 0 && size_t(opn) <= NumberOf(frameCommands));
  return frameCommands[opn-1];
}

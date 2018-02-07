/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#pragma once

// Windowsdefs.h
//-----------------------------------------------------------------------------

#define UNIQUE_PWS_GUID L"PasswordSafe-{3FE0D665-1AE6-49b2-8359-326407D56470}"

// Custom message event used for system tray handling
#define PWS_MSG_ICON_NOTIFY             (WM_APP + 10)

// To catch post Header drag
#define PWS_MSG_HDR_DRAG_COMPLETE       (WM_APP + 20)
#define PWS_MSG_CCTOHDR_DD_COMPLETE     (WM_APP + 21)
#define PWS_MSG_HDRTOCC_DD_COMPLETE     (WM_APP + 22)

// Process Compare Result Dialog click/menu functions
#define PWS_MSG_COMPARE_RESULT_FUNCTION (WM_APP + 30)
#define PWS_MSG_COMPARE_RESULT_ALLFNCTN (WM_APP + 31)

// Equivalent one from Expired Password dialog
#define PWS_MSG_EXPIRED_PASSWORD_EDIT   (WM_APP + 32)

// Copy subset of password to clipboard
#define PWS_MSG_DISPLAYPASSWORDSUBSET   (WM_APP + 33)

// Edit/Add extra context menu messages
#define PWS_MSG_CALL_EXTERNAL_EDITOR    (WM_APP + 40)
#define PWS_MSG_EXTERNAL_EDITOR_ENDED   (WM_APP + 41)
#define PWS_MSG_EDIT_WORDWRAP           (WM_APP + 42)
#define PWS_MSG_EDIT_SHOWNOTES          (WM_APP + 43)
#define PWS_MSG_EDIT_APPLY              (WM_APP + 44)
#define PWS_MSG_CALL_NOTESZOOMIN        (WM_APP + 45)
#define PWS_MSG_CALL_NOTESZOOMOUT       (WM_APP + 46)

// Simulate Ctrl+F from Find Toolbar "enter"
#define PWS_MSG_TOOLBAR_FIND            (WM_APP + 50)

// Perform Drag Autotype
#define PWS_MSG_DRAGAUTOTYPE            (WM_APP + 55)

// Update current filters whilst SetFilters dialog is open
#define PWS_MSG_EXECUTE_FILTERS         (WM_APP + 60)

// Notification from tree control that a file was dropped on it
#define PWS_MSG_DROPPED_FILE            (WM_APP + 65)

// Message to get Virtual Keyboard buffer.
#define PWS_MSG_INSERTBUFFER            (WM_APP + 70)
#define PWS_MSG_RESETTIMER              (WM_APP + 71)

/*
Timer related values (note - all documented her but some defined only where needed.
*/

/* Timer event number used to by PupText.  Here for doc. only
#define TIMER_PUPTEXT             0x03 */
// Timer event number used to check if the workstation is locked
#define TIMER_LOCKONWTSLOCK       0x04
// Timer event number used to support lock on user-defined idle timeout
#define TIMER_LOCKDBONIDLETIMEOUT 0x05
// Definition of a minute in milliseconds
#define MINUTE 60000
// How ofter should idle timeout timer check:
#define IDLE_CHECK_RATE 2
#define IDLE_CHECK_INTERVAL (MINUTE/IDLE_CHECK_RATE)
// Timer event number used to support Find in PWListCtrl when icons visible
#define TIMER_FIND                0x06
// Timer event number used to support display of notes in List & Tree controls
#define TIMER_ND_HOVER            0x07
#define TIMER_ND_SHOWING          0x08
// Timer event number used to support DragBar
#define TIMER_DRAGBAR             0x09
/* Timer event numbers used to by ControlExtns for ListBox tooltips.  Here for doc. only
#define TIMER_LB_HOVER            0x0A
#define TIMER_LB_SHOWING          0x0B
/* Timer event numbers used by StatusBar for tooltips.  Here for doc. only
#define TIMER_SB_HOVER            0x0C
#define TIMER_SB_SHOWING          0x0D */
// Timer event for daily expired entries check
#define TIMER_EXPENT              0x0E
// Timer event number used to to poll the YubiKey when used
#define TIMER_YUBIKEYPOLL         0x0F

/*
HOVER_TIME_ND       The length of time the pointer must remain stationary
within a tool's bounding rectangle before the tool tip
window appears.
*/
#define HOVER_TIME_ND      2000

/*
TIMEINT_ND_SHOWING The length of time the tool tip window remains visible
if the pointer is stationary within a tool's bounding
rectangle.
*/
#define TIMEINT_ND_SHOWING 5000

// DragBar time interval 
#define TIMER_DRAGBAR_TIME 100

// Hotkey value ID to maximum value allowed by Windows for an app.
#define PWS_HOTKEY_ID      0xBFFF

// Arbitrary string to mean that the saved DB preferences are empty.
#define EMPTYSAVEDDBPREFS L"#Empty#"

// Maximum number of characters that can be added to a MFC CEdit control
// by default (i.e. without calling SetLimitText). Note: 30000 not 32K!
// Although this limit can be changed to up to 2GB of characters
// (4GB memory if Unicode), it would make the database size absolutely enormous!
#define MAXTEXTCHARS       30000

// For Layered Windows
typedef DWORD(WINAPI *PSLWA) (HWND, DWORD, BYTE, DWORD);
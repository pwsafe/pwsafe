/*
* Copyright (c) 2003-2016 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

#include "os/UUID.h"

// Windowsdefs.h
//-----------------------------------------------------------------------------

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


// Windows structures and enums

// ... More to be moved here

// Used by SelectAttachments & ManageAttachments
enum { LCATT_NUM = 0, LCATT_TITLE, LCATT_NAME, LCATT_PATH,
       LCATT_MEDIA, LCATT_SIZE, LCATT_CTIME, LCATT_FILECTIME, LCATT_FILEMTIME, LCATT_FILEATIME};

struct st_att {

  st_att() : att_uuid(pws_os::CUUID::NullUUID()), bOrphaned(false),
    bToBePurged(false), bInitalToBePurged(false),
    sxFileTitle(L""), sxFileName(L""), sxFilePath(L""),
    sxFileMediaType(L""), tCTime(0),tFileMTime(0), tFileCTime(0), tFileATime(0),
    size(0), numreferenced(-1)
  {}

  st_att(const st_att &that)
    : att_uuid(that.att_uuid), bOrphaned(that.bOrphaned),
    bToBePurged(that.bToBePurged), bInitalToBePurged(that.bInitalToBePurged),
    sxFileTitle(that.sxFileTitle), sxFileName(that.sxFileName), sxFilePath(that.sxFilePath),
    sxFileMediaType(that.sxFileMediaType),
    tCTime(that.tCTime), tFileMTime(that.tFileMTime), tFileCTime(that.tFileCTime),
    tFileATime(that.tFileATime), size(that.size), numreferenced(that.numreferenced)
  {}

  st_att &operator =(const st_att &that)
  {
    if (this != &that) {
      att_uuid = that.att_uuid;
      bOrphaned = that.bOrphaned;
      bToBePurged = that.bToBePurged;
      bInitalToBePurged = that.bInitalToBePurged;

      sxFileTitle = that.sxFileTitle;
      sxFileName = that.sxFileName;
      sxFilePath = that.sxFilePath;
      sxFileMediaType = that.sxFileMediaType;

      tCTime = that.tCTime;
      tFileCTime = that.tFileCTime;
      tFileMTime = that.tFileMTime;
      tFileATime = that.tFileATime;

      size = that.size;
      numreferenced = that.numreferenced;
    }
    return *this;
  }

  pws_os::CUUID att_uuid;
  bool bOrphaned;
  bool bToBePurged;
  bool bInitalToBePurged;

  StringX sxFileTitle;
  StringX sxFileName;
  StringX sxFilePath;
  StringX sxFileMediaType;

  time_t tCTime;
  time_t tFileCTime;
  time_t tFileMTime;
  time_t tFileATime;

  size_t size;
  int numreferenced;
};

// Used by ViewAttachmentEntriesDlg
struct st_gtui {
  st_gtui() : sxGroup(L""), sxTitle(L""), sxUser(L""), image(-1)
  {}

  st_gtui(const st_gtui &that)
    : sxGroup(that.sxGroup), sxTitle(that.sxTitle), sxUser(that.sxUser),
    image(that.image) {}

  st_gtui &operator =(const st_gtui &that)
  {
    if (this != &that) {
      sxGroup = that.sxGroup;
      sxTitle = that.sxTitle;
      sxUser = that.sxUser;
      image = that.image;
    }
    return *this;
  }

  StringX sxGroup;
  StringX sxTitle;
  StringX sxUser;
  int image;
};

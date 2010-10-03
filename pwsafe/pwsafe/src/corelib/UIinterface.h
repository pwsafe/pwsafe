/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#ifndef __UIINTERFACE_H
#define __UIINTERFACE_H

#include "Command.h"
#include "ItemData.h"
#include "attachments.h"
#include "PWSAttfile.h"

/**
 * An abstract base class representing all of the UI functionality
 * that corelib needs to know about.
 * The concrete UI main class should publically inherit this, and
 * implement all the interface member functions.
 *
 * This is the classic 'mixin' design pattern.
 */

class UIInterFace {
public:
  UIInterFace() {}

  /** Functions that can be supported by the UI.
   * It is the responsiblity of the UI to tell the core via SetUIInterFace
   * which ones it supports.  It will not get called for ones that it does not
   * support.
   * NOTE: New functions must be placed at the end of the list.
   * Sequential values must be used as they are used to access positions in
   * a std::bitset<NUM_SUPPORTED> variable.
   */
  enum Functions {
    DATABASEMODIFIED = 0, UPDATEGUI, GUISETUPDISPLAYINFO, GUIREFRESHENTRY,
    ATTACHMENTPROGRESS, WRITEATTACHMENTFILE, COMPLETEIMPORTFILE,
    // Add new functions here!
    NUM_SUPPORTED};

  /*
   * DatabaseModified(bChanged):
   * bChanged = false if the database has been modified, (e.g. the
   * last find results may no longer be valid) but is now unchanged
   * from the last saved version.
   */
  virtual void DatabaseModified(bool bChanged) = 0;
  
  // UpdateGUI - used by GUI if one or more entries have changed
  // and the entry/entries needs refreshing in GUI:
  virtual void UpdateGUI(UpdateGUICommand::GUI_Action ga,
                         uuid_array_t &entry_uuid,
                         CItemData::FieldType ft = CItemData::START,
                         bool bUpdateGUI = true) = 0;
                         
  // GUISetupDisplayInfo: let GUI populate DisplayInfo field in an entry
  virtual void GUISetupDisplayInfo(CItemData &ci) = 0;
  
  // GUIRefreshEntry: called when the entry's graphic representation
  // may have changed - GUI should update and invalidate its display.
  virtual void GUIRefreshEntry(const CItemData &ci) = 0;

  // Attachment Progress: called when the adding an attachment
  // may have changed - GUI should update and invalidate its display.
  virtual int AttachmentProgress(const ATTProgress &st_atpg) = 0;

  // Attachment functions that will be worker threads if supported
  // or internal if not.
  virtual int WriteAttachmentFile(const bool bCleanup = false,
                    PWSAttfile::VERSION version = PWSAttfile::VCURRENT) = 0;
  virtual int CompleteImportFile(const stringT &impfilename,
                    PWSAttfile::VERSION version = PWSAttfile::VCURRENT) = 0;

  virtual ~UIInterFace() {}
};

#endif /* __UIINTERFACE_H */

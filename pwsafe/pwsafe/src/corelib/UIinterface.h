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
  /**
   * UpdateGUI(bChanged):
   * bChanged = false if the database has been modified, (e.g. the
   * last find results may no longer be valid) but is now unchanged
   * from the last saved version. */
  virtual void DatabaseModified(bool bChanged) = 0;
  // UpdateGUI - used by GUI if one or more entries have changed
  // and the entry/entries needs refreshing in GUI:
  virtual void UpdateGUI(Command::GUI_Action ga,
                         uuid_array_t &entry_uuid,
                         CItemData::FieldType ft = CItemData::START,
                         bool bUpdateGUI = true) = 0;
  // GUISetupDisplayInfo: let GUI populate DisplayInfo field in an entry
  virtual void GUISetupDisplayInfo(CItemData &ci) = 0;
  virtual void GUICommandInterface(Command::ExecuteFn When,
                                   PWSGUICmdIF *pGUICmdIF) = 0;
  virtual ~UIInterFace() {}
};

#endif /* __UIINTERFACE_H */

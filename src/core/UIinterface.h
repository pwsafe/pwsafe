/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#ifndef __UIINTERFACE_H
#define __UIINTERFACE_H

#include "Command.h"
#include "ItemData.h"

#include <algorithm>

/**
 * An abstract base class representing all of the UI functionality
 * that core needs to know about.
 * The concrete UI main class should publically inherit this, and
 * implement all the interface member functions.
 *
 * This is the classic 'mixin' design pattern.
 */

class Observer
{
public:
  Observer() {}

  /*
   * UpdateGUI(bChanged):
   * bChanged = false if the database has been modified, (e.g. the
   * last find results may no longer be valid) but is now unchanged
   * from the last saved version.
   */
  virtual void DatabaseModified(bool /* bChanged */) {}

  // UpdateGUI - used by GUI if one or more entries have changed
  // and the entry/entries needs refreshing in GUI:
  virtual void UpdateGUI(UpdateGUICommand::GUI_Action /* ga */,
                         const pws_os::CUUID &/* entry_uuid */,
                         CItemData::FieldType /* ft */ = CItemData::START) {}

  // Version for groups
  virtual void UpdateGUI(UpdateGUICommand::GUI_Action /* ga */,
                         const std::vector<StringX> &/* vGroups */) {}

  // GUIRefreshEntry: called when the entry's graphic representation
  // may have changed - GUI should update and invalidate its display.
  virtual void GUIRefreshEntry(const CItemData &/* ci */, bool /* bAllowFail */ = false) {}

  // UpdateWizard: called to update text in Wizard during export Text/XML.
  virtual void UpdateWizard(const stringT &) {}

  virtual ~Observer() {}
};

class Observable
{
public:
  void RegisterObserver(Observer* observer)
  {
    if (std::find(m_Observers.begin(), m_Observers.end(), observer) == m_Observers.end()) {
      m_Observers.push_back(observer);
    }
  }

  void UnregisterObserver(Observer* observer)
  {
    m_Observers.erase(std::remove_if(
      m_Observers.begin(), m_Observers.end(), 
      [observer](Observer* registeredObserver){ return registeredObserver == observer; }), 
      m_Observers.end());
  }

  virtual ~Observable() {};

protected:
  std::vector<Observer*> m_Observers;
};

#endif /* __UIINTERFACE_H */

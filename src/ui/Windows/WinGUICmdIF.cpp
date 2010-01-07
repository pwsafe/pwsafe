/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * \file MFC GUI-specific implementation of GUICommandInterface.h
 */
 
 // This is the MFC version!!!
 // This is the MFC version!!!
 // This is the MFC version!!!
 // This is the MFC version!!!
 // This is the MFC version!!!

 /*
 
 Currently, this is only required when an entry is deleted from database
 but this will not automatically fix the Tree View where whole groups 
 might disappear.
 
 This is a command sent to the GUI from corelib to allow it to perform
 such cleanups/modifications.

 corelib knows nothing about the content of this class, it merely calls
 the pre-registered GUI routine to process it.
 */
 
#include "WinGUICmdIF.h"

WinGUICmdIF::WinGUICmdIF(const WinGUICmdIF &that)
  : m_iType(that.m_iType),
    m_vDeleteGroup(that.m_vDeleteGroup),
    m_vDeleteListItems(that.m_vDeleteListItems),
    m_vDeleteTreeItemsWithParents(that.m_vDeleteTreeItemsWithParents),
    m_vVerifyAliasBase(that.m_vVerifyAliasBase),
    m_vVerifyShortcutBase(that.m_vVerifyShortcutBase),
    m_vAliasDependents(that.m_vAliasDependents)
{
}

WinGUICmdIF::~WinGUICmdIF()
{
}

WinGUICmdIF& WinGUICmdIF::operator=(const WinGUICmdIF &that)
{
  if (this != &that) {
    m_iType = that.m_iType;
    m_vDeleteGroup = that.m_vDeleteGroup;
    m_vDeleteListItems = that.m_vDeleteListItems;
    m_vDeleteTreeItemsWithParents = that.m_vDeleteTreeItemsWithParents;
    m_vVerifyAliasBase = that.m_vVerifyAliasBase;
    m_vVerifyShortcutBase = that.m_vVerifyShortcutBase;
    m_vAliasDependents = that.m_vAliasDependents;
  }
  return *this;
}

bool WinGUICmdIF::IsValid() const
{
  if (m_iType == GCT_DELETE) {
    if (!m_vDeleteGroup.empty()                ||
        !m_vDeleteListItems.empty()            ||
        !m_vDeleteTreeItemsWithParents.empty() ||
        !m_vVerifyAliasBase.empty()            ||
        !m_vVerifyShortcutBase.empty()         ||
        !m_vAliasDependents.empty())
      return true;
  }
  return false;
}

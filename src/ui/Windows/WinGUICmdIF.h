/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * Interface for to GUI during Command processing
 */

#pragma once

#include "corelib/GUICommandInterface.h"
#include "corelib/UUIDGen.h"

#include <vector>

class WinGUICmdIF : public PWSGUICmdIF
{
public:
  friend class DboxMain;

  enum CmdType { 
    GCT_INVALID = -1,
    GCT_DELETE = 0
  };

  WinGUICmdIF() : m_iType(GCT_INVALID) {}
  WinGUICmdIF(const CmdType &iType) : m_iType(iType) {}
  WinGUICmdIF(const WinGUICmdIF &that);
  ~WinGUICmdIF();

  bool IsValid() const;
  CmdType GetType() {return m_iType;}

  WinGUICmdIF& operator=(const WinGUICmdIF &that);

private:
  CmdType m_iType;

  // Member variables for the Tree Delete function support
  std::vector<StringX>  m_vDeleteGroup;
  std::vector<CUUIDGen> m_vDeleteListItems;
  std::vector<CUUIDGen> m_vDeleteTreeItemsWithParents;
  std::vector<CUUIDGen> m_vVerifyAliasBase;
  std::vector<CUUIDGen> m_vVerifyShortcutBase;
  std::vector<CUUIDGen> m_vAliasDependents;
};

//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:

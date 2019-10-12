/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#ifndef __GUIINFO_H__
#define __GUIINFO_H__

#include "../../os/UUID.h"
#include <wx/string.h>
#include <wx/gdicmn.h>
#include <wx/arrstr.h>

class PasswordSafeFrame;
class PWSTreeCtrl;
class PWSGrid;

class string_or_uuid
{
  public:
    enum class ItemType { NONE, NORMAL, GROUP };
    
    string_or_uuid() : m_type(ItemType::NONE) {}
      
    ItemType Type() const { return m_type; }
    
    void Clear() { m_type = ItemType::NONE; }

    string_or_uuid& operator=(const wxString& str) {
      m_str = str;
      m_type  = ItemType::GROUP;
      return *this;
    }
      
    string_or_uuid& operator=(const pws_os::CUUID &uu) {
      m_uu = uu;
      m_type  = ItemType::NORMAL;
      return *this;
    }

    operator wxString () const { 
      wxASSERT(m_type == ItemType::GROUP);
      return m_str;
    }
    
    operator const pws_os::CUUID() const { 
      wxASSERT(m_type == ItemType::NORMAL);
      return pws_os::CUUID(m_uu);
    }

  private:
    wxString       m_str;
    pws_os::CUUID  m_uu;
    ItemType       m_type;
};

class GUIInfo
{
  public:
    //Let the compiler generate these
    //GUIInfo();
    //GUIInfo(const GUIInfo& other);
    //GUIInfo& operator=(const GUIInfo& other);

    void Save(PasswordSafeFrame* frame);
    void Restore(PasswordSafeFrame* frame);

    void SaveTreeViewInfo(PWSTreeCtrl* tree);
    void SaveGridViewInfo(PWSGrid* grid);

    void RestoreTreeViewInfo(PWSTreeCtrl* tree);
    void RestoreGridViewInfo(PWSGrid* grid);

  private:

    string_or_uuid m_treeTop;
    string_or_uuid m_treeSelection;

    wxArrayString  m_expanded;                      //expanded elements in treeview

    pws_os::CUUID  m_gridTop;                       //top element in gridview
    pws_os::CUUID  m_gridSelection;                 //selected elements, only one per view
};

#endif

/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#ifndef __GUIINFO_H__
#define __GUIINFO_H__

#include "../../corelib/UUIDGen.h"
#include <wx/string.h>
#include <wx/gdicmn.h>
#include <wx/arrstr.h>

class PasswordSafeFrame;
class PWSTreeCtrl;
class PWSGrid;

class string_or_uuid
{
  public:
    typedef enum { ITEM_NONE = 0, ITEM_NORMAL, ITEM_GROUP } ItemType;
    
    string_or_uuid() : m_type(ITEM_NONE) {}
      
    ItemType Type() const { return m_type; }
    void Clear() { m_type = ITEM_NONE; }

    string_or_uuid& operator=(const wxString& str) {
      m_str = str;
      m_type  = ITEM_GROUP;
      return *this;
    }
      
    string_or_uuid& operator=(const uuid_array_t uu) {
      memcpy(m_uu, uu, sizeof(uu));
      m_type  = ITEM_NORMAL;
      return *this;
    }

    operator wxString () const { wxASSERT(m_type == ITEM_GROUP); return m_str; }
    operator const uuid_array_t&() const { wxASSERT(m_type == ITEM_NORMAL); return m_uu; }
    operator uuid_array_t&() { m_type = ITEM_NORMAL; return m_uu; }

  private:
    wxString     m_str;
    uuid_array_t m_uu;
    ItemType     m_type;
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


  private:

    void SaveTreeViewInfo(PWSTreeCtrl* tree);
    void SaveGridViewInfo(PWSGrid* grid);

    void RestoreTreeViewInfo(PWSTreeCtrl* tree);
    void RestoreGridViewInfo(PWSGrid* grid);

    string_or_uuid     m_treeTop;
    string_or_uuid     m_treeSelection;

    wxArrayString      m_expanded;                      //expanded elements in treeview

    uuid_array_t       m_gridTop;                       //top element in gridview
    uuid_array_t       m_gridSelection;                 //selected elements, only one per view

};

#endif


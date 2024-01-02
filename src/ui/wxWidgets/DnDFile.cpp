/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file DndFile.cpp
* 
*/

// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#ifdef __WXMSW__
#include <wx/msw/msvcrt.h>
#endif

////@begin includes
#include <wx/dnd.h>
#include <wx/filename.h>
////@end includes

#include "core/XML/XMLDefs.h"
#include "DnDFile.h"

/*!
 * OnDropFiles implementation
 */

bool DnDFile::OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames)
{
  size_t nFiles = filenames.GetCount();
  if(nFiles > 1) // We can handle one file only
    return false;
  
  wxFileName filename(filenames[0]);
  if(! filename.IsOk() || ! filename.IsFileReadable())
    return false;
  
  if (m_pOwner != NULL)
  {
    if(! m_pOwner->m_core.IsDbOpen()) {
      if(m_pOwner->Open(filenames[0]) != PWScore::SUCCESS)
        return false;
    }
    else {
      if(m_pOwner->m_core.IsReadOnly()) // Data base is opened read only
        return false;
      // Depending on file name suffix run
      // Import Text, XML. KeePass or Merge Another Safe
      // TDOD
      if(filename.GetExt().IsSameAs("txt", false)) { // Text Import
        wxCommandEvent evt(ID_IMPORT_PLAINTEXT, SYMBOL_PASSWORDSAFEFRAME_IDNAME);
        evt.SetString(filenames[0]);
        // Direct call of handler function
        m_pOwner->OnImportText(evt);
      }
#if (!defined(_WIN32) && USE_XML_LIBRARY == MSXML)
      else if(filename.GetExt().IsSameAs("xml", false)) { // XML Import
        wxCommandEvent evt(ID_IMPORT_XML, SYMBOL_PASSWORDSAFEFRAME_IDNAME);
        evt.SetString(filenames[0]);
        // Direct call of handler function
        m_pOwner->OnImportXML(evt);
      }
#endif
      else if(filename.GetExt().IsSameAs("csv", false)) { // KeePass Import
        wxCommandEvent evt(ID_IMPORT_KEEPASS, SYMBOL_PASSWORDSAFEFRAME_IDNAME);
        evt.SetString(filenames[0]);
        // Direct call of handler function
        m_pOwner->OnImportKeePass(evt);
      }
      else if(filename.GetExt().IsSameAs("psafe4", false) ||
              filename.GetExt().IsSameAs("psafe3", false) ||
              filename.GetExt().IsSameAs("dat", false) ||
              filename.GetExt().IsSameAs("ibak", false)) {
        if(::wxGetKeyState(WXK_CONTROL)) { // Synchronize with actual data base on CTRL pressed (is Command in macOS)
          wxCommandEvent evt(ID_SYNCHRONIZE, SYMBOL_PASSWORDSAFEFRAME_IDNAME);
          evt.SetString(filenames[0]);
          // Direct call of handler function
          m_pOwner->OnSynchronize(evt);
          
        }
        else { // Merge with actual data base
          wxCommandEvent evt(ID_MERGE, SYMBOL_PASSWORDSAFEFRAME_IDNAME);
          evt.SetString(filenames[0]);
          // Direct call of handler function
          m_pOwner->OnMergeAnotherSafe(evt);
        }
      }
      else // Unknown file suffix
        return false;
    }
  }
  // At this end we succeeded
  return true;
}


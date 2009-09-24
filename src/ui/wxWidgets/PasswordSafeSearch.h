/*
 * Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */
/** \file
* 
*/

#ifndef __PASSWORDSAFESEARCH_H__
#define __PASSWORDSAFESEARCH_H__

/*!
 * Includes
 */

////@begin includes
#include <wx/event.h>
#include <wx/toolbar.h>
////@end includes

/*!
 * PasswordSafeSearch class declaration
 */

class PasswordSafeSearch : public wxEvtHandler
{
    DECLARE_CLASS( PasswordSafeSearch )
    DECLARE_EVENT_TABLE()

    DECLARE_NO_COPY_CLASS(PasswordSafeSearch);

public:
  /// Constructors
  PasswordSafeSearch(wxFrame* parent);
  /// Destructor
  ~PasswordSafeSearch();

  // wxEVT_COMMAND_TEXT_ENTER event handler for ENTER key press in search text box
  void OnDoSearch( wxCommandEvent& evt );
  void OnSearchClose(wxCommandEvent& evt);
  void Activate(void);

private:
  void CreateSearchBar(void);

  wxToolBar* m_toolbar;
  wxFrame*   m_parentFrame;
};

#endif

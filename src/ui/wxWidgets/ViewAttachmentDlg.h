/*
 * Initial version created as 'ViewAttachmentDlg.h'
 * by rafaelx on 2020-04-25.
 *
 * Copyright (c) 2019-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file ViewAttachmentDlg.h
* 
*/

#ifndef _VIEWATTACHMENTDLG_H_
#define _VIEWATTACHMENTDLG_H_

//(*Headers(ViewAttachmentDlg)
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/scrolwin.h>
#include <wx/stattext.h>
//*)
#include "core/ItemAtt.h"
#include "wxUtilities.h"


class ViewAttachmentDlg: public wxDialog
{
public:
    static ViewAttachmentDlg* Create(wxWindow *parent, wxWindowID id = -1);
    virtual ~ViewAttachmentDlg() = default;

    bool LoadImage(const CItemAtt &itemAttachment);
protected:
    ViewAttachmentDlg(wxWindow *parent, wxWindowID id);
private:

    //(*Handlers(ViewAttachmentDlg)
    void OnClose(wxCommandEvent& event);
    //*)

    //(*Identifiers(ViewAttachmentDlg)
    //*)

    //(*Declarations(ViewAttachmentDlg)
    ImagePanel *m_AttachmentImagePanel;
    //*)

    DECLARE_EVENT_TABLE()
};

#endif // _VIEWATTACHMENTDLG_H_

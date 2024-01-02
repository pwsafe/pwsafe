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

/** \file ViewAttachmentDlg.cpp
* 
*/

//(*InternalHeaders(ViewAttachmentDlg)
#include <wx/button.h>
#include <wx/intl.h>
#include <wx/msgdlg.h>
#include <wx/string.h>
//*)

#include "ViewAttachmentDlg.h"

//(*IdInit(ViewAttachmentDlg)
//*)

BEGIN_EVENT_TABLE(ViewAttachmentDlg, wxDialog)
  //(*EventTable(ViewAttachmentDlg)
    EVT_BUTTON( wxID_CLOSE, ViewAttachmentDlg::OnClose )
  //*)
END_EVENT_TABLE()

ViewAttachmentDlg::ViewAttachmentDlg(wxWindow *parent, wxWindowID id)
{
  wxASSERT(!parent || parent->IsTopLevel());
  //(*Initialize(ViewAttachmentDlg)
  wxDialog::Create(parent, id, _("View Attachment"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("ID_VIEWATTACHMENT"));
  SetMinSize(wxSize(300,400));

  auto *panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
  auto BoxSizerMain = new wxBoxSizer(wxVERTICAL);

  auto StaticBoxSizerView = new wxStaticBoxSizer(wxHORIZONTAL, panel, _("View"));
  m_AttachmentImagePanel = new ImagePanel(panel, wxDefaultSize);
  StaticBoxSizerView->Add(m_AttachmentImagePanel, 1, wxALL|wxEXPAND, 5);
  BoxSizerMain->Add(StaticBoxSizerView, 1, wxALL|wxEXPAND, 5);

  auto StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
  StdDialogButtonSizer1->AddButton(new wxButton(panel, wxID_CLOSE, wxEmptyString));
  StdDialogButtonSizer1->Realize();
  BoxSizerMain->Add(StdDialogButtonSizer1, 0, wxALL|wxEXPAND, 5);

  panel->SetSizer(BoxSizerMain);

  Center();
  Layout();
  //*)
}

ViewAttachmentDlg* ViewAttachmentDlg::Create(wxWindow *parent, wxWindowID id)
{
  return new ViewAttachmentDlg(parent, id);
}

/**
 * Loads the attachments image data for showing it on the image panel.
 */
bool ViewAttachmentDlg::LoadImage(const CItemAtt &itemAttachment)
{
  return m_AttachmentImagePanel->LoadFromAttachment(itemAttachment, this, _("View Attachment"));
}

void ViewAttachmentDlg::OnClose(wxCommandEvent& event)
{
  EndModal(wxID_CLOSE);
}

/*
 * Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file QRCodeDlg.cpp
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

#include <wx/statline.h>

#include "QREncode.h"
#include "QRCodeDlg.h"
#include "wxUtilities.h"

#include <algorithm>

////////////////////////////////////////////////////////////////////////////
// PasswordSafeSerach implementation
IMPLEMENT_CLASS(QRCodeDlg, wxDialog)

BEGIN_EVENT_TABLE(QRCodeDlg, wxDialog)
  EVT_BUTTON(wxID_CLOSE, QRCodeDlg::OnClose)
  EVT_INIT_DIALOG(QRCodeDlg::OnInitDialog)
  EVT_TIMER(wxID_ANY, QRCodeDlg::OnTimer)
END_EVENT_TABLE()

QRCodeDlg::QRCodeDlg(wxWindow *parent, const StringX &data,
                     const wxString &dlgTitle,
                     const int seconds,
                     const wxPoint &pos,
                     const wxSize &size,
                     long style,
                     const wxString &name) : wxDialog(parent, wxID_ANY, dlgTitle, pos, size, style, name), timer(this), secondsRemaining(seconds)
{
  wxASSERT(!parent || parent->IsTopLevel());

  CreateControls(data);
}

QRCodeDlg* QRCodeDlg::Create(wxWindow *parent, const StringX &data, const wxString &dlgTitle,
                     const int seconds, const wxPoint &pos,
                     const wxSize &size, long style, const wxString &name)
{
  return new QRCodeDlg(parent, data, dlgTitle, seconds, pos, size, style, name);
}

void QRCodeDlg::CreateControls(const StringX &data)
{
  wxSizer *dlgSizer = new wxBoxSizer(wxVERTICAL);

  dlgSizer->AddSpacer(TopMargin);

  wxBoxSizer *promptSizer = new wxBoxSizer(wxHORIZONTAL);
  promptSizer->Add(new wxStaticText(this, wxID_ANY, _("Closing in ")));
  promptSizer->Add(secondsText = new wxStaticText(this, wxID_ANY, _T("")));
  dlgSizer->Add(promptSizer);

  dlgSizer->AddSpacer(RowSeparation);
  dlgSizer->Add(new wxStaticLine(this), wxSizerFlags().Expand());
  dlgSizer->AddSpacer(RowSeparation);

  wxBitmap bmp = QRCodeBitmap(data);
  if (bmp.IsOk())
  {
    dlgSizer->Add(new wxStaticBitmap(this, wxID_ANY, bmp), wxSizerFlags().Expand().Proportion(1));
  }
  else
  {
    dlgSizer->Add(new wxStaticText(this, wxID_ANY, _("Could not generate QR code")));
  }

  dlgSizer->AddSpacer(RowSeparation);
  dlgSizer->Add(CreateSeparatedButtonSizer(wxCLOSE), wxSizerFlags().Expand());
  dlgSizer->AddSpacer(BottomMargin);
  wxSizer *hSizer = new wxBoxSizer(wxHORIZONTAL);
  hSizer->Add(dlgSizer, wxSizerFlags().Border(wxLEFT | wxRIGHT, SideMargin).Expand().Proportion(1));
  SetSizerAndFit(hSizer);
}

void QRCodeDlg::OnClose(wxCommandEvent& WXUNUSED(evt))
{
  EndModal(wxID_CLOSE);
}

void QRCodeDlg::OnTimer(wxTimerEvent& WXUNUSED(evt))
{
  if (--secondsRemaining > 0)
  {
    UpdateTimeRemaining();
    timer.Start(1000);
  }
  else
  {
    EndModal(0);
  }
}

void QRCodeDlg::OnInitDialog(wxInitDialogEvent& WXUNUSED(evt))
{
  // true => oneshot. We don't want to be called every millisecond
  timer.Start(1000, true);
  UpdateTimeRemaining();
}

void QRCodeDlg::UpdateTimeRemaining()
{
  secondsText->SetLabel(wxString::Format(_("%d seconds"), secondsRemaining));
}

#ifdef __TEST_QR_CODE__
///////////////////////////////////////////////////////
// build it like this
// g++ -D__TEST_QR_CODE__ -o qrtest
//                  `/usr/bin/wx-config-3.0 --debug=yes --unicode=yes --inplace --cxxflags --libs`
//                  -DUNICODE -I../.. QRCodeDlg.cpp  -lcore -los -lcore -lqrencode -L../../../lib/unicodedebug
//
#include <wx/cmdline.h>
#include <wx/filename.h>
class QRTestApp : public wxApp
{
public:
  void OnInitCmdLine(wxCmdLineParser &parser)
  {
    wxAppConsole::OnInitCmdLine(parser);
    static const wxCmdLineEntryDesc cmdLineDesc[] =
        {
            {wxCMD_LINE_PARAM, nullptr, nullptr, "secret", wxCMD_LINE_VAL_STRING, 0},
            {wxCMD_LINE_NONE}};
    parser.SetDesc(cmdLineDesc);
  }

  bool OnInit() override
  {
    if (!wxApp::OnInit())
      return false;

    if (this->argc != 2)
    {
      std::cerr << "usage: " << wxFileName(argv[0]).GetFullName() << " <text to generate qr code for>\n";
      return false;
    }
    else
    {
      ShowModalAndGetResult<QRCodeDlg>(this, StringX(argv[1]), "Scan this QR Code and comapre with the program argument");
      // Normall we return true here
      return false;
    }
  }
};

wxIMPLEMENT_APP(QRTestApp);
#endif
